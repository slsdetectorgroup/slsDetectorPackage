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
    ServerInterface2(int socketId) : DataSocket(socketId){}

    int sendResult(bool update, int ret, void *retval, int retvalSize,
                   char *mess = nullptr);
    int receiveArg(void *arg, int sizeofArg);



private:

};

} // namespace sls