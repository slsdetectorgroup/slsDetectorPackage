#include "MatterhornServer.h"

namespace sls {

MatterhornServer::MatterhornServer(uint16_t port)
    : BaseMatterhornServer<MatterhornServer>(
          std::make_unique<MatterhornServerImpl>(), port) {

    // should maybe be part of the constructor?
    tcpInterface->startTCPServer();

    // TODO: no init_server function for now is it neccessary to set the init
    // flag
    getImpl()->setupDetector();
}

} // namespace sls