#include "VirtualMatterhornServer.hpp"
#include "helpers/Defs.hpp"
#include "helpers/Helpers.hpp"
#include "sls/ToString.h"

namespace sls {

VirtualMatterhornServer::VirtualMatterhornServer(uint16_t port)
    : BaseMatterhornServer<VirtualMatterhornServer>(
          std::make_unique<VirtualMatterhornServerImpl>(), port) {

    LOG(logDEBUG) << "Initializing virtual Matterhorn server on port " << port;

    // should maybe be part of the constructor?
    tcpInterface->startTCPServer();

    // TODO: no init_server function for now is it neccessary to set the init
    // flag
    getImpl()->setupDetector();

    getImpl()->set_source_udp_ip(
        LOCALHOSTIP_INT); // TODO: should this be done in setupDetector?
}

} // namespace sls