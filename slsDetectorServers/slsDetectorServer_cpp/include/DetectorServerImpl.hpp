#pragma once
#include "sls/SharedMemory.h"
#include <array>
#include <atomic>

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

/// @brief struct to store detector setup status
struct detector_setup_status {
    /// @brief true if setupDetector() was successful, false otherwise
    bool successful_setup{false};
    /// @brief error message if setupDetector() failed, empty otherwise
    std::string error_message{};
};

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

class DetectorServerImpl {

  public:
    DetectorServerImpl();
    ~DetectorServerImpl();

    bool get_update_mode() const;

    uint64_t get_source_udp_mac() const;

    void set_source_udp_ip(const uint32_t srcip);

    void set_destination_udp_ip(const uint32_t dstip);

    uint32_t get_destination_udp_ip() const;

    uint32_t get_source_udp_ip() const;

    void set_destination_udp_mac(const uint64_t dstmac);

    uint64_t get_destination_udp_mac() const;

    void set_destination_udp_port(const uint16_t dstport);

    uint16_t get_destination_udp_port() const;

    detector_setup_status initial_checks() const;

  protected:
    std::array<UDPInfo, 1>
        udpDetails{}; // TODO: for now only one receiver per module

    /// @brief  TODO what is this?
    bool updateMode{
        false}; // what should the default be - can update module size etc.

    /// @brief shared mempory with aquisition status
    mutable SharedMemory<acquisitionStatus> shm{
        0, 0}; // TODO: is mutable really neccessary?

    /// @brief sets source UDP MAC address in udpDetails and updates udp
    /// header
    /// @param srcmac
    void updateSrcMacAddress(const uint64_t srcmac);

    /// @brief true if setupDetector() was successful, false otherwise
    detector_setup_status detectorSetupStatus{};

  private:
    /// @brief creates and maps shared memory
    void createSharedMemory();
};

} // namespace sls
