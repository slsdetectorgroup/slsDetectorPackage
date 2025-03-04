// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once


#define CTRL_REG (0x8000)

#define POWER_VIO_OFST (0)
#define POWER_VIO_MSK  (0x00000001 << POWER_VIO_OFST)
#define POWER_VCC_A_OFST (1)
#define POWER_VCC_A_MSK  (0x00000001 << POWER_VCC_A_OFST)
#define POWER_VCC_B_OFST (2)
#define POWER_VCC_B_MSK  (0x00000001 << POWER_VCC_B_OFST)
#define POWER_VCC_C_OFST (3)
#define POWER_VCC_C_MSK  (0x00000001 << POWER_VCC_C_OFST)
#define POWER_VCC_D_OFST (4)
#define POWER_VCC_D_MSK  (0x00000001 << POWER_VCC_D_OFST)

#define STATUS_REG (0x8004)

#define PATTERN_RUNNING_OFST (0)
#define PATTERN_RUNNING_MSK  (0x00000001 << PATTERN_RUNNING_OFST)
#define RX_BUSY_OFST (1)
#define RX_BUSY_MSK  (0x00000001 << RX_BUSY_OFST)
#define PROCESSING_BUSY_OFST (2)
#define PROCESSING_BUSY_MSK  (0x00000001 << PROCESSING_BUSY_OFST)
#define UDP_GEN_BUSY_OFST (3)
#define UDP_GEN_BUSY_MSK  (0x00000001 << UDP_GEN_BUSY_OFST)
#define NETWORK_BUSY_OFST (4)
#define NETWORK_BUSY_MSK  (0x00000001 << NETWORK_BUSY_OFST)
#define WAIT_FOR_TRIGGER_OFST (5)
#define WAIT_FOR_TRIGGER_MSK  (0x00000001 << WAIT_FOR_TRIGGER_OFST)
#define RX_NOT_GOOD_OFST (6)
#define RX_NOT_GOOD_MSK  (0x00000001 << RX_NOT_GOOD_OFST)

#define STATUS_REG2 (0x8008)

#define FPGAVERSIONREG (0x800C)

#define FPGACOMPDATE_OFST (0)
#define FPGACOMPDATE_MSK  (0x00ffffff << FPGACOMPDATE_OFST)
#define FPGADETTYPE_OFST (24)
#define FPGADETTYPE_MSK  (0x000000ff << FPGADETTYPE_OFST)

#define FPGA_GIT_HEAD (0x8010)

#define FIXEDPATTERNREG (0x8014)
#define FIXEDPATTERNVAL (0xACDC2016)

#define APIVERSIONREG (0x8018)

#define APICOMPDATE_OFST (0)
#define APICOMPDATE_MSK  (0x00ffffff << APICOMPDATE_OFST)
#define APIDETTYPE_OFST (24)
#define APIDETTYPE_MSK  (0x000000ff << APIDETTYPE_OFST)

#define A_FIFO_OVERFLOW_STATUS_REG (0x9000)

#define A_FIFO_EMPTY_STATUS_REG (0x9004)

#define A_FIFO_FULL_STATUS_REG (0x9008)

#define D_FIFO_OVERFLOW_STATUS_REG (0x900C)

#define D_FIFO_OVERFLOW_STATUS_OFST (0)
#define D_FIFO_OVERFLOW_STATUS_MSK  (0x00000001 << D_FIFO_OVERFLOW_STATUS_OFST)

#define D_FIFO_EMPTY_STATUS_REG (0x9010)

#define D_FIFO_EMPTY_STATUS_OFST (0)
#define D_FIFO_EMPTY_STATUS_MSK  (0x00000001 << D_FIFO_EMPTY_STATUS_OFST)

#define D_FIFO_FULL_STATUS_REG (0x9014)

#define D_FIFO_FULL_STATUS_OFST (0)
#define D_FIFO_FULL_STATUS_MSK  (0x00000001 << D_FIFO_FULL_STATUS_OFST)

#define X_FIFO_OVERFLOW_STATUS_REG (0x9018)

