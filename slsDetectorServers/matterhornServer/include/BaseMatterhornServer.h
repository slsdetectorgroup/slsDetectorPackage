#pragma once
#include "DetectorServer.h"
#include "TCPInterface.h"
// #include "communication_funcs.h"
#include "fmt/format.h"
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

/// @brief Base class for Matterhorn Server, can be used to implement a virtual
/// server for testing and actual server
template <typename DerivedServer>
class BaseMatterhornServer
    : public DetectorServer<BaseMatterhornServer<DerivedServer>> {

  public:
    /**
     * Constructor
     * Starts up a Matterhorn server.
     * Assembles a Matterhorn server using TCP and UDP detector interfaces
     * throws an exception in case of failure
     * @param port TCP/IP port number
     */
    explicit BaseMatterhornServer(uint16_t port = DEFAULT_TCP_CNTRL_PORTNO)
        : DetectorServer<BaseMatterhornServer<DerivedServer>>(port) {}

    ~BaseMatterhornServer() = default;

    ReturnCode get_version(ServerInterface &socket);

    ReturnCode get_detector_type(ServerInterface &socket);

    ReturnCode initial_checks(ServerInterface &socket);

    ReturnCode get_num_udp_interfaces(ServerInterface &socket) const;

    ReturnCode get_run_status(ServerInterface &socket) const;

    /**
     * @brief call function corresponding to the function ID received from the
     * client and send back the result
     * @param function_id the function ID received from the client
     * @param socket the socket to send the result back to the client
     */
    ReturnCode processFunction(const detFuncs function_id,
                               ServerInterface &socket);

  private:
    static std::string getMatterhornServerVersion();

    static constexpr uint8_t numUDPInterfaces =
        1; // only one udp per module for now
};

template <typename DerivedServer>
ReturnCode
BaseMatterhornServer<DerivedServer>::processFunction(const detFuncs function_id,
                                                     ServerInterface &socket) {

    switch (function_id) {
    default:
        throw RuntimeError(
            fmt::format("Function {} not implemented",
                        getFunctionNameFromEnum((enum detFuncs)function_id)));
    }
}

template <typename DerivedServer>
ReturnCode BaseMatterhornServer<DerivedServer>::get_num_udp_interfaces(
    ServerInterface &socket) const {
    return static_cast<ReturnCode>(
        socket.sendResult(static_cast<int>(numUDPInterfaces)));
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
ReturnCode
BaseMatterhornServer<DerivedServer>::initial_checks(ServerInterface &socket) {

    return static_cast<DerivedServer *>(this)->initial_checks(socket);
}

template <typename DerivedServer>
ReturnCode BaseMatterhornServer<DerivedServer>::get_run_status(
    ServerInterface &socket) const {

    return static_cast<const DerivedServer *>(this)->get_run_status(socket);
}

} // namespace sls