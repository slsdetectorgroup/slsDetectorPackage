#pragma once
#include "Defs.hpp"
#include "DetectorServerImpl.hpp"
#include "sls/SharedMemory.h"
#include "sls/sls_detector_defs.h"
#include <cstdint>
#include <cstdlib>

namespace sls {

constexpr uint64_t mac_mask = 0xffffffffffff0000;
constexpr uint8_t offset_row_position_in_mac = 8; // given in bits
constexpr uint8_t offset_col_position_in_mac = 0; // given in bits

/// @brief generates a random locally administered unicast MAC address for the
/// source UDP
/// @return generated MAC address
inline uint64_t generateRandomMacAddress() {
    uint64_t mac =
        0xAA0000000000; // locally administered unicast address (0xA: 0b1010) //
                        // TODO maybe 0x02000000000 better?
    for (int i = 2; i < 5; ++i) {
        mac |= (static_cast<uint64_t>(std::rand() % 256) << (i * 8));
    }
    return mac;
}

/// @brief generates a MAC address based on the module's row and column
/// position, last 32 bits of the MAC address are set to module_row and
/// module_col
/// @param module_row
/// @param module_col
/// @return generated MAC address
inline uint64_t generateMacAddressfromModulePosition(const uint8_t module_row,
                                                     const uint8_t module_col) {

    uint64_t newSrcMac = generateRandomMacAddress();
    newSrcMac = (newSrcMac & mac_mask) |
                (module_row << offset_row_position_in_mac) |
                (module_col << offset_col_position_in_mac);

    return newSrcMac;
}

/// @brief check that mac is unicast and locally administered
/// @param mac
/// @return true if mac is valid, false otherwise
inline bool isValidMac(const uint64_t mac) {
    if ((mac << INDIVIDUAL_GROUP_BIT_OFFSET) == 0 &&
        (mac << UNIVERSAL_LOCAL_BIT_OFFSET) == 1) {
        return true;
    }
    return false;
}

inline void freeSharedMemory() {
    SharedMemory<acquisitionStatus> shm(0, -1, "server");

    if (shm.exists()) {
        shm.removeSharedMemory();
    }
}

} // namespace sls
