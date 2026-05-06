#include "MatterhornServer.h"

namespace sls {

MatterhornServer::MatterhornServer(uint16_t port)
    : BaseMatterhornServer<MatterhornServer>(port) {

    // TODO: when do i set the udp mac and ip ?

    // should maybe be part of the constructor?
    tcpInterface->startTCPServer();

    // need a function to setup detector - e.g. set all registers etc.
}

ReturnCode MatterhornServer::initial_checks(ServerInterface &socket) {

    // TODO: add more checks here, for now just return true to be able to test
    // the should check firmware -client compatibility
    bool initial_checks_passed = true;
    return static_cast<ReturnCode>(socket.sendResult(initial_checks_passed));
}

} // namespace sls