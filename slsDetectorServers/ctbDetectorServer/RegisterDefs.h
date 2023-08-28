// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once
// clang-format off

/* Definitions for FPGA */
#define MEM_MAP_SHIFT 1

/* FPGA Version RO register */
#define FPGA_VERSION_REG (0x00 << MEM_MAP_SHIFT)

#define FPGA_VERSION_BRD_RVSN_OFST  (0)
#define FPGA_VERSION_BRD_RVSN_MSK   (0x00FFFFFF << FPGA_VERSION_BRD_RVSN_OFST)
#define FPGA_VERSION_DTCTR_TYP_OFST (24)
#define FPGA_VERSION_DTCTR_TYP_MSK  (0x000000FF << FPGA_VERSION_DTCTR_TYP_OFST)
#define FPGA_VERSION_DTCTR_TYP_CTB_VAL                                         \
    ((0x4 << FPGA_VERSION_DTCTR_TYP_OFST) & FPGA_VERSION_DTCTR_TYP_MSK)

/* Fix pattern RO register */
#define FIX_PATT_REG (0x01 << MEM_MAP_SHIFT)

#define FIX_PATT_VAL (0xACDC2016)

/* Status RO register */
#define STATUS_REG (0x02 << MEM_MAP_SHIFT)

#define STATUS_RN_BSY_OFST            (0)
#define STATUS_RN_BSY_MSK             (0x00000001 << STATUS_RN_BSY_OFST)
#define STATUS_RDT_BSY_OFST           (1)
#define STATUS_RDT_BSY_MSK            (0x00000001 << STATUS_RDT_BSY_OFST)
#define STATUS_ANY_FF_FLL_OFST        (2)
#define STATUS_ANY_FF_FLL_MSK         (0x00000001 << STATUS_ANY_FF_FLL_OFST)
#define STATUS_WTNG_FR_TRGGR_OFST     (3)
#define STATUS_WTNG_FR_TRGGR_MSK      (0x00000001 << STATUS_WTNG_FR_TRGGR_OFST)
#define STATUS_DLY_BFR_OFST           (4)
#define STATUS_DLY_BFR_MSK            (0x00000001 << STATUS_DLY_BFR_OFST)
#define STATUS_DLY_AFTR_OFST          (5)
#define STATUS_DLY_AFTR_MSK           (0x00000001 << STATUS_DLY_AFTR_OFST)
#define STATUS_EXPSNG_OFST            (6)
#define STATUS_EXPSNG_MSK             (0x00000001 << STATUS_EXPSNG_OFST)
#define STATUS_CNT_ENBL_OFST          (7)
#define STATUS_CNT_ENBL_MSK           (0x00000001 << STATUS_CNT_ENBL_OFST)
#define STATUS_SM_FF_FLL_OFST         (11)
#define STATUS_SM_FF_FLL_MSK          (0x00000001 << STATUS_SM_FF_FLL_OFST)
#define STATUS_STPPD_OFST             (15)
#define STATUS_STPPD_MSK              (0x00000001 << STATUS_STPPD_OFST)
#define STATUS_ALL_FF_EMPTY_OFST      (16)
#define STATUS_ALL_FF_EMPTY_MSK       (0x00000001 << STATUS_ALL_FF_EMPTY_OFST)
#define STATUS_CYCL_RN_BSY_OFST       (17)
#define STATUS_CYCL_RN_BSY_MSK        (0x00000001 << STATUS_CYCL_RN_BSY_OFST)
#define STATUS_FRM_RN_BSY_OFST        (18)
#define STATUS_FRM_RN_BSY_MSK         (0x00000001 << STATUS_FRM_RN_BSY_OFST)
#define STATUS_ADC_DESERON_OFST       (19)
#define STATUS_ADC_DESERON_MSK        (0x00000001 << STATUS_ADC_DESERON_OFST)
#define STATUS_PLL_RCNFG_BSY_OFST     (20)
#define STATUS_PLL_RCNFG_BSY_MSK      (0x00000001 << STATUS_PLL_RCNFG_BSY_OFST)
#define STATUS_DT_STRMNG_BSY_OFST     (21)
#define STATUS_DT_STRMNG_BSY_MSK      (0x00000001 << STATUS_DT_STRMNG_BSY_OFST)
#define STATUS_FRM_PCKR_BSY_OFST      (22)
#define STATUS_FRM_PCKR_BSY_MSK       (0x00000001 << STATUS_FRM_PCKR_BSY_OFST)
#define STATUS_PLL_PHS_DN_OFST        (23)
#define STATUS_PLL_PHS_DN_MSK         (0x00000001 << STATUS_PLL_PHS_DN_OFST)
#define STATUS_PT_CNTRL_STTS_OFF_OFST (24)
#define STATUS_PT_CNTRL_STTS_OFF_MSK                                           \
    (0x000000FF << STATUS_PT_CNTRL_STTS_OFF_OFST)
#define STATUS_IDLE_MSK (0x677FF)

/* Look at me RO register TODO */
#define LOOK_AT_ME_REG (0x03 << MEM_MAP_SHIFT)

/* System Status RO register */
#define SYSTEM_STATUS_REG (0x04 << MEM_MAP_SHIFT)

#define SYSTEM_STATUS_DDR3_CLBRTN_OK_OFST (0)
#define SYSTEM_STATUS_DDR3_CLBRTN_OK_MSK                                       \
    (0x00000001 << SYSTEM_STATUS_DDR3_CLBRTN_OK_OFST)
#define SYSTEM_STATUS_DDR3_CLBRTN_FL_OFST (1)
#define SYSTEM_STATUS_DDR3_CLBRTN_FL_MSK                                       \
    (0x00000001 << SYSTEM_STATUS_DDR3_CLBRTN_FL_OFST)
#define SYSTEM_STATUS_DDR3_INT_DN_OFST (2)
#define SYSTEM_STATUS_DDR3_INT_DN_MSK                                          \
    (0x00000001 << SYSTEM_STATUS_DDR3_INT_DN_OFST)
#define SYSTEM_STATUS_RCNFG_PLL_LCK_OFST (3)
#define SYSTEM_STATUS_RCNFG_PLL_LCK_MSK                                        \
    (0x00000001 << SYSTEM_STATUS_RCNFG_PLL_LCK_OFST)
#define SYSTEM_STATUS_PLL_A_LCK_OFST (4)
#define SYSTEM_STATUS_PLL_A_LCK_MSK  (0x00000001 << SYSTEM_STATUS_PLL_A_LCK_OFST)

/* PLL Param (Reconfiguratble PLL Parameter) RO register TODO FIXME: Same as
 * PLL_PARAM_REG 0x50 */
// #define PLL_PARAM_REG                       (0x05 << MEM_MAP_SHIFT)

