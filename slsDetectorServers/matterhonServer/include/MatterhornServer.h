#pragma once
#include "MatterhornClientInterface.h"
#include "sls/sls_detector_defs.h"
#include <memory>

namespace sls {

/// @brief struct saving udp details (one UDP port per module)
struct UDPInfo {
    uint16_t srcport;
    uint16_t dstport;
    uint64_t srcmac;
    uint64_t dstmac;
    uint32_t srcip;
    uint32_t dstip;
};

class MatterhornServer {

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

  private:
    std::unique_ptr<MatterhornClientInterface> tcpipInterface;
    UDPInfo udpDetails{}; // TODO: for now only one receiver per module
};

} // namespace sls