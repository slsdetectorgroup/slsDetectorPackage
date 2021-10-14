// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once

#include "sls/DataSocket.h"
namespace sls {
class ServerInterface;
}

#include "sls/ServerSocket.h"
#include "sls/sls_detector_defs.h"
namespace sls {

class ServerInterface : public DataSocket {
    using defs = slsDetectorDefs;

  public:
    ServerInterface(int socketId) : DataSocket(socketId) {}

    int sendResult(int ret, void *retval, int retvalSize, char *mess = nullptr);

    template <typename T> int sendResult(int ret, T &retval) {
        return sendResult(ret, &retval, sizeof(retval, nullptr));
    }

    template <typename T> int sendResult(T &&retval) {
        Send(defs::OK);
        Send(retval);
        return defs::OK;
    }
};

} // namespace sls