/* FIFO Data RO register TODO */
#define FIFO_DATA_REG (0x06 << MEM_MAP_SHIFT)

#define FIFO_DATA_HRDWR_SRL_NMBR_OFST (0)
#define FIFO_DATA_HRDWR_SRL_NMBR_MSK                                           \
    (0x0000FFFF << FIFO_DATA_HRDWR_SRL_NMBR_OFST)
// #define FIFO_DATA_WRD_OFST                  (16)
// #define FIFO_DATA_WRD_MSK                   (0x0000FFFF <<
// FIFO_DATA_WRD_OFST)

/* FIFO Status RO register TODO */
#define FIFO_STATUS_REG (0x07 << MEM_MAP_SHIFT)

/* FIFO Empty RO register TODO */
#define FIFO_EMPTY_REG           (0x08 << MEM_MAP_SHIFT)
#define FIFO_EMPTY_ALL_EMPTY_MSK (0xFFFFFFFF)

/* FIFO Full RO register TODO */
#define FIFO_FULL_REG (0x09 << MEM_MAP_SHIFT)

/* MCB Serial Number RO register */
#define MOD_SERIAL_NUMBER_REG (0x0A << MEM_MAP_SHIFT)

#define MOD_SERIAL_NUMBER_OFST      (0)
#define MOD_SERIAL_NUMBER_MSK       (0x000000FF << MOD_SERIAL_NUMBER_OFST)
#define MOD_SERIAL_NUMBER_VRSN_OFST (16)
#define MOD_SERIAL_NUMBER_VRSN_MSK  (0x0000003F << MOD_SERIAL_NUMBER_VRSN_OFST)

/* API Version RO register */
#define API_VERSION_REG (0x0F << MEM_MAP_SHIFT)

#define API_VERSION_OFST           (0)
#define API_VERSION_MSK            (0x00FFFFFF << API_VERSION_OFST)
#define API_VERSION_DTCTR_TYP_OFST (24)
#define API_VERSION_DTCTR_TYP_MSK  (0x000000FF << API_VERSION_DTCTR_TYP_OFST)

/* Time from Start 64 bit RO register. t = GCLK x 50 ns. Reset using
 * CONTROL_CRST. TODO */
#define TIME_FROM_START_LSB_REG (0x10 << MEM_MAP_SHIFT)
#define TIME_FROM_START_MSB_REG (0x11 << MEM_MAP_SHIFT)

/* Delay Left 64 bit RO register. t = DLY x 50 ns. TODO */
#define DELAY_LEFT_LSB_REG (0x12 << MEM_MAP_SHIFT)
#define DELAY_LEFT_MSB_REG (0x13 << MEM_MAP_SHIFT)

/* Triggers Left 64 bit RO register TODO */
#define CYCLES_LEFT_LSB_REG (0x14 << MEM_MAP_SHIFT)
#define CYCLES_LEFT_MSB_REG (0x15 << MEM_MAP_SHIFT)

/* Frames Left 64 bit RO register TODO */
#define FRAMES_LEFT_LSB_REG (0x16 << MEM_MAP_SHIFT)
#define FRAMES_LEFT_MSB_REG (0x17 << MEM_MAP_SHIFT)

/* Period Left 64 bit RO register. t = T x 50 ns. TODO */
#define PERIOD_LEFT_LSB_REG (0x18 << MEM_MAP_SHIFT)
#define PERIOD_LEFT_MSB_REG (0x19 << MEM_MAP_SHIFT)

/* Exposure Time Left 64 bit RO register */
// #define EXPTIME_LEFT_LSB_REG                (0x1A << MEM_MAP_SHIFT) // Not
//  used in FW #define EXPTIME_LEFT_MSB_REG                (0x1B <<
//  MEM_MAP_SHIFT)
//// Not used in FW

/* Gates Left 64 bit RO register */
// #define GATES_LEFT_LSB_REG                  (0x1C << MEM_MAP_SHIFT) // Not
//  used in FW #define GATES_LEFT_MSB_REG                  (0x1D <<
//  MEM_MAP_SHIFT)
//// Not used in FW

/* Data In 64 bit RO register TODO */
#define DATA_IN_LSB_REG (0x1E << MEM_MAP_SHIFT)
#define DATA_IN_MSB_REG (0x1F << MEM_MAP_SHIFT)

/* Pattern Out 64 bit RO register */
#define PATTERN_OUT_LSB_REG (0x20 << MEM_MAP_SHIFT)
#define PATTERN_OUT_MSB_REG (0x21 << MEM_MAP_SHIFT)

/* Frame number of next acquisition register (64 bit register) */
#define NEXT_FRAME_NUMB_LOCAL_LSB_REG (0x22 << MEM_MAP_SHIFT)
#define NEXT_FRAME_NUMB_LOCAL_MSB_REG (0x23 << MEM_MAP_SHIFT)

/* Frames From Start PG 64 bit RO register. Reset using CONTROL_CRST. TODO */
#define FRAMES_FROM_START_PG_LSB_REG (0x24 << MEM_MAP_SHIFT)
#define FRAMES_FROM_START_PG_MSB_REG (0x25 << MEM_MAP_SHIFT)

/* Start Frame Time (Measurement Time) 64 bit register (timestamp at a frame
 * start until reset) TODO */
#define START_FRAME_TIME_LSB_REG (0x26 << MEM_MAP_SHIFT)
#define START_FRAME_TIME_MSB_REG (0x27 << MEM_MAP_SHIFT)

/* Power Status RO register */
#define POWER_STATUS_REG (0x29 << MEM_MAP_SHIFT)

#define POWER_STATUS_ALRT_OFST (27)
#define POWER_STATUS_ALRT_MSK  (0x0000001F << POWER_STATUS_ALRT_OFST)

/* FIFO Transceiver In Status RO register */
#define FIFO_TIN_STATUS_REG                 (0x30 << MEM_MAP_SHIFT)
#define FIFO_TIN_STATUS_FIFO_EMPTY_1_OFST   (4)
#define FIFO_TIN_STATUS_FIFO_EMPTY_1_MSK    (0x00000001 << FIFO_TIN_STATUS_FIFO_EMPTY_1_OFST)
#define FIFO_TIN_STATUS_FIFO_EMPTY_2_OFST   (5)
#define FIFO_TIN_STATUS_FIFO_EMPTY_2_MSK    (0x00000001 << FIFO_TIN_STATUS_FIFO_EMPTY_2_OFST)
#define FIFO_TIN_STATUS_FIFO_EMPTY_3_OFST   (6)
#define FIFO_TIN_STATUS_FIFO_EMPTY_3_MSK    (0x00000001 << FIFO_TIN_STATUS_FIFO_EMPTY_3_OFST)
#define FIFO_TIN_STATUS_FIFO_EMPTY_4_OFST   (7)
#define FIFO_TIN_STATUS_FIFO_EMPTY_4_MSK    (0x00000001 << FIFO_TIN_STATUS_FIFO_EMPTY_4_OFST)
#define FIFO_TIN_STATUS_FIFO_EMPTY_ALL_MSK  (0x0000000F << FIFO_TIN_STATUS_FIFO_EMPTY_1_OFST)

