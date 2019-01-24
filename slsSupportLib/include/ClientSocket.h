#pragma once
#include "DataSocket.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <string>

namespace sls{

class ClientSocket: public DataSocket{
public:
    ClientSocket(const std::string& hostname, uint16_t port_number);
    int connect();
private:
    struct sockaddr_in serverAddr {};
};

}; //namespace sls