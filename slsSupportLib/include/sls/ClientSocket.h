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
    void sendCommandThenRead(int fnum, const void *args, size_t args_size,
                             void *retval, size_t retval_size);
    std::string readErrorMessage();

    template <typename Arg>
    void sendCommandVariableSize(int fnum, const std::vector<Arg> &args,
                                 void *retval, size_t retval_size) {
        int count = args.size();
        sendCommand(fnum, &count, sizeof(count));
        if (count > 0) {
            Send(args);
        }
        readReply(retval, retval_size);
    }

    template <typename Ret>
    void sendCommandVariableSize(int fnum, const void *args, size_t args_size,
                                 std::vector<Ret> &retval) {
        sendCommand(fnum, args, args_size);
        int count = 0;
        readReply(&count, sizeof(count));
        retval.resize(count);
        if (count > 0) {
            Receive(retval);
        }
    }
    void sendCommand(int fnum, const void *args, size_t args_size);
    void readReply(void *retval, size_t retval_size);

  private:
    [[noreturn]] void throwError(const std::string &msg) const;
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