/* FIFO Transceiver In 64 bit RO register */
#define FIFO_TIN_LSB_REG (0x31 << MEM_MAP_SHIFT)
#define FIFO_TIN_MSB_REG (0x32 << MEM_MAP_SHIFT)

/* FIFO Digital In Status RO register */
#define FIFO_DIN_STATUS_REG             (0x3B << MEM_MAP_SHIFT)
#define FIFO_DIN_STATUS_FIFO_FULL_OFST  (30)
#define FIFO_DIN_STATUS_FIFO_FULL_MSK   (0x00000001 << FIFO_DIN_STATUS_FIFO_FULL_OFST)
#define FIFO_DIN_STATUS_FIFO_EMPTY_OFST (31)
#define FIFO_DIN_STATUS_FIFO_EMPTY_MSK  (0x00000001 << FIFO_DIN_STATUS_FIFO_EMPTY_OFST)

/* FIFO Digital In 64 bit RO register */
#define FIFO_DIN_LSB_REG (0x3C << MEM_MAP_SHIFT)
#define FIFO_DIN_MSB_REG (0x3D << MEM_MAP_SHIFT)

/* SPI (Serial Peripheral Interface) DAC, HV RW register */
#define SPI_REG (0x40 << MEM_MAP_SHIFT)

#define SPI_DAC_SRL_DGTL_OTPT_OFST (0)
#define SPI_DAC_SRL_DGTL_OTPT_MSK  (0x00000001 << SPI_DAC_SRL_DGTL_OTPT_OFST)
#define SPI_DAC_SRL_CLK_OTPT_OFST  (1)
#define SPI_DAC_SRL_CLK_OTPT_MSK   (0x00000001 << SPI_DAC_SRL_CLK_OTPT_OFST)
#define SPI_DAC_SRL_CS_OTPT_OFST   (2)
#define SPI_DAC_SRL_CS_OTPT_MSK    (0x00000001 << SPI_DAC_SRL_CS_OTPT_OFST)
#define SPI_HV_SRL_DGTL_OTPT_OFST  (8)
#define SPI_HV_SRL_DGTL_OTPT_MSK   (0x00000001 << SPI_HV_SRL_DGTL_OTPT_OFST)
#define SPI_HV_SRL_CLK_OTPT_OFST   (9)
#define SPI_HV_SRL_CLK_OTPT_MSK    (0x00000001 << SPI_HV_SRL_CLK_OTPT_OFST)
#define SPI_HV_SRL_CS_OTPT_OFST    (10)
#define SPI_HV_SRL_CS_OTPT_MSK     (0x00000001 << SPI_HV_SRL_CS_OTPT_OFST)

/* ADC SPI (Serial Peripheral Interface) RW register */
#define ADC_SPI_REG (0x41 << MEM_MAP_SHIFT)

#define ADC_SPI_SRL_CLK_OTPT_OFST (0)
#define ADC_SPI_SRL_CLK_OTPT_MSK  (0x00000001 << ADC_SPI_SRL_CLK_OTPT_OFST)
#define ADC_SPI_SRL_DT_OTPT_OFST  (1)
#define ADC_SPI_SRL_DT_OTPT_MSK   (0x00000001 << ADC_SPI_SRL_DT_OTPT_OFST)
#define ADC_SPI_SRL_CS_OTPT_OFST  (2)
#define ADC_SPI_SRL_CS_OTPT_MSK   (0x0000000F << ADC_SPI_SRL_CS_OTPT_OFST)

/* ADC Offset RW register */
#define ADC_OFFSET_REG (0x42 << MEM_MAP_SHIFT)

#define ADC_OFFSET_ADC_PPLN_OFST (0)
#define ADC_OFFSET_ADC_PPLN_MSK  (0x000000FF << ADC_OFFSET_ADC_PPLN_OFST)
#define ADC_OFFSET_DBT_PPLN_OFST (16)
#define ADC_OFFSET_DBT_PPLN_MSK  (0x000000FF << ADC_OFFSET_DBT_PPLN_OFST)

/* ADC Port Invert RW register */
#define ADC_PORT_INVERT_REG (0x43 << MEM_MAP_SHIFT)

#define ADC_PORT_INVERT_0_INPT_OFST (0)
#define ADC_PORT_INVERT_0_INPT_MSK  (0x000000FF << ADC_PORT_INVERT_0_INPT_OFST)
#define ADC_PORT_INVERT_1_INPT_OFST (8)
#define ADC_PORT_INVERT_1_INPT_MSK  (0x000000FF << ADC_PORT_INVERT_1_INPT_OFST)
#define ADC_PORT_INVERT_2_INPT_OFST (16)
#define ADC_PORT_INVERT_2_INPT_MSK  (0x000000FF << ADC_PORT_INVERT_2_INPT_OFST)
#define ADC_PORT_INVERT_3_INPT_OFST (24)
#define ADC_PORT_INVERT_3_INPT_MSK  (0x000000FF << ADC_PORT_INVERT_3_INPT_OFST)

/* Dummy RW register */
#define DUMMY_REG (0x44 << MEM_MAP_SHIFT)

#define DUMMY_FIFO_CHNNL_SLCT_OFST          (0)
#define DUMMY_FIFO_CHNNL_SLCT_MSK           (0x0000003F << DUMMY_FIFO_CHNNL_SLCT_OFST)
#define DUMMY_ANLG_FIFO_RD_STRBE_OFST       (8)
#define DUMMY_ANLG_FIFO_RD_STRBE_MSK        (0x00000001 << DUMMY_ANLG_FIFO_RD_STRBE_OFST)
#define DUMMY_DGTL_FIFO_RD_STRBE_OFST       (9)
#define DUMMY_DGTL_FIFO_RD_STRBE_MSK        (0x00000001 << DUMMY_DGTL_FIFO_RD_STRBE_OFST)
#define DUMMY_TRNSCVR_FIFO_CHNNL_SLCT_OFST  (12)
#define DUMMY_TRNSCVR_FIFO_CHNNL_SLCT_MSK   (0x00000003 << DUMMY_TRNSCVR_FIFO_CHNNL_SLCT_OFST)
#define DUMMY_TRNSCVR_FIFO_RD_STRBE_OFST    (14)
#define DUMMY_TRNSCVR_FIFO_RD_STRBE_MSK     (0x00000001 << DUMMY_TRNSCVR_FIFO_RD_STRBE_OFST)

