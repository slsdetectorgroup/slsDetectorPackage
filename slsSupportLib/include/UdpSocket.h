
#include <cstdint>
#include <errno.h>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "sls_detector_exceptions.h"

namespace sls {

class UdpSocket {
    int port;
    size_t packet_size;
    size_t buffer_size;
    int fd = -1;

  public:
    UdpSocket(int port, size_t packet_size)
        : port(port), packet_size(packet_size) {
        const char *hostname = 0; /* wildcard */
        const std::string portname = std::to_string(port);
        struct addrinfo hints;
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_DGRAM;
        hints.ai_protocol = 0;
        hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG;
        struct addrinfo *res = 0;
        if (getaddrinfo(hostname, portname.c_str(), &hints, &res)){
            throw RuntimeError("Failed getaddinfo");
        }
        fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if(fd==-1){
            throw RuntimeError("Failed creating socket");
        }
        if (bind(fd, res->ai_addr, res->ai_addrlen) == -1) {
            throw RuntimeError("Failed to bind socket");
        }
        freeaddrinfo(res);
    }

    int ReceivePacket() {
        char buffer[549];
        struct sockaddr_storage src_addr;
        socklen_t src_addr_len = sizeof(src_addr);
        ssize_t count = recvfrom(fd, buffer, sizeof(buffer), 0,
                                 (struct sockaddr *)&src_addr, &src_addr_len);
        return count;
    }
};

} // namespace sls