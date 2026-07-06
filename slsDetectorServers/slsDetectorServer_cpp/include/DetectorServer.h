#pragma once
#include "DetectorServerImpl.hpp"
#include "TCPInterface.h"
#include "helpers/Helpers.hpp"
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

using ReturnCode = slsDetectorDefs::ReturnCode;

template <typename DerivedDetectorServer> class DetectorServer {

  public:
    /**
     * Constructor
     * Creates a detector server.
     * Assembles a detector server using TCP and UDP detector interfaces
     * throws an exception in case of failure
     * @param port TCP/IP port number
     */
    explicit DetectorServer(std::unique_ptr<DetectorServerImpl> impl_,
                            uint16_t port = DEFAULT_TCP_CNTRL_PORTNO);

    ~DetectorServer() = default;

  protected:
    /// @brief TCP/IP interface for communication with the client
    std::unique_ptr<TCPInterface> tcpInterface;

    std::unique_ptr<DetectorServerImpl> impl;

    auto *getDerivedImpl() {
        return static_cast<typename DerivedDetectorServer::ImplType *>(
            impl.get());
    }

    const auto *getDerivedImpl() const {
        return static_cast<const typename DerivedDetectorServer::ImplType *>(
            impl.get());
    }

  private:
    /// @brief get derived class
    DerivedDetectorServer *getDerived() {
        return static_cast<DerivedDetectorServer *>(this);
    }

    const DerivedDetectorServer *getDerived() const {
        return static_cast<const DerivedDetectorServer *>(this);
    }

    ProcessedResult processFunction(const detFuncs function_id,
                                    ServerInterface &socket);

    // TODO dont know what this does?
    ProcessedResult get_update_mode(ServerInterface &socket) const;

    ProcessedResult get_source_udp_mac(ServerInterface &socket) const;

    ProcessedResult set_source_udp_mac(ServerInterface &socket);

    ProcessedResult get_source_udp_ip(ServerInterface &socket) const;

    ProcessedResult set_source_udp_ip(ServerInterface &socket);

    ProcessedResult get_source_udp_port(ServerInterface &socket) const;

    ProcessedResult set_destination_udp_mac(ServerInterface &socket);

    ProcessedResult get_destination_udp_mac(ServerInterface &socket) const;

    ProcessedResult set_destination_udp_ip(ServerInterface &socket);

    ProcessedResult get_destination_udp_ip(ServerInterface &socket) const;

    ProcessedResult set_destination_udp_port(ServerInterface &socket);

    ProcessedResult get_destination_udp_port(ServerInterface &socket) const;

    ProcessedResult get_num_frames(ServerInterface &socket) const;

    ProcessedResult set_num_frames(ServerInterface &socket);

    ProcessedResult get_num_triggers(ServerInterface &socket) const;

    ProcessedResult set_num_triggers(ServerInterface &socket);

    ProcessedResult get_version(ServerInterface &socket) const;

    ProcessedResult get_num_udp_interfaces(ServerInterface &socket) const;

    ProcessedResult get_detector_type(ServerInterface &socket) const;

    ProcessedResult get_receiver_parameters(ServerInterface &socket) const;

    ProcessedResult get_run_status(ServerInterface &socket) const;

    ProcessedResult initial_checks(ServerInterface &socket) const;

    ProcessedResult
    set_module_position_and_update_srcudpmac(ServerInterface &socket);
};

template <typename DerivedDetectorServer>
DetectorServer<DerivedDetectorServer>::DetectorServer(
    std::unique_ptr<DetectorServerImpl> impl_, uint16_t port)
    : impl(std::move(impl_)) {
    validatePortNumber(port);

    std::function<ProcessedResult(const detFuncs &, ServerInterface &)> fn =
        [this](const detFuncs &function_id, ServerInterface &socket) {
            return this->processFunction(function_id, socket);
        };

    tcpInterface = std::make_unique<TCPInterface>(fn, port);
}