/* Receiver IP Address RW register */
#define RX_IP_REG (0x45 << MEM_MAP_SHIFT)

/* UDP Port RW register */
#define UDP_PORT_REG (0x46 << MEM_MAP_SHIFT)

#define UDP_PORT_RX_OFST (0)
#define UDP_PORT_RX_MSK  (0x0000FFFF << UDP_PORT_RX_OFST)
#define UDP_PORT_TX_OFST (16)
#define UDP_PORT_TX_MSK  (0x0000FFFF << UDP_PORT_TX_OFST)

/* Receiver Mac Address 64 bit RW register */
#define RX_MAC_LSB_REG (0x47 << MEM_MAP_SHIFT)
#define RX_MAC_MSB_REG (0x48 << MEM_MAP_SHIFT)

#define RX_MAC_LSB_OFST (0)
#define RX_MAC_LSB_MSK  (0xFFFFFFFF << RX_MAC_LSB_OFST)
#define RX_MAC_MSB_OFST (0)
#define RX_MAC_MSB_MSK  (0x0000FFFF << RX_MAC_MSB_OFST)

/* Detector/ Transmitter Mac Address 64 bit RW register */
#define TX_MAC_LSB_REG (0x49 << MEM_MAP_SHIFT)
#define TX_MAC_MSB_REG (0x4A << MEM_MAP_SHIFT)

#define TX_MAC_LSB_OFST (0)
#define TX_MAC_LSB_MSK  (0xFFFFFFFF << TX_MAC_LSB_OFST)
#define TX_MAC_MSB_OFST (0)
#define TX_MAC_MSB_MSK  (0x0000FFFF << TX_MAC_MSB_OFST)

/* Detector/ Transmitter IP Address RW register */
#define TX_IP_REG (0x4B << MEM_MAP_SHIFT)

/* Detector/ Transmitter IP Checksum RW register */
#define TX_IP_CHECKSUM_REG (0x4C << MEM_MAP_SHIFT)

#define TX_IP_CHECKSUM_OFST (0)
#define TX_IP_CHECKSUM_MSK  (0x0000FFFF << TX_IP_CHECKSUM_OFST)

/* Configuration RW register */
#define CONFIG_REG (0x4D << MEM_MAP_SHIFT)

#define CONFIG_LED_DSBL_OFST           (0)
#define CONFIG_LED_DSBL_MSK            (0x00000001 << CONFIG_LED_DSBL_OFST)
#define CONFIG_ENBLE_ANLG_OTPT_OFST     (8)
#define CONFIG_ENBLE_ANLG_OTPT_MSK      (0x00000001 << CONFIG_ENBLE_ANLG_OTPT_OFST)
#define CONFIG_ENBLE_DGTL_OTPT_OFST    (9)
#define CONFIG_ENBLE_DGTL_OTPT_MSK     (0x00000001 << CONFIG_ENBLE_DGTL_OTPT_OFST)
#define CONFIG_ENBLE_TRNSCVR_OTPT_OFST (10)
#define CONFIG_ENBLE_TRNSCVR_OTPT_MSK                                          \
    (0x00000001 << CONFIG_ENBLE_TRNSCVR_OTPT_OFST)
#define CONFIG_GB10_SND_UDP_OFST (12)
#define CONFIG_GB10_SND_UDP_MSK  (0x00000001 << CONFIG_GB10_SND_UDP_OFST)

/* External Signal RW register */
#define EXT_SIGNAL_REG (0x4E << MEM_MAP_SHIFT)

#define EXT_SIGNAL_OFST      (0)
#define EXT_SIGNAL_MSK       (0x00000001 << EXT_SIGNAL_OFST)
#define EXT_SIGNAL_AUTO_VAL  ((0x0 << EXT_SIGNAL_OFST) & EXT_SIGNAL_MSK)
#define EXT_SIGNAL_TRGGR_VAL ((0x1 << EXT_SIGNAL_OFST) & EXT_SIGNAL_MSK)

/* Control RW register */
#define CONTROL_REG (0x4F << MEM_MAP_SHIFT)

#define CONTROL_STRT_ACQSTN_OFST (0)
#define CONTROL_STRT_ACQSTN_MSK  (0x00000001 << CONTROL_STRT_ACQSTN_OFST)
#define CONTROL_STP_ACQSTN_OFST  (1)
#define CONTROL_STP_ACQSTN_MSK   (0x00000001 << CONTROL_STP_ACQSTN_OFST)
// #define CONTROL_STRT_FF_TST_OFST            (2)
// #define CONTROL_STRT_FF_TST_MSK             (0x00000001 <<
//  CONTROL_STRT_FF_TST_OFST) #define CONTROL_STP_FF_TST_OFST             (3)
// #define CONTROL_STP_FF_TST_MSK              (0x00000001 <<
//  CONTROL_STP_FF_TST_OFST) #define CONTROL_STRT_RDT_OFST               (4)
// #define CONTROL_STRT_RDT_MSK                (0x00000001 <<
//  CONTROL_STRT_RDT_OFST) #define CONTROL_STP_RDT_OFST                (5)
//  #define CONTROL_STP_RDT_MSK                 (0x00000001 <<
//  CONTROL_STP_RDT_OFST)
#define CONTROL_STRT_EXPSR_OFST (6)
#define CONTROL_STRT_EXPSR_MSK  (0x00000001 << CONTROL_STRT_EXPSR_OFST)
// #define CONTROL_STP_EXPSR_OFST              (7)
// #define CONTROL_STP_EXPSR_MSK               (0x00000001 <<
//  CONTROL_STP_RDT_OFST) #define CONTROL_STRT_TRN_OFST               (8)
//  #define CONTROL_STRT_TRN_MSK                (0x00000001 <<
//  CONTROL_STRT_RDT_OFST)
// #define CONTROL_STP_TRN_OFST                (9)
// #define CONTROL_STP_TRN_MSK                 (0x00000001 <<
//  CONTROL_STP_RDT_OFST)
#define CONTROL_CRE_RST_OFST    (10)
#define CONTROL_CRE_RST_MSK     (0x00000001 << CONTROL_CRE_RST_OFST)
#define CONTROL_PRPHRL_RST_OFST (11) // Only GBE10?
#define CONTROL_PRPHRL_RST_MSK  (0x00000001 << CONTROL_PRPHRL_RST_OFST)
#define CONTROL_MMRY_RST_OFST   (12)
#define CONTROL_MMRY_RST_MSK    (0x00000001 << CONTROL_MMRY_RST_OFST)
// #define CONTROL_PLL_RCNFG_WR_OFST           (13)
// #define CONTROL_PLL_RCNFG_WR_MSK            (0x00000001 <<
//  CONTROL_PLL_RCNFG_WR_OFST)
#define CONTROL_SND_10GB_PCKT_OFST   (14)
#define CONTROL_SND_10GB_PCKT_MSK    (0x00000001 << CONTROL_SND_10GB_PCKT_OFST)
#define CONTROL_CLR_ACQSTN_FIFO_OFST (15)
#define CONTROL_CLR_ACQSTN_FIFO_MSK  (0x00000001 << CONTROL_CLR_ACQSTN_FIFO_OFST)

