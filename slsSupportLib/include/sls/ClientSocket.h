// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once
#include "sls/DataSocket.h"
#include <netdb.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>

namespace sls {

class ClientSocket : public DataSocket {
  public:
    ClientSocket(std::string stype, const std::string &hostname,
                 uint16_t port_number);
    ClientSocket(std::string stype, struct sockaddr_in addr);
    int sendCommandThenRead(int fnum, const void *args, size_t args_size,
                            void *retval, size_t retval_size);

    std::string readErrorMessage();

  private:
    void readReply(int &ret, void *retval, size_t retval_size);
    struct sockaddr_in serverAddr {};
    std::string socketType;
};

class ReceiverSocket : public ClientSocket {
  public:
    ReceiverSocket(const std::string &hostname, uint16_t port_number)
        : ClientSocket("Receiver", hostname, port_number){};
    ReceiverSocket(struct sockaddr_in addr) : ClientSocket("Receiver", addr){};
};

class DetectorSocket : public ClientSocket {
  public:
    DetectorSocket(const std::string &hostname, uint16_t port_number)
        : ClientSocket("Detector", hostname, port_number){};
    DetectorSocket(struct sockaddr_in addr) : ClientSocket("Detector", addr){};
};

class GuiSocket : public ClientSocket {
  public:
    GuiSocket(const std::string &hostname, uint16_t port_number)
        : ClientSocket("Gui", hostname, port_number){};
    GuiSocket(struct sockaddr_in addr) : ClientSocket("Gui", addr){};
};

}; // namespace sls
