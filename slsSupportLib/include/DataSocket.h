#pragma once

#include <cstddef>
#include <cstdint>
#include <netdb.h>
namespace sls {

class DataSocket {
  public:
    DataSocket(int socketId);
    DataSocket(DataSocket &&move) noexcept;
    virtual ~DataSocket();
    DataSocket &operator=(DataSocket &&move) noexcept;
    void swap(DataSocket &other) noexcept;
    DataSocket(DataSocket const &) = delete;
    DataSocket &operator=(DataSocket const &) = delete;
    int getSocketId() const {
        return socketId_;
    }
    size_t sendData(void *buffer, size_t size);
    size_t receiveData(void *buffer, size_t size);
    int setTimeOut(int t_seconds);
    void close();

  private:
    int socketId_ = -1;
};

int ConvertHostnameToInternetAddress(const char *const hostname, struct ::addrinfo **res);
int ConvertInternetAddresstoIpString(struct ::addrinfo *res, char *ip, const int ipsize);

}; // namespace sls