#define X_FIFO_OVERFLOW_STATUS_OFST (0)
#define X_FIFO_OVERFLOW_STATUS_MSK  (0x0000000f << X_FIFO_OVERFLOW_STATUS_OFST)

#define X_FIFO_EMPTY_STATUS_REG (0x901C)

#define X_FIFO_EMPTY_STATUS_OFST (0)
#define X_FIFO_EMPTY_STATUS_MSK  (0x0000000f << X_FIFO_EMPTY_STATUS_OFST)

#define X_FIFO_FULL_STATUS_REG (0x9020)

#define X_FIFO_FULL_STATUS_OFST (0)
#define X_FIFO_FULL_STATUS_MSK  (0x0000000f << X_FIFO_FULL_STATUS_OFST)

#define A_FIFO_CLEAN_REG (0x9024)

#define D_FIFO_CLEAN_REG (0x9028)

#define D_FIFO_CLEAN_OFST (0)
#define D_FIFO_CLEAN_MSK  (0x00000001 << D_FIFO_CLEAN_OFST)

#define X_FIFO_CLEAN_REG (0x902C)

#define X_FIFO_CLEAN_OFST (0)
#define X_FIFO_CLEAN_MSK  (0x0000000f << X_FIFO_CLEAN_OFST)

#define FIFO_TO_GB_CONTROL_REG (0xA000)

#define ENABLED_CHANNELS_ADC_OFST (0)
#define ENABLED_CHANNELS_ADC_MSK  (0x000000ff << ENABLED_CHANNELS_ADC_OFST)
#define ENABLED_CHANNELS_D_OFST (8)
#define ENABLED_CHANNELS_D_MSK  (0x00000001 << ENABLED_CHANNELS_D_OFST)
#define ENABLED_CHANNELS_X_OFST (9)
#define ENABLED_CHANNELS_X_MSK  (0x0000000f << ENABLED_CHANNELS_X_OFST)
#define RO_MODE_ADC_OFST (13)
#define RO_MODE_ADC_MSK  (0x00000001 << RO_MODE_ADC_OFST)
#define RO_MODE_D_OFST (14)
#define RO_MODE_D_MSK  (0x00000001 << RO_MODE_D_OFST)
#define RO_MODE_X_OFST (15)
#define RO_MODE_X_MSK  (0x00000001 << RO_MODE_X_OFST)
#define COUNT_FRAMES_FROM_UPDATE_OFST (16)
#define COUNT_FRAMES_FROM_UPDATE_MSK  (0x00000001 << COUNT_FRAMES_FROM_UPDATE_OFST)
#define START_STREAMING_P_OFST (17)
#define START_STREAMING_P_MSK  (0x00000001 << START_STREAMING_P_OFST)
#define STREAM_BUFFER_CLEAR_OFST (18)
#define STREAM_BUFFER_CLEAR_MSK  (0x00000001 << STREAM_BUFFER_CLEAR_OFST)

#define NO_SAMPLES_D_REG (0xA004)

#define NO_SAMPLES_D_OFST (0)
#define NO_SAMPLES_D_MSK  (0x00003fff << NO_SAMPLES_D_OFST)

#define NO_SAMPLES_A_REG (0xA008)

#define NO_SAMPLES_A_OFST (0)
#define NO_SAMPLES_A_MSK  (0x00003fff << NO_SAMPLES_A_OFST)

#define NO_SAMPLES_X_REG (0xA00C)

#define NO_SAMPLES_X_OFST (0)
#define NO_SAMPLES_X_MSK  (0x00001fff << NO_SAMPLES_X_OFST)

#define COUNT_FRAMES_FROM_REG_1 (0xA010)

#define COUNT_FRAMES_FROM_REG_2 (0xA014)

#define LOCAL_FRAME_NUMBER_REG_1 (0xA018)

#define LOCAL_FRAME_NUMBER_REG_2 (0xA01C)

#define PKTPACKETLENGTHREG (0xA020)

