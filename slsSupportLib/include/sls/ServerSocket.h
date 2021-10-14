// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once

#include "sls/DataSocket.h"
#include "sls/ServerInterface.h"
#include "sls/network_utils.h"
#include <cstdint>
#include <netdb.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>

namespace sls {

class ServerSocket : public DataSocket {
  public:
    ServerSocket(int port);
    ServerInterface accept();
    IpAddr getLastClient() const noexcept { return lastClient; }
    IpAddr getThisClient() const noexcept { return thisClient; }
    IpAddr getLockedBy() const noexcept { return lockedBy; }
    bool differentClients() const noexcept { return lastClient != thisClient; }
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