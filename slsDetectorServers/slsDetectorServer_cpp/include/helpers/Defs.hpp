/** @file Defs.hpp
 * @brief this file contains some definitions used in the slsDetectorServer_cpp
 * project.
 */
#pragma once
#include <cstdint>

namespace sls {

/// @brief Individual/Group bit offset in a 48 bit MAC address - 0 indicates
/// unicast mac address
constexpr uint8_t INDIVIDUAL_GROUP_BIT_OFFSET = 40; // 1000 0000

/// @brief Universal/Local bit offset in a 48 bit MAC address - 1 indicates
/// locally administered mac address, 0 indicates universally administered mac
/// address
constexpr uint8_t UNIVERSAL_LOCAL_BIT_OFFSET = 41; // 0100 0000

} // namespace sls