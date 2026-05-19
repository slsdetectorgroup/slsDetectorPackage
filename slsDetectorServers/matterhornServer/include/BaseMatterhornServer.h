#pragma once
#include "ArmBusCommunication.hpp"
#include "DetectorServer.h"
#include "MatterhornDefs.hpp"
#include "MemoryModel.hpp"
#include "RegisterDefs.hpp"
#include "SPICommunication.h"
#include "TCPInterface.h"
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

class VirtualMatterhornServer; // forward declaration
class MatterhornServer;        // forward declaration

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

    ReturnCode get_receiver_parameters(ServerInterface &socket) const;

    ReturnCode set_module_position(ServerInterface &socket);

    ReturnCode set_counter_mask(ServerInterface &socket);

    ReturnCode get_counter_mask(ServerInterface &socket) const;

    uint64_t getNumFrames() const;

    void setNumFrames(const uint64_t num_frames);

    uint32_t getNumTriggers() const;

    void setNumTriggers(const uint32_t num_triggers);

    /**
     * @brief call function corresponding to the function ID received from the
     * client and send back the result
     * @param function_id the function ID received from the client
     * @param socket the socket to send the result back to the client
     */
    ReturnCode processFunction(const detFuncs function_id,
                               ServerInterface &socket);

  protected:
    using MemoryModel = std::conditional_t<
        std::is_same_v<DerivedServer, VirtualMatterhornServer>,
        VirtualMemoryModel<uint32_t>, HardwareMemoryModel>; // 32 bit registers

    // TODO: for now in MatterhornServer and not generic Server but can be
    // templated on different IPCore types for each detector
    BusCommunication<MatterhornDefs::MatterHornIPCores, MemoryModel>
        busCommunication{};

    using SPICommunicationClass = std::conditional_t<
        std::is_same_v<DerivedServer, VirtualMatterhornServer>,
        SPICommunication<
            VirtualSPICommunication<MatterhornDefs::MatterhornSPIRegisters>>,
        SPICommunication<HardwareSPICommunication>>;

    SPICommunicationClass spiCommunication{};

    // TODO: probably virtaul server specific details, can be moved to derived
    // class
    /// @brief initial setup of detector
    void setupDetector();

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
    case detFuncs::F_SET_COUNTER_MASK:
        return set_counter_mask(socket);
    case detFuncs::F_GET_COUNTER_MASK:
        return get_counter_mask(socket);
    default:
        throw RuntimeError(
            fmt::format("Function {} not implemented",
                        getFunctionNameFromEnum((enum detFuncs)function_id)));
    }
}

