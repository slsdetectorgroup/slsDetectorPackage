// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "sls/ServerSocket.h"
#include "sls/ServerInterface.h"

#include "sls/DataSocket.h"
#include "sls/logger.h"
#include "sls/sls_detector_defs.h"
#include "sls/sls_detector_exceptions.h"
#include "sls/string_utils.h"

#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <unistd.h>

namespace sls {

#define DEFAULT_PACKET_SIZE 1286
#define SOCKET_BUFFER_SIZE  (100 * 1024 * 1024) // 100 MB
#define DEFAULT_BACKLOG     5

ServerSocket::ServerSocket(int port)
    : DataSocket(socket(AF_INET, SOCK_STREAM, 0)), serverPort(port) {

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(getSocketId(), (struct sockaddr *)&serverAddr,
             sizeof(serverAddr)) != 0) {
        close();
        throw SocketError(
            std::string("Server ERROR: cannot bind socket with port number ") +
            std::to_string(port) +
            std::string(". Please check if another instance is running."));
    }
    if (listen(getSocketId(), DEFAULT_BACKLOG) != 0) {
        close();
        throw std::runtime_error("Server ERROR: cannot  listen to socket");
    }
}

ServerInterface ServerSocket::accept() {
    lastClient = thisClient; // update from previous connection
    struct sockaddr_in clientAddr;
    socklen_t addr_size = sizeof clientAddr;
    int newSocket =
        ::accept(getSocketId(), (struct sockaddr *)&clientAddr, &addr_size);
    if (newSocket == -1) {
        throw SocketError("Server ERROR: socket accept failed\n");
    }
    char tc[INET_ADDRSTRLEN]{};
    inet_ntop(AF_INET, &(clientAddr.sin_addr), tc, INET_ADDRSTRLEN);
    thisClient = IpAddr{tc};
    return ServerInterface(newSocket);
}

}; // namespace sls
