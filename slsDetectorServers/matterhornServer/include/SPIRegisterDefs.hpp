#pragma once
#include "SPIRegisterHelperStructs.hpp"
#include <cstdint>

namespace sls {

namespace SPIRegisters {

// SPI registers
constexpr SPIRegister SPI_REG_ConfigUnit{0, 8};

constexpr SPIRegister SPI_REG_ConfigCML{1, 1};

constexpr SPIRegister SPI_REG_ManualSelector{2, 2};

constexpr SPIRegister SPI_REG_CoreRSTUnit{3, 4};

constexpr SPIRegister SPI_REG_StoreRSTUnit{4, 2};

constexpr SPIRegister SPI_REG_Trimbits{5, 256};

constexpr SPIRegister SPI_REG_McGyver{6, 512};

constexpr SPIRegister SPI_REG_McGyver_par_load{7, 512};

constexpr SPIRegister SPI_REG_ActionReg{11, 1};

constexpr SPIRegister SPI_REG_InternalDACs{13, 4};

constexpr SPIRegister SPI_REG_ChecksumReg{14, 32};

// Used to generate extra clocks after writing to trigger the load of the new
// value
constexpr SPIRegister SPI_REG_ExtraClocks{12,
                                          1}; // TODO: dont know what size is

// SPI register fields
constexpr SPIRegisterField OUTPUT_MODE{SPI_REG_ConfigUnit, 4, 3};

/// @brief first two bits starting counter, second two bits number of counters
/// to read e.g. 0001 -> read counter 0 and 1, 0100 -> read counter 1
constexpr SPIRegisterField NUM_COUNTERS{SPI_REG_ConfigUnit, 8, 4};

/// @brief 00-> 16 bit, 01 -> 8 bit, 10 -> 4 bit, 11 -> reserved 16 bit
constexpr SPIRegisterField DYNAMIC_RANGE{SPI_REG_ConfigUnit, 14, 2};

// TODO: continue defining the rest of the fields as needed

} // namespace SPIRegisters

} // namespace sls