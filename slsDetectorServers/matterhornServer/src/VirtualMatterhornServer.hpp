
#include "BaseMatterhornServer.hpp"
#include "VirtualMatterhornServerImpl.hpp"

namespace sls {

class VirtualMatterhornServer
    : public BaseMatterhornServer<VirtualMatterhornServer> {

  public:
    using ImplType = VirtualMatterhornServerImpl;
    /**
     * Constructor
     * Starts up a virtual Matterhorn server.
     * Assembles a virtual Matterhorn server using TCP and UDP detector
     * interfaces throws an exception in case of failure
     * @param port TCP/IP port number
     */
    explicit VirtualMatterhornServer(uint16_t port = DEFAULT_TCP_CNTRL_PORTNO);

    ~VirtualMatterhornServer() = default;

  private:
    ImplType *getImpl() { return this->getDerivedImpl(); }

    const ImplType *getImpl() const { return this->getDerivedImpl(); }
};

} // namespace sls