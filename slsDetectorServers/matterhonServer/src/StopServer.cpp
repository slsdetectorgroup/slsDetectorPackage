
#include "StopServer.h"
#include "sls/network_utils.h"
#include "sls/sls_detector_defs.h"

namespace sls {
StopServer::StopServer(uint16_t port) : MatterhornServer(port) {

    /*
    // open shared memory segment of control server map to virtual memory space
    if (sharedMemory_open(port - 1) == slsDetectorDefs::FAIL) {
        throw sls::RuntimeError("Failed to open shared memory");
    }
    */
}

} // namespace sls
