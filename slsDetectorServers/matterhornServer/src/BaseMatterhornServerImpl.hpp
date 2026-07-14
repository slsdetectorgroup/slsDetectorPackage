#pragma once
#include "ArmBusCommunication.hpp"
#include "DetectorServerImpl.hpp"
#include "MemoryModel.hpp"
#include "communication/SPICommunication.hpp"
#include "defs/MatterhornDefs.hpp"
#include "defs/RegisterDefs.hpp"
#include "sls/versionAPI.h"
#include "utils/HelperFunctions.hpp"
#include "utils/type_traits.hpp"
#include <cstdint>
#include <string>

namespace sls {

template <bool isStopServer>
class VirtualMatterhornServerImpl; // forward declare

template <typename DerivedMatterhornServerImpl>
class BaseMatterhornServerImpl : public DetectorServerImpl {
  public:
    BaseMatterhornServerImpl();
    ~BaseMatterhornServerImpl() = default;

    // TODO: probably virtaul server specific details, can be moved to derived
    // class
    /// @brief initial setup of detector
    void setupDetector();

    /// @brief get matterhorn server version
    std::string get_server_version() const;

    static uint8_t get_detector_type();

    static uint8_t get_num_udp_interfaces();

    uint64_t get_num_frames() const;
    void set_num_frames(const uint64_t num_frames);

    uint32_t get_num_triggers() const;
    void set_num_triggers(const uint32_t num_triggers);

    uint32_t get_counter_mask() const;
    void set_counter_mask(const uint32_t counter_mask);

    void set_module_position(const size_t module_row, const size_t module_col,
                             const size_t module_index);

    slsDetectorDefs::rxParameters get_receiver_parameters() const;

  protected:
    using MemoryModel = std::conditional_t<
        std::is_same_v<DerivedMatterhornServerImpl,
                       VirtualMatterhornServerImpl<
                           is_stop_server<DerivedMatterhornServerImpl>::value>>,
        VirtualMemoryModel<uint32_t>,
        HardwareMemoryModel>; // 32 bit registers

    // TODO: for now in MatterhornServer and not generic Server but can be
    // templated on different IPCore types for each detector
    BusCommunication<MatterhornDefs::MatterHornIPCores, MemoryModel>
        busCommunication{};

    using SPICommunicationClass = std::conditional_t<
        std::is_same_v<DerivedMatterhornServerImpl,
                       VirtualMatterhornServerImpl<
                           is_stop_server<DerivedMatterhornServerImpl>::value>>,
        VirtualSPICommunication<MatterhornDefs::MatterhornSPIRegisters>,
        HardwareSPICommunication>;

    SPICommunicationClass spiCommunication{};

  private:
    static constexpr uint8_t numUDPInterfaces =
        1; // only one udp per module for now

