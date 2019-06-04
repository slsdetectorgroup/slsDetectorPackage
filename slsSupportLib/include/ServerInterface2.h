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
      sendData(defs::OK);
      sendData(retval);
      return defs::OK;
    }

    template <typename T> int sendResult(T &&retval) {
      sendData(defs::OK);
      sendData(retval);
      return defs::OK;
    }




    int receiveArg(void *arg, int sizeofArg);

    template <typename T> int receiveArg(T &arg) {
        return receiveArg(&arg, sizeof(arg));
    }
    template <typename T>
    T receive(){
      T arg;
      receiveArg(&arg, sizeof(arg));
      return arg;
    }

  private:
};

} // namespace sls