/* Reconfiguratble PLL Paramater RW register */
#define PLL_PARAM_REG (0x50 << MEM_MAP_SHIFT)

/* Reconfiguratble PLL Control RW regiser */
#define PLL_CNTRL_REG (0x51 << MEM_MAP_SHIFT)

#define PLL_CNTRL_RCNFG_PRMTR_RST_OFST (0)
#define PLL_CNTRL_RCNFG_PRMTR_RST_MSK                                          \
    (0x00000001 << PLL_CNTRL_RCNFG_PRMTR_RST_OFST)
#define PLL_CNTRL_WR_PRMTR_OFST (2)
#define PLL_CNTRL_WR_PRMTR_MSK  (0x00000001 << PLL_CNTRL_WR_PRMTR_OFST)
#define PLL_CNTRL_PLL_RST_OFST  (3)
#define PLL_CNTRL_PLL_RST_MSK   (0x00000001 << PLL_CNTRL_PLL_RST_OFST)
#define PLL_CNTRL_ADDR_OFST     (16)
#define PLL_CNTRL_ADDR_MSK      (0x0000003F << PLL_CNTRL_ADDR_OFST)

/* Pattern Control RW register */
#define PATTERN_CNTRL_REG (0x52 << MEM_MAP_SHIFT)

#define PATTERN_CNTRL_WR_OFST   (0)
#define PATTERN_CNTRL_WR_MSK    (0x00000001 << PATTERN_CNTRL_WR_OFST)
#define PATTERN_CNTRL_RD_OFST   (1)
#define PATTERN_CNTRL_RD_MSK    (0x00000001 << PATTERN_CNTRL_RD_OFST)
#define PATTERN_CNTRL_ADDR_OFST (16)
#define PATTERN_CNTRL_ADDR_MSK  (0x00001FFF << PATTERN_CNTRL_ADDR_OFST)

/* Pattern Limit RW regiser */
#define PATTERN_LIMIT_REG (0x53 << MEM_MAP_SHIFT)

#define PATTERN_LIMIT_STRT_OFST (0)
#define PATTERN_LIMIT_STRT_MSK  (0x00001FFF << PATTERN_LIMIT_STRT_OFST)
#define PATTERN_LIMIT_STP_OFST  (16)
#define PATTERN_LIMIT_STP_MSK   (0x00001FFF << PATTERN_LIMIT_STP_OFST)

/* Pattern Loop 0 Address RW regiser */
#define PATTERN_LOOP_0_ADDR_REG (0x54 << MEM_MAP_SHIFT)

#define PATTERN_LOOP_0_ADDR_STRT_OFST (0)
#define PATTERN_LOOP_0_ADDR_STRT_MSK                                           \
    (0x00001FFF << PATTERN_LOOP_0_ADDR_STRT_OFST)
#define PATTERN_LOOP_0_ADDR_STP_OFST (16)
#define PATTERN_LOOP_0_ADDR_STP_MSK  (0x00001FFF << PATTERN_LOOP_0_ADDR_STP_OFST)

/* Pattern Loop 0 Iteration RW regiser */
#define PATTERN_LOOP_0_ITERATION_REG (0x55 << MEM_MAP_SHIFT)

/* Pattern Loop 1 Address RW regiser */
#define PATTERN_LOOP_1_ADDR_REG (0x56 << MEM_MAP_SHIFT)

#define PATTERN_LOOP_1_ADDR_STRT_OFST (0)
#define PATTERN_LOOP_1_ADDR_STRT_MSK                                           \
    (0x00001FFF << PATTERN_LOOP_1_ADDR_STRT_OFST)
#define PATTERN_LOOP_1_ADDR_STP_OFST (16)
#define PATTERN_LOOP_1_ADDR_STP_MSK  (0x00001FFF << PATTERN_LOOP_1_ADDR_STP_OFST)

/* Pattern Loop 1 Iteration RW regiser */
#define PATTERN_LOOP_1_ITERATION_REG (0x57 << MEM_MAP_SHIFT)

/* Pattern Loop 2 Address RW regiser */
#define PATTERN_LOOP_2_ADDR_REG (0x58 << MEM_MAP_SHIFT)

#define PATTERN_LOOP_2_ADDR_STRT_OFST (0)
#define PATTERN_LOOP_2_ADDR_STRT_MSK                                           \
    (0x00001FFF << PATTERN_LOOP_2_ADDR_STRT_OFST)
#define PATTERN_LOOP_2_ADDR_STP_OFST (16)
#define PATTERN_LOOP_2_ADDR_STP_MSK  (0x00001FFF << PATTERN_LOOP_2_ADDR_STP_OFST)

/* Pattern Loop 2 Iteration RW regiser */
#define PATTERN_LOOP_2_ITERATION_REG (0x59 << MEM_MAP_SHIFT)

/* Pattern Wait 0 RW regiser */
#define PATTERN_WAIT_0_ADDR_REG (0x5A << MEM_MAP_SHIFT)

#define PATTERN_WAIT_0_ADDR_OFST (0)
#define PATTERN_WAIT_0_ADDR_MSK  (0x00001FFF << PATTERN_WAIT_0_ADDR_OFST)
// FIXME: is mask 3FF

/* Pattern Wait 1 RW regiser */
#define PATTERN_WAIT_1_ADDR_REG (0x5B << MEM_MAP_SHIFT)

#define PATTERN_WAIT_1_ADDR_OFST (0)
#define PATTERN_WAIT_1_ADDR_MSK  (0x00001FFF << PATTERN_WAIT_1_ADDR_OFST)

/* Pattern Wait 2 RW regiser */
#define PATTERN_WAIT_2_ADDR_REG (0x5C << MEM_MAP_SHIFT)

#define PATTERN_WAIT_2_ADDR_OFST (0)
#define PATTERN_WAIT_2_ADDR_MSK  (0x00001FFF << PATTERN_WAIT_2_ADDR_OFST)

/* Samples RW register */
#define SAMPLES_REG (0x5D << MEM_MAP_SHIFT)

#define SAMPLES_DIGITAL_OFST (0)
#define SAMPLES_DIGITAL_MSK  (0x0000FFFF << SAMPLES_DIGITAL_OFST)
#define SAMPLES_ANALOG_OFST  (16)
#define SAMPLES_ANALOG_MSK   (0x0000FFFF << SAMPLES_ANALOG_OFST)

