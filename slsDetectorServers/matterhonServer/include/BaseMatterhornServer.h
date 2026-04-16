#pragma once
#include "TCPInterface.h"
#include "communication_funcs.h"
#include "sls/logger.h"
#include "sls/network_utils.h"
#include "sls/sls_detector_defs.h"
#include "sls/versionAPI.h"
#include <array>
#include <cstring>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

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
template <typename DerivedServer> class BaseMatterhornServer {

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

    ReturnCode initial_checks(ServerInterface &socket);

    ReturnCode get_num_udp_interfaces(ServerInterface &socket);

    ReturnCode get_update_mode(ServerInterface &socket);

    ReturnCode get_source_udp_mac(ServerInterface &socket);

    ReturnCode get_source_udp_ip(ServerInterface &socket);

    ReturnCode get_source_udp_port(ServerInterface &socket);

    ReturnCode get_destination_udp_mac(ServerInterface &socket);

    ReturnCode get_destination_udp_ip(ServerInterface &socket);

    ReturnCode get_destination_udp_port(ServerInterface &socket);

  protected:
    size_t num_udp_interfaces() const;

    /// @brief  TODO what is this?
    bool updateMode{true};

    /// @brief TCP/IP interface for communication with the client
    std::unique_ptr<TCPInterface> tcpInterface;
    std::array<UDPInfo, 1>
        udpDetails{}; // TODO: for now only one receiver per module

  private:
    static std::string getMatterhornServerVersion();

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
             [this](ServerInterface &si) {
                 return static_cast<DerivedServer *>(this)->initial_checks(si);
             }},
            {detFuncs::F_GET_NUM_INTERFACES,
             [this](ServerInterface &si) {
                 return this->get_num_udp_interfaces(si);
             }},
            {detFuncs::F_GET_UPDATE_MODE,
             [this](ServerInterface &si) {
                 return static_cast<DerivedServer *>(this)->get_update_mode(si);
             }},
            {detFuncs::F_GET_SOURCE_UDP_MAC,
             [this](ServerInterface &si) {
                 return this->get_source_udp_mac(si);
             }},

            {detFuncs::F_GET_SOURCE_UDP_IP,
             [this](ServerInterface &si) {
                 return this->get_source_udp_ip(si);
             }},
            {detFuncs::F_GET_DEST_UDP_MAC,
             [this](ServerInterface &si) {
                 return this->get_destination_udp_mac(si);
             }},
            {detFuncs::F_GET_DEST_UDP_IP,
             [this](ServerInterface &si) {
                 return this->get_destination_udp_ip(si);
             }},
            {detFuncs::F_GET_DEST_UDP_PORT, [this](ServerInterface &si) {
                 return this->get_destination_udp_port(si);
             }}};
};

template <typename DerivedServer>
BaseMatterhornServer<DerivedServer>::BaseMatterhornServer(uint16_t port) {

    validatePortNumber(port);

    udpDetails[0].srcport = DEFAULT_UDP_SRC_PORTNO;
    udpDetails[0].dstport = DEFAULT_UDP_DST_PORTNO;

    // TODO: when do i set the udp mac and ip ?

    tcpInterface = std::make_unique<TCPInterface>(
        function_table, port); // TODO: need a tcp and udp interface

    // need a function to setup detector - e.g. set all registers etc.
}

template <typename DerivedServer>
ReturnCode
BaseMatterhornServer<DerivedServer>::get_version(ServerInterface &socket) {

    auto version = getMatterhornServerVersion();
    char version_cstr[MAX_STR_LENGTH]{};
    strncpy(version_cstr, version.c_str(), version.size());
    LOG(TLogLevel::logDEBUG) << "Matterhorn Server Version: " << version;
    return static_cast<ReturnCode>(socket.sendResult(
        version_cstr)); // TODO: check what would be possible return codes!!!
}

template <typename DerivedServer>
ReturnCode BaseMatterhornServer<DerivedServer>::get_detector_type(
    ServerInterface &socket) {
    int detectortype = slsDetectorDefs::detectorType::MATTERHORN;
    return static_cast<ReturnCode>(socket.sendResult(detectortype));
}

template <typename DerivedServer>
std::string BaseMatterhornServer<DerivedServer>::getMatterhornServerVersion() {
    return APIMATTERHORN;
}

template <typename DerivedServer>
size_t BaseMatterhornServer<DerivedServer>::num_udp_interfaces() const {
    return udpDetails.size();
}

template <typename DerivedServer>
ReturnCode BaseMatterhornServer<DerivedServer>::get_num_udp_interfaces(
    ServerInterface &socket) {
    int numUDPInterfaces = static_cast<int>(num_udp_interfaces());
    return static_cast<ReturnCode>(socket.sendResult(numUDPInterfaces));
}

template <typename DerivedServer>
ReturnCode BaseMatterhornServer<DerivedServer>::get_source_udp_mac(
    ServerInterface &socket) {
    return static_cast<ReturnCode>(socket.sendResult(udpDetails[0].srcmac));
}

template <typename DerivedServer>
ReturnCode BaseMatterhornServer<DerivedServer>::get_source_udp_ip(
    ServerInterface &socket) {
    return static_cast<ReturnCode>(socket.sendResult(udpDetails[0].srcip));
}

template <typename DerivedServer>
ReturnCode BaseMatterhornServer<DerivedServer>::get_source_udp_port(
    ServerInterface &socket) {
    return static_cast<ReturnCode>(
        socket.sendResult(static_cast<int>(udpDetails[0].srcport)));
}

template <typename DerivedServer>
ReturnCode BaseMatterhornServer<DerivedServer>::get_destination_udp_mac(
    ServerInterface &socket) {
    return static_cast<ReturnCode>(socket.sendResult(udpDetails[0].dstmac));
}

template <typename DerivedServer>
ReturnCode BaseMatterhornServer<DerivedServer>::get_destination_udp_ip(
    ServerInterface &socket) {
    return static_cast<ReturnCode>(socket.sendResult(udpDetails[0].dstip));
}

template <typename DerivedServer>
ReturnCode BaseMatterhornServer<DerivedServer>::get_destination_udp_port(
    ServerInterface &socket) {
    return static_cast<ReturnCode>(socket.sendResult(udpDetails[0].dstport));
};

} // namespace sls