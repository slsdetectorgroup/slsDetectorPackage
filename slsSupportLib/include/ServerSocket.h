#pragma once

#include "DataSocket.h"

#include <cstdint>
#include <netdb.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>

namespace sls {

class ServerSocket : public DataSocket {
  public:
    ServerSocket(int port);
    DataSocket accept();
    const std::string &getLastClient() { return lastClient_; }

  private:
    std::string lastClient_ = std::string(INET_ADDRSTRLEN, '\0');
    std::string thisClient_ = std::string(INET_ADDRSTRLEN, '\0');
    // char lastClient_[INET_ADDRSTRLEN]{};
};

}; //namespace sls