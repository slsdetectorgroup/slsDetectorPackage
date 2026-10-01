#pragma once
#include "BaseMatterhornServer.hpp"
#include "MatterhornServerImpl.hpp"
#include "TCPInterface.hpp"
#include "sls/sls_detector_defs.h"
#include <array>
#include <memory>

namespace sls {

template <bool isStopServer = false>
class MatterhornServer
    : public BaseMatterhornServer<MatterhornServer<isStopServer>> {

  public:
    /**
     * Constructor
     * Starts up a Matterhorn server.
     * Assembles a Matterhorn server using TCP and UDP detector interfaces
     * throws an exception in case of failure
     * @param port TCP/IP port number
     */
    explicit MatterhornServer(uint16_t port = DEFAULT_TCP_CNTRL_PORTNO);

    ~MatterhornServer() = default;
};

template <bool isStopServer>
MatterhornServer<isStopServer>::MatterhornServer(uint16_t port)
    : BaseMatterhornServer<MatterhornServer<isStopServer>>(
          std::make_unique<MatterhornServerImpl<isStopServer>>(), port) {

    // should maybe be part of the constructor?
    this->tcpInterface->startTCPServer();

    // TODO: no init_server function for now is it neccessary to set the init
    // flag
    this->getImpl()->setupDetector();
}

} // namespace sls