template <typename DerivedServer>
void BaseMatterhornServer<DerivedServer>::setupDetector() {
    // TODO: extend
    setNumFrames(1); // maybe have a file with constexpr default values

    setNumTriggers(1);
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

template <typename DerivedServer>
ReturnCode BaseMatterhornServer<DerivedServer>::get_receiver_parameters(
    ServerInterface &socket) const {

    slsDetectorDefs::rxParameters rx_params{};

    rx_params.udpInterfaces = numUDPInterfaces;

    rx_params.udp_dstip = this->udpDetails[0].dstip;

    rx_params.udp_dstport = this->udpDetails[0].dstport;

    rx_params.udp_dstmac = this->udpDetails[0].dstmac;

    rx_params.frames = getNumFrames();

    rx_params.triggers = getNumTriggers();

    // TODO: extend

    // rx_params.expTimeNs = 0;

    // rx_params.periodNs = 0;

    // rx_params.dynamicRange = 0;

    // rx_params.timMode = AUTO_TIMING;

    // rx_params.counterMask = 0;

    return static_cast<ReturnCode>(socket.sendResult(rx_params));
}

template <typename DerivedServer>
uint64_t BaseMatterhornServer<DerivedServer>::getNumFrames() const {

    try {
        uint32_t num_frames =
            busCommunication.readRegister(Reg::MH_SM_Frames_Reg);
        return static_cast<uint64_t>(num_frames);
    } catch (const std::exception &e) {
        LOG(logERROR) << "Failed to read number of frames from register: "
                      << e.what();
        throw;
    }
}

template <typename DerivedServer>
void BaseMatterhornServer<DerivedServer>::setNumFrames(
    const uint64_t num_frames) {

    try {
        busCommunication.writeRegister(Reg::MH_SM_Frames_Reg,
                                       static_cast<uint32_t>(num_frames));
    } catch (const std::exception &e) {
        LOG(logERROR) << "Failed to set number of frames: " << e.what();
        throw;
    }
    // TODO: maybe always check that the value is correctly set by reading back
    // the register and comparing
}

template <typename DerivedServer>
void BaseMatterhornServer<DerivedServer>::setNumTriggers(
    const uint32_t num_triggers) {

    try {
        busCommunication.writeRegister(Reg::MH_SM_Triggers_Reg, num_triggers);
    } catch (const std::exception &e) {
        LOG(logERROR) << "Failed to set number of triggers: " << e.what();
        throw;
    }
}

template <typename DerivedServer>
uint32_t BaseMatterhornServer<DerivedServer>::getNumTriggers() const {

    try {
        uint32_t num_triggers =
            busCommunication.readRegister(Reg::MH_SM_Triggers_Reg);
        return num_triggers;
    } catch (const std::exception &e) {
        LOG(logERROR) << "Failed to read number of triggers from register: "
                      << e.what();
        throw;
    }
}

template <typename DerivedServer>
ReturnCode BaseMatterhornServer<DerivedServer>::set_module_position(
    ServerInterface &socket) {

    std::array<int, 2> position_info{}; // [num_modules_in_y, module_index]
    try {
        int ret = socket.Receive(position_info.data(),
                                 position_info.size() * sizeof(int));
    } catch (const SocketError &e) {
        LOG(logERROR)
            << "Failed to receive num modules in y dimension and module index: "
            << e.what();
        return ReturnCode::FAIL;
    }

    const size_t module_row = position_info[1] % position_info[0];
    const size_t module_col = position_info[1] / position_info[0];

    // write to register
    uint32_t register_value_LSB{};
    uint32_t register_value_MSB{};

    try {
        register_value_LSB =
            busCommunication.readRegister(Reg::Frame_HDR_ModCoord_LSB_Reg);
        register_value_MSB =
            busCommunication.readRegister(Reg::Frame_HDR_ModCoord_MSB_Reg);
    } catch (const std::exception &e) {
        LOG(logERROR) << "Failed to read module position register: "
                      << e.what();
        return ReturnCode::FAIL;
    }

    setRegisterField(register_value_LSB, Reg::ModuleRow, module_row);
    setRegisterField(register_value_LSB, Reg::ModuleCol, module_col);
    setRegisterField(register_value_MSB, Reg::ModuleCoordz, 0);
    setRegisterField(register_value_MSB, Reg::ModuleIndex, position_info[1]);

    try {
        busCommunication.writeRegister(Reg::Frame_HDR_ModCoord_LSB_Reg,
                                       register_value_LSB);
        busCommunication.writeRegister(Reg::Frame_HDR_ModCoord_MSB_Reg,
                                       register_value_MSB);
    } catch (const std::exception &e) {
        LOG(logERROR) << "Failed to write module position register: "
                      << e.what();
        return ReturnCode::FAIL;
    }

    // configure mac address based on module position
    if (this->udpDetails[0].srcmac ==
        0) { // only configure if source mac address is not set already
        this->udpDetails[0].srcmac = generaterandomMacAddress();
        uint64_t newSrcMac = (this->udpDetails[0].srcmac & 0xffffffffffff0000) |
                             (module_row << 16) | module_col;
        try {
            this->updateSrcMacAddress(newSrcMac);
        } catch (const std::invalid_argument &e) {
            LOG(logERROR) << "Failed to update source MAC address: "
                          << e.what();
            return ReturnCode::FAIL;
        }
    }

    return static_cast<ReturnCode>(socket.Send(ReturnCode::OK));
}

template <typename DerivedServer>
ReturnCode
BaseMatterhornServer<DerivedServer>::set_counter_mask(ServerInterface &socket) {

    // TODO: update properly

    uint32_t counter_mask{};
    try {
        int ret = socket.Receive(counter_mask);
    } catch (const SocketError &e) {
        LOG(logERROR) << "Failed to receive counter mask: " << e.what();
        return ReturnCode::FAIL;
    }

    try {
        auto reg_value = spiCommunication.SPIread(
            SPIRegisters::NUM_COUNTERS.register_,
            0); // TODO: how to handle different chip ids -> e.g. broadcast do
                // we want it to be configurable for different chip ids? -
                // Command overload for some of the SPI registers

        setSPIRegisterField(reg_value, SPIRegisters::NUM_COUNTERS,
                            counter_mask);

        spiCommunication.SPIwrite(SPIRegisters::NUM_COUNTERS.register_, 0,
                                  reg_value);
    } catch (const std::exception &e) {
        LOG(logERROR) << "Failed to set counter mask: " << e.what();
        return ReturnCode::FAIL;
    }

    return static_cast<ReturnCode>(socket.Send(ReturnCode::OK));
}

template <typename DerivedServer>
ReturnCode BaseMatterhornServer<DerivedServer>::get_counter_mask(
    ServerInterface &socket) const {

    // TODO: update properly

    std::vector<std::byte> reg_value{};
    try {
        reg_value = spiCommunication.SPIread(
            SPIRegisters::NUM_COUNTERS.register_,
            0); // TODO: how to handle different chip ids -
    } catch (const std::exception &e) {
        LOG(logERROR) << "Failed to read counter mask from SPI register: "
                      << e.what();
        return ReturnCode::FAIL;
    }

    uint32_t counter_mask =
        getSPIRegisterField(reg_value, SPIRegisters::NUM_COUNTERS);

    return static_cast<ReturnCode>(socket.sendResult(counter_mask));
}

} // namespace sls