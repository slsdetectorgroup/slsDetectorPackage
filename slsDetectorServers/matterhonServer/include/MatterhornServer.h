#pragma once
#include "TCPInterface.h"
#include "sls/sls_detector_defs.h"
#include <array>
#include <memory>

namespace sls {

/// @brief struct saving udp details (one UDP port per module)
struct UDPInfo {
    uint16_t srcport{};
    uint16_t dstport{};
    uint64_t srcmac{};
    uint64_t dstmac{};
    uint32_t srcip{};
    uint32_t dstip{};
};

/// @brief Base class for Matterhorn Server, can be used to implement a virtual
/// server for testing and actual server
class BaseMatterhornServer {

  public:
    /**
     * Constructor
     * Starts up a Matterhorn server.
     * Assembles a Matterhorn server using TCP and UDP detector interfaces
     * throws an exception in case of failure
     * @param port TCP/IP port number
     */
    explicit BaseMatterhornServer(uint16_t port = DEFAULT_TCP_CNTRL_PORTNO);

    ~BaseMatterhornServer() = default;

    ReturnCode get_version(ServerInterface &socket);

    ReturnCode get_detector_type(ServerInterface &socket);

    virtual ReturnCode initial_checks(ServerInterface &socket) = 0;

    ReturnCode get_num_udp_interfaces(ServerInterface &socket);

    virtual ReturnCode get_update_mode(ServerInterface &socket) = 0;

    ReturnCode get_source_udp_mac(ServerInterface &socket);

  protected:
    static std::string getMatterhornServerVersion();

    size_t num_udp_interfaces() const;

    /// @brief  TODO what is this?
    bool updateMode{true};

  protected:
    /// @brief TCP/IP interface for communication with the client
    std::unique_ptr<TCPInterface> tcpInterface;
    std::array<UDPInfo, 1>
        udpDetails{}; // TODO: for now only one receiver per module

  private:
    /// @brief map of function IDs and corresponding functions
    // maybe load from additional file cleaner
    std::unordered_map<detFuncs, std::function<ReturnCode(ServerInterface &)>>
        function_table = {
            {detFuncs::F_GET_SERVER_VERSION,
             [this](ServerInterface &si) { return this->get_version(si); }},
            {detFuncs::F_GET_DETECTOR_TYPE,
             [this](ServerInterface &si) {
                 return this->get_detector_type(si);
             }},
            {detFuncs::F_INITIAL_CHECKS,
             [this](ServerInterface &si) { return this->initial_checks(si); }},
            {detFuncs::F_GET_NUM_INTERFACES,
             [this](ServerInterface &si) {
                 return this->get_num_udp_interfaces(si);
             }},
            {detFuncs::F_GET_UPDATE_MODE,
             [this](ServerInterface &si) { return this->get_update_mode(si); }},
            {detFuncs::F_GET_SOURCE_UDP_MAC, [this](ServerInterface &si) {
                 return this->get_source_udp_mac(si);
             }}};
};

class MatterhornServer : public BaseMatterhornServer {

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

    ReturnCode initial_checks(ServerInterface &socket) override;

    ReturnCode get_update_mode(ServerInterface &socket) override;
};

class VirtualMatterhornServer : public BaseMatterhornServer {

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

    ReturnCode initial_checks(ServerInterface &socket) override;

    ReturnCode get_update_mode(ServerInterface &socket) override;
};

} // namespace sls