template <typename DerivedDetectorServer>
ProcessedResult DetectorServer<DerivedDetectorServer>::processFunction(
    const detFuncs function_id, ServerInterface &socket) {

    switch (function_id) {
    case detFuncs::F_GET_SERVER_VERSION:
        return get_version(socket);
    case detFuncs::F_GET_DETECTOR_TYPE:
        return get_detector_type(socket);
    case detFuncs::F_INITIAL_CHECKS:
        return initial_checks(socket);
    case detFuncs::F_GET_NUM_INTERFACES:
        return get_num_udp_interfaces(socket);
    case detFuncs::F_GET_UPDATE_MODE:
        return get_update_mode(socket);
    case detFuncs::F_SET_SOURCE_UDP_MAC:
        return set_source_udp_mac(socket);
    case detFuncs::F_GET_SOURCE_UDP_MAC:
        return get_source_udp_mac(socket);
    case detFuncs::F_SET_SOURCE_UDP_IP:
        return set_source_udp_ip(socket);
    case detFuncs::F_GET_SOURCE_UDP_IP:
        return get_source_udp_ip(socket);
    case detFuncs::F_SET_DEST_UDP_MAC:
        return set_destination_udp_mac(socket);
    case detFuncs::F_GET_DEST_UDP_MAC:
        return get_destination_udp_mac(socket);
    case detFuncs::F_SET_DEST_UDP_IP:
        return set_destination_udp_ip(socket);
    case detFuncs::F_GET_DEST_UDP_IP:
        return get_destination_udp_ip(socket);
    case detFuncs::F_SET_DEST_UDP_PORT:
        return set_destination_udp_port(socket);
    case detFuncs::F_GET_DEST_UDP_PORT:
        return get_destination_udp_port(socket);
    case detFuncs::F_GET_RUN_STATUS:
        return get_run_status(socket);
    case detFuncs::F_GET_NUM_FRAMES:
        return get_num_frames(socket);
    case detFuncs::F_SET_NUM_FRAMES:
        return set_num_frames(socket);
    case detFuncs::F_GET_NUM_TRIGGERS:
        return get_num_triggers(socket);
    case detFuncs::F_SET_NUM_TRIGGERS:
        return set_num_triggers(socket);
    case detFuncs::F_GET_RECEIVER_PARAMETERS:
        return get_receiver_parameters(socket);
    case detFuncs::F_SET_POSITION:
        return set_module_position_and_update_srcudpmac(socket);
    default:
        LOG(logDEBUG) << "Checking specific server functions for function ID: "
                      << function_id;
        // process detector specific functions
        return getDerived()->processFunction(function_id, socket);
    }

    return return_fail("Function not implemented");
}

template <typename DerivedDetectorServer>
ProcessedResult DetectorServer<DerivedDetectorServer>::get_update_mode(
    ServerInterface &socket) const {

    const bool updateMode = impl->get_update_mode();

    // TODO: catch the socket error during Send and add error message to the
    // ProcessedResult but DatSocket shared with receiver - some refactoring
    return ProcessedResult{static_cast<ReturnCode>(
        socket.sendResult(static_cast<int>(updateMode)))};
}

template <typename DerivedDetectorServer>
ProcessedResult DetectorServer<DerivedDetectorServer>::get_source_udp_mac(
    ServerInterface &socket) const {
    auto srcUdpMac = impl->get_source_udp_mac();

    return ProcessedResult{
        static_cast<ReturnCode>(socket.sendResult(srcUdpMac))};
}

template <typename DerivedDetectorServer>
ProcessedResult DetectorServer<DerivedDetectorServer>::set_source_udp_mac(
    ServerInterface &socket) {

    uint64_t newsrcudpMac;

    try {
        (void)socket.Receive<uint64_t>(newsrcudpMac);
    } catch (const SocketError &e) {
        LOG(logERROR) << "Failed to receive new source UDP MAC address: "
                      << e.what();
        return return_fail("Failed to receive new source UDP MAC address: " +
                           std::string(e.what()));
    }

    try {
        getDerivedImpl()->set_source_udp_mac(newsrcudpMac);
    } catch (const std::exception &e) {
        LOG(logERROR) << "Failed to set source UDP MAC address: " << e.what();
        return_fail("Failed to set source UDP MAC address: " +
                    std::string(e.what()));
    }

    return send_ok(socket);
}

template <typename DerivedDetectorServer>
ProcessedResult DetectorServer<DerivedDetectorServer>::set_source_udp_ip(
    ServerInterface &socket) {

    uint32_t newSrcIp;

    try {
        (void)socket.Receive(newSrcIp);
    } catch (const SocketError &e) {
        auto error_message = "Failed to receive new source UDP IP address: " +
                             std::string(e.what());
        LOG(logERROR) << error_message;

        return return_fail(error_message);
    }

    impl->set_source_udp_ip(newSrcIp);
    return send_ok(socket);
}

