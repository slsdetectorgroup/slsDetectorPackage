// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "catch.hpp"
#include "sls/ClientSocket.h"
#include "sls/ServerSocket.h"
#include <chrono>
#include <future>
#include <iostream>
#include <thread>

namespace sls {

std::vector<char> server() {
    std::cout << "starting server\n";
    auto server = ServerSocket(1950);
    auto s = server.accept();
    std::vector<char> buffer(100, '\0');
    s.Receive(buffer.data(), buffer.size());
    std::cout << "ServerReceived: " << std::string(buffer.begin(), buffer.end())
              << '\n';

    std::vector<char> to_send(100, '\0');
    to_send[0] = 'O';
    to_send[1] = 'K';
    s.Send(to_send.data(), to_send.size());
    s.close();
    return buffer;
}

TEST_CASE("The server recive the same message as we send", "[support]") {
    std::vector<char> received_message(100, '\0');
    std::vector<char> sent_message(100, '\0');
    const char m[]{"some message"};
    std::copy(std::begin(m), std::end(m), sent_message.data());

    auto s = std::async(std::launch::async, server);
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

} // namespace sls
