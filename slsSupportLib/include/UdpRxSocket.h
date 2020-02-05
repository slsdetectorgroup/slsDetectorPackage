
/*
UdpRxSocket provies socket control to receive
data on a udp socket.

It provides a drop in replacement for
genericSocket. But please be careful since
this might be deprecated in the future.


*/

#include "genericSocket.h"
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
#include <vector>

namespace sls {

class UdpRxSocket {
    int port;
    const ssize_t packet_size;
    ssize_t buffer_size;
    char *buff;
    int fd = -1;

    // If possible we could listen to only one source but for our setup
    // we should have only our data on this port network???
    // recvfrom(fd, buff, packet_size, 0, (struct sockaddr *)&src_addr,
    //          &src_addr_len);

    // struct sockaddr_storage src_addr;
    // socklen_t src_addr_len = sizeof(src_addr);

  public:
    UdpRxSocket(int port, ssize_t packet_size, const char *hostname = nullptr,
                ssize_t buffer_size = 0)
        : port(port), packet_size(packet_size), buffer_size(buffer_size) {
        /* hostname = nullptr -> wildcard */
        const std::string portname = std::to_string(port);
        struct addrinfo hints;
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_DGRAM;
        hints.ai_protocol = 0;
        hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG;
        struct addrinfo *res = 0;
        if (getaddrinfo(hostname, portname.c_str(), &hints, &res)) {
            throw RuntimeError("Failed getaddinfo");
        }
        fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (fd == -1) {
            throw RuntimeError("Failed creating socket");
        }
        if (bind(fd, res->ai_addr, res->ai_addrlen) == -1) {
            throw RuntimeError("Failed to bind socket");
        }
        freeaddrinfo(res);

        // If we get a specified buffer size that is larger than the set one
        // we set it. Otherwise we leave it there since it could have been
        // set by the rx_udpsocksize command
        if (buffer_size) {
            auto current = getBufferSize() / 2;
            if (current < buffer_size) {
                setBufferSize(buffer_size);
                if (getBufferSize() / 2 < buffer_size) {
                    FILE_LOG(logWARNING) << "Could not set buffer size. Got: " << getBufferSize()/2 << " instead of " << buffer_size;
                }
            }
        }
        // Allocate at the end to avoid memory leak if we throw
        buff = new char[packet_size];
    }

    // Delegating constructor to allow drop in replacement for old socket class
    // This one might be removed in the future
    UdpRxSocket(unsigned short int const port_number,
                genericSocket::communicationProtocol p,
                int ps = DEFAULT_PACKET_SIZE, const char *eth = NULL,
                int hsize = 0, uint64_t buf_size = SOCKET_BUFFER_SIZE)
        : UdpRxSocket(port_number, ps, InterfaceNameToIp(eth).str().c_str(),
                      buf_size) {}

    const char *LastPacket() const { return buff; }

    ~UdpRxSocket() {
        delete[] buff;
        Shutdown();
    }

    // Receive one packet to the internal buffer of the socket class, preferred
    // method?
    bool ReceivePacket() {
        ssize_t count = recvfrom(fd, buff, packet_size, 0, nullptr, nullptr);
        return count == packet_size;
    }

    // Not sure we keep this
    bool ReceivePacket(char *dst) {
        ssize_t count = recvfrom(fd, buff, packet_size, 0, nullptr, nullptr);
        return count == packet_size;
    }

    // Only for backwards compatibility will be removed
    ssize_t ReceiveDataOnly(char *dst) {
        return recvfrom(fd, dst, packet_size, 0, nullptr, nullptr);
    }

    ssize_t getBufferSize() const {
        uint64_t ret_size = 0;
        socklen_t optlen = sizeof(uint64_t);
        if (getsockopt(fd, SOL_SOCKET, SO_RCVBUF, &ret_size, &optlen) == -1)
            return -1;
        else
            return ret_size;
    }

    // Only for backwards compatibility will be removed
    ssize_t getActualUDPSocketBufferSize() const { return getBufferSize(); }

    // Only for backwards compatibility will be removed
    void ShutDownSocket() { Shutdown(); }

    void setBufferSize(ssize_t size) {
        socklen_t optlen = sizeof(size);
        if (setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &size, optlen)) {
            throw RuntimeError("Could not set socket buffer size");
        }
    }

    void Shutdown() {
        shutdown(fd, SHUT_RDWR);
        if (fd >= 0) {
            close(fd);
            fd = -1;
        }
    }
};

} // namespace sls