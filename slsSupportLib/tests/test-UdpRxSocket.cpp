#include "sls/UdpRxSocket.h"
#include "catch.hpp"
#include "sls/sls_detector_exceptions.h"
#include <cstdint>
#include <errno.h>
#include <future>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <vector>

constexpr int default_port = 50001;

int open_socket(int port) {
    const char *host = nullptr; // localhost

    // Create a socket for sending
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = 0;
    hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG;
    struct addrinfo *res = nullptr;

    const std::string portname = std::to_string(port);
    if (getaddrinfo(host, portname.c_str(), &hints, &res)) {
        throw sls::RuntimeError("Failed at getaddrinfo with " +
                                std::string(host));
    }
    int fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (fd == -1) {
        throw sls::RuntimeError("Failed to create UDP RX socket");
    }

    if (connect(fd, res->ai_addr, res->ai_addrlen)) {
        throw sls::RuntimeError("Failed to connect socket");
    }
    freeaddrinfo(res);
    return fd;
}

TEST_CASE("Get packet size returns the packet size we set in the constructor") {
    constexpr int port = 50001;
    constexpr ssize_t packet_size = 8000;
    sls::UdpRxSocket s{port, packet_size};
    CHECK(s.getPacketSize() == packet_size);
}

TEST_CASE("Receive data from a vector") {
    constexpr int port = 50001;
    std::vector<int> data_to_send{4, 5, 3, 2, 5, 7, 2, 3};
    std::vector<int> data_received(data_to_send.size());
    ssize_t packet_size =
        sizeof(decltype(data_to_send)::value_type) * data_to_send.size();

    sls::UdpRxSocket udpsock{port, packet_size};

    int fd = open_socket(port);
    auto n = write(fd, data_to_send.data(), packet_size);
    CHECK(n == packet_size);

    CHECK(udpsock.ReceivePacket((char *)data_received.data()));
    close(fd);
    CHECK(data_to_send == data_received);
}

TEST_CASE("Shutdown socket without hanging when waiting for data") {
    constexpr int port = 50001;
    constexpr ssize_t packet_size = 8000;
    sls::UdpRxSocket s{port, packet_size};
    char buff[packet_size];

    // Start a thread and wait for package
    // if the socket is left open we would block
    std::future<bool> ret =
        std::async(&sls::UdpRxSocket::ReceivePacket, &s, (char *)&buff);

    s.Shutdown();
    auto r = ret.get();

    CHECK(r == false); // since we didn't get the packet
}

TEST_CASE("Too small packet") {
    constexpr int port = 50001;
    sls::UdpRxSocket s(port, 2 * sizeof(uint32_t));
    auto fd = open_socket(port);
    uint32_t val = 10;
    write(fd, &val, sizeof(val));
    uint32_t buff[2];
    CHECK(s.ReceivePacket((char *)&buff) == false);
    close(fd);
}

TEST_CASE("Receive an int to an external buffer") {
    int to_send = 5;
    int received = -1;
    auto fd = open_socket(default_port);
    sls::UdpRxSocket s(default_port, sizeof(int));
    write(fd, &to_send, sizeof(to_send));
    CHECK(s.ReceivePacket(reinterpret_cast<char *>(&received)));
    CHECK(received == to_send);
}
