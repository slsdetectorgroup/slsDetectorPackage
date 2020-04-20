#pragma once

#include <cstddef>
#include <cstdint>
#include <netdb.h>
#include <numeric>
#include <string>
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
  
    //No copy since the class manage the underlying socket
    DataSocket(const DataSocket &) = delete;
    DataSocket &operator=(DataSocket const &) = delete;
    int getSocketId() const { return sockfd_; }

    
    int Send(const void *buffer, size_t size);
    template <typename T> int Send(T &&data) {
        return Send(&data, sizeof(data));
    }
    // Variadic template to send all arguments
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
    void shutdown();

  private:
    int sockfd_ = -1;
};

}; // namespace sls
