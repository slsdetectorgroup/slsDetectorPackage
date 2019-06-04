#pragma once

#include "DataSocket.h"
namespace sls {
class ServerInterface2;
}

#include "ServerSocket.h"
#include "sls_detector_defs.h"
namespace sls {

class ServerInterface2 : public DataSocket {
    using defs = slsDetectorDefs;

  public:
    ServerInterface2(int socketId) : DataSocket(socketId) {}

    int sendResult(int ret, void *retval, int retvalSize, char *mess = nullptr);

    template <typename T> int sendResult(int ret, T &retval) {
        return sendResult(ret, &retval, sizeof(retval, nullptr));
    }

    template <typename T> int sendResult(T &retval) {
      int ret = defs::OK;
      sendData(ret);
      sendData(retval);
      return ret;
    }

    int receiveArg(void *arg, int sizeofArg);

    template <typename T> int receiveArg(T &arg) {
        return receiveArg(&arg, sizeof(arg));
    }

  private:
};

} // namespace sls