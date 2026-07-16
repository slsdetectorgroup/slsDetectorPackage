#pragma once
#include "RegisterDefs.hpp"
#include "SPIRegisterDefs.hpp"
#include <array>
#include <cstdint>

namespace sls {

namespace MatterhornDefs {

constexpr uint8_t NUM_CHIPS_PER_MODULE = 8;

// TODO: should probably be a specialized template struct

/// @brief list of Matterhorn SPI registers
struct MatterhornSPIRegisters {

    constexpr static std::array<SPIRegister, 12> spiregisters{
        SPIRegisters::SPI_REG_ConfigUnit,
        SPIRegisters::SPI_REG_ConfigCML,
        SPIRegisters::SPI_REG_ManualSelector,
        SPIRegisters::SPI_REG_CoreRSTUnit,
        SPIRegisters::SPI_REG_StoreRSTUnit,
        SPIRegisters::SPI_REG_Trimbits,
        SPIRegisters::SPI_REG_McGyver,
        SPIRegisters::SPI_REG_McGyver_par_load,
        SPIRegisters::SPI_REG_ActionReg,
        SPIRegisters::SPI_REG_InternalDACs,
        SPIRegisters::SPI_REG_ChecksumReg,
        SPIRegisters::SPI_REG_ExtraClocks};

    constexpr static uint8_t NUM_CHIPS_PER_MODULE =
        MatterhornDefs::NUM_CHIPS_PER_MODULE;
};

/// @brief list of Matterhorn IP cores
struct MatterHornIPCores {

    using ipcore_enum_type = IPCore;

    constexpr static std::array<IPCore, 5> ipcores{
        IPCore::MH_RO_SM_AXI, IPCore::FHDR_AXI, IPCore::AURORA_STATUS,
        IPCore::AURORA_STATUS2, IPCore::PACKETIZERREG};

    constexpr static size_t ip_core_block_size = IPCORE_REGISTER_BLOCK_SIZE;
};

} // namespace MatterhornDefs

} // namespace sls