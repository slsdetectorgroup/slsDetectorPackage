#include "UdpRxSocket.h"
#include "catch.hpp"
#include "sls_detector_exceptions.h"
#include <future>
#include <thread>
#include <vector>

int open_socket(int port) {
    const char *host = nullptr; // localhost

    // Create a socket for sending
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = 0;
    hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG;
    struct addrinfo *res = 0;

    const std::string portname = std::to_string(port);
    if (getaddrinfo(host, portname.c_str(), &hints, &res)) {
        throw sls::RuntimeError("Failed at getaddrinfo with " +
                                std::string(host));
    }
    int fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (fd == -1) {
        throw sls::RuntimeError("Failed to create UDP RX socket");
    }

    if (connect(fd, res->ai_addr, res->ai_addrlen)){
        throw sls::RuntimeError("Failed to connect socket");
    }
    freeaddrinfo(res);
    return fd;
}

TEST_CASE("Receive a packet on localhost") {
    constexpr int port = 50001;

    std::vector<int> data_to_send{4, 5, 3, 2, 5, 7, 2, 3};
    ssize_t packet_size =
        sizeof(decltype(data_to_send)::value_type) * data_to_send.size();
    sls::UdpRxSocket udpsock{port, packet_size};


    int fd = open_socket(port);
    // int n = sendto(fd, data_to_send.data(), packet_size, 0, res->ai_addr,
    //                res->ai_addrlen);

    auto n = write(fd, data_to_send.data(), packet_size);
    CHECK(n == packet_size);
    CHECK(udpsock.ReceivePacket());
    close(fd);
    // Copy data from buffer and compare values
    std::vector<int> data_received(data_to_send.size());
    memcpy(data_received.data(), udpsock.LastPacket(), udpsock.getPacketSize());
    CHECK(data_received.size() == data_to_send.size()); // sanity check
    for (size_t i = 0; i != data_to_send.size(); ++i) {
        CHECK(data_to_send[i] == data_received[i]);
    }
}

TEST_CASE("Shutdown socket without hanging") {
    constexpr int port = 50001;
    constexpr ssize_t packet_size = 8000;
    sls::UdpRxSocket s{port, packet_size};

    // Start a thread and wait for package
    // if the socket is left open we would block
    std::future<bool> ret =
        std::async(static_cast<bool (sls::UdpRxSocket::*)()>(
                       &sls::UdpRxSocket::ReceivePacket),
                   &s);

    s.Shutdown();
    auto r = ret.get();

    CHECK(r == false); // since we didn't get the packet
}

TEST_CASE("Too small packet"){
    constexpr int port = 50001;
    sls::UdpRxSocket s(port, 2*sizeof(uint32_t));
    auto fd = open_socket(port);
    uint32_t val = 10;
    write(fd, &val, sizeof(val));
    CHECK(s.ReceivePacket() == false);
    close(fd);
}