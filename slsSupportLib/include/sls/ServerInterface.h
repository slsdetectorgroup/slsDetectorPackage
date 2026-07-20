// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once

#include "sls/DataSocket.h"
#include "sls/logger.h"

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

    template <typename T> int sendVariableResult(T &&retval) {
        Send(defs::OK);
        int count = static_cast<int>(retval.size());
        Send(count);
        if (count > 0)
            Send(retval);
        return defs::OK;
    }

    template <typename T> T receiveVariableArgs() {
        size_t count = 0;
        Receive(&count, sizeof(count));
        T retval{count};
        if (count > 0)
            Receive(retval);
        return retval;
    }
};

} // namespace sls