#pragma once
#include "BaseMatterhornServer.h"
#include "MatterhornServerImpl.hpp"
#include "TCPInterface.h"
#include "sls/sls_detector_defs.h"
#include <array>
#include <memory>

namespace sls {

class MatterhornServer : public BaseMatterhornServer<MatterhornServer> {

  public:
    using ImplType = MatterhornServerImpl;

    /**
     * Constructor
     * Starts up a Matterhorn server.
     * Assembles a Matterhorn server using TCP and UDP detector interfaces
     * throws an exception in case of failure
     * @param port TCP/IP port number
     */
    explicit MatterhornServer(uint16_t port = DEFAULT_TCP_CNTRL_PORTNO);

    ~MatterhornServer() = default;

  private:
    ImplType *getImpl() { return this->getDerivedImpl(); }

    const ImplType *getImpl() const { return this->getDerivedImpl(); }
};

} // namespace sls