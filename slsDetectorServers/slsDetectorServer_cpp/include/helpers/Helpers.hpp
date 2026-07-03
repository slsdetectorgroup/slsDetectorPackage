#pragma once
#include <cstdint>
#include <cstdlib>

constexpr uint64_t mac_mask = 0xffffffffffff0000;
constexpr uint8_t offset_row_position_in_mac = 16; // given in bits
constexpr uint8_t offset_col_position_in_mac = 0;  // given in bits

/// @brief generates a random locally administered unicast MAC address for the
/// source UDP
/// @return generated MAC address
inline uint64_t generaterandomMacAddress() {
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
inline uint64_t
generate_mac_address_from_module_position(const size_t module_row,
                                          const size_t module_col) {

    uint64_t newSrcMac = generaterandomMacAddress();
    newSrcMac = (newSrcMac & mac_mask) |
                (module_row << offset_row_position_in_mac) |
                (module_col << offset_col_position_in_mac);

    return newSrcMac;
}