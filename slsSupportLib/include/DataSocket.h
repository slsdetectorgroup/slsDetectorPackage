#pragma once

#include <cstdint>
#include <cstddef>
namespace sls {

class DataSocket {
  public:
    DataSocket(int socketId);

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
