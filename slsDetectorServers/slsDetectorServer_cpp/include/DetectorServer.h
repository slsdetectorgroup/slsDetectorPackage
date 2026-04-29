#pragma once
#include "SharedMemory.h"
#include "TCPInterface.h"
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

// TODO move to defs?
/// @brief struct saving udp details (one UDP port per module)
struct UDPInfo {
    uint16_t srcport{};
    uint16_t dstport{};
    uint64_t srcmac{};
    uint64_t dstmac{};
    uint32_t srcip{};
    uint32_t dstip{};
};

using ReturnCode = slsDetectorDefs::ReturnCode;
/// @brief Shared memory structure for stop server to store run status
struct acquisitionStatus {

    /* FIXED PATTERN FOR STATIC FUNCTIONS. DO NOT CHANGE, ONLY APPEND ------*/
    int shmversion;

    bool isValid{true}; // false if freed to block access from python or c++ api

    std::atomic<slsDetectorDefs::runStatus> scanStatus{
        slsDetectorDefs::runStatus::IDLE}; // idle, running or error
    std::atomic<bool> scanStop{false};

    // TODO: only neccessary for virtual, maybe have two shared memory
    // structures, one for virtual
    std::atomic<slsDetectorDefs::runStatus> status{
        slsDetectorDefs::runStatus::IDLE};
    std::atomic<bool> stop{false};
};

template <typename DerivedDetectorServer> class DetectorServer {

  public:
    /**
     * Constructor
     * Creates a detector server.
     * Assembles a detector server using TCP and UDP detector interfaces
     * throws an exception in case of failure
     * @param port TCP/IP port number
     */
    explicit DetectorServer(uint16_t port = DEFAULT_TCP_CNTRL_PORTNO);

  protected:
    /// @brief TCP/IP interface for communication with the client
    std::unique_ptr<TCPInterface> tcpInterface;

    std::array<UDPInfo, 1>
        udpDetails{}; // TODO: for now only one receiver per module

    /// @brief  TODO what is this?
    bool updateMode{true};

    /// @brief
    mutable SharedMemory<acquisitionStatus> shm{
        0, 0}; // TODO: is mutable really neccessary?

  private:
    /// @brief creates and maps shared memory
    void createSharedMemory();

    ReturnCode processFunction(const detFuncs function_id,
                               ServerInterface &socket);

    // TODO dont know what this does?
    ReturnCode get_update_mode(ServerInterface &socket) const;

    ReturnCode get_source_udp_mac(ServerInterface &socket) const;

    ReturnCode set_source_udp_mac(ServerInterface &socket);

    ReturnCode get_source_udp_ip(ServerInterface &socket) const;

    ReturnCode set_source_udp_ip(ServerInterface &socket);

    ReturnCode get_source_udp_port(ServerInterface &socket) const;

    ReturnCode set_destination_udp_mac(ServerInterface &socket);

    ReturnCode get_destination_udp_mac(ServerInterface &socket) const;

    ReturnCode set_destination_udp_ip(ServerInterface &socket);

    ReturnCode get_destination_udp_ip(ServerInterface &socket) const;

    ReturnCode set_destination_udp_port(ServerInterface &socket);

    ReturnCode get_destination_udp_port(ServerInterface &socket) const;
};

template <typename DerivedDetectorServer>
DetectorServer<DerivedDetectorServer>::DetectorServer(uint16_t port) {
    validatePortNumber(port);

    udpDetails[0].srcport = DEFAULT_UDP_SRC_PORTNO;
    udpDetails[0].dstport = DEFAULT_UDP_DST_PORTNO;

    createSharedMemory();

    std::function<ReturnCode(const detFuncs &, ServerInterface &)> fn =
        [this](const detFuncs &function_id, ServerInterface &socket) {
            return this->processFunction(function_id, socket);
        };

    tcpInterface = std::make_unique<TCPInterface>(fn, port);
}

template <typename DerivedDetectorServer>
ReturnCode DetectorServer<DerivedDetectorServer>::processFunction(
    const detFuncs function_id, ServerInterface &socket) {

    switch (function_id) {
    case detFuncs::F_GET_SERVER_VERSION:
        return static_cast<DerivedDetectorServer *>(this)->get_version(socket);
    case detFuncs::F_GET_DETECTOR_TYPE:
        return static_cast<DerivedDetectorServer *>(this)->get_detector_type(
            socket);
    case detFuncs::F_INITIAL_CHECKS:
        return static_cast<DerivedDetectorServer *>(this)->initial_checks(
            socket);
    case detFuncs::F_GET_NUM_INTERFACES:
        return static_cast<DerivedDetectorServer *>(this)
            ->get_num_udp_interfaces(socket);
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
        return static_cast<DerivedDetectorServer *>(this)->get_run_status(
            socket);

    default:
        LOG(logDEBUG) << "Checking specific server functions for function ID: "
                      << function_id;
        // process detector specific functions
        static_cast<DerivedDetectorServer *>(this)->processFunction(function_id,
                                                                    socket);
    }

    return ReturnCode::FAIL;
}

