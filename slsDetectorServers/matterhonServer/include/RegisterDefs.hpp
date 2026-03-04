
#include "RegisterHelperStructs.hpp"

/// @brief Enum for IP cores, value are adresses
constexpr enum class IPCore : uint32_t {
    MH_RO_SM_AXI = 0, // dummy adresses for now
    FHDR_AXI = 1,
    AURORA_STATUS = 2,
    AURORA_STATUS2 = 3,
    PACKETIZERREG = 4,
    UNKNOWN = 5
};


// Register definitions
constexpr Register CTRL_Reg{IPCore::UNKNOWN, 0};

constexpr Register Status_Reg{IPCore::UNKNOWN, 4};

constexpr Register FPGAVersionReg{IPCore::UNKNOWN, 8};

constexpr Register FPGA_GIT_HEAD{IPCore::UNKNOWN, C};

constexpr Register FixedPatternReg{IPCore::UNKNOWN, 10};

constexpr Register ApiVersionReg{IPCore::UNKNOWN, 14};

constexpr Register Chip_ID_Reg{IPCore::UNKNOWN, 18};

constexpr Register MH_SM_Ctrl_Reg{IPCore::MH_RO_SM_AXI, 0};

constexpr Register MH_SM_Exposure_Reg{IPCore::MH_RO_SM_AXI, 4};

constexpr Register MH_SM_Period_Reg{IPCore::MH_RO_SM_AXI, 8};

constexpr Register MH_SM_Frames_Reg{IPCore::MH_RO_SM_AXI, C};

constexpr Register MH_SM_StoreLength_Reg{IPCore::MH_RO_SM_AXI, 10};

constexpr Register MH_SM_ResetMHLength_Reg{IPCore::MH_RO_SM_AXI, 14};

constexpr Register Frame_HDR_Set_Reg{IPCore::FHDR_AXI, 0};

constexpr Register Frame_HDR_FrameNumLSB_Reg{IPCore::FHDR_AXI, 4};

constexpr Register Frame_HDR_FrameNumMSB_Reg{IPCore::FHDR_AXI, 8};

constexpr Register Frame_HDR_TimestampLSB_Reg{IPCore::FHDR_AXI, C};

constexpr Register Frame_HDR_TimestampMSB_Reg{IPCore::FHDR_AXI, 10};

constexpr Register Frame_HDR_ModCoord_LSB_Reg{IPCore::FHDR_AXI, 14};

constexpr Register Frame_HDR_ModCoord_MSB_Reg{IPCore::FHDR_AXI, 18};

constexpr Register Frame_HDR_PktctrMax_Reg{IPCore::FHDR_AXI, 1C};

constexpr Register Aurora_Valid_DW_Reg{IPCore::AURORA_STATUS, 0};

constexpr Register Aurora_Valid_Bytes_Reg{IPCore::AURORA_STATUS, 4};

constexpr Register Aurora_Busy_Up_Cycles_Reg{IPCore::AURORA_STATUS, 8};

constexpr Register Aurora_Hard_Errors_Reg{IPCore::AURORA_STATUS, C};

constexpr Register Aurora_Soft_Errors_Reg{IPCore::AURORA_STATUS, 10};

constexpr Register Aurora_Channel_n_Lanes_Up_Reg{IPCore::AURORA_STATUS, 14};

constexpr Register Aurora_GT_PLL_Lock_Reg{IPCore::AURORA_STATUS2, 0};

constexpr Register PktPacketLengthReg{IPCore::PACKETIZERREG, A100};

constexpr Register PktNoPacketsReg{IPCore::PACKETIZERREG, A104};

constexpr Register PktCtrlReg{IPCore::PACKETIZERREG, A108};

constexpr Register PktCoordReg1{IPCore::PACKETIZERREG, A10C};

constexpr Register PktCoordReg2{IPCore::PACKETIZERREG, A110};



// Register fields
constexpr RegisterField Power_VIO{CTRL_Reg, 0x1, 0};

constexpr RegisterField Power_Vcc_A{CTRL_Reg, 0x1, 1};

constexpr RegisterField Power_Vcc_B{CTRL_Reg, 0x1, 2};

constexpr RegisterField Power_Vcc_C{CTRL_Reg, 0x1, 3};

constexpr RegisterField Power_Vcc_D{CTRL_Reg, 0x1, 4};

constexpr RegisterField MH_Enable_Enable{CTRL_Reg, 0x1, 5};

constexpr RegisterField MH_Clk_Enable{CTRL_Reg, 0x1, 6};

constexpr RegisterField sm_busy{Status_Reg, 0x1, 0};

constexpr RegisterField FPGACompDate{FPGAVersionReg, 0xffffff, 0};

constexpr RegisterField FPGADetType{FPGAVersionReg, 0xff, 24};

