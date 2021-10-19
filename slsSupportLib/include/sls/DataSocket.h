// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once

#include "sls/TypeTraits.h"
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <netdb.h>
#include <numeric>
#include <string>
#include <vector>
namespace sls {

/* Base class for TCP socket, this is used to send data between detector, client
   and receiver. Specific protocols inherit from this class.*/

class DataSocket {
  public:
    DataSocket(int socketId);
    DataSocket(DataSocket &&move) noexcept;
    virtual ~DataSocket();
    DataSocket &operator=(DataSocket &&move) noexcept;
    void swap(DataSocket &other) noexcept;

    // No copy since the class manage the underlying socket
    DataSocket(const DataSocket &) = delete;
    DataSocket &operator=(DataSocket const &) = delete;
    int getSocketId() const { return sockfd_; }
    int getFnum() const { return fnum_; }
    void setFnum(const int fnum);

    int Send(const void *buffer, size_t size);

    // Send everything that is not a vector or string by using address and
    // sizeof
    // TODO! We probably should restrict this even more to avoid bugs when
    // we send object instead of data
    template <typename T>
    typename std::enable_if<
        !is_vector<typename std::remove_reference<T>::type>::value &&
            !std::is_same<typename std::remove_reference<T>::type,
                          std::string>::value,
        int>::type
    Send(T &&data) {
        return Send(&data, sizeof(data));
    }

    template <typename T> int Send(const std::vector<T> &vec) {
        return Send(vec.data(), sizeof(T) * vec.size());
    }

    int Send(const std::string &s);

    // Variadic template to send all arguments
    template <class... Args> int SendAll(Args &&...args) {
        auto l = std::initializer_list<int>{Send(args)...};
        auto sum = std::accumulate(begin(l), end(l), 0);
        return sum;
    }
    int Receive(void *buffer, size_t size);

    template <typename T> int Receive(T &arg) {
        return Receive(&arg, sizeof(arg));
    }

    template <typename T> int Receive(std::vector<T> &buff) {
        return Receive(buff.data(), sizeof(T) * buff.size());
    }

    template <typename T> T Receive() {
        T arg;
        Receive(&arg, sizeof(arg));
        return arg;
    }

    std::string Receive(size_t length);

    int read(void *buffer, size_t size);
    int write(void *buffer, size_t size);
    int setTimeOut(int t_seconds);
    int setReceiveTimeout(int us);
    void close();
    void shutDownSocket();
    void shutdown();

  private:
    int sockfd_ = -1;
    int fnum_{0};
};

}; // namespace sls
