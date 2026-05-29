
#include "BaseMatterhornServer.h"

namespace sls {

class VirtualMatterhornServer
    : public BaseMatterhornServer<VirtualMatterhornServer> {

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

    ProcessedResult initial_checks(ServerInterface &socket);

    ProcessedResult get_run_status(ServerInterface &socket) const;

    ProcessedResult
    set_module_position_and_update_srcudpmac(ServerInterface &socket);

    ProcessedResult set_source_udp_mac(ServerInterface &socket);
};

} // namespace sls