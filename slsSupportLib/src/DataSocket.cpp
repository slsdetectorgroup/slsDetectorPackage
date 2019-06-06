#include "DataSocket.h"
#include "logger.h"
#include "sls_detector_exceptions.h"
#include <algorithm>
#include <arpa/inet.h>
#include <cassert>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <netdb.h>
#include <sstream>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

namespace sls {

DataSocket::DataSocket(int socketId) : socketId_(socketId) {
    int value = 1;
    setsockopt(socketId_, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(value));
}

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

void DataSocket::swap(DataSocket &other) noexcept {
    std::swap(socketId_, other.socketId_);
}

DataSocket::DataSocket(DataSocket &&move) noexcept { move.swap(*this); }
DataSocket &DataSocket::operator=(DataSocket &&move) noexcept {
    move.swap(*this);
    return *this;
}

int DataSocket::receiveData(void *buffer, size_t size) {
    // TODO!(Erik) Add sleep? how many reties?
    assert(size > 0);
    int bytes_expected = static_cast<int>(size); // signed size
    int bytes_read = 0;
    while (bytes_read < bytes_expected) {
        auto this_read =
            ::read(getSocketId(), reinterpret_cast<char *>(buffer) + bytes_read,
                   bytes_expected - bytes_read);
        if (this_read <= 0)
            break;
        bytes_read += this_read;
    }
    if (bytes_read == bytes_expected) {
        return bytes_read;
    } else {
        std::ostringstream ss;
        ss << "TCP socket error read " << bytes_read << " bytes instead of "
           << bytes_expected << " bytes";
        throw sls::SocketError(ss.str());
    }
}

int DataSocket::sendData(const void *buffer, size_t size) {
    int bytes_sent = 0;
    int data_size = static_cast<int>(size); // signed size
    while (bytes_sent < (data_size)) {
        auto this_send = ::write(getSocketId(), buffer, size);
        if (this_send <= 0)
            break;
        bytes_sent += this_send;
    }
    if (bytes_sent != data_size){
        std::ostringstream ss;
        ss << "TCP socket error sent " << bytes_sent << " bytes instead of "
           << data_size << " bytes";
        throw sls::SocketError(ss.str());
    }
    return bytes_sent;
}

int DataSocket::write(void *buffer, size_t size) {
    return ::write(getSocketId(), buffer, size);
}

int DataSocket::read(void *buffer, size_t size) {
    return ::read(getSocketId(), buffer, size);
}

int DataSocket::setReceiveTimeout(int us) {
    timeval t{};
    t.tv_sec = 0;
    t.tv_usec = us;
    return ::setsockopt(getSocketId(), SOL_SOCKET, SO_RCVTIMEO, &t,
                        sizeof(struct timeval));
}

int DataSocket::setTimeOut(int t_seconds) {
    if (t_seconds <= 0)
        return -1;

    struct timeval t;
    t.tv_sec = 0;
    t.tv_usec = 0;
    // Receive timeout indefinet
    if (::setsockopt(getSocketId(), SOL_SOCKET, SO_RCVTIMEO, &t,
                     sizeof(struct timeval)) < 0) {
        FILE_LOG(logERROR) << "setsockopt SO_RCVTIMEO " << 0;
    }

    t.tv_sec = t_seconds;
    t.tv_usec = 0;
    // Sending timeout in seconds
    if (::setsockopt(getSocketId(), SOL_SOCKET, SO_SNDTIMEO, &t,
                     sizeof(struct timeval)) < 0) {
        FILE_LOG(logERROR) << "setsockopt SO_SNDTIMEO " << t_seconds;
    }
    return 0;
}

void DataSocket::close() {
    if (socketId_ > 0) {
        if (::close(socketId_)) {
            throw SocketError("could not close socket");
        }
        socketId_ = -1;

    } else {
        throw std::runtime_error("Socket ERROR: close called on bad socket\n");
    }
}

void DataSocket::shutDownSocket() {
    shutdown(getSocketId(), SHUT_RDWR);
    close();
}

struct sockaddr_in
ConvertHostnameToInternetAddress(const std::string &hostname) {
    struct addrinfo hints, *result;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags |= AI_CANONNAME;

    struct sockaddr_in serverAddr {};
    if (getaddrinfo(hostname.c_str(), nullptr, &hints, &result) != 0) {
        freeaddrinfo(result);
        std::string msg = "ClientSocket cannot decode host:" + hostname + "\n";
        throw SocketError(msg);
    }
    serverAddr.sin_family = AF_INET;
    memcpy((char *)&serverAddr.sin_addr.s_addr,
           &((struct sockaddr_in *)result->ai_addr)->sin_addr,
           sizeof(in_addr_t));
    freeaddrinfo(result);
    return serverAddr;
}

int ConvertHostnameToInternetAddress(const char *const hostname,
                                     struct ::addrinfo **res) {
    // criteria in selecting socket address structures returned by res
    struct ::addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    // get host info into res
    int errcode = getaddrinfo(hostname, nullptr, &hints, res);
    if (errcode != 0) {
        FILE_LOG(logERROR) << "Could not convert hostname (" << hostname
                           << ") to internet address (zmq):"
                           << gai_strerror(errcode);
    } else {
        if (*res == nullptr) {
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
int ConvertInternetAddresstoIpString(struct ::addrinfo *res, char *ip,
                                     const int ipsize) {
    if (inet_ntop(res->ai_family,
                  &((struct sockaddr_in *)res->ai_addr)->sin_addr, ip,
                  ipsize) != nullptr) {
        ::freeaddrinfo(res);
        return 0;
    }
    FILE_LOG(logERROR) << "Could not convert internet address to ip string";
    return 1;
}

} // namespace sls
