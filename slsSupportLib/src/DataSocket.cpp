// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "sls/DataSocket.h"
#include "sls/Timer.h"
#include "sls/logger.h"
#include "sls/sls_detector_exceptions.h"
#include "sls/sls_detector_funcs.h"
#include <algorithm>
#include <arpa/inet.h>
#include <cassert>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <netdb.h>
#include <sstream>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

namespace sls {

DataSocket::DataSocket(int socketId) : sockfd_(socketId) {
    int value = 1;
    setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(value));
#ifdef SO_NOSIGPIPE
    // macOS/BSD: suppress SIGPIPE when sending to a peer that closed the
    // connection, so a failed send returns an error instead of killing the
    // process. On Linux we instead pass MSG_NOSIGNAL to send() (see Send()).
    int nosigpipe = 1;
    setsockopt(sockfd_, SOL_SOCKET, SO_NOSIGPIPE, &nosigpipe,
               sizeof(nosigpipe));
#endif
}

DataSocket::~DataSocket() {
    if (sockfd_ <= 0) {
        return;
    } else {
        try {
            close();
        } catch (...) {
        }
    }
}

void DataSocket::swap(DataSocket &other) noexcept {
    std::swap(sockfd_, other.sockfd_);
    std::swap(fnum_, other.fnum_);
}

DataSocket::DataSocket(DataSocket &&move) noexcept { move.swap(*this); }
DataSocket &DataSocket::operator=(DataSocket &&move) noexcept {
    move.swap(*this);
    return *this;
}

void DataSocket::setFnum(const int fnum) { fnum_ = fnum; }

int DataSocket::Receive(void *buffer, size_t size) {
    // TODO!(Erik) Add sleep? how many reties?
    ssize_t bytes_expected = static_cast<ssize_t>(size);
    ssize_t bytes_read = 0;
    ssize_t this_read = 0; // last read result, kept for diagnostics
    Timer timer;
    while (bytes_read < bytes_expected) {
        this_read =
            ::read(getSocketId(), reinterpret_cast<char *>(buffer) + bytes_read,
                   bytes_expected - bytes_read);
        if (this_read < 0 && errno == EINTR)
            continue; // interrupted by a signal, retry
        if (this_read <= 0)
            break;
        bytes_read += this_read;
    }
    if (bytes_read == bytes_expected) {
        return static_cast<int>(bytes_read);
    } else {
        int err = errno; // capture before any other call can clobber it
        std::ostringstream ss;
        ss << "TCP socket read " << bytes_read << " bytes instead of "
           << bytes_expected << " bytes ("
           << getFunctionNameFromEnum(static_cast<detFuncs>(fnum_)) << ')';
        if (this_read == 0)
            ss << ": connection closed by peer (EOF)";
        else if (this_read < 0)
            ss << ": read error: " << std::strerror(err) << " ("
               << errno_name(err) << ")";
        ss << " after " << timer.elapsed_ms() << " ms";
        throw SocketError(ss.str());
    }
}

std::string DataSocket::Receive(size_t length) {
    std::string buff(length, '\0');
    Receive(&buff[0], buff.size());
    auto pos = buff.find('\0');
    if (pos != std::string::npos)
        buff.erase(pos);
    return buff;
}
int DataSocket::Send(const void *buffer, size_t size) {
    ssize_t bytes_expected = static_cast<ssize_t>(size);
    ssize_t bytes_sent = 0;
    ssize_t this_send = 0; // last write result, kept for diagnostics
    // Linux: avoid SIGPIPE on a broken connection by using send() with
    // MSG_NOSIGNAL. macOS/BSD lack the flag and use SO_NOSIGPIPE instead
    // (set in the constructor).
#ifdef MSG_NOSIGNAL
    const int send_flags = MSG_NOSIGNAL;
#else
    const int send_flags = 0;
#endif
    Timer timer;
    while (bytes_sent < bytes_expected) {
        this_send = ::send(getSocketId(),
                           reinterpret_cast<const char *>(buffer) + bytes_sent,
                           bytes_expected - bytes_sent, send_flags);
        if (this_send < 0 && errno == EINTR)
            continue; // interrupted by a signal, retry
        if (this_send <= 0)
            break;
        bytes_sent += this_send;
    }
    if (bytes_sent == bytes_expected) {
        return static_cast<int>(bytes_sent);
    } else {
        int err = errno; // capture before any other call can clobber it
        std::ostringstream ss;
        ss << "TCP socket sent " << bytes_sent << " bytes instead of "
           << bytes_expected << " bytes ("
           << getFunctionNameFromEnum(static_cast<detFuncs>(fnum_)) << ')';
        if (this_send < 0)
            ss << ": write error: " << std::strerror(err) << " ("
               << errno_name(err) << ")";
        ss << " after " << timer.elapsed_ms() << " ms";
        throw SocketError(ss.str());
    }
}

int DataSocket::Send(const std::string &s) { return Send(&s[0], s.size()); }

int DataSocket::setReceiveTimeout(int us) {
    timeval t{};
    t.tv_sec = us / 1000000;
    t.tv_usec = us % 1000000;
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
        LOG(logERROR) << "setsockopt SO_RCVTIMEO " << 0;
    }

    t.tv_sec = t_seconds;
    t.tv_usec = 0;
    // Sending timeout in seconds
    if (::setsockopt(getSocketId(), SOL_SOCKET, SO_SNDTIMEO, &t,
                     sizeof(struct timeval)) < 0) {
        LOG(logERROR) << "setsockopt SO_SNDTIMEO " << t_seconds;
    }
    return 0;
}

void DataSocket::close() {
    if (sockfd_ > 0) {
        if (::close(sockfd_)) {
            std::stringstream ss;
            ss << "could not close socket (fd: " << sockfd_ << ")";
            throw SocketError(ss.str());
        }
        sockfd_ = -1;
    } else {
        throw std::runtime_error("Socket ERROR: close called on bad socket\n");
    }
}

void DataSocket::shutDownSocket() {
    ::shutdown(getSocketId(), SHUT_RDWR);
    close();
}

void DataSocket::shutdown() { ::shutdown(sockfd_, SHUT_RDWR); }

std::string_view DataSocket::errno_name(int e) {
    switch (e) {
#ifdef EACCES
    case EACCES:
        return "EACCES";
#endif
#ifdef EAGAIN
    case EAGAIN:
        return "EAGAIN";
#endif
#ifdef EBADF
    case EBADF:
        return "EBADF";
#endif
#ifdef ECONNABORTED
    case ECONNABORTED:
        return "ECONNABORTED";
#endif
#ifdef ECONNREFUSED
    case ECONNREFUSED:
        return "ECONNREFUSED";
#endif
#ifdef ECONNRESET
    case ECONNRESET:
        return "ECONNRESET";
#endif
#ifdef EINPROGRESS
    case EINPROGRESS:
        return "EINPROGRESS";
#endif
#ifdef EINTR
    case EINTR:
        return "EINTR";
#endif
#ifdef EINVAL
    case EINVAL:
        return "EINVAL";
#endif
#ifdef EPIPE
    case EPIPE:
        return "EPIPE";
#endif
#ifdef ETIMEDOUT
    case ETIMEDOUT:
        return "ETIMEDOUT";
#endif
#ifdef EWOULDBLOCK
#if EWOULDBLOCK != EAGAIN
    case EWOULDBLOCK:
        return "EWOULDBLOCK";
#endif
#endif
    default:
        return "UNKNOWN_ERRNO";
    }
}

} // namespace sls