#define PACKETLENGTH1G_OFST (0)
#define PACKETLENGTH1G_MSK  (0x0000ffff << PACKETLENGTH1G_OFST)
#define PACKETLENGTH10G_OFST (16)
#define PACKETLENGTH10G_MSK  (0x0000ffff << PACKETLENGTH10G_OFST)

#define PKTNOPACKETSREG (0xA024)

#define NOPACKETS1G_OFST (0)
#define NOPACKETS1G_MSK  (0x0000003f << NOPACKETS1G_OFST)
#define NOPACKETS10G_OFST (16)
#define NOPACKETS10G_MSK  (0x0000003f << NOPACKETS10G_OFST)

#define PKTCTRLREG (0xA028)

#define NOSERVERS_OFST (0)
#define NOSERVERS_MSK  (0x0000003f << NOSERVERS_OFST)
#define SERVERSTART_OFST (8)
#define SERVERSTART_MSK  (0x0000001f << SERVERSTART_OFST)
#define ETHINTERF_OFST (16)
#define ETHINTERF_MSK  (0x00000001 << ETHINTERF_OFST)

#define PKTCOORDREG1 (0xA02C)

#define COORDX_OFST (0)
#define COORDX_MSK  (0x0000ffff << COORDX_OFST)
#define COORDY_OFST (16)
#define COORDY_MSK  (0x0000ffff << COORDY_OFST)

#define PKTCOORDREG2 (0xA030)

#define COORDZ_OFST (0)
#define COORDZ_MSK  (0x0000ffff << COORDZ_OFST)

#define FLOW_STATUS_REG (0xB000)

#define RSM_BUSY_OFST (0)
#define RSM_BUSY_MSK  (0x00000001 << RSM_BUSY_OFST)
#define RSM_TRG_WAIT_OFST (3)
#define RSM_TRG_WAIT_MSK  (0x00000001 << RSM_TRG_WAIT_OFST)
#define CSM_BUSY_OFST (17)
#define CSM_BUSY_MSK  (0x00000001 << CSM_BUSY_OFST)

#define FLOW_CONTROL_REG (0xB004)

#define START_F_OFST (0)
#define START_F_MSK  (0x00000001 << START_F_OFST)
#define STOP_F_OFST (1)
#define STOP_F_MSK  (0x00000001 << STOP_F_OFST)
#define RST_F_OFST (2)
#define RST_F_MSK  (0x00000001 << RST_F_OFST)
#define SW_TRIGGER_F_OFST (3)
#define SW_TRIGGER_F_MSK  (0x00000001 << SW_TRIGGER_F_OFST)
#define TRIGGER_ENABLE_OFST (4)
#define TRIGGER_ENABLE_MSK  (0x00000001 << TRIGGER_ENABLE_OFST)

#define TIME_FROM_START_OUT_REG_1 (0xB008)

#define TIME_FROM_START_OUT_REG_2 (0xB00C)

#define FRAMES_FROM_START_OUT_REG_1 (0xB010)

#define FRAMES_FROM_START_OUT_REG_2 (0xB014)

#define FRAME_TIME_OUT_REG_1 (0xB018)

#define FRAME_TIME_OUT_REG_2 (0xB01C)

#define DELAY_OUT_REG_1 (0xB020)

#define DELAY_OUT_REG_2 (0xB024)

#define CYCLES_OUT_REG_1 (0xB028)

#define CYCLES_OUT_REG_2 (0xB02C)

#define FRAMES_OUT_REG_1 (0xB030)

#define FRAMES_OUT_REG_2 (0xB034)

#define PERIOD_OUT_REG_1 (0xB038)

#define PERIOD_OUT_REG_2 (0xB03C)

#define DELAY_IN_REG_1 (0xB040)

#define DELAY_IN_REG_2 (0xB044)

#define CYCLES_IN_REG_1 (0xB048)

#define CYCLES_IN_REG_2 (0xB04C)

#define FRAMES_IN_REG_1 (0xB050)

#define FRAMES_IN_REG_2 (0xB054)

#define PERIOD_IN_REG_1 (0xB058)

