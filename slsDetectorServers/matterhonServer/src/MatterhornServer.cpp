#include "MatterhornServer.h"
#include "sls/network_utils.h"
#include "sls/sls_detector_defs.h"
// #include "sharedMemory.h"
#include "communication_funcs.h"

namespace sls {

MatterhornServer::MatterhornServer(uint16_t port) {

    validatePortNumber(port);
    /*
    // TODO: keep the c code for now
    if (sharedMemory_create(port) == slsDetectorDefs::FAIL) {
        throw sls::RuntimeError("Failed to create shared memory");
    }
    */

// mmh do I want a virtual server inheriting from parent Server class? and
// parent Matterhorn class - probably better
#ifdef VIRTUAL
    udpDetails.srcip = LOCALHOSTIP_INT;
#endif
    udpDetails.srcport = DEFAULT_UDP_SRC_PORTNO;
    udpDetails.dstport = DEFAULT_UDP_DST_PORTNO;

    // TODO: when do i set the udp mac and ip ?

    tcpipInterface = std::make_unique<MatterhornClientInterface>(
        port); // TODO: need a tcp and udp interface

    // need a function to setup detector - e.g. set all registers etc.
}

} // namespace sls