/** Power RW register */
#define POWER_REG (0x5E << MEM_MAP_SHIFT)

#define POWER_ENBL_VLTG_RGLTR_OFST  (16)
#define POWER_ENBL_VLTG_RGLTR_MSK   (0x0000001F << POWER_ENBL_VLTG_RGLTR_OFST)
#define POWER_HV_INTERNAL_SLCT_OFST (31)
#define POWER_HV_INTERNAL_SLCT_MSK  (0x00000001 << POWER_HV_INTERNAL_SLCT_OFST)

/* Number of samples from transceiver RW register */
#define SAMPLES_TRANSCEIVER_REG  (0x5F << MEM_MAP_SHIFT)
#define SAMPLES_TRANSCEIVER_OFST (0)
#define SAMPLES_TRANSCEIVER_MSK  (0x0000FFFF << SAMPLES_TRANSCEIVER_OFST)

/* Delay 64 bit RW register. t = DLY x 50 ns. */
#define DELAY_LSB_REG (0x60 << MEM_MAP_SHIFT)
#define DELAY_MSB_REG (0x61 << MEM_MAP_SHIFT)

/* Triggers 64 bit RW register */
#define CYCLES_LSB_REG (0x62 << MEM_MAP_SHIFT)
#define CYCLES_MSB_REG (0x63 << MEM_MAP_SHIFT)

/* Frames 64 bit RW register */
#define FRAMES_LSB_REG (0x64 << MEM_MAP_SHIFT)
#define FRAMES_MSB_REG (0x65 << MEM_MAP_SHIFT)

/* Period 64 bit RW register */
#define PERIOD_LSB_REG (0x66 << MEM_MAP_SHIFT)
#define PERIOD_MSB_REG (0x67 << MEM_MAP_SHIFT)

/* Period 64 bit RW register */
// #define EXPTIME_LSB_REG    			    (0x68 << MEM_MAP_SHIFT) //
//  Not used in FW #define EXPTIME_MSB_REG    			    (0x69 <<
//  MEM_MAP_SHIFT)                 // Not used in FW

/* Gates 64 bit RW register */
// #define GATES_LSB_REG                      (0x6A << MEM_MAP_SHIFT) // Not
// used
//  in FW #define GATES_MSB_REG                      (0x6B << MEM_MAP_SHIFT) //
//  Not used in FW

/* Pattern IO Control 64 bit RW regiser
 * Each bit configured as output(1)/ input(0) */
#define PATTERN_IO_CNTRL_LSB_REG (0x6C << MEM_MAP_SHIFT)
#define PATTERN_IO_CNTRL_MSB_REG (0x6D << MEM_MAP_SHIFT)

/* Pattern IO Clock Control 64 bit RW regiser
 * When bit n enabled (1), clocked output for DIO[n] (T run clock)
 * When bit n disabled (0), Dio[n] driven by its pattern output */
#define PATTERN_IO_CLK_CNTRL_LSB_REG (0x6E << MEM_MAP_SHIFT)
#define PATTERN_IO_CLK_CNTRL_MSB_REG (0x6F << MEM_MAP_SHIFT)

/* Pattern In 64 bit RW register */
#define PATTERN_IN_LSB_REG (0x70 << MEM_MAP_SHIFT)
#define PATTERN_IN_MSB_REG (0x71 << MEM_MAP_SHIFT)

/* Pattern Wait Timer 0 64 bit RW register. t = PWT1 x T run clock */
#define PATTERN_WAIT_TIMER_0_LSB_REG (0x72 << MEM_MAP_SHIFT)
#define PATTERN_WAIT_TIMER_0_MSB_REG (0x73 << MEM_MAP_SHIFT)

/* Pattern Wait Timer 1 64 bit RW register. t = PWT2 x T run clock */
#define PATTERN_WAIT_TIMER_1_LSB_REG (0x74 << MEM_MAP_SHIFT)
#define PATTERN_WAIT_TIMER_1_MSB_REG (0x75 << MEM_MAP_SHIFT)

/* Pattern Wait Timer 2 64 bit RW register. t = PWT3 x T run clock */
#define PATTERN_WAIT_TIMER_2_LSB_REG (0x76 << MEM_MAP_SHIFT)
#define PATTERN_WAIT_TIMER_2_MSB_REG (0x77 << MEM_MAP_SHIFT)

/* Readout enable RW register */
#define READOUT_10G_ENABLE_REG (0x79 << MEM_MAP_SHIFT)

#define READOUT_10G_ENABLE_ANLG_OFST    (0)
#define READOUT_10G_ENABLE_ANLG_MSK     (0x000000FF << READOUT_10G_ENABLE_ANLG_OFST)
#define READOUT_10G_ENABLE_DGTL_OFST    (8)
#define READOUT_10G_ENABLE_DGTL_MSK     (0x00000001 << READOUT_10G_ENABLE_DGTL_OFST)
#define READOUT_10G_ENABLE_TRNSCVR_OFST (9)
#define READOUT_10G_ENABLE_TRNSCVR_MSK                                         \
    (0x0000000F << READOUT_10G_ENABLE_TRNSCVR_OFST)

/* Digital Bit External Trigger RW register */
#define DBIT_EXT_TRG_REG (0x7B << MEM_MAP_SHIFT)

#define DBIT_EXT_TRG_SRC_OFST      (0)
#define DBIT_EXT_TRG_SRC_MSK       (0x0000003F << DBIT_EXT_TRG_SRC_OFST)
#define DBIT_EXT_TRG_OPRTN_MD_OFST (16)
#define DBIT_EXT_TRG_OPRTN_MD_MSK  (0x00000001 << DBIT_EXT_TRG_OPRTN_MD_OFST)

/* Pin Delay 0 RW register */
#define OUTPUT_DELAY_0_REG              (0x7C << MEM_MAP_SHIFT)
#define OUTPUT_DELAY_0_OTPT_STTNG_STEPS (25)
#define OUTPUT_DELAY_0_OTPT_STTNG_OFST                                         \
    (0) // t = OTPT_STTNG * 25 ps, max for Cyclone V = 775 ps
#define OUTPUT_DELAY_0_OTPT_STTNG_MSK                                          \
    (0x0000001F << OUTPUT_DELAY_0_OTPT_STTNG_OFST)
// 1: load dynamic output settings, 0: trigger start of dynamic output delay
// configuration pn falling edge of ODT (output delay trigger) bit
#define OUTPUT_DELAY_0_OTPT_TRGGR_OFST (31)
#define OUTPUT_DELAY_0_OTPT_TRGGR_MSK                                          \
    (0x00000001 << OUTPUT_DELAY_0_OTPT_TRGGR_OFST)
#define OUTPUT_DELAY_0_OTPT_TRGGR_LD_VAL   (1)
#define OUTPUT_DELAY_0_OTPT_TRGGR_STRT_VAL (0)

