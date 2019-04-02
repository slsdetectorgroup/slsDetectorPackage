#include "DataSocket.h"
#include "logger.h"
#include "sls_detector_exceptions.h"
#include <algorithm>
#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>


namespace sls {

DataSocket::DataSocket(int socketId) : socketId_(socketId) {}

DataSocket::~DataSocket() {
    if (socketId_ <= 0) {
        return;
    } else {
        try {
            close();
        } catch (...) {
            // pass
        }
    }
}

void DataSocket::swap(DataSocket &other) noexcept { std::swap(socketId_, other.socketId_); }

DataSocket::DataSocket(DataSocket &&move) noexcept { move.swap(*this); }
DataSocket &DataSocket::operator=(DataSocket &&move) noexcept {
    move.swap(*this);
    return *this;
}

size_t DataSocket::receiveData(void *buffer, size_t size) {
    // std::cout << "Sending\n";
    size_t dataRead = 0;
    while (dataRead < size) {
        dataRead +=
            read(getSocketId(), reinterpret_cast<char *>(buffer) + dataRead, size - dataRead);
    }
    return dataRead;
}

size_t DataSocket::sendData(void *buffer, size_t size) {
    // std::cout << "Receiving\n";
    size_t dataSent = 0;
    while (dataSent < size) {
        dataSent +=
            write(getSocketId(), reinterpret_cast<char *>(buffer) + dataSent, size - dataSent);
    }
    return dataSent;
}

int DataSocket::setTimeOut(int t_seconds) {
    if (t_seconds <= 0)
        return -1;

    struct timeval t;
    t.tv_sec = 0;
    t.tv_usec = 0;
    // Receive timeout indefinet
    if (::setsockopt(getSocketId(), SOL_SOCKET, SO_RCVTIMEO, &t, sizeof(struct timeval)) < 0) {
        FILE_LOG(logERROR) << "setsockopt SO_RCVTIMEO " << 0;
    }

    t.tv_sec = t_seconds;
    t.tv_usec = 0;
    // Sending timeout in seconds
    if (::setsockopt(getSocketId(), SOL_SOCKET, SO_SNDTIMEO, &t, sizeof(struct timeval)) < 0) {
        FILE_LOG(logERROR) << "setsockopt SO_SNDTIMEO " << t_seconds;
    }
    return 0;
}

void DataSocket::close() {
    if (socketId_ > 0) {
        if(::close(socketId_)){
            throw SocketError("could not close socket");
        }
        socketId_ = 0;
        
    } else {
        throw std::runtime_error("Socket ERROR: close called on bad socket\n");
    }
}

struct sockaddr_in ConvertHostnameToInternetAddress(const std::string &hostname) {
    struct addrinfo hints, *result;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags |= AI_CANONNAME;

    struct sockaddr_in serverAddr {};
    if (getaddrinfo(hostname.c_str(), NULL, &hints, &result) != 0) {
        freeaddrinfo(result);
        std::string msg = "ClientSocket cannot decode host:" + hostname + "\n";
        throw SocketError(msg);
    }
    serverAddr.sin_family = AF_INET;
    memcpy((char *)&serverAddr.sin_addr.s_addr, &((struct sockaddr_in *)result->ai_addr)->sin_addr,
           sizeof(in_addr_t));
    freeaddrinfo(result);
    return serverAddr;
}

int ConvertHostnameToInternetAddress(const char *const hostname, struct ::addrinfo **res) {
    // criteria in selecting socket address structures returned by res
    struct ::addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    // get host info into res
    int errcode = getaddrinfo(hostname, NULL, &hints, res);
    if (errcode != 0) {
        FILE_LOG(logERROR) << "Could not convert hostname (" << hostname
                           << ") to internet address (zmq):" << gai_strerror(errcode);
    } else {
        if (*res == NULL) {
            FILE_LOG(logERROR) << "Could not converthostname (" << hostname
                               << ") to internet address (zmq):"
                                  "gettaddrinfo returned null";
        } else {
            return 0;
        }
    }
    FILE_LOG(logERROR) << "Could not convert hostname to internet address";
    return 1;
};

/**
 * Convert Internet Address structure pointer to ip string (char*)
 * Clears the internet address structure as well
 * @param res pointer to internet address structure
 * @param ip pointer to char array to store result in
 * @param ipsize size available in ip buffer
 * @return 1 for fail, 0 for success
 */
// Do not make this static (for multi threading environment)
int ConvertInternetAddresstoIpString(struct ::addrinfo *res, char *ip, const int ipsize) {
    if (inet_ntop(res->ai_family, &((struct sockaddr_in *)res->ai_addr)->sin_addr, ip, ipsize) !=
        NULL) {
        ::freeaddrinfo(res);
        return 0;
    }
    FILE_LOG(logERROR) << "Could not convert internet address to ip string";
    return 1;
}

} // namespace sls
