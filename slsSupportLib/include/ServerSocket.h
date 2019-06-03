#pragma once

#include "DataSocket.h"
#include "ServerInterface2.h"
#include "network_utils.h"
#include <cstdint>
#include <netdb.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>

namespace sls {

class ServerSocket : public DataSocket {
  public:
    ServerSocket(int port);
    ServerInterface2 accept();
    IpAddr getLastClient() const noexcept { return lastClient; }
    IpAddr getThisClient() const noexcept { return thisClient; }
    IpAddr getLockedBy() const noexcept { return lockedBy; }
    bool differentClients() const noexcept {return lastClient != thisClient;}
    void setLockedBy(IpAddr addr) { lockedBy = addr; }
    void setLastClient(IpAddr addr) { lastClient = addr; }
    int getPort() const noexcept { return serverPort; }

  private:
    IpAddr thisClient;
    IpAddr lastClient;
    IpAddr lockedBy;
    int serverPort;
};

}; // namespace sls