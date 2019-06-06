#pragma once

#include <cstddef>
#include <cstdint>
#include <netdb.h>
#include <string>
#include <numeric>
namespace sls {

class DataSocket {
  public:
    DataSocket(int socketId);
    DataSocket(DataSocket &&move) noexcept;
    virtual ~DataSocket();
    DataSocket &operator=(DataSocket &&move) noexcept;
    void swap(DataSocket &other) noexcept;
    DataSocket(const DataSocket &) = delete;
    DataSocket &operator=(DataSocket const &) = delete;
    int getSocketId() const { return socketId_; }

    int Send(const void *buffer, size_t size);
    template <typename T> int Send(T &&data) {
        return Send(&data, sizeof(data));
    }

    // Trick to send all
    template <class... Args> int SendAll(Args &&... args) {
        auto l = std::initializer_list<int>{Send(args)...};
        auto sum = std::accumulate(begin(l), end(l), 0);
        return sum;
    }

    int Receive(void *buffer, size_t size);

    template <typename T> int Receive(T &arg) {
        return Receive(&arg, sizeof(arg));
    }
    template <typename T> T Receive() {
        T arg;
        Receive(&arg, sizeof(arg));
        return arg;
    }

    int read(void *buffer, size_t size);
    int write(void *buffer, size_t size);
    int setTimeOut(int t_seconds);
    int setReceiveTimeout(int us);
    void close();
    void shutDownSocket();

  private:
    int socketId_ = -1;
};

int ConvertHostnameToInternetAddress(const char *const hostname,
                                     struct ::addrinfo **res);
int ConvertInternetAddresstoIpString(struct ::addrinfo *res, char *ip,
                                     const int ipsize);

struct ::sockaddr_in
ConvertHostnameToInternetAddress(const std::string &hostname);

}; // namespace sls
