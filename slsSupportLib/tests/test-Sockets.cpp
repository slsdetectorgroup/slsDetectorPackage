#include "ClientSocket.h"
#include "ServerSocket.h"
#include "catch.hpp"
#include <future>
#include <iostream>
#include <chrono>

std::vector<char> server() {
    std::cout << "starting server\n";
    auto server = sls::ServerSocket(1950);
    auto s = server.accept();
    std::vector<char> buffer(100, '\0');
    s.receiveData(buffer.data(), buffer.size());
    std::cout << "ServerReceived: " << std::string(buffer.begin(), buffer.end())
              << '\n';
    
    std::vector<char> to_send(100, '\0');
    to_send[0] = 'O';
    to_send[1] = 'K';
    s.sendData(to_send.data(), to_send.size());
    s.close();
    return buffer;
}

TEST_CASE("The server recive the same message as we send", "[support]") {
    std::vector<char> received_message(100, '\0');
    std::vector<char> sent_message(100, '\0');
    const char m[]{"some message"};
    std::copy(std::begin(m), std::end(m), sent_message.data());
    
    auto s = std::async(std::launch::async, server);
    auto client = sls::DetectorSocket("localhost", 1950);
    client.sendData(sent_message.data(), sent_message.size());
    client.receiveData(received_message.data(), received_message.size());
    client.close();
    auto server_message = s.get();

    CHECK(server_message == sent_message);
    CHECK(std::string(received_message.data()) == "OK" );
    CHECK(client.getSocketId() == -1);
}

TEST_CASE("throws on no server", "[support]"){
    CHECK_THROWS(sls::DetectorSocket("localhost", 1950));
}