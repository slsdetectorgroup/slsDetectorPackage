#pragma once
#include <cstdint>

namespace sls {

namespace MatterhornDefs {

constexpr uint8_t NUM_CHIPS_PER_MODULE = 8;

struct MatterhornSPIRegisters {

    constexpr static std::array<SPIRegister, 10> spiregisters{
        SPIRegisters::SPI_REG_ConfigCML,
        SPIRegisters::SPI_REG_ManualSelector,
        SPIRegisters::SPI_REG_CoreRSTUnit,
        SPIRegisters::SPI_REG_StoreRSTUnit,
        SPIRegisters::SPI_REG_Trimbits,
        SPIRegisters::SPI_REG_McGyver,
        SPIRegisters::SPI_REG_McGyver_par_load,
        SPIRegisters::SPI_REG_ActionReg,
        SPIRegisters::SPI_REG_InternalDACs,
        SPIRegisters::SPI_REG_ChecksumReg};

    constexpr static uint8_t NUM_CHIPS_PER_MODULE =
        MatterhornDefs::NUM_CHIPS_PER_MODULE;
};

} // namespace MatterhornDefs

} // namespace sls