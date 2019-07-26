#include "ServerInterface2.h"
#include "ServerSocket.h"

#include "DataSocket.h"
#include "logger.h"
#include "sls_detector_defs.h"
#include "sls_detector_exceptions.h"
#include "string_utils.h"

#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <unistd.h>
#define DEFAULT_PACKET_SIZE 1286
#define SOCKET_BUFFER_SIZE (100 * 1024 * 1024) // 100 MB
#define DEFAULT_BACKLOG 5

namespace sls {

ServerSocket::ServerSocket(int port)
    : DataSocket(socket(AF_INET, SOCK_STREAM, 0)), serverPort(port) {

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(getSocketId(), (struct sockaddr *)&serverAddr,
             sizeof(serverAddr)) != 0) {
        close();
        throw sls::SocketError("Server ERROR: cannot bind socket. Please check if another instance is running.");
    }
    if (listen(getSocketId(), DEFAULT_BACKLOG) != 0) {
        close();
        throw std::runtime_error("Server ERROR: cannot  listen to socket");
    }
}

ServerInterface2 ServerSocket::accept() {
    lastClient = thisClient; //update from previous connection
    struct sockaddr_in clientAddr;
    socklen_t addr_size = sizeof clientAddr;
    int newSocket =
        ::accept(getSocketId(), (struct sockaddr *)&clientAddr, &addr_size);
    if (newSocket == -1) {
        throw sls::SocketError("Server ERROR: socket accept failed\n");
    }
    char tc[INET_ADDRSTRLEN]{};
    inet_ntop(AF_INET, &(clientAddr.sin_addr), tc, INET_ADDRSTRLEN);
    thisClient = tc;
    return ServerInterface2(newSocket);
}

}; // namespace sls
