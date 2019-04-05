#include "ServerSocket.h"
#include "DataSocket.h"
#include <arpa/inet.h>
#include <iostream>
#include <unistd.h>
#include <stdexcept>
#define DEFAULT_PACKET_SIZE 1286
#define SOCKET_BUFFER_SIZE (100 * 1024 * 1024) //100 MB
#define DEFAULT_BACKLOG 5

namespace sls {

ServerSocket::ServerSocket(int port) : DataSocket(socket(AF_INET, SOCK_STREAM, 0)) {

    std::cout << "Server constructed\n";

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(getSocketId(), (struct sockaddr *)&serverAddr, sizeof(serverAddr)) != 0) {
        close();
        throw std::runtime_error("Server ERROR: cannot  bind socket");
    }
    if (listen(getSocketId(), DEFAULT_BACKLOG) != 0) {
        close();
        throw std::runtime_error("Server ERROR: cannot  listen to socket");
    }
}

DataSocket ServerSocket::accept() {
    struct sockaddr_in clientAddr;
    socklen_t addr_size = sizeof clientAddr;
    int newSocket = ::accept(getSocketId(), (struct sockaddr *)&clientAddr, &addr_size);
    if (newSocket == -1) {
        throw std::runtime_error("Server ERROR: socket accept failed\n");
    }
    inet_ntop(AF_INET, &(clientAddr.sin_addr), &thisClient_.front(), INET_ADDRSTRLEN);
    std::cout << "lastClient: " << lastClient_ << " thisClient: " << thisClient_ << '\n';
    //Here goes any check for locks etc
    lastClient_ = thisClient_;

    return DataSocket(newSocket);
}

}; //namespace sls
