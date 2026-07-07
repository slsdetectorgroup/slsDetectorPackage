
// clang-format off
#pragma once
#include "RegisterHelperStructs.hpp"

namespace sls {

/// @brief Enum for IP cores, value are adresses
enum class IPCore : uint32_t {
    MH_RO_SM_AXI = 0xB0010000,
    FHDR_AXI = 0xB0011000,
    AURORA_STATUS = 0xB0014000,
    AURORA_STATUS2 = 0xB0015000,
    PACKETIZERREG = 0x00000000, // TODO: need to update with actual address
    UNKNOWN = 0x00000000 // dont know yet
};

constexpr size_t IPCORE_REGISTER_BLOCK_SIZE =
    0x1000; // size of each IP core address space in bytes // TODO: maybe add in
            // other file definitions

// clang-format off
namespace Reg {


// Register definitions
constexpr Register CTRL_Reg{IPCore::UNKNOWN, 0x0};

constexpr Register Status_Reg{IPCore::UNKNOWN, 0x4};

constexpr Register FPGAVersionReg{IPCore::UNKNOWN, 0x8};

constexpr Register FPGA_GIT_HEADReg{IPCore::UNKNOWN, 0xc};

constexpr Register FixedPatternReg{IPCore::UNKNOWN, 0x10};

constexpr Register ApiVersionReg{IPCore::UNKNOWN, 0x14};

constexpr Register Chip_ID_Reg{IPCore::UNKNOWN, 0x18};

constexpr Register MH_SM_Ctrl_Reg{IPCore::MH_RO_SM_AXI, 0x0};

constexpr Register MH_SM_Exposure_Reg{IPCore::MH_RO_SM_AXI, 0x4};

constexpr Register MH_SM_Period_Reg{IPCore::MH_RO_SM_AXI, 0x8};

constexpr Register MH_SM_Frames_Reg{IPCore::MH_RO_SM_AXI, 0xc};

constexpr Register MH_SM_StoreLength_Reg{IPCore::MH_RO_SM_AXI, 0x10};

constexpr Register MH_SM_ResetMHLength_Reg{IPCore::MH_RO_SM_AXI, 0x14};

constexpr Register MH_SM_Triggers_Reg{IPCore::MH_RO_SM_AXI, 0x18};

constexpr Register Frame_HDR_Set_Reg{IPCore::FHDR_AXI, 0x0};

constexpr Register Frame_HDR_FrameNumLSB_Reg{IPCore::FHDR_AXI, 0x4};

constexpr Register Frame_HDR_FrameNumMSB_Reg{IPCore::FHDR_AXI, 0x8};

constexpr Register Frame_HDR_TimestampLSB_Reg{IPCore::FHDR_AXI, 0xc};

constexpr Register Frame_HDR_TimestampMSB_Reg{IPCore::FHDR_AXI, 0x10};

constexpr Register Frame_HDR_ModCoord_LSB_Reg{IPCore::FHDR_AXI, 0x14};

constexpr Register Frame_HDR_ModCoord_MSB_Reg{IPCore::FHDR_AXI, 0x18};

constexpr Register Frame_HDR_PktctrMax_Reg{IPCore::FHDR_AXI, 0x1c};

constexpr Register Aurora_Valid_DW_Reg{IPCore::AURORA_STATUS, 0x0};

constexpr Register Aurora_Valid_Bytes_Reg{IPCore::AURORA_STATUS, 0x4};

constexpr Register Aurora_Busy_Up_Cycles_Reg{IPCore::AURORA_STATUS, 0x8};

constexpr Register Aurora_Hard_Errors_Reg{IPCore::AURORA_STATUS, 0xc};

constexpr Register Aurora_Soft_Errors_Reg{IPCore::AURORA_STATUS, 0x10};

constexpr Register Aurora_Channel_n_Lanes_Up_Reg{IPCore::AURORA_STATUS, 0x14};

constexpr Register Aurora_GT_PLL_Lock_Reg{IPCore::AURORA_STATUS2, 0x0};

constexpr Register PktPacketLengthReg{IPCore::PACKETIZERREG, 0xa100};

constexpr Register PktNoPacketsReg{IPCore::PACKETIZERREG, 0xa104};

constexpr Register PktCtrlReg{IPCore::PACKETIZERREG, 0xa108};

constexpr Register PktCoordReg1{IPCore::PACKETIZERREG, 0xa10c};

constexpr Register PktCoordReg2{IPCore::PACKETIZERREG, 0xa110};



// Register fields
constexpr RegisterField Power_VIO{
     CTRL_Reg, 0, 0x1};

constexpr RegisterField Power_Vcc_A{
     CTRL_Reg, 1, 0x1};

constexpr RegisterField Power_Vcc_B{
     CTRL_Reg, 2, 0x1};

constexpr RegisterField Power_Vcc_C{
     CTRL_Reg, 3, 0x1};

constexpr RegisterField Power_Vcc_D{
     CTRL_Reg, 4, 0x1};

constexpr RegisterField MH_Enable_Enable{
     CTRL_Reg, 5, 0x1};

constexpr RegisterField MH_Clk_Enable{
     CTRL_Reg, 6, 0x1};

constexpr RegisterField sm_busy{
     Status_Reg, 0, 0x1};

constexpr RegisterField FPGACompDate{
     FPGAVersionReg, 0, 0xffffff};

constexpr RegisterField FPGADetType{
     FPGAVersionReg, 24, 0xff};

constexpr RegisterField FPGA_GIT_HEAD{
     FPGA_GIT_HEADReg, 0, 0xffffffff};

constexpr RegisterField FixedPattern{
     FixedPatternReg, 0, 0xffffffff};

constexpr RegisterField ApiCompDate{
     ApiVersionReg, 0, 0xffffff};

constexpr RegisterField ApiDetType{
     ApiVersionReg, 24, 0xff};

constexpr RegisterField ChipID{
     Chip_ID_Reg, 0, 0x7};

constexpr RegisterField Start_Acquistion{
     MH_SM_Ctrl_Reg, 0, 0x1};

constexpr RegisterField Stop_Acquistion{
     MH_SM_Ctrl_Reg, 1, 0x1};

constexpr RegisterField External_Counter_Enable{
     MH_SM_Ctrl_Reg, 2, 0x1};

constexpr RegisterField Parallel_RO{
     MH_SM_Ctrl_Reg, 3, 0x1};

constexpr RegisterField Trigger_Mode{
     MH_SM_Ctrl_Reg, 4, 0x3};

constexpr RegisterField HW_Trigger_Polarity{
     MH_SM_Ctrl_Reg, 6, 0x1};

constexpr RegisterField SW_Trigger{
     MH_SM_Ctrl_Reg, 7, 0x1};

constexpr RegisterField Reset_Readout_SM{
     MH_SM_Ctrl_Reg, 8, 0x1};

constexpr RegisterField MH_Readout_Exposure_Time{
     MH_SM_Exposure_Reg, 0, 0xffffffff};

constexpr RegisterField MH_Readout_Period_Time{
     MH_SM_Period_Reg, 0, 0xffffffff};

constexpr RegisterField MH_Readout_Frames{
     MH_SM_Frames_Reg, 0, 0xffffffff};

constexpr RegisterField MH_SM_StoreLength{
     MH_SM_StoreLength_Reg, 0, 0xffffffff};

constexpr RegisterField MH_SM_ResetMHLength{
     MH_SM_ResetMHLength_Reg, 0, 0xffffffff};

constexpr RegisterField MH_SM_Triggers{
     MH_SM_Triggers_Reg, 0, 0xffffffff};

constexpr RegisterField Frame_Hdr_Set_Framenumber{
     Frame_HDR_Set_Reg, 0, 0x1};

constexpr RegisterField Frame_Hdr_Set_Timestamp{
     Frame_HDR_Set_Reg, 1, 0x1};

constexpr RegisterField Frame_Hdr_Framenumber_LSB{
     Frame_HDR_FrameNumLSB_Reg, 0, 0xffffffff};

constexpr RegisterField Frame_Hdr_Framenumber_MSB{
     Frame_HDR_FrameNumMSB_Reg, 0, 0xffffffff};

constexpr RegisterField Frame_Hdr_Timestamp_LSB{
     Frame_HDR_TimestampLSB_Reg, 0, 0xffffffff};

constexpr RegisterField Frame_Hdr_Timestamp_MSB{
     Frame_HDR_TimestampMSB_Reg, 0, 0xffffffff};

constexpr RegisterField Frame_HDR_ModCoord_LSB{
     Frame_HDR_ModCoord_LSB_Reg, 0, 0xffffffff};

constexpr RegisterField Frame_HDR_ModCoord_MSB{
     Frame_HDR_ModCoord_MSB_Reg, 0, 0xffffffff};

constexpr RegisterField Frame_HDR_PktctrMax{
     Frame_HDR_PktctrMax_Reg, 0, 0xff};

constexpr RegisterField Aurora_Number_Valid_DW{
     Aurora_Valid_DW_Reg, 0, 0xffffffff};

constexpr RegisterField Aurora_Valid_Bytes{
     Aurora_Valid_Bytes_Reg, 0, 0xffffffff};

constexpr RegisterField Aurora_Busy_Up_Cycles{
     Aurora_Busy_Up_Cycles_Reg, 0, 0xffffffff};

constexpr RegisterField Aurora_Hard_Errors{
     Aurora_Hard_Errors_Reg, 0, 0xffffffff};

constexpr RegisterField Aurora_Soft_Errors{
     Aurora_Soft_Errors_Reg, 0, 0xffffffff};

constexpr RegisterField Aurora_Lanes_Up{
     Aurora_Channel_n_Lanes_Up_Reg, 0, 0xf};

constexpr RegisterField Aurora_Channel_Up{
     Aurora_Channel_n_Lanes_Up_Reg, 4, 0x1};

constexpr RegisterField Aurora_GT_PLL_Lock{
     Aurora_GT_PLL_Lock_Reg, 0, 0x1};

constexpr RegisterField Aurora_GT_PLL_Lock_Counter{
     Aurora_GT_PLL_Lock_Reg, 4, 0x1ffffff};

constexpr RegisterField PacketLength1G{
     PktPacketLengthReg, 0, 0xffff};

constexpr RegisterField PacketLength10G{
     PktPacketLengthReg, 16, 0xffff};

constexpr RegisterField NoPackets1G{
     PktNoPacketsReg, 0, 0x3f};

constexpr RegisterField NoPackets10G{
     PktNoPacketsReg, 16, 0x3f};

constexpr RegisterField NoServers{
     PktCtrlReg, 0, 0x3f};

constexpr RegisterField ServerStart{
     PktCtrlReg, 8, 0x1f};

constexpr RegisterField EthInterf{
     PktCtrlReg, 16, 0x1};

constexpr RegisterField Coordx{
     PktCoordReg1, 0, 0xffff};

constexpr RegisterField Coordy{
     PktCoordReg1, 16, 0xffff};

constexpr RegisterField Coordz{
     PktCoordReg2, 0, 0xffff};

 
constexpr RegisterField ModuleRow{
     Frame_HDR_ModCoord_LSB_Reg, 0, 0xffff}; 

constexpr RegisterField ModuleCol{
     Frame_HDR_ModCoord_LSB_Reg, 16, 0xffff};

constexpr RegisterField ModuleCoordz{
     Frame_HDR_ModCoord_MSB_Reg, 0, 0xffff};

constexpr RegisterField ModuleIndex{
     Frame_HDR_ModCoord_MSB_Reg, 16, 0xffff};
} // namespace Reg
} // namespace sls
// clang-format on
