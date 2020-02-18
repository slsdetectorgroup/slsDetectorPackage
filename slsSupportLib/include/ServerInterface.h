#pragma once

#include "DataSocket.h"
namespace sls {
class ServerInterface;
}

#include "ServerSocket.h"
#include "sls_detector_defs.h"
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