// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "catch.hpp"
#include "sls/ClientSocket.h"
#include "sls/ServerSocket.h"
#include "sls/Timer.h"
#include "sls/sls_detector_defs.h"
#include "sls/sls_detector_exceptions.h"
#include "sls/sls_detector_funcs.h"
#include <atomic>
#include <chrono>
#include <future>
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <sys/time.h>
#include <thread>
#include <unistd.h>

namespace sls {

// One configurable test server: accept a connection, read a 100-byte request,
// reply with `bytes_to_send` bytes (first two set to 'O','K'), optionally wait
// `hold` so the client can time out, then close. Returns the received request.
std::vector<char> echo_server(uint16_t port, size_t bytes_to_send,
                              std::chrono::milliseconds hold) {
    std::cout << "starting server on port " << port << '\n';
    auto server = ServerSocket(port);
    auto s = server.accept();
    std::vector<char> buffer(100, '\0');
    s.Receive(buffer.data(), buffer.size());

    if (port == 1960) {
        struct linger ling = {.l_onoff = 1, .l_linger = 0};

        auto fd = s.getSocketId();
        setsockopt(fd, SOL_SOCKET, SO_LINGER, &ling, sizeof ling);
        ::close(fd);
        return buffer;
    }

    if (bytes_to_send > 0) {
        std::vector<char> to_send(bytes_to_send, '\0');
        to_send[0] = 'O';
        to_send[1] = 'K';
        s.Send(to_send.data(), to_send.size());
    }
    std::this_thread::sleep_for(hold);
    s.close();
    return buffer;
}

// Minimal command server speaking the same protocol as sendCommandThenRead:
// accept a connection, read the function number and a single int argument,
// then reply via ServerInterface::sendResult with OK and (arg * 2). Returns
// the {fnum, arg} pair the server received so the test can verify them.
std::pair<int, int> command_server(uint16_t port) {
    auto server = ServerSocket(port);
    auto s = server.accept();
    int fnum = -1;
    int arg = 0;
    s.Receive(&fnum, sizeof(fnum));
    s.Receive(&arg, sizeof(arg));
    int retval = arg * 2;
    s.sendResult(slsDetectorDefs::OK, &retval, sizeof(retval));
    s.close();
    return {fnum, arg};
}

// Command server that replies with a truncated message: it reads the request,
// sends the OK return code, but then sends fewer retval bytes than the client
// expects before closing, so the client's Receive hits EOF mid-read.
void short_reply_server(uint16_t port, size_t retval_bytes_to_send) {
    auto server = ServerSocket(port);
    auto s = server.accept();
    int fnum = -1;
    int arg = 0;
    s.Receive(&fnum, sizeof(fnum));
    s.Receive(&arg, sizeof(arg));
    int ret = slsDetectorDefs::OK;
    s.Send(&ret, sizeof(ret));
    if (retval_bytes_to_send > 0) {
        std::vector<char> partial(retval_bytes_to_send, '\0');
        s.Send(partial.data(), partial.size());
    }
    s.close();
}

// Server that accepts a connection but never reads from it, so a client
// trying to send more than fits in the kernel buffers will stall. A small
// receive buffer keeps the amount the test must send modest. Stays open until
// the client signals it is done (or a safety timeout) so it never closes
// mid-transfer and races the client's Send.
void non_reading_server(uint16_t port, std::atomic<bool> *client_done) {
    auto server = ServerSocket(port);
    auto s = server.accept();
    int rcvbuf = 1024;
    setsockopt(s.getSocketId(), SOL_SOCKET, SO_RCVBUF, &rcvbuf, sizeof(rcvbuf));
    // Intentionally never Receive(): let the client's send path back up.
    Timer t;
    while (!client_done->load() && t.elapsed_ms() < 10000)
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    s.close();
}

TEST_CASE("The server recive the same message as we send", "[support]") {
    std::vector<char> received_message(100, '\0');
    std::vector<char> sent_message(100, '\0');
    const char m[]{"some message"};
    std::copy(std::begin(m), std::end(m), sent_message.data());

    auto s = std::async(std::launch::async, echo_server, 1950, 100,
                        std::chrono::milliseconds(0));
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    auto client = DetectorSocket("localhost", 1950);
    client.Send(sent_message.data(), sent_message.size());
    client.Receive(received_message.data(), received_message.size());
    client.close();
    auto server_message = s.get();

    CHECK(server_message == sent_message);
    CHECK(std::string(received_message.data()) == "OK");
    CHECK(client.getSocketId() == -1);
}

TEST_CASE("throws on no server", "[support]") {
    CHECK_THROWS(DetectorSocket("localhost", 1950));
    CHECK_THROWS(ReceiverSocket("localhost", 1950));
    CHECK_THROWS(GuiSocket("localhost", 1950));
}

TEST_CASE("Receiving a too short message throws and reports EOF", "[support]") {
    std::vector<char> received_message(100, '\0');
    std::vector<char> sent_message(100, '\0');
    const char m[]{"some message"};
    std::copy(std::begin(m), std::end(m), sent_message.data());

    // Server replies with only 10 of the 100 expected bytes, then closes.
    auto s = std::async(std::launch::async, echo_server, 1951, 10,
                        std::chrono::milliseconds(0));
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    auto client = DetectorSocket("localhost", 1951);
    client.Send(sent_message.data(), sent_message.size());

    client.setFnum(F_GET_SERVER_VERSION);

    // The server only sends 10 of the 100 expected bytes and then closes the
    // connection, so Receive must throw a SocketError reporting the EOF.
    std::string error_message;
    try {
        client.Receive(received_message.data(), received_message.size());
        FAIL("Receive should have thrown on a too short message");
    } catch (const SocketError &e) {
        error_message = e.what();
    }
    client.close();
    s.get();

    CHECK_THAT(error_message,
               Catch::Matchers::Contains("read 10 bytes instead of 100 bytes"));
    CHECK_THAT(error_message,
               Catch::Matchers::Contains("connection closed by peer (EOF)"));
}

TEST_CASE("Receiving with a socket error throws and reports the error",
          "[support]") {
    std::vector<char> received_message(100, '\0');
    std::vector<char> sent_message(100, '\0');
    const char m[]{"some message"};
    std::copy(std::begin(m), std::end(m), sent_message.data());

    // Server stays silent (sends nothing) but keeps the connection open long
    // enough for the client to time out, so the read fails with an error.
    auto s = std::async(std::launch::async, echo_server, 1952, 0,
                        std::chrono::milliseconds(500));
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    auto client = DetectorSocket("localhost", 1952);
    client.Send(sent_message.data(), sent_message.size());

    // Force read() to fail with EAGAIN/EWOULDBLOCK instead of returning EOF by
    // setting a short receive timeout while the server stays silent.
    client.setReceiveTimeout(100000); // 100 ms

    std::string error_message;
    try {
        client.Receive(received_message.data(), received_message.size());
        FAIL("Receive should have thrown on a socket error");
    } catch (const SocketError &e) {
        error_message = e.what();
    }
    client.close();
    s.get();

    CHECK_THAT(error_message,
               Catch::Matchers::Contains("read 0 bytes instead of 100 bytes"));
    CHECK_THAT(error_message, Catch::Matchers::Contains("read error:"));
}

TEST_CASE("Socket crash?", "[support]") {
    std::vector<char> received_message(100, '\0');
    std::vector<char> sent_message(100, '\0');
    const char m[]{"some message"};
    std::copy(std::begin(m), std::end(m), sent_message.data());

    auto s = std::async(std::launch::async, echo_server, 1960, 100,
                        std::chrono::milliseconds(0));
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    auto client = DetectorSocket("localhost", 1960);
    client.Send(sent_message.data(), sent_message.size());

    REQUIRE_THROWS(
        client.Receive(received_message.data(), received_message.size()));

    // Now try to send more
    //  client.Send(sent_message.data(), sent_message.size());
}

TEST_CASE("ClientSocket throws on invalid hostname", "[support]") {
    CHECK_THROWS(ReceiverSocket("invalidhostname", 1950));
    CHECK_THROWS(DetectorSocket("invalidhostname", 1950));
    CHECK_THROWS(GuiSocket("invalidhostname", 1950));
}

TEST_CASE("Using DetectorSocket to talk to a Server Socket", "[support]") {
    constexpr uint16_t port = 1961;
    constexpr int fnum = F_GET_DETECTOR_TYPE;
    constexpr int arg = 21;

    auto s = std::async(std::launch::async, command_server, port);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    auto client = DetectorSocket("localhost", port);
    int retval = 0;
    int ret = client.sendCommandThenRead(fnum, &arg, sizeof(arg), &retval,
                                         sizeof(retval));
    client.close();

    auto server_received = s.get();

    // Client got OK and the expected return value back from the server
    CHECK(ret == slsDetectorDefs::OK);
    CHECK(retval == arg * 2);
    // Server received the function number and argument we sent
    CHECK(server_received.first == fnum);
    CHECK(server_received.second == arg);
    // close() resets the underlying fd
    CHECK(client.getSocketId() == -1);
}

TEST_CASE("ServerSocket replies with a too short message", "[support]") {
    constexpr uint16_t port = 1962;
    constexpr int fnum = F_GET_DETECTOR_TYPE;
    constexpr int arg = 21;

    // Server sends the OK return code, then only 1 of the 4 expected retval
    // bytes before closing.
    auto s = std::async(std::launch::async, short_reply_server, port, 1);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    auto client = DetectorSocket("localhost", port);
    int retval = 0;

    std::string error_message;
    try {
        client.sendCommandThenRead(fnum, &arg, sizeof(arg), &retval,
                                   sizeof(retval));
        FAIL("sendCommandThenRead should have thrown on a too short message");
    } catch (const DetectorError &e) {
        error_message = e.what();
    }
    client.close();
    s.get();

    // The client read only 1 of the 4 expected retval bytes and then hit EOF.
    CHECK_THAT(error_message,
               Catch::Matchers::Contains("read 1 bytes instead of 4 bytes"));
    CHECK_THAT(error_message,
               Catch::Matchers::Contains("connection closed by peer (EOF)"));
}

TEST_CASE("Client cannot send the expected number of bytes", "[support]") {
    constexpr uint16_t port = 1963;

    // Server accepts but never reads; it stays open until we tell it the
    // client is done, so it cannot close mid-transfer.
    std::atomic<bool> client_done{false};
    auto s =
        std::async(std::launch::async, non_reading_server, port, &client_done);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    auto client = DetectorSocket("localhost", port);

    // Shrink the send buffer and add a short send timeout so the write stalls
    // and returns before all the data is sent.
    int sndbuf = 4096;
    setsockopt(client.getSocketId(), SOL_SOCKET, SO_SNDBUF, &sndbuf,
               sizeof(sndbuf));
    struct timeval tv {};
    tv.tv_sec = 0;
    tv.tv_usec = 300000; // 300 ms
    setsockopt(client.getSocketId(), SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

    // Much more than fits in the (shrunken) send + receive buffers, so the
    // send cannot complete while the server refuses to read.
    std::vector<char> big_message(8 * 1024 * 1024, '\0');

    std::string error_message;
    try {
        client.Send(big_message.data(), big_message.size());
        FAIL("Send should have thrown when it could not send all bytes");
    } catch (const SocketError &e) {
        error_message = e.what();
    }
    client_done = true;
    client.close();
    s.get();

    // Fewer bytes were sent than expected, reported as a write error (the
    // send timed out with EAGAIN/EWOULDBLOCK).
    CHECK_THAT(error_message, Catch::Matchers::Contains("bytes instead of"));
    CHECK_THAT(error_message, Catch::Matchers::Contains("write error:"));
}

} // namespace sls
