
#include "BaseMatterhornServer.hpp"
#include "VirtualMatterhornServerImpl.hpp"

namespace sls {

template <bool isStopServer = false>
class VirtualMatterhornServer
    : public BaseMatterhornServer<VirtualMatterhornServer<isStopServer>> {

  public:
    /**
     * Constructor
     * Starts up a virtual Matterhorn server.
     * Assembles a virtual Matterhorn server using TCP and UDP detector
     * interfaces throws an exception in case of failure
     * @param port TCP/IP port number
     */
    explicit VirtualMatterhornServer(uint16_t port = DEFAULT_TCP_CNTRL_PORTNO);

    ~VirtualMatterhornServer() = default;
};

template <bool isStopServer>
VirtualMatterhornServer<isStopServer>::VirtualMatterhornServer(uint16_t port)
    : BaseMatterhornServer<VirtualMatterhornServer<isStopServer>>(
          std::make_unique<VirtualMatterhornServerImpl<isStopServer>>(), port) {

    LOG(logDEBUG) << "Initializing virtual Matterhorn server on port " << port;

    // should maybe be part of the constructor?
    this->tcpInterface->startTCPServer();

    // TODO: no init_server function for now is it neccessary to set the init
    // flag
    this->getImpl()->setupDetector();
}

} // namespace sls