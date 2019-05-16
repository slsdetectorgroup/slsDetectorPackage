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
    IpAddr getLastClient() noexcept { return lastClient; }
    IpAddr getThisClient() noexcept { return thisClient; }
    IpAddr getLockedBy() noexcept { return lockedBy; }
    void setLockedBy(IpAddr addr) { lockedBy = addr; }
    void setLastClient(IpAddr addr) { lastClient = addr; }
    int getPort() const;
    void SendResult(int &ret, void *retval, int retvalSize, char *mess);

  private:
    IpAddr thisClient;
    IpAddr lastClient;
    IpAddr lockedBy;
    int serverPort;
};

}; // namespace sls