template <typename DerivedDetectorServer>
ProcessedResult DetectorServer<DerivedDetectorServer>::get_source_udp_ip(
    ServerInterface &socket) const {
    uint32_t src_UdpIp = impl->get_source_udp_ip();
    return ProcessedResult{
        static_cast<ReturnCode>(socket.sendResult(src_UdpIp))};
}

template <typename DerivedDetectorServer>
ProcessedResult DetectorServer<DerivedDetectorServer>::set_destination_udp_mac(
    ServerInterface &socket) {
    uint64_t newDstMac;

    try {
        (void)socket.Receive<uint64_t>(newDstMac);
    } catch (const SocketError &e) {
        auto error_message =
            "Failed to receive new destination UDP MAC address: " +
            std::string(e.what());
        LOG(logERROR) << error_message;
        return return_fail(error_message);
    }

    impl->set_destination_udp_mac(newDstMac);

    return send_ok(socket);
}

template <typename DerivedDetectorServer>
ProcessedResult DetectorServer<DerivedDetectorServer>::get_destination_udp_mac(
    ServerInterface &socket) const {

    auto dstUdpMac = impl->get_destination_udp_mac();
    return ProcessedResult{
        static_cast<ReturnCode>(socket.sendResult(dstUdpMac))};
}

template <typename DerivedDetectorServer>
ProcessedResult DetectorServer<DerivedDetectorServer>::set_destination_udp_ip(
    ServerInterface &socket) {
    uint32_t newDstIp;

    try {
        (void)socket.Receive(newDstIp);
    } catch (const SocketError &e) {
        auto error_message =
            "Failed to receive new destination UDP IP address: " +
            std::string(e.what());
        LOG(logERROR) << error_message;
        return return_fail(error_message);
    }

    impl->set_destination_udp_ip(newDstIp);

    return send_ok(socket);
}

template <typename DerivedDetectorServer>
ProcessedResult DetectorServer<DerivedDetectorServer>::get_destination_udp_ip(
    ServerInterface &socket) const {

    uint32_t dstUdpIp = impl->get_destination_udp_ip();
    return ProcessedResult{
        static_cast<ReturnCode>(socket.sendResult(dstUdpIp))};
}

template <typename DerivedDetectorServer>
ProcessedResult DetectorServer<DerivedDetectorServer>::set_destination_udp_port(
    ServerInterface &socket) {
    uint16_t newDstPort;

    try {
        (void)socket.Receive(newDstPort);
    } catch (const SocketError &e) {
        auto error_message =
            "Failed to receive new destination UDP port number: " +
            std::string(e.what());
        LOG(logERROR) << error_message;
        return return_fail(error_message);
    }

    impl->set_destination_udp_port(newDstPort);

    return send_ok(socket);
}

template <typename DerivedDetectorServer>
ProcessedResult DetectorServer<DerivedDetectorServer>::get_destination_udp_port(
    ServerInterface &socket) const {
    uint16_t dstUdpPort = impl->get_destination_udp_port();
    return ProcessedResult{
        static_cast<ReturnCode>(socket.sendResult(dstUdpPort))};
};

template <typename DerivedDetectorServer>
ProcessedResult DetectorServer<DerivedDetectorServer>::get_num_frames(
    ServerInterface &socket) const {
    uint64_t num_frames{};
    try {
        num_frames = getDerivedImpl()->get_num_frames();
    } catch (const std::exception &e) {
        auto error_message =
            "Failed to get number of frames: " + std::string(e.what());
        LOG(logERROR) << error_message;
        return return_fail(error_message);
    }
    return ProcessedResult{
        static_cast<ReturnCode>(socket.sendResult(num_frames))};
}

template <typename DerivedDetectorServer>
ProcessedResult
DetectorServer<DerivedDetectorServer>::set_num_frames(ServerInterface &socket) {
    int64_t num_frames{};
    try {
        (void)socket.Receive(num_frames);
    } catch (const SocketError &e) {
        auto error_message =
            "Failed to receive number of frames: " + std::string(e.what());
        LOG(logERROR) << error_message;
        return return_fail(error_message);
    }
    try {
        getDerivedImpl()->set_num_frames(num_frames);
    } catch (const std::exception &e) {
        auto error_message =
            "Failed to set number of frames: " + std::string(e.what());
        LOG(logERROR) << error_message;
        return return_fail(error_message);
    }
    return send_ok(socket);
}