/* Pin Delay 1 RW register
 * Each bit configured as enable for dynamic output delay configuration */
#define PIN_DELAY_1_REG (0x7D << MEM_MAP_SHIFT)

/** Pattern Mask 64 bit RW regiser */
#define PATTERN_MASK_LSB_REG (0x80 << MEM_MAP_SHIFT)
#define PATTERN_MASK_MSB_REG (0x81 << MEM_MAP_SHIFT)

/** Pattern Set 64 bit RW regiser */
#define PATTERN_SET_LSB_REG (0x82 << MEM_MAP_SHIFT)
#define PATTERN_SET_MSB_REG (0x83 << MEM_MAP_SHIFT)

/* Pattern Loop 3 Address RW regiser */
#define PATTERN_LOOP_3_ADDR_REG (0x84 << MEM_MAP_SHIFT)

#define PATTERN_LOOP_3_ADDR_STRT_OFST (0)
#define PATTERN_LOOP_3_ADDR_STRT_MSK                                           \
    (0x00001FFF << PATTERN_LOOP_3_ADDR_STRT_OFST)
#define PATTERN_LOOP_3_ADDR_STP_OFST (16)
#define PATTERN_LOOP_3_ADDR_STP_MSK  (0x00001FFF << PATTERN_LOOP_3_ADDR_STP_OFST)

/* Pattern Loop 3 Iteration RW regiser */
#define PATTERN_LOOP_3_ITERATION_REG (0x85 << MEM_MAP_SHIFT)

/* Pattern Loop 4 Address RW regiser */
#define PATTERN_LOOP_4_ADDR_REG (0x86 << MEM_MAP_SHIFT)

#define PATTERN_LOOP_4_ADDR_STRT_OFST (0)
#define PATTERN_LOOP_4_ADDR_STRT_MSK                                           \
    (0x00001FFF << PATTERN_LOOP_4_ADDR_STRT_OFST)
#define PATTERN_LOOP_4_ADDR_STP_OFST (16)
#define PATTERN_LOOP_4_ADDR_STP_MSK  (0x00001FFF << PATTERN_LOOP_4_ADDR_STP_OFST)

/* Pattern Loop 4 Iteration RW regiser */
#define PATTERN_LOOP_4_ITERATION_REG (0x87 << MEM_MAP_SHIFT)

/* Pattern Loop 5 Address RW regiser */
#define PATTERN_LOOP_5_ADDR_REG (0x88 << MEM_MAP_SHIFT)

#define PATTERN_LOOP_5_ADDR_STRT_OFST (0)
#define PATTERN_LOOP_5_ADDR_STRT_MSK                                           \
    (0x00001FFF << PATTERN_LOOP_5_ADDR_STRT_OFST)
#define PATTERN_LOOP_5_ADDR_STP_OFST (16)
#define PATTERN_LOOP_5_ADDR_STP_MSK  (0x00001FFF << PATTERN_LOOP_5_ADDR_STP_OFST)

/* Pattern Loop 5 Iteration RW regiser */
#define PATTERN_LOOP_5_ITERATION_REG (0x89 << MEM_MAP_SHIFT)

/* Pattern Wait 3 RW regiser */
#define PATTERN_WAIT_3_ADDR_REG (0x8A << MEM_MAP_SHIFT)

#define PATTERN_WAIT_3_ADDR_OFST (0)
#define PATTERN_WAIT_3_ADDR_MSK  (0x00001FFF << PATTERN_WAIT_3_ADDR_OFST)

/* Pattern Wait 4 RW regiser */
#define PATTERN_WAIT_4_ADDR_REG (0x8B << MEM_MAP_SHIFT)

#define PATTERN_WAIT_4_ADDR_OFST (0)
#define PATTERN_WAIT_4_ADDR_MSK  (0x00001FFF << PATTERN_WAIT_4_ADDR_OFST)

/* Pattern Wait 5 RW regiser */
#define PATTERN_WAIT_5_ADDR_REG (0x8C << MEM_MAP_SHIFT)

#define PATTERN_WAIT_5_ADDR_OFST (0)
#define PATTERN_WAIT_5_ADDR_MSK  (0x00001FFF << PATTERN_WAIT_5_ADDR_OFST)

/* Pattern Wait Timer 3 64 bit RW register. t = PWT1 x T run clock */
#define PATTERN_WAIT_TIMER_3_LSB_REG (0x8D << MEM_MAP_SHIFT)
#define PATTERN_WAIT_TIMER_3_MSB_REG (0x8E << MEM_MAP_SHIFT)

/* Pattern Wait Timer 4 64 bit RW register. t = PWT1 x T run clock */
#define PATTERN_WAIT_TIMER_4_LSB_REG (0x8F << MEM_MAP_SHIFT)
#define PATTERN_WAIT_TIMER_4_MSB_REG (0x90 << MEM_MAP_SHIFT)

/* Pattern Wait Timer 5 64 bit RW register. t = PWT1 x T run clock */
#define PATTERN_WAIT_TIMER_5_LSB_REG (0x91 << MEM_MAP_SHIFT)
#define PATTERN_WAIT_TIMER_5_MSB_REG (0x92 << MEM_MAP_SHIFT)

/* Slow ADC SPI Value RO register */
#define ADC_SLOW_DATA_REG (0x93 << MEM_MAP_SHIFT)

/* Slow ADC SPI Value Config register */
#define ADC_SLOW_CFG_REG (0x94 << MEM_MAP_SHIFT)
/** Read back CFG Register */
#define ADC_SLOW_CFG_RB_OFST (2)
#define ADC_SLOW_CFG_RB_MSK  (0x00000001 << ADC_SLOW_CFG_RB_OFST)

/** Channel sequencer */
#define ADC_SLOW_CFG_SEQ_OFST (3)
#define ADC_SLOW_CFG_SEQ_MSK  (0x00000003 << ADC_SLOW_CFG_SEQ_OFST)
#define ADC_SLOW_CFG_SEQ_DSBLE_VAL                                             \
    ((0x0 << ADC_SLOW_CFG_SEQ_OFST) & ADC_SLOW_CFG_SEQ_MSK)
#define ADC_SLOW_CFG_SEQ_UPDTE_DRNG_SQNCE_VAL                                  \
    ((0x1 << ADC_SLOW_CFG_SEQ_OFST) & ADC_SLOW_CFG_SEQ_MSK)
#define ADC_SLOW_CFG_SEQ_SCN_WTH_TMP_VAL                                       \
    ((0x2 << ADC_SLOW_CFG_SEQ_OFST) & ADC_SLOW_CFG_SEQ_MSK)
#define ADC_SLOW_CFG_SEQ_SCN_WTHT_TMP_VAL                                      \
    ((0x3 << ADC_SLOW_CFG_SEQ_OFST) & ADC_SLOW_CFG_SEQ_MSK)