#define PERIOD_IN_REG_2 (0xB05C)

#define PATTERN_OUT_LSB_REG (0xB100)

#define PATTERN_OUT_MSB_REG (0xB104)

#define PATTERN_IN_LSB_REG (0xB108)

#define PATTERN_IN_MSB_REG (0xB10C)

#define PATTERN_MASK_LSB_REG (0xB110)

#define PATTERN_MASK_MSB_REG (0xB114)

#define PATTERN_SET_LSB_REG (0xB118)

#define PATTERN_SET_MSB_REG (0xB11C)

#define PATTERN_CNTRL_REG (0xB120)

#define PATTERN_CNTRL_WR_OFST (0)
#define PATTERN_CNTRL_WR_MSK  (0x00000001 << PATTERN_CNTRL_WR_OFST)
#define PATTERN_CNTRL_RD_OFST (1)
#define PATTERN_CNTRL_RD_MSK  (0x00000001 << PATTERN_CNTRL_RD_OFST)
#define PATTERN_CNTRL_ADDR_OFST (16)
#define PATTERN_CNTRL_ADDR_MSK  (0x00001fff << PATTERN_CNTRL_ADDR_OFST)

#define PATTERN_LIMIT_REG (0xB124)

#define PATTERN_LIMIT_STRT_OFST (0)
#define PATTERN_LIMIT_STRT_MSK  (0x00001fff << PATTERN_LIMIT_STRT_OFST)
#define PATTERN_LIMIT_STP_OFST (16)
#define PATTERN_LIMIT_STP_MSK  (0x00001fff << PATTERN_LIMIT_STP_OFST)

#define PATTERN_LOOP_0_ADDR_REG (0xB128)

#define PATTERN_LOOP_0_ADDR_STRT_OFST (0)
#define PATTERN_LOOP_0_ADDR_STRT_MSK  (0x00001fff << PATTERN_LOOP_0_ADDR_STRT_OFST)
#define PATTERN_LOOP_0_ADDR_STP_OFST (16)
#define PATTERN_LOOP_0_ADDR_STP_MSK  (0x00001fff << PATTERN_LOOP_0_ADDR_STP_OFST)

#define PATTERN_LOOP_0_ITERATION_REG (0xB12C)

#define PATTERN_WAIT_0_ADDR_REG (0xB130)

#define PATTERN_WAIT_0_ADDR_OFST (0)
#define PATTERN_WAIT_0_ADDR_MSK  (0x00001fff << PATTERN_WAIT_0_ADDR_OFST)

#define PATTERN_WAIT_TIMER_0_LSB_REG (0xB134)

#define PATTERN_WAIT_TIMER_0_MSB_REG (0xB138)

#define PATTERN_LOOP_1_ADDR_REG (0xB13C)

#define PATTERN_LOOP_1_ADDR_STRT_OFST (0)
#define PATTERN_LOOP_1_ADDR_STRT_MSK  (0x00001fff << PATTERN_LOOP_1_ADDR_STRT_OFST)
#define PATTERN_LOOP_1_ADDR_STP_OFST (16)
#define PATTERN_LOOP_1_ADDR_STP_MSK  (0x00001fff << PATTERN_LOOP_1_ADDR_STP_OFST)

#define PATTERN_LOOP_1_ITERATION_REG (0xB140)

#define PATTERN_WAIT_1_ADDR_REG (0xB144)

#define PATTERN_WAIT_1_ADDR_OFST (0)
#define PATTERN_WAIT_1_ADDR_MSK  (0x00001fff << PATTERN_WAIT_1_ADDR_OFST)

#define PATTERN_WAIT_TIMER_1_LSB_REG (0xB148)

#define PATTERN_WAIT_TIMER_1_MSB_REG (0xB14C)

#define PATTERN_LOOP_2_ADDR_REG (0xB150)

