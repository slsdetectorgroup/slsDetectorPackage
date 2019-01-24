#pragma once

#include <cstdint>
#include <cstddef>
namespace sls {

class DataSocket {
  public:
    DataSocket(int socketId);
    DataSocket(DataSocket&& move) noexcept;
    DataSocket& operator=(DataSocket&& move) noexcept;
    void swap(DataSocket& other) noexcept;
    DataSocket(DataSocket const&) = delete;
    DataSocket& operator=(DataSocket const&) = delete;
    int getSocketId() const{
        return socketId_;
    }
    size_t sendData(char *buffer, size_t size);
    size_t receiveData(char * buffer, size_t size);

    void close();

  private:
    int socketId_ = -1;
};

}; // namespace sls