/** Reference/ buffer selection */
#define ADC_SLOW_CFG_REF_OFST (5)
#define ADC_SLOW_CFG_REF_MSK  (0x00000007 << ADC_SLOW_CFG_REF_OFST)
/** Internal reference. REF = 2.5V buffered output. Temperature sensor enabled.
 */
#define ADC_SLOW_CFG_REF_INT_2500MV_VAL                                        \
    ((0x0 << ADC_SLOW_CFG_REF_OFST) & ADC_SLOW_CFG_REF_OFST)
/** Internal reference. REF = 4.096V buffered output. Temperature sensor
 * enabled. */
#define ADC_SLOW_CFG_REF_INT_4096MV_VAL                                        \
    ((0x1 << ADC_SLOW_CFG_REF_OFST) & ADC_SLOW_CFG_REF_MSK)
/** External reference. Temperature sensor enabled. Internal buffer disabled. */
#define ADC_SLOW_CFG_REF_EXT_TMP_VAL                                           \
    ((0x2 << ADC_SLOW_CFG_REF_OFST) & ADC_SLOW_CFG_REF_MSK)
/** External reference. Temperature sensor enabled. Internal buffer enabled. */
#define ADC_SLOW_CFG_REF_EXT_TMP_INTBUF_VAL                                    \
    ((0x3 << ADC_SLOW_CFG_REF_OFST) & ADC_SLOW_CFG_REF_MSK)
/** External reference. Temperature sensor disabled. Internal buffer disabled.
 */
#define ADC_SLOW_CFG_REF_EXT_VAL                                               \
    ((0x6 << ADC_SLOW_CFG_REF_OFST) & ADC_SLOW_CFG_REF_MSK)
/** External reference. Temperature sensor disabled. Internal buffer enabled. */
#define ADC_SLOW_CFG_REF_EXT_INTBUF_VAL                                        \
    ((0x7 << ADC_SLOW_CFG_REF_OFST) & ADC_SLOW_CFG_REF_MSK)

/** bandwidth of low pass filter */
#define ADC_SLOW_CFG_BW_OFST (8)
#define ADC_SLOW_CFG_BW_MSK  (0x00000001 << ADC_SLOW_CFG_REF_OFST)
#define ADC_SLOW_CFG_BW_ONE_FOURTH_VAL                                         \
    ((0x0 << ADC_SLOW_CFG_BW_OFST) & ADC_SLOW_CFG_BW_MSK)
#define ADC_SLOW_CFG_BW_FULL_VAL                                               \
    ((0x1 << ADC_SLOW_CFG_BW_OFST) & ADC_SLOW_CFG_BW_MSK)

/** input channel selection IN0 - IN7 */
#define ADC_SLOW_CFG_IN_OFST (9)
#define ADC_SLOW_CFG_IN_MSK  (0x00000007 << ADC_SLOW_CFG_IN_OFST)

/** input channel configuration */
#define ADC_SLOW_CFG_INCC_OFST (12)
#define ADC_SLOW_CFG_INCC_MSK  (0x00000007 << ADC_SLOW_CFG_INCC_OFST)
#define ADC_SLOW_CFG_INCC_BPLR_DFFRNTL_PRS_VAL                                 \
    ((0x0 << ADC_SLOW_CFG_INCC_OFST) & ADC_SLOW_CFG_INCC_MSK)
#define ADC_SLOW_CFG_INCC_BPLR_IN_COM_VAL                                      \
    ((0x2 << ADC_SLOW_CFG_INCC_OFST) & ADC_SLOW_CFG_INCC_MSK)
#define ADC_SLOW_CFG_INCC_TMP_VAL                                              \
    ((0x3 << ADC_SLOW_CFG_INCC_OFST) & ADC_SLOW_CFG_INCC_MSK)
#define ADC_SLOW_CFG_INCC_UNPLR_DFFRNTL_PRS_VAL                                \
    ((0x4 << ADC_SLOW_CFG_INCC_OFST) & ADC_SLOW_CFG_INCC_MSK)
#define ADC_SLOW_CFG_INCC_UNPLR_IN_COM_VAL                                     \
    ((0x6 << ADC_SLOW_CFG_INCC_OFST) & ADC_SLOW_CFG_INCC_MSK)
#define ADC_SLOW_CFG_INCC_UNPLR_IN_GND_VAL                                     \
    ((0x7 << ADC_SLOW_CFG_INCC_OFST) & ADC_SLOW_CFG_INCC_MSK)

/** configuration update */
#define ADC_SLOW_CFG_CFG_OFST (15)
#define ADC_SLOW_CFG_CFG_MSK  (0x00000001 << ADC_SLOW_CFG_CFG_OFST)
#define ADC_SLOW_CFG_CFG_NO_UPDATE_VAL                                         \
    ((0x0 << ADC_SLOW_CFG_CFG_OFST) & ADC_SLOW_CFG_CFG_MSK)
#define ADC_SLOW_CFG_CFG_OVRWRTE_VAL                                           \
    ((0x1 << ADC_SLOW_CFG_CFG_OFST) & ADC_SLOW_CFG_CFG_MSK)

/* Slow ADC SPI Value Control register */
#define ADC_SLOW_CTRL_REG (0x95 << MEM_MAP_SHIFT)

#define ADC_SLOW_CTRL_STRT_OFST (0)
#define ADC_SLOW_CTRL_STRT_MSK  (0x00000001 << ADC_SLOW_CTRL_STRT_OFST)
#define ADC_SLOW_CTRL_DONE_OFST (1)
#define ADC_SLOW_CTRL_DONE_MSK  (0x00000001 << ADC_SLOW_CTRL_DONE_OFST)

/** I2C Control register */
#define I2C_TRANSFER_COMMAND_FIFO_REG (0x100 << MEM_MAP_SHIFT)
#define I2C_RX_DATA_FIFO_REG          (0x101 << MEM_MAP_SHIFT)
#define I2C_CONTROL_REG               (0x102 << MEM_MAP_SHIFT)
#define I2C_STATUS_REG                (0x105 << MEM_MAP_SHIFT)
#define I2C_RX_DATA_FIFO_LEVEL_REG    (0x107 << MEM_MAP_SHIFT)
#define I2C_SCL_LOW_COUNT_REG         (0x108 << MEM_MAP_SHIFT)
#define I2C_SCL_HIGH_COUNT_REG        (0x109 << MEM_MAP_SHIFT)
#define I2C_SDA_HOLD_REG              (0x10A << MEM_MAP_SHIFT)
// fixme: upto 0x10f

/* Round Robin  */
#define RXR_ENDPOINT_START_REG (0x1000 << MEM_MAP_SHIFT)

// clang-format on
