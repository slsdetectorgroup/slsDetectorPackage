#include "ClientSocket.h"
#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <unistd.h>

namespace sls {

ClientSocket::ClientSocket(const std::string &host, uint16_t port) : DataSocket(socket(AF_INET, SOCK_STREAM, 0)) {

    struct addrinfo hints, *result;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags |= AI_CANONNAME;
    
    if (getaddrinfo(host.c_str(), NULL, &hints, &result) != 0) {
        throw std::runtime_error("ClientSocket ERROR: cannot decode host\n");
    }

    //TODO! Erik, results could have multiple entries do we need to loop through them?
    // struct sockaddr_in serverAddr {};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    memcpy((char *)&serverAddr.sin_addr.s_addr,
                &((struct sockaddr_in *)result->ai_addr)->sin_addr, sizeof(in_addr_t));

    if (::connect(getSocketId(), (struct sockaddr *)&serverAddr, sizeof(serverAddr)) != 0){
        freeaddrinfo(result);
        throw std::runtime_error("ClientSocket ERROR: cannot connect to host\n");
    }
    freeaddrinfo(result);
}

int ClientSocket::connect(){
    //used to reconnect after closing may be removed
    return ::connect(getSocketId(), (struct sockaddr *)&serverAddr, sizeof(serverAddr));
}

}; //namespace sls