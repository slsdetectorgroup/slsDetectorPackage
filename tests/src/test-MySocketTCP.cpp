#include "MySocketTCP.h"
#include "catch.hpp"
// #include "multiSlsDetector.h"
#include "logger.h"
#include <iostream>
#include <vector>

#define VERBOSE

TEST_CASE("Sending and receiving data with two sockets") {

    const int port_number{1966}; //TODO! Avoid hardcoded port number!!!
    auto sender = MySocketTCP("localhost", port_number);
    auto receiver = MySocketTCP(port_number);

    auto s = sender.Connect();
    auto r = receiver.Connect();

    REQUIRE(s > 0);
    REQUIRE(r > 0);
    REQUIRE(sender.getPortNumber() == port_number);
    REQUIRE(receiver.getPortNumber() == port_number);

    std::vector<char> message_to_send{'H', 'e', 'l', 'l', 'o'};
    std::vector<char> received_message(message_to_send.size());

    auto sent = sender.SendDataOnly(message_to_send.data(), message_to_send.size());
    auto received = receiver.ReceiveDataOnly(received_message.data(), message_to_send.size());
    REQUIRE(sent == message_to_send.size());
    REQUIRE(received == received_message.size());
    REQUIRE(sent == received);
    REQUIRE(message_to_send == received_message);

    receiver.CloseServerTCPSocketDescriptor();
    receiver.Disconnect();
    sender.Disconnect();

    REQUIRE(receiver.getsocketDescriptor() == -1);
    REQUIRE(receiver.getFileDes() == -1);
    REQUIRE(sender.getFileDes() == -1);
}

TEST_CASE("Open two sockets on the same port fails and throws") {
    const int port_number{1966};
    auto server = MySocketTCP(port_number);
    CHECK_THROWS(MySocketTCP(port_number));
}

// TEST_CASE("Conversions"){

//     std::cout << "name " << MySocketTCP::nameToMac("enp10s0u1u3u3") << '\n';

// }

TEST_CASE("Have two clients connect to the same server") {
    const int port_number{1966};

    auto server = MySocketTCP(port_number);

    auto client1 = MySocketTCP("localhost", port_number);
    auto client2 = MySocketTCP("localhost", port_number);
    client1.SetTimeOut(1);
    client2.SetTimeOut(1);
    server.SetTimeOut(1);
    auto fd1 = client1.Connect();

    auto fd2 = client2.Connect();
    server.Connect();

    REQUIRE(fd1 > 0);
    REQUIRE(fd2 > 0);

    std::cout << "fd1 " << fd1 << '\n';
    std::cout << "fd2 " << fd2 << '\n';

    std::vector<char> message_to_send{'H', 'e', 'l', 'l', 'o'};
    std::vector<char> received_message(message_to_send.size());

    client1.SendDataOnly(message_to_send.data(), message_to_send.size());
    auto n1 = server.ReceiveDataOnly(received_message.data(), received_message.size());
    std::cout << "n1 " << n1 << '\n';

    client2.SendDataOnly(message_to_send.data(), message_to_send.size());
    auto n2 = server.ReceiveDataOnly(received_message.data(), received_message.size());
    std::cout << "n2 " << n2 << '\n';
}