template <typename DerivedDetectorServer>
void DetectorServer<DerivedDetectorServer>::createSharedMemory() {

    shm = SharedMemory<acquisitionStatus>(0, -1, "server");

    if (shm.exists()) {
        shm.openSharedMemory(true); // stop server TODO: should I verify size
    } else {
        LOG(logINFOBLUE) << "Creating shared memory for acquisition status";
        shm.createSharedMemory();
    }
}

template <typename DerivedDetectorServer>
ReturnCode DetectorServer<DerivedDetectorServer>::get_update_mode(
    ServerInterface &socket) const {

    return static_cast<ReturnCode>(
        socket.sendResult(static_cast<int>(updateMode)));
}

template <typename DerivedDetectorServer>
ReturnCode DetectorServer<DerivedDetectorServer>::set_source_udp_mac(
    ServerInterface &socket) {
    uint64_t newsrcudpMac;

    try {
        int ret = socket.Receive<uint64_t>(newsrcudpMac);
    } catch (const SocketError &e) {
        LOG(logERROR) << "Failed to receive new source UDP MAC address: "
                      << e.what();
        return ReturnCode::FAIL;
    }

    udpDetails[0].srcmac = newsrcudpMac;
    // TODO: configuremac, check unicast address
    return static_cast<ReturnCode>(socket.Send(ReturnCode::OK));
}

template <typename DerivedDetectorServer>
ReturnCode DetectorServer<DerivedDetectorServer>::get_source_udp_mac(
    ServerInterface &socket) const {
    return static_cast<ReturnCode>(socket.sendResult(udpDetails[0].srcmac));
}

template <typename DerivedDetectorServer>
ReturnCode DetectorServer<DerivedDetectorServer>::set_source_udp_ip(
    ServerInterface &socket) {

    uint32_t newSrcIp;

    try {
        int ret = socket.Receive(newSrcIp);
    } catch (const SocketError &e) {
        LOG(logERROR) << "Failed to receive new source UDP IP address: "
                      << e.what();
        return ReturnCode::FAIL;
    }

    udpDetails[0].srcip = newSrcIp;
    return static_cast<ReturnCode>(socket.Send(ReturnCode::OK));
}

template <typename DerivedDetectorServer>
ReturnCode DetectorServer<DerivedDetectorServer>::get_source_udp_ip(
    ServerInterface &socket) const {
    return static_cast<ReturnCode>(socket.sendResult(udpDetails[0].srcip));
}

template <typename DerivedDetectorServer>
ReturnCode DetectorServer<DerivedDetectorServer>::set_destination_udp_mac(
    ServerInterface &socket) {
    uint64_t newDstMac;

    try {
        int ret = socket.Receive<uint64_t>(newDstMac);
    } catch (const SocketError &e) {
        LOG(logERROR) << "Failed to receive new destination UDP MAC address: "
                      << e.what();
        return ReturnCode::FAIL;
    }

    udpDetails[0].dstmac = newDstMac;
    // TODO: configuremac, check unicast address
    return static_cast<ReturnCode>(socket.Send(ReturnCode::OK));
}

template <typename DerivedDetectorServer>
ReturnCode DetectorServer<DerivedDetectorServer>::get_destination_udp_mac(
    ServerInterface &socket) const {
    return static_cast<ReturnCode>(socket.sendResult(udpDetails[0].dstmac));
}

template <typename DerivedDetectorServer>
ReturnCode DetectorServer<DerivedDetectorServer>::set_destination_udp_ip(
    ServerInterface &socket) {
    uint32_t newDstIp;

    try {
        int ret = socket.Receive(newDstIp);
    } catch (const SocketError &e) {
        LOG(logERROR) << "Failed to receive new destination UDP IP address: "
                      << e.what();
        return ReturnCode::FAIL;
    }

    udpDetails[0].dstip = newDstIp;
    return static_cast<ReturnCode>(socket.Send(ReturnCode::OK));
}

template <typename DerivedDetectorServer>
ReturnCode DetectorServer<DerivedDetectorServer>::get_destination_udp_ip(
    ServerInterface &socket) const {
    return static_cast<ReturnCode>(socket.sendResult(udpDetails[0].dstip));
}

template <typename DerivedDetectorServer>
ReturnCode DetectorServer<DerivedDetectorServer>::set_destination_udp_port(
    ServerInterface &socket) {
    uint16_t newDstPort;

    try {
        int ret = socket.Receive(newDstPort);
    } catch (const SocketError &e) {
        LOG(logERROR) << "Failed to receive new destination UDP port number: "
                      << e.what();
        return ReturnCode::FAIL;
    }

    udpDetails[0].dstport = newDstPort;
    return static_cast<ReturnCode>(socket.Send(ReturnCode::OK));
}

template <typename DerivedDetectorServer>
ReturnCode DetectorServer<DerivedDetectorServer>::get_destination_udp_port(
    ServerInterface &socket) const {
    return static_cast<ReturnCode>(socket.sendResult(udpDetails[0].dstport));
};

} // namespace sls