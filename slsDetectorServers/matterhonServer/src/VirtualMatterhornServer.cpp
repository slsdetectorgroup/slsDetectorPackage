#include "VirtualMatterhornServer.h"

namespace sls {

VirtualMatterhornServer::VirtualMatterhornServer(uint16_t port)
    : BaseMatterhornServer<VirtualMatterhornServer>(port) {

    udpDetails[0].srcip = LOCALHOSTIP_INT;

    // should maybe be part of the constructor?
    tcpInterface->startTCPServer();

    // need a function to setup detector - e.g. set all registers etc.
}

ReturnCode VirtualMatterhornServer::initial_checks(ServerInterface &socket) {

    // TODO: add more checks here, for now just return true to be able to test
    // the should check firmware -client compatibility
    bool initial_checks_passed = true;
    return static_cast<ReturnCode>(socket.sendResult(initial_checks_passed));
}

} // namespace sls