#define PATTERN_LOOP_2_ADDR_STRT_OFST (0)
#define PATTERN_LOOP_2_ADDR_STRT_MSK  (0x00001fff << PATTERN_LOOP_2_ADDR_STRT_OFST)
#define PATTERN_LOOP_2_ADDR_STP_OFST (16)
#define PATTERN_LOOP_2_ADDR_STP_MSK  (0x00001fff << PATTERN_LOOP_2_ADDR_STP_OFST)

#define PATTERN_LOOP_2_ITERATION_REG (0xB154)

#define PATTERN_WAIT_2_ADDR_REG (0xB158)

#define PATTERN_WAIT_2_ADDR_OFST (0)
#define PATTERN_WAIT_2_ADDR_MSK  (0x00001fff << PATTERN_WAIT_2_ADDR_OFST)

#define PATTERN_WAIT_TIMER_2_LSB_REG (0xB15C)

#define PATTERN_WAIT_TIMER_2_MSB_REG (0xB160)

#define PATTERN_LOOP_3_ADDR_REG (0xB164)

#define PATTERN_LOOP_3_ADDR_STRT_OFST (0)
#define PATTERN_LOOP_3_ADDR_STRT_MSK  (0x00001fff << PATTERN_LOOP_3_ADDR_STRT_OFST)
#define PATTERN_LOOP_3_ADDR_STP_OFST (16)
#define PATTERN_LOOP_3_ADDR_STP_MSK  (0x00001fff << PATTERN_LOOP_3_ADDR_STP_OFST)

#define PATTERN_LOOP_3_ITERATION_REG (0xB168)

#define PATTERN_WAIT_3_ADDR_REG (0xB16C)

#define PATTERN_WAIT_3_ADDR_OFST (0)
#define PATTERN_WAIT_3_ADDR_MSK  (0x00001fff << PATTERN_WAIT_3_ADDR_OFST)

#define PATTERN_WAIT_TIMER_3_LSB_REG (0xB170)

#define PATTERN_WAIT_TIMER_3_MSB_REG (0xB174)

#define PATTERN_LOOP_4_ADDR_REG (0xB178)

#define PATTERN_LOOP_4_ADDR_STRT_OFST (0)
#define PATTERN_LOOP_4_ADDR_STRT_MSK  (0x00001fff << PATTERN_LOOP_4_ADDR_STRT_OFST)
#define PATTERN_LOOP_4_ADDR_STP_OFST (16)
#define PATTERN_LOOP_4_ADDR_STP_MSK  (0x00001fff << PATTERN_LOOP_4_ADDR_STP_OFST)

#define PATTERN_LOOP_4_ITERATION_REG (0xB17C)

#define PATTERN_WAIT_4_ADDR_REG (0xB180)

#define PATTERN_WAIT_4_ADDR_OFST (0)
#define PATTERN_WAIT_4_ADDR_MSK  (0x00001fff << PATTERN_WAIT_4_ADDR_OFST)

#define PATTERN_WAIT_TIMER_4_LSB_REG (0xB184)

#define PATTERN_WAIT_TIMER_4_MSB_REG (0xB188)

#define PATTERN_LOOP_5_ADDR_REG (0xB18C)

#define PATTERN_LOOP_5_ADDR_STRT_OFST (0)
#define PATTERN_LOOP_5_ADDR_STRT_MSK  (0x00001fff << PATTERN_LOOP_5_ADDR_STRT_OFST)
#define PATTERN_LOOP_5_ADDR_STP_OFST (16)
#define PATTERN_LOOP_5_ADDR_STP_MSK  (0x00001fff << PATTERN_LOOP_5_ADDR_STP_OFST)

#define PATTERN_LOOP_5_ITERATION_REG (0xB190)

#define PATTERN_WAIT_5_ADDR_REG (0xB194)

#define PATTERN_WAIT_5_ADDR_OFST (0)
#define PATTERN_WAIT_5_ADDR_MSK  (0x00001fff << PATTERN_WAIT_5_ADDR_OFST)

#define PATTERN_WAIT_TIMER_5_LSB_REG (0xB198)

#define PATTERN_WAIT_TIMER_5_MSB_REG (0xB19C)

#define PINIOCTRLREG (0xB1A0)