    /// @brief true if the derived server is a stop server, false otherwise
    static constexpr bool isStopServer =
        is_stop_server<DerivedMatterhornServerImpl>::value;
};

template <typename DerivedMatterhornServerImpl>
BaseMatterhornServerImpl<
    DerivedMatterhornServerImpl>::BaseMatterhornServerImpl() {

    // map the IP core base addresses to memory
    busCommunication.mapToMemory(); // TODO: should this happen in constructor?

    // TODO: need to check if chip is attached
    spiCommunication.open_spi(); // TODO: should this happen in constructor?
}

template <typename DerivedMatterhornServerImpl>
void BaseMatterhornServerImpl<DerivedMatterhornServerImpl>::setupDetector() {
    // TODO: extend
    try {
        // stop server does not talk to the board
        if constexpr (!isStopServer) {
            set_num_frames(1);
            set_num_triggers(1);
            set_counter_mask(0xF); // enable counter all counters by default
        }
    } catch (const std::exception &e) {
        LOG(logERROR) << "Failed to setup detector: " << e.what();
        detectorSetupStatus.error_message = std::string(e.what());
        detectorSetupStatus.setup_status =
            detector_setup_status::SETUP_STATUS::FAILED_SETUP;
    }

    detectorSetupStatus.setup_status =
        detector_setup_status::SETUP_STATUS::SUCCESSFUL_SETUP;
}

template <typename DerivedMatterhornServerImpl>
std::string
BaseMatterhornServerImpl<DerivedMatterhornServerImpl>::get_server_version()
    const {

    return APIMATTERHORN;
}

template <typename DerivedMatterhornServerImpl>
uint8_t BaseMatterhornServerImpl<
    DerivedMatterhornServerImpl>::get_num_udp_interfaces() {
    return numUDPInterfaces;
}

template <typename DerivedMatterhornServerImpl>
uint8_t
BaseMatterhornServerImpl<DerivedMatterhornServerImpl>::get_detector_type() {
    return slsDetectorDefs::detectorType::MATTERHORN;
}

template <typename DerivedMatterhornServerImpl>
uint64_t
BaseMatterhornServerImpl<DerivedMatterhornServerImpl>::get_num_frames() const {

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

template <typename DerivedMatterhornServerImpl>
void BaseMatterhornServerImpl<DerivedMatterhornServerImpl>::set_num_frames(
    const uint64_t num_frames) {

    try {
        busCommunication.writeRegister(Reg::MH_SM_Frames_Reg,
                                       static_cast<uint32_t>(num_frames));
        auto written_num_frames = busCommunication.readRegister(
            Reg::MH_SM_Frames_Reg); // check if write was successful

        if (num_frames != static_cast<uint64_t>(written_num_frames)) {
            throw std::runtime_error(
                fmt::format("Requested {} frames, but set {}", num_frames,
                            static_cast<uint64_t>(written_num_frames)));
        }
    } catch (const std::exception &e) {
        LOG(logERROR) << "Failed to set number of frames: " << e.what();
        throw;
    }
}

template <typename DerivedMatterhornServerImpl>
void BaseMatterhornServerImpl<DerivedMatterhornServerImpl>::set_num_triggers(
    const uint32_t num_triggers) {

    try {
        busCommunication.writeRegister(Reg::MH_SM_Triggers_Reg, num_triggers);
        auto written_num_triggers = busCommunication.readRegister(
            Reg::MH_SM_Triggers_Reg); // check if write was successful
        if (num_triggers != written_num_triggers) {
            throw std::runtime_error(
                fmt::format("Requested {} triggers, but set {}", num_triggers,
                            written_num_triggers));
        }
    } catch (const std::exception &e) {
        LOG(logERROR) << "Failed to set number of triggers: " << e.what();
        throw;
    }
}

template <typename DerivedMatterhornServerImpl>
uint32_t
BaseMatterhornServerImpl<DerivedMatterhornServerImpl>::get_num_triggers()
    const {

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

template <typename DerivedMatterhornServerImpl>
void BaseMatterhornServerImpl<DerivedMatterhornServerImpl>::set_counter_mask(
    const uint32_t counter_mask) {

    // counter mask update to num consecutive counters and starting_counter
    uint32_t spi_counter_mask{};
    try {
        spi_counter_mask = convertCounterMaskToSPICounterMask(counter_mask);
    } catch (const std::invalid_argument &e) {
        LOG(logERROR) << "Failed to convert counter mask to SPI counter mask: "
                      << e.what();

        throw std::invalid_argument(
            "Failed to convert counter mask to SPI counter mask: " +
            std::string(e.what()));
    }

    try {
        auto reg_value = spiCommunication.SPIread(
            SPIRegisters::NUM_COUNTERS.register_,
            0); // TODO: how to handle different chip ids -> e.g. broadcast do
                // we want it to be configurable for different chip ids? -
                // Command overload for some of the SPI registers

        setSPIRegisterField(reg_value, SPIRegisters::NUM_COUNTERS,
                            spi_counter_mask);

        spiCommunication.SPIwrite(SPIRegisters::NUM_COUNTERS.register_, 0,
                                  reg_value);
    } catch (const std::exception &e) {
        throw RuntimeError("Failed to set counter mask: " +
                           std::string(e.what()));
    }
}

template <typename DerivedMatterhornServerImpl>
uint32_t
BaseMatterhornServerImpl<DerivedMatterhornServerImpl>::get_counter_mask()
    const {

    std::vector<std::byte> reg_value{};
    try {
        reg_value = spiCommunication.SPIread(
            SPIRegisters::NUM_COUNTERS.register_,
            0); // TODO: how to handle different chip ids -
    } catch (const std::exception &e) {
        throw sls::RuntimeError(
            "Failed to read counter mask from SPI register: " +
            std::string(e.what()));
    }

    // stores num_counters and starting_counter 0b0000 -> counter 0 enabled,
    // 0b0001 -> counter 1 enabled, 0b0010
    uint32_t spi_counter_mask =
        getSPIRegisterField(reg_value, SPIRegisters::NUM_COUNTERS);

    uint32_t actual_counter_mask =
        convertSPICounterMaskToCounterMask(spi_counter_mask);

    return actual_counter_mask;
}

template <typename DerivedMatterhornServerImpl>
void BaseMatterhornServerImpl<DerivedMatterhornServerImpl>::set_module_position(
    const size_t module_row, const size_t module_col,
    const size_t module_index) {

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
        throw;
    }

    try {
        setRegisterField(register_value_LSB, Reg::ModuleRow, module_row);
        setRegisterField(register_value_LSB, Reg::ModuleCol, module_col);
        setRegisterField(register_value_MSB, Reg::ModuleCoordz, 0);
        setRegisterField(register_value_MSB, Reg::ModuleIndex, module_index);
    } catch (const std::exception &e) {
        LOG(logERROR) << "Failed to set module position register fields: "
                      << e.what();
        throw;
    }

    try {
        busCommunication.writeRegister(Reg::Frame_HDR_ModCoord_LSB_Reg,
                                       register_value_LSB);

        auto written_register_value_LSB = busCommunication.readRegister(
            Reg::Frame_HDR_ModCoord_LSB_Reg); // check if write was successful

        busCommunication.writeRegister(Reg::Frame_HDR_ModCoord_MSB_Reg,
                                       register_value_MSB);

        auto written_register_value_MSB = busCommunication.readRegister(
            Reg::Frame_HDR_ModCoord_MSB_Reg); // check if write was successful

        if (register_value_LSB != written_register_value_LSB ||
            register_value_MSB != written_register_value_MSB) {
            throw std::runtime_error(
                fmt::format("LSB: requested {}, but set {}. "
                            "MSB: requested {}, but set {}",
                            register_value_LSB, written_register_value_LSB,
                            register_value_MSB, written_register_value_MSB));
        }
    } catch (const std::exception &e) {
        LOG(logERROR) << "Failed to write module position register: "
                      << e.what();
        throw;
    }
}

template <typename DerivedMatterhornServerImpl>
slsDetectorDefs::rxParameters
BaseMatterhornServerImpl<DerivedMatterhornServerImpl>::get_receiver_parameters()
    const {

    slsDetectorDefs::rxParameters rx_params{};

    rx_params.udpInterfaces = numUDPInterfaces;

    rx_params.udp_dstip = this->udpDetails[0].dstip;

    rx_params.udp_dstport = this->udpDetails[0].dstport;

    rx_params.udp_dstmac = this->udpDetails[0].dstmac;

    rx_params.frames = get_num_frames();

    rx_params.triggers = get_num_triggers();

    // TODO: extend

    // rx_params.expTimeNs = 0;

    // rx_params.periodNs = 0;

    // rx_params.dynamicRange = 0;

    // rx_params.timMode = AUTO_TIMING;

    // rx_params.counterMask = 0;

    return rx_params;
}

} // namespace sls