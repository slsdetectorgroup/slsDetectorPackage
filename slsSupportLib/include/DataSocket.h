#pragma once

#include <cstddef>
#include <cstdint>
#include <netdb.h>
#include <string>
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
    int getSocketId() const {
        return socketId_;
    }
    int sendData(const void *buffer, size_t size);

    template<typename T>
    int sendData(T& data){
      return sendData(&data, sizeof(data));
    }

    template<typename T>
    int sendData(T&& data){
      return sendData(&data, sizeof(data));
    }

    int receiveData(void *buffer, size_t size);
    int read(void *buffer, size_t size);
    int write(void *buffer, size_t size);
    int setTimeOut(int t_seconds);
    int setReceiveTimeout(int us);
    void close();
    void shutDownSocket();

  private:
    int socketId_ = -1;
};

int ConvertHostnameToInternetAddress(const char *const hostname, struct ::addrinfo **res);
int ConvertInternetAddresstoIpString(struct ::addrinfo *res, char *ip, const int ipsize);

struct ::sockaddr_in ConvertHostnameToInternetAddress(const std::string &hostname);

}; // namespace sls
