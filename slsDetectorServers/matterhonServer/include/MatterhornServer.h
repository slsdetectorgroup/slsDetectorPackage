#pragma once
#include "BaseMatterhornServer.h"
#include "TCPInterface.h"
#include "sls/sls_detector_defs.h"
#include <array>
#include <memory>

namespace sls {

class MatterhornServer : public BaseMatterhornServer<MatterhornServer> {

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

    ReturnCode initial_checks(ServerInterface &socket);

    ReturnCode get_update_mode(ServerInterface &socket);
};

} // namespace sls