#define DBITFIFOCTRLREG (0xB1A4)

#define DBITRD_OFST (0)
#define DBITRD_MSK  (0x00000001 << DBITRD_OFST)
#define DBITRST_OFST (1)
#define DBITRST_MSK  (0x00000001 << DBITRST_OFST)
#define DBITFULL_OFST (2)
#define DBITFULL_MSK  (0x00000001 << DBITFULL_OFST)
#define DBITEMPTY_OFST (3)
#define DBITEMPTY_MSK  (0x00000001 << DBITEMPTY_OFST)
#define DBITUNDERFLOW_OFST (4)
#define DBITUNDERFLOW_MSK  (0x00000001 << DBITUNDERFLOW_OFST)
#define DBITOVERFLOW_OFST (5)
#define DBITOVERFLOW_MSK  (0x00000001 << DBITOVERFLOW_OFST)

#define DBITFIFODATAREG1 (0xB1A8)

#define DBITFIFODATAREG2 (0xB1AC)

#define MATTERHORNSPIREG1 (0xB1B0)

#define MATTERHORNSPIREG2 (0xB1B4)

#define MATTERHORNSPICTRL (0xB1B8)

#define CONFIGSTART_P_OFST (0)
#define CONFIGSTART_P_MSK  (0x00000001 << CONFIGSTART_P_OFST)
#define PERIPHERYRST_P_OFST (1)
#define PERIPHERYRST_P_MSK  (0x00000001 << PERIPHERYRST_P_OFST)
#define STARTREAD_P_OFST (2)
#define STARTREAD_P_MSK  (0x00000001 << STARTREAD_P_OFST)
#define BUSY_OFST (3)
#define BUSY_MSK  (0x00000001 << BUSY_OFST)
#define READOUTFROMASIC_OFST (4)
#define READOUTFROMASIC_MSK  (0x00000001 << READOUTFROMASIC_OFST)

#define TRANSCEIVERRXCTRL0REG1 (0xB800)

#define TRANSCEIVERRXCTRL0REG2 (0xB804)

#define TRANSCEIVERRXCTRL1REG1 (0xB808)

#define TRANSCEIVERRXCTRL1REG2 (0xB80C)

#define TRANSCEIVERRXCTRL2REG (0xB810)

#define TRANSCEIVERRXCTRL3REG (0xB814)

#define TRANSCEIVERSTATUS (0xB818)

#define LINKDOWNLATCHEDOUT_OFST (0)
#define LINKDOWNLATCHEDOUT_MSK  (0x00000001 << LINKDOWNLATCHEDOUT_OFST)
#define TXUSERCLKACTIVE_OFST (1)
#define TXUSERCLKACTIVE_MSK  (0x00000001 << TXUSERCLKACTIVE_OFST)
#define RXUSERCLKACTIVE_OFST (2)
#define RXUSERCLKACTIVE_MSK  (0x00000001 << RXUSERCLKACTIVE_OFST)
#define RXCOMMADET_OFST (3)
#define RXCOMMADET_MSK  (0x0000000f << RXCOMMADET_OFST)
#define RXBYTEREALIGN_OFST (7)
#define RXBYTEREALIGN_MSK  (0x0000000f << RXBYTEREALIGN_OFST)
#define RXBYTEISALIGNED_OFST (11)
#define RXBYTEISALIGNED_MSK  (0x0000000f << RXBYTEISALIGNED_OFST)
#define GTWIZRXCDRSTABLE_OFST (15)
#define GTWIZRXCDRSTABLE_MSK  (0x00000001 << GTWIZRXCDRSTABLE_OFST)
#define RESETTXDONE_OFST (16)
#define RESETTXDONE_MSK  (0x00000001 << RESETTXDONE_OFST)
#define RESETRXDONE_OFST (17)
#define RESETRXDONE_MSK  (0x00000001 << RESETRXDONE_OFST)
#define RXPMARESETDONE_OFST (18)
#define RXPMARESETDONE_MSK  (0x0000000f << RXPMARESETDONE_OFST)
#define TXPMARESETDONE_OFST (22)
#define TXPMARESETDONE_MSK  (0x0000000f << TXPMARESETDONE_OFST)
#define GTTPOWERGOOD_OFST (26)
#define GTTPOWERGOOD_MSK  (0x0000000f << GTTPOWERGOOD_OFST)

