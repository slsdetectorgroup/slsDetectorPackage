#include "ClientSocket.h"
#include "ServerSocket.h"
#include "catch.hpp"
#include <future>
#include <iostream>
#include <chrono>

int server() {
    std::cout << "starting server\n";
    auto server = sls::ServerSocket(1950);

    auto s = server.accept();
    std::vector<char> buffer(100, '\0');
    s.receiveData(buffer.data(), buffer.size());
    std::cout << "ServerReceived: " << std::string(buffer.begin(), buffer.end())
              << '\n';
    std::string message(100, '\0');
    message[0] = 'O';
    message[1] = 'K';
    s.sendData(&message.front(), message.size());
    s.close();
    return 0;
}

TEST_CASE("something", "[support][socket]") {
    auto s = std::async(std::launch::async, server);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    auto client = sls::DetectorSocket("localhost", 1950);
    std::cout << "client\n";
    std::vector<char> buffer(100, '\0');
    client.sendData(buffer.data(), buffer.size());
    client.receiveData(buffer.data(), buffer.size());
    s.get();
    client.close();
}