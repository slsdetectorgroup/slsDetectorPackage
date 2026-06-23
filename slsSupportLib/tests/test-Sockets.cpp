// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "catch.hpp"
#include "sls/ClientSocket.h"
#include "sls/ServerSocket.h"
#include "sls/sls_detector_exceptions.h"
#include "sls/sls_detector_funcs.h"
#include <chrono>
#include <future>
#include <iostream>
#include <string>
#include <thread>

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

} // namespace sls