template <typename DerivedDetectorServer>
ProcessedResult DetectorServer<DerivedDetectorServer>::get_num_triggers(
    ServerInterface &socket) const {
    uint64_t num_triggers{};
    try {
        num_triggers =
            static_cast<uint64_t>(getDerivedImpl()->get_num_triggers());
    } catch (const std::exception &e) {
        auto error_message =
            "Failed to get number of triggers: " + std::string(e.what());
        LOG(logERROR) << error_message;
        return return_fail(error_message);
    }
    return ProcessedResult{
        static_cast<ReturnCode>(socket.sendResult(num_triggers))};
}

template <typename DerivedDetectorServer>
ProcessedResult DetectorServer<DerivedDetectorServer>::set_num_triggers(
    ServerInterface &socket) {
    uint32_t num_triggers{};
    try {
        (void)socket.Receive(num_triggers);
    } catch (const SocketError &e) {
        auto error_message =
            "Failed to receive number of triggers: " + std::string(e.what());
        LOG(logERROR) << error_message;
        return return_fail(error_message);
    }
    try {
        getDerivedImpl()->set_num_triggers(num_triggers);
    } catch (const std::exception &e) {
        auto error_message =
            "Failed to set number of triggers: " + std::string(e.what());
        LOG(logERROR) << error_message;
        return return_fail(error_message);
    }
    return send_ok(socket);
}

template <typename DerivedDetectorServer>
ProcessedResult DetectorServer<DerivedDetectorServer>::get_version(
    ServerInterface &socket) const {

    auto version =
        getDerivedImpl()
            ->get_server_version(); // TODO: get Impl from derived server

    char version_cstr[MAX_STR_LENGTH]{};
    std::snprintf(version_cstr, sizeof(version_cstr), "%s",
                  version.c_str()); // ensures temination
    LOG(TLogLevel::logDEBUG) << "Server Version: " << version;
    return ProcessedResult{static_cast<ReturnCode>(socket.sendResult(
        version_cstr))}; // TODO: check what would be possible return codes!!!
}

template <typename DerivedDetectorServer>
ProcessedResult DetectorServer<DerivedDetectorServer>::get_num_udp_interfaces(
    ServerInterface &socket) const {
    auto num_udp_interfaces = getDerivedImpl()->get_num_udp_interfaces();

    return ProcessedResult{static_cast<ReturnCode>(
        socket.sendResult(static_cast<int>(num_udp_interfaces)))};
}

template <typename DerivedDetectorServer>
ProcessedResult DetectorServer<DerivedDetectorServer>::get_detector_type(
    ServerInterface &socket) const {
    uint8_t detectortype = getDerivedImpl()->get_detector_type();
    return ProcessedResult{
        static_cast<ReturnCode>(socket.sendResult(detectortype))};
}

template <typename DerivedDetectorServer>
ProcessedResult DetectorServer<DerivedDetectorServer>::get_receiver_parameters(
    ServerInterface &socket) const {

    slsDetectorDefs::rxParameters rx_params =
        getDerivedImpl()->get_receiver_parameters();

    return ProcessedResult{
        static_cast<ReturnCode>(socket.sendResult(rx_params))};
}

template <typename DerivedDetectorServer>
ProcessedResult DetectorServer<DerivedDetectorServer>::get_run_status(
    ServerInterface &socket) const {

    slsDetectorDefs::runStatus status = getDerivedImpl()->get_run_status();

    return ProcessedResult{static_cast<ReturnCode>(socket.sendResult(status))};
}

template <typename DerivedDetectorServer>
ProcessedResult DetectorServer<DerivedDetectorServer>::initial_checks(
    ServerInterface &socket) const {
    bool initial_checks_passed = getDerivedImpl()->initial_checks();

    return ProcessedResult{
        static_cast<ReturnCode>(socket.sendResult(initial_checks_passed))};
}

template <typename DerivedDetectorServer>
ProcessedResult
DetectorServer<DerivedDetectorServer>::set_module_position_and_update_srcudpmac(
    ServerInterface &socket) {

    std::array<int, 2> position_info{}; // [num_modules_in_y, module_index]
    try {
        (void)socket.Receive(position_info.data(),
                             position_info.size() * sizeof(int));
    } catch (const SocketError &e) {
        LOG(logERROR)
            << "Failed to receive num modules in y dimension and module index: "
            << e.what();
        return_fail(
            "Failed to receive num modules in y dimension and module index: " +
            std::string(e.what()));
    }

    try {
        getDerivedImpl()->set_module_position_and_update_srcudpmac(
            position_info);
    } catch (const std::exception &e) {
        return_fail("Failed to set module position: " + std::string(e.what()));
    }

    return send_ok(socket);
}

} // namespace sls