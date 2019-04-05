#pragma once
#include "DataSocket.h"
#include <netdb.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>

namespace sls {

class ClientSocket : public DataSocket {
  public:
    ClientSocket(const bool isRx, const std::string &hostname, uint16_t port_number);
    ClientSocket(const bool isRx, struct sockaddr_in addr);
    int sendCommandThenRead(int fnum, void *args, size_t args_size, void *retval,
                            size_t retval_size);

  private:
    void readReply(int &ret, void *retval, size_t retval_size);
    struct sockaddr_in serverAddr {};
    bool isReceiver;
};

class ReceiverSocket : public ClientSocket {
  public:
    ReceiverSocket(const std::string &hostname, uint16_t port_number)
        : ClientSocket(true, hostname, port_number){};
    ReceiverSocket(struct sockaddr_in addr) : ClientSocket(true, addr){};
};

class DetectorSocket : public ClientSocket {
  public:
    DetectorSocket(const std::string &hostname, uint16_t port_number)
        : ClientSocket(false, hostname, port_number){};
    DetectorSocket(struct sockaddr_in addr) : ClientSocket(false, addr){};
};

}; // namespace sls
