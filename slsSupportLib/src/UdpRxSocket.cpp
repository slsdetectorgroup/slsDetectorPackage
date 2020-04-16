#include "UdpRxSocket.h"
#include "network_utils.h"
#include "sls_detector_exceptions.h"
#include <cstdint>
#include <errno.h>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

namespace sls {

UdpRxSocket::UdpRxSocket(int port, ssize_t packet_size, const char *hostname,
                         size_t kernel_buffer_size)
    : packet_size_(packet_size) {
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = 0;
    hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG;
    struct addrinfo *res = 0;

    const std::string portname = std::to_string(port);
    if (getaddrinfo(hostname, portname.c_str(), &hints, &res)) {
        throw RuntimeError("Failed at getaddrinfo with " +
                           std::string(hostname));
    }
    sockfd_ = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd_ == -1) {
        throw RuntimeError("Failed to create UDP RX socket");
    }
    if (bind(sockfd_, res->ai_addr, res->ai_addrlen) == -1) {
        throw RuntimeError("Failed to bind UDP RX socket");
    }
    freeaddrinfo(res);

    // If we get a specified buffer size that is larger than the set one
    // we set it. Otherwise we leave it there since it could have been
    // set by the rx_udpsocksize command
    if (kernel_buffer_size) {
        auto current = getBufferSize() / 2;
        if (current < kernel_buffer_size) {
            setBufferSize(kernel_buffer_size);
            if (getBufferSize() / 2 < kernel_buffer_size) {
                LOG(logWARNING)
                    << "Could not set buffer size. Got: " << getBufferSize() / 2
                    << " instead of " << kernel_buffer_size;
            }
        }
    }
}

UdpRxSocket::~UdpRxSocket() { Shutdown(); }
ssize_t UdpRxSocket::getPacketSize() const noexcept { return packet_size_; }

bool UdpRxSocket::ReceivePacket(char *dst) noexcept{
    auto bytes_received =
        recvfrom(sockfd_, dst, packet_size_, 0, nullptr, nullptr);
    return bytes_received == packet_size_;
}

ssize_t UdpRxSocket::ReceiveDataOnly(char *dst) noexcept {
        auto r = recvfrom(sockfd_, dst, packet_size_, 0, nullptr, nullptr);
        constexpr ssize_t eiger_header_packet =
            40; // only detector that has this
        if (r == eiger_header_packet) {
            LOG(logWARNING) << "Got header pkg";
            r = recvfrom(sockfd_, dst, packet_size_, 0, nullptr, nullptr);
        }
        return r;
    }

size_t UdpRxSocket::getBufferSize() const {
    size_t ret = 0;
    socklen_t optlen = sizeof(ret);
    if (getsockopt(sockfd_, SOL_SOCKET, SO_RCVBUF, &ret, &optlen) == -1)
        throw RuntimeError("Could not get socket buffer size");
    return ret;
}

void UdpRxSocket::setBufferSize(ssize_t size) {
    if (setsockopt(sockfd_, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size)))
        throw RuntimeError("Could not set socket buffer size");
}

void UdpRxSocket::Shutdown() {
        shutdown(sockfd_, SHUT_RDWR);
        if (sockfd_ >= 0) {
            close(sockfd_);
            sockfd_ = -1;
        }
    }
} // namespace sls