constexpr RegisterField FPGA_GIT_HEAD{FPGA_GIT_HEAD, 0xffffffff, 0};

constexpr RegisterField FixedPattern{FixedPatternReg, 0xffffffff, 0};

constexpr RegisterField ApiCompDate{ApiVersionReg, 0xffffff, 0};

constexpr RegisterField ApiDetType{ApiVersionReg, 0xff, 24};

constexpr RegisterField ChipID{Chip_ID_Reg, 0x7, 0};

constexpr RegisterField Start_Acquistion{MH_SM_Ctrl_Reg, 0x1, 0};

constexpr RegisterField Stop_Acquistion{MH_SM_Ctrl_Reg, 0x1, 1};

constexpr RegisterField MH_Readout_Exposure_Time{MH_SM_Exposure_Reg, 0xffffffff, 0};

constexpr RegisterField MH_Readout_Period_Time{MH_SM_Period_Reg, 0xffffffff, 0};

constexpr RegisterField MH_Readout_Frames{MH_SM_Frames_Reg, 0xffffffff, 0};

constexpr RegisterField MH_SM_StoreLength{MH_SM_StoreLength_Reg, 0xffffffff, 0};

constexpr RegisterField MH_SM_ResetMHLength{MH_SM_ResetMHLength_Reg, 0xffffffff, 0};

constexpr RegisterField Frame_Hdr_Set_Framenumber{Frame_HDR_Set_Reg, 0x1, 0};

constexpr RegisterField Frame_Hdr_Set_Timestamp{Frame_HDR_Set_Reg, 0x1, 1};

constexpr RegisterField Frame_Hdr_Framenumber_LSB{Frame_HDR_FrameNumLSB_Reg, 0xffffffff, 0};

constexpr RegisterField Frame_Hdr_Framenumber_MSB{Frame_HDR_FrameNumMSB_Reg, 0xffffffff, 0};

constexpr RegisterField Frame_Hdr_Timestamp_LSB{Frame_HDR_TimestampLSB_Reg, 0xffffffff, 0};

constexpr RegisterField Frame_Hdr_Timestamp_MSB{Frame_HDR_TimestampMSB_Reg, 0xffffffff, 0};

constexpr RegisterField Frame_HDR_ModCoord_LSB{Frame_HDR_ModCoord_LSB_Reg, 0xffffffff, 0};

constexpr RegisterField Frame_HDR_ModCoord_MSB{Frame_HDR_ModCoord_MSB_Reg, 0xffffffff, 0};

constexpr RegisterField Frame_HDR_PktctrMax{Frame_HDR_PktctrMax_Reg, 0xff, 0};

constexpr RegisterField Aurora_Number_Valid_DW{Aurora_Valid_DW_Reg, 0xffffffff, 0};

constexpr RegisterField Aurora_Valid_Bytes{Aurora_Valid_Bytes_Reg, 0xffffffff, 0};

constexpr RegisterField Aurora_Busy_Up_Cycles{Aurora_Busy_Up_Cycles_Reg, 0xffffffff, 0};

constexpr RegisterField Aurora_Hard_Errors{Aurora_Hard_Errors_Reg, 0xffffffff, 0};

constexpr RegisterField Aurora_Soft_Errors{Aurora_Soft_Errors_Reg, 0xffffffff, 0};

constexpr RegisterField Aurora_Lanes_Up{Aurora_Channel_n_Lanes_Up_Reg, 0xf, 0};

constexpr RegisterField Aurora_Channel_Up{Aurora_Channel_n_Lanes_Up_Reg, 0x1, 4};

constexpr RegisterField Aurora_GT_PLL_Lock{Aurora_GT_PLL_Lock_Reg, 0x1, 0};

constexpr RegisterField Aurora_GT_PLL_Lock_Counter{Aurora_GT_PLL_Lock_Reg, 0x1ffffff, 4};

constexpr RegisterField PacketLength1G{PktPacketLengthReg, 0xffff, 0};

constexpr RegisterField PacketLength10G{PktPacketLengthReg, 0xffff, 16};

constexpr RegisterField NoPackets1G{PktNoPacketsReg, 0x3f, 0};

constexpr RegisterField NoPackets10G{PktNoPacketsReg, 0x3f, 16};

constexpr RegisterField NoServers{PktCtrlReg, 0x3f, 0};

constexpr RegisterField ServerStart{PktCtrlReg, 0x1f, 8};

constexpr RegisterField EthInterf{PktCtrlReg, 0x1, 16};

constexpr RegisterField Coordx{PktCoordReg1, 0xffff, 0};

constexpr RegisterField Coordy{PktCoordReg1, 0xffff, 16};

constexpr RegisterField Coordz{PktCoordReg2, 0xffff, 0};

