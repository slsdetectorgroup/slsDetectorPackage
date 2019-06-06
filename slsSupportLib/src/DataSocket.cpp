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

int DataSocket::Receive(void *buffer, size_t size) {
    // TODO!(Erik) Add sleep? how many reties?
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
        ss << "TCP socket read " << bytes_read << " bytes instead of "
           << bytes_expected << " bytes";
        throw sls::SocketError(ss.str());
    }
}

int DataSocket::Send(const void *buffer, size_t size) {
    int bytes_sent = 0;
    int data_size = static_cast<int>(size); // signed size
    while (bytes_sent < (data_size)) {
        auto this_send = ::write(getSocketId(), buffer, size);
        if (this_send <= 0)
            break;
        bytes_sent += this_send;
    }
    if (bytes_sent != data_size) {
        std::ostringstream ss;
        ss << "TCP socket sent " << bytes_sent << " bytes instead of "
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

} // namespace sls