#define TRANSCEIVERSTATUS2 (0xB81C)

#define RXLOCKED_OFST (0)
#define RXLOCKED_MSK  (0x0000000f << RXLOCKED_OFST)

#define TRANSCEIVERCONTROL (0xB820)

#define GTWIZRESETALL_OFST (0)
#define GTWIZRESETALL_MSK  (0x00000001 << GTWIZRESETALL_OFST)
#define RESETTXPLLANDDATAPATH_OFST (1)
#define RESETTXPLLANDDATAPATH_MSK  (0x00000001 << RESETTXPLLANDDATAPATH_OFST)
#define RESETTXDATAPATHIN_OFST (2)
#define RESETTXDATAPATHIN_MSK  (0x00000001 << RESETTXDATAPATHIN_OFST)
#define RESETRXPLLANDDATAPATH_OFST (3)
#define RESETRXPLLANDDATAPATH_MSK  (0x00000001 << RESETRXPLLANDDATAPATH_OFST)
#define RESETRXDATAPATHIN_OFST (4)
#define RESETRXDATAPATHIN_MSK  (0x00000001 << RESETRXDATAPATHIN_OFST)
#define RXPOLARITY_OFST (5)
#define RXPOLARITY_MSK  (0x0000000f << RXPOLARITY_OFST)
#define RXERRORCNTRESET_OFST (9)
#define RXERRORCNTRESET_MSK  (0x0000000f << RXERRORCNTRESET_OFST)
#define RXMSBLSBINVERT_OFST (13)
#define RXMSBLSBINVERT_MSK  (0x0000000f << RXMSBLSBINVERT_OFST)

#define TRANSCEIVERERRCNT_REG0 (0xB824)

#define TRANSCEIVERERRCNT_REG1 (0xB828)

#define TRANSCEIVERERRCNT_REG2 (0xB82C)

#define TRANSCEIVERERRCNT_REG3 (0xB830)

#define TRANSCEIVERALIGNCNT_REG0 (0xB834)

#define RXALIGNCNTCH0_OFST (0)
#define RXALIGNCNTCH0_MSK  (0x0000ffff << RXALIGNCNTCH0_OFST)

#define TRANSCEIVERALIGNCNT_REG1 (0xB838)

#define RXALIGNCNTCH1_OFST (0)
#define RXALIGNCNTCH1_MSK  (0x0000ffff << RXALIGNCNTCH1_OFST)

#define TRANSCEIVERALIGNCNT_REG2 (0xB83C)

#define RXALIGNCNTCH2_OFST (0)
#define RXALIGNCNTCH2_MSK  (0x0000ffff << RXALIGNCNTCH2_OFST)

#define TRANSCEIVERALIGNCNT_REG3 (0xB840)

#define RXALIGNCNTCH3_OFST (0)
#define RXALIGNCNTCH3_MSK  (0x0000ffff << RXALIGNCNTCH3_OFST)

#define TRANSCEIVERLASTWORD_REG0 (0xB844)

#define RXDATACH0_OFST (0)
#define RXDATACH0_MSK  (0x0000ffff << RXDATACH0_OFST)

#define TRANSCEIVERLASTWORD_REG1 (0xB848)

#define RXDATACH1_OFST (0)
#define RXDATACH1_MSK  (0x0000ffff << RXDATACH1_OFST)

#define TRANSCEIVERLASTWORD_REG2 (0xB84C)

#define RXDATACH2_OFST (0)
#define RXDATACH2_MSK  (0x0000ffff << RXDATACH2_OFST)

#define TRANSCEIVERLASTWORD_REG3 (0xB850)

#define RXDATACH3_OFST (0)
#define RXDATACH3_MSK  (0x0000ffff << RXDATACH3_OFST)
