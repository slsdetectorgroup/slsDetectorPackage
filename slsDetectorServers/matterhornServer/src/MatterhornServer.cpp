#include "MatterhornServer.h"

namespace sls {

MatterhornServer::MatterhornServer(uint16_t port)
    : BaseMatterhornServer<MatterhornServer>(port) {
    // map the IP core base addresses to memory
    busCommunication.mapToMemory(); // TODO: should this happen in constructor?

    // should maybe be part of the constructor?
    tcpInterface->startTCPServer();

    // TODO: no init_server function for now is it neccessary to set the init
    // flag
    this->setupDetector();
}

ReturnCode MatterhornServer::initial_checks(ServerInterface &socket) {

    // TODO: add more checks here, for now just return true to be able to test
    // the should check firmware -client compatibility
    bool initial_checks_passed = true;
    return static_cast<ReturnCode>(socket.sendResult(initial_checks_passed));
}

} // namespace sls