// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once

/* Definitions for FPGA*/
#define MEM_MAP_SHIFT (11)

/** Gain register */
#define GAIN_REG (0x10 << MEM_MAP_SHIFT)

#define GAIN_CONFGAIN_OFST (0)
#define GAIN_CONFGAIN_MSK  (0x000000FF << GAIN_CONFGAIN_OFST)
#define GAIN_CONFGAIN_HGH_GAIN_VAL                                             \
    ((0x0 << GAIN_CONFGAIN_OFST) & GAIN_CONFGAIN_MSK)
#define GAIN_CONFGAIN_DYNMC_GAIN_VAL                                           \
    ((0x8 << GAIN_CONFGAIN_OFST) & GAIN_CONFGAIN_MSK)
#define GAIN_CONFGAIN_LW_GAIN_VAL                                              \
    ((0x6 << GAIN_CONFGAIN_OFST) & GAIN_CONFGAIN_MSK)
#define GAIN_CONFGAIN_MDM_GAIN_VAL                                             \
    ((0x2 << GAIN_CONFGAIN_OFST) & GAIN_CONFGAIN_MSK)
#define GAIN_CONFGAIN_VRY_HGH_GAIN_VAL                                         \
    ((0x1 << GAIN_CONFGAIN_OFST) & GAIN_CONFGAIN_MSK)

/** Flow Control register */
// #define FLOW_CONTROL_REG                (0x11 << MEM_MAP_SHIFT)

/** Flow Status register */
// #define FLOW_STATUS_REG                 (0x12 << MEM_MAP_SHIFT)

/** Frame register */
// #define FRAME_REG                       (0x13 << MEM_MAP_SHIFT)

/** Multi Purpose register */
#define MULTI_PURPOSE_REG (0x14 << MEM_MAP_SHIFT)

#define PHS_STP_OFST            (0)
#define PHS_STP_MSK             (0x00000001 << PHS_STP_OFST)
#define RST_CNTR_OFST           (2)
#define RST_CNTR_MSK            (0x00000001 << RST_CNTR_OFST)
#define SW1_OFST                (5)
#define SW1_MSK                 (0x00000001 << SW1_OFST)
#define WRT_BCK_OFST            (6)
#define WRT_BCK_MSK             (0x00000001 << WRT_BCK_OFST)
#define RST_OFST                (7)
#define RST_MSK                 (0x00000001 << RST_OFST)
#define PLL_CLK_SL_OFST         (8)
#define PLL_CLK_SL_MSK          (0x00000007 << PLL_CLK_SL_OFST)
#define PLL_CLK_SL_MSTR_VAL     ((0x1 << PLL_CLK_SL_OFST) & PLL_CLK_SL_MSK)
#define PLL_CLK_SL_MSTR_ADC_VAL ((0x2 << PLL_CLK_SL_OFST) & PLL_CLK_SL_MSK)
#define PLL_CLK_SL_SLV_VAL      ((0x3 << PLL_CLK_SL_OFST) & PLL_CLK_SL_MSK)
#define PLL_CLK_SL_SLV_ADC_VAL  ((0x4 << PLL_CLK_SL_OFST) & PLL_CLK_SL_MSK)
#define ENT_RSTN_OFST           (11)
#define ENT_RSTN_MSK            (0x00000001 << ENT_RSTN_OFST)
#define INT_RSTN_OFST           (12)
#define INT_RSTN_MSK            (0x00000001 << INT_RSTN_OFST)
#define DGTL_TST_OFST           (14)
#define DGTL_TST_MSK            (0x00000001 << DGTL_TST_OFST)
#define CHNG_AT_PWR_ON_OFST     (15) // Not used in SW
#define CHNG_AT_PWR_ON_MSK      (0x00000001 << CHNG_AT_PWR_ON_OFST) // Not used in SW
#define RST_TO_SW1_DLY_OFST     (16)
#define RST_TO_SW1_DLY_MSK      (0x0000000F << RST_TO_SW1_DLY_OFST)
#define STRT_ACQ_DLY_OFST       (20)
#define STRT_ACQ_DLY_MSK        (0x0000000F << STRT_ACQ_DLY_OFST)

/** DAQ register */
#define DAQ_REG (0x15 << MEM_MAP_SHIFT)

#define DAQ_TKN_TMNG_OFST (0)
#define DAQ_TKN_TMNG_MSK  (0x0000FFFF << DAQ_TKN_TMNG_OFST)
#define DAQ_TKN_TMNG_BRD_RVSN_1_VAL                                            \
    ((0x1f16 << DAQ_TKN_TMNG_OFST) & DAQ_TKN_TMNG_MSK)
#define DAQ_TKN_TMNG_BRD_RVSN_2_VAL                                            \
    ((0x1f10 << DAQ_TKN_TMNG_OFST) & DAQ_TKN_TMNG_MSK)
#define DAQ_PCKT_LNGTH_OFST (16)
#define DAQ_PCKT_LNGTH_MSK  (0x0000FFFF << DAQ_PCKT_LNGTH_OFST)
#define DAQ_PCKT_LNGTH_NO_ROI_VAL                                              \
    ((0x0013f << DAQ_PCKT_LNGTH_OFST) & DAQ_PCKT_LNGTH_MSK)
#define DAQ_PCKT_LNGTH_ROI_VAL                                                 \
    ((0x0007f << DAQ_PCKT_LNGTH_OFST) & DAQ_PCKT_LNGTH_MSK)

/** Time From Start register */
// #define TIME_FROM_START_REG             (0x16 << MEM_MAP_SHIFT)

/** DAC Control register */
#define SPI_REG (0x17 << MEM_MAP_SHIFT)

#define SPI_DAC_SRL_CS_OTPT_OFST   (0)
#define SPI_DAC_SRL_CS_OTPT_MSK    (0x00000001 << SPI_DAC_SRL_CS_OTPT_OFST)
#define SPI_DAC_SRL_CLK_OTPT_OFST  (1)
#define SPI_DAC_SRL_CLK_OTPT_MSK   (0x00000001 << SPI_DAC_SRL_CLK_OTPT_OFST)
#define SPI_DAC_SRL_DGTL_OTPT_OFST (2)
#define SPI_DAC_SRL_DGTL_OTPT_MSK  (0x00000001 << SPI_DAC_SRL_DGTL_OTPT_OFST)

/** ADC SPI register */
#define ADC_SPI_REG (0x18 << MEM_MAP_SHIFT)

#define ADC_SPI_SRL_CLK_OTPT_OFST (0)
#define ADC_SPI_SRL_CLK_OTPT_MSK  (0x00000001 << ADC_SPI_SRL_CLK_OTPT_OFST)
#define ADC_SPI_SRL_DT_OTPT_OFST  (1)
#define ADC_SPI_SRL_DT_OTPT_MSK   (0x00000001 << ADC_SPI_SRL_DT_OTPT_OFST)
#define ADC_SPI_SRL_CS_OTPT_OFST  (2)
#define ADC_SPI_SRL_CS_OTPT_MSK   (0x0000001F << ADC_SPI_SRL_CS_OTPT_OFST)

/** ADC Sync register */
#define ADC_SYNC_REG (0x19 << MEM_MAP_SHIFT)

#define ADC_SYNC_ENET_STRT_DLY_OFST (0)
#define ADC_SYNC_ENET_STRT_DLY_MSK  (0x0000000F << ADC_SYNC_ENET_STRT_DLY_OFST)
#define ADC_SYNC_ENET_STRT_DLY_VAL                                             \
    ((0x4 << ADC_SYNC_ENET_STRT_DLY_OFST) & ADC_SYNC_ENET_STRT_DLY_MSK)
#define ADC_SYNC_TKN1_HGH_DLY_OFST (4)
#define ADC_SYNC_TKN1_HGH_DLY_MSK  (0x0000000F << ADC_SYNC_TKN1_HGH_DLY_OFST)
#define ADC_SYNC_TKN1_HGH_DLY_VAL                                              \
    ((0x1 << ADC_SYNC_TKN1_HGH_DLY_OFST) & ADC_SYNC_TKN1_HGH_DLY_MSK)
#define ADC_SYNC_TKN2_HGH_DLY_OFST (8)
#define ADC_SYNC_TKN2_HGH_DLY_MSK  (0x0000000F << ADC_SYNC_TKN2_HGH_DLY_OFST)
#define ADC_SYNC_TKN2_HGH_DLY_VAL                                              \
    ((0x2 << ADC_SYNC_TKN2_HGH_DLY_OFST) & ADC_SYNC_TKN2_HGH_DLY_MSK)
#define ADC_SYNC_TKN1_LOW_DLY_OFST (12)
#define ADC_SYNC_TKN1_LOW_DLY_MSK  (0x0000000F << ADC_SYNC_TKN1_LOW_DLY_OFST)
#define ADC_SYNC_TKN1_LOW_DLY_VAL                                              \
    ((0x2 << ADC_SYNC_TKN1_LOW_DLY_OFST) & ADC_SYNC_TKN1_LOW_DLY_MSK)
#define ADC_SYNC_TKN2_LOW_DLY_OFST (16)
#define ADC_SYNC_TKN2_LOW_DLY_MSK  (0x0000000F << ADC_SYNC_TKN2_LOW_DLY_OFST)
#define ADC_SYNC_TKN2_LOW_DLY_VAL                                              \
    ((0x3 << ADC_SYNC_TKN2_LOW_DLY_OFST) & ADC_SYNC_TKN2_LOW_DLY_MSK)
// 0x32214
#define ADC_SYNC_TKN_VAL                                                       \
    (ADC_SYNC_ENET_STRT_DLY_VAL | ADC_SYNC_TKN1_HGH_DLY_VAL |                  \
     ADC_SYNC_TKN2_HGH_DLY_VAL | ADC_SYNC_TKN1_LOW_DLY_VAL |                   \
     ADC_SYNC_TKN2_LOW_DLY_VAL)
#define ADC_SYNC_CLEAN_FIFOS_OFST (20)
#define ADC_SYNC_CLEAN_FIFOS_MSK  (0x00000001 << ADC_SYNC_CLEAN_FIFOS_OFST)
#define ADC_SYNC_ENET_DELAY_OFST  (24)
#define ADC_SYNC_ENET_DELAY_MSK   (0x000000FF << ADC_SYNC_ENET_DELAY_OFST)
#define ADC_SYNC_ENET_DELAY_NO_ROI_VAL                                         \
    ((0x88 << ADC_SYNC_ENET_DELAY_OFST) & ADC_SYNC_ENET_DELAY_MSK)
#define ADC_SYNC_ENET_DELAY_ROI_VAL                                            \
    ((0x1b << ADC_SYNC_ENET_DELAY_OFST) & ADC_SYNC_ENET_DELAY_MSK)

/** Time From Start register */
// #define MU_TIME_REG                     (0x1a << MEM_MAP_SHIFT)

/** Temperatre SPI In register */
#define TEMP_SPI_IN_REG (0x1b << MEM_MAP_SHIFT)

#define TEMP_SPI_IN_T1_CLK_OFST (0)
#define TEMP_SPI_IN_T1_CLK_MSK  (0x00000001 << TEMP_SPI_IN_T1_CLK_OFST)
#define TEMP_SPI_IN_T1_CS_OFST  (1)
#define TEMP_SPI_IN_T1_CS_MSK   (0x00000001 << TEMP_SPI_IN_T1_CS_OFST)
#define TEMP_SPI_IN_T2_CLK_OFST (2)
#define TEMP_SPI_IN_T2_CLK_MSK  (0x00000001 << TEMP_SPI_IN_T2_CLK_OFST)
#define TEMP_SPI_IN_T2_CS_OFST  (3)
#define TEMP_SPI_IN_T2_CS_MSK   (0x00000001 << TEMP_SPI_IN_T2_CS_OFST)
#define TEMP_SPI_IN_IDLE_MSK                                                   \
    (TEMP_SPI_IN_T1_CS_MSK | TEMP_SPI_IN_T2_CS_MSK | TEMP_SPI_IN_T1_CLK_MSK |  \
     TEMP_SPI_IN_T2_CLK_MSK)

/** Temperatre SPI Out register */
#define TEMP_SPI_OUT_REG (0x1c << MEM_MAP_SHIFT)

#define TEMP_SPI_OUT_T1_DT_OFST (0)
#define TEMP_SPI_OUT_T1_DT_MSK  (0x00000001 << TEMP_SPI_OUT_T1_DT_OFST)
#define TEMP_SPI_OUT_T2_DT_OFST (1)
#define TEMP_SPI_OUT_T2_DT_MSK  (0x00000001 << TEMP_SPI_OUT_T2_DT_OFST)

/** TSE Configure register */
#define TSE_CONF_REG (0x1d << MEM_MAP_SHIFT)

/** SPI Configure register */
#define ENET_CONF_REG (0x1e << MEM_MAP_SHIFT)

/** Write TSE Shadow register */
// #define WRITE_TSE_SHADOW_REG            (0x1f << MEM_MAP_SHIFT)

/** High Voltage register */
#define HV_REG (0x20 << MEM_MAP_SHIFT)

#define HV_ENBL_OFST   (0)
#define HV_ENBL_MSK    (0x00000001 << HV_ENBL_OFST)
#define HV_SEL_OFST    (1)
#define HV_SEL_MSK     (0x00000007 << HV_SEL_OFST)
#define HV_SEL_90_VAL  ((0x0 << HV_SEL_OFST) & HV_SEL_MSK)
#define HV_SEL_110_VAL ((0x1 << HV_SEL_OFST) & HV_SEL_MSK)
#define HV_SEL_120_VAL ((0x2 << HV_SEL_OFST) & HV_SEL_MSK)
#define HV_SEL_150_VAL ((0x3 << HV_SEL_OFST) & HV_SEL_MSK)
#define HV_SEL_180_VAL ((0x4 << HV_SEL_OFST) & HV_SEL_MSK)
#define HV_SEL_200_VAL ((0x5 << HV_SEL_OFST) & HV_SEL_MSK)

/** Dummy register */
#define DUMMY_REG (0x21 << MEM_MAP_SHIFT)

/** Firmware Version register */
#define FPGA_VERSION_REG (0x22 << MEM_MAP_SHIFT)

#define FPGA_VERSION_OFST (0)
#define FPGA_VERSION_MSK                                                       \
    (0x00FFFFFF << FPGA_VERSION_OFST) // to get in format yymmdd

/* Fix Pattern register */
#define FIX_PATT_REG (0x23 << MEM_MAP_SHIFT)

#define FIX_PATT_VAL (0xACDC1980)

/** 16 bit Control register */
#define CONTROL_REG (0x24 << MEM_MAP_SHIFT)

#define CONTROL_STRT_ACQ_OFST    (0)
#define CONTROL_STRT_ACQ_MSK     (0x00000001 << CONTROL_STRT_ACQ_OFST)
#define CONTROL_STP_ACQ_OFST     (1)
#define CONTROL_STP_ACQ_MSK      (0x00000001 << CONTROL_STP_ACQ_OFST)
#define CONTROL_STRT_FF_TST_OFST (2) // Not used in FW & SW
#define CONTROL_STRT_FF_TST_MSK  (0x00000001 << CONTROL_STRT_FF_TST_OFST)
#define CONTROL_STP_FF_TST_OFST  (3) // Not used in FW & SW
#define CONTROL_STP_FF_TST_MSK   (0x00000001 << CONTROL_STP_FF_TST_OFST)
#define CONTROL_STRT_RDT_OFST    (4)
#define CONTROL_STRT_RDT_MSK     (0x00000001 << CONTROL_STRT_RDT_OFST)
#define CONTROL_STP_RDT_OFST     (5)
#define CONTROL_STP_RDT_MSK      (0x00000001 << CONTROL_STP_RDT_OFST)
#define CONTROL_STRT_EXPSR_OFST  (6)
#define CONTROL_STRT_EXPSR_MSK   (0x00000001 << CONTROL_STRT_EXPSR_OFST)
#define CONTROL_STP_EXPSR_OFST   (7)
#define CONTROL_STP_EXPSR_MSK    (0x00000001 << CONTROL_STP_EXPSR_OFST)
#define CONTROL_STRT_TRN_OFST    (8)
#define CONTROL_STRT_TRN_MSK     (0x00000001 << CONTROL_STRT_TRN_OFST)
#define CONTROL_STP_TRN_OFST     (9)
#define CONTROL_STP_TRN_MSK      (0x00000001 << CONTROL_STP_TRN_OFST)
#define CONTROL_SYNC_RST_OFST    (10)
#define CONTROL_SYNC_RST_MSK     (0x00000001 << CONTROL_SYNC_RST_OFST)

/** Status register */
#define STATUS_REG (0x25 << MEM_MAP_SHIFT)

#define STATUS_RN_BSY_OFST        (0)
#define STATUS_RN_BSY_MSK         (0x00000001 << STATUS_RN_BSY_OFST)
#define STATUS_RDT_BSY_OFST       (1)
#define STATUS_RDT_BSY_MSK        (0x00000001 << STATUS_RDT_BSY_OFST)
#define STATUS_WTNG_FR_TRGGR_OFST (3)
#define STATUS_WTNG_FR_TRGGR_MSK  (0x00000001 << STATUS_WTNG_FR_TRGGR_OFST)
#define STATUS_DLY_BFR_OFST       (4)
#define STATUS_DLY_BFR_MSK        (0x00000001 << STATUS_DLY_BFR_OFST)
#define STATUS_DLY_AFTR_OFST      (5)
#define STATUS_DLY_AFTR_MSK       (0x00000001 << STATUS_DLY_AFTR_OFST)
#define STATUS_EXPSNG_OFST        (6)
#define STATUS_EXPSNG_MSK         (0x00000001 << STATUS_EXPSNG_OFST)
#define STATUS_CNT_ENBL_OFST      (7)
#define STATUS_CNT_ENBL_MSK       (0x00000001 << STATUS_CNT_ENBL_OFST)
#define STATUS_RD_STT_OFST        (8)
#define STATUS_RD_STT_MSK         (0x00000007 << STATUS_RD_STT_OFST)
#define STATUS_RN_STT_OFST        (12)
#define STATUS_RN_STT_MSK         (0x00000007 << STATUS_RN_STT_OFST)
#define STATUS_SM_FF_FLL_OFST     (15)
#define STATUS_SM_FF_FLL_MSK      (0x00000001 << STATUS_SM_FF_FLL_OFST)
#define STATUS_ALL_FF_EMPTY_OFST  (11)
#define STATUS_ALL_FF_EMPTY_MSK   (0x00000001 << STATUS_ALL_FF_EMPTY_OFST)
#define STATUS_RN_MSHN_BSY_OFST   (17)
#define STATUS_RN_MSHN_BSY_MSK    (0x00000001 << STATUS_RN_MSHN_BSY_OFST)
#define STATUS_RD_MSHN_BSY_OFST   (18)
#define STATUS_RD_MSHN_BSY_MSK    (0x00000001 << STATUS_RD_MSHN_BSY_OFST)
#define STATUS_RN_FNSHD_OFST      (20)
#define STATUS_RN_FNSHD_MSK       (0x00000001 << STATUS_RN_FNSHD_OFST)
#define STATUS_IDLE_MSK           (0x0000FFFF << 0)

/** Config register */
#define CONFIG_REG (0x26 << MEM_MAP_SHIFT)

#define CONFIG_SLAVE_OFST       (0) // Not used in FW & SW
#define CONFIG_SLAVE_MSK        (0x00000001 << CONFIG_SLAVE_OFST)
#define CONFIG_MASTER_OFST      (1) // Not used in FW & SW
#define CONFIG_MASTER_MSK       (0x00000001 << CONFIG_MASTER_OFST)
#define CONFIG_TM_GT_ENBL_OFST  (2) // Not used in FW & SW
#define CONFIG_TM_GT_ENBL_MSK   (0x00000001 << CONFIG_TM_GT_ENBL_OFST)
#define CONFIG_CPU_RDT_OFST     (12)
#define CONFIG_CPU_RDT_MSK      (0x00000001 << CONFIG_CPU_RDT_OFST)
#define CONFIG_CNTNS_RDT_OFST   (23) // Not used in FW & SW
#define CONFIG_CNTNS_RDT_MSK    (0x00000001 << CONFIG_CNTNS_RDT_OFST)
#define CONFIG_ACCMLT_CNTS_OFST (24) // Not used in FW & SW
#define CONFIG_ACCMLT_CNTS_MSK  (0x00000001 << CONFIG_ACCMLT_CNTS_OFST)

/** External Signal register */
#define EXT_SIGNAL_REG (0x27 << MEM_MAP_SHIFT)

#define EXT_SIGNAL_OFST              (0)
#define EXT_SIGNAL_MSK               (0x00000007 << EXT_SIGNAL_OFST)
#define EXT_SIGNAL_OFF_VAL           ((0x0 << EXT_SIGNAL_OFST) & EXT_SIGNAL_MSK)
#define EXT_SIGNAL_TRGGR_IN_RSNG_VAL ((0x3 << EXT_SIGNAL_OFST) & EXT_SIGNAL_MSK)
#define EXT_SIGNAL_TRGGR_IN_FLLNG_VAL                                          \
    ((0x4 << EXT_SIGNAL_OFST) & EXT_SIGNAL_MSK)

/** Look at me register */
// #define LOOK_AT_ME_REG                  (0x28 << MEM_MAP_SHIFT)

/** FPGA SVN register */
// #define FPGA_SVN_REG                    (0x29 << MEM_MAP_SHIFT)

/** Chip of Interest register */
#define CHIP_OF_INTRST_REG (0x2a << MEM_MAP_SHIFT)

#define CHIP_OF_INTRST_ADC_SEL_OFST    (0)
#define CHIP_OF_INTRST_ADC_SEL_MSK     (0x0000001F << CHIP_OF_INTRST_ADC_SEL_OFST)
#define CHIP_OF_INTRST_NUM_CHNNLS_OFST (16)
#define CHIP_OF_INTRST_NUM_CHNNLS_MSK                                          \
    (0x0000FFFF << CHIP_OF_INTRST_NUM_CHNNLS_OFST)

/** Out MUX register */
// #define OUT_MUX_REG                     (0x2b << MEM_MAP_SHIFT)

/** Board Version register */
#define BOARD_REVISION_REG (0x2c << MEM_MAP_SHIFT)

#define BOARD_REVISION_OFST (0)
#define BOARD_REVISION_MSK  (0x0000FFFF << BOARD_REVISION_OFST)
#define DETECTOR_TYPE_OFST  (16)
#define DETECTOR_TYPE_MSK   (0x0000000F << DETECTOR_TYPE_OFST)
// #define DETECTOR_TYPE_GOTTHARD_VAL          (??)
#define DETECTOR_TYPE_MOENCH_VAL (2)

/** Memory Test register */
// #define MEMORY_TEST_REG                 (0x2d << MEM_MAP_SHIFT)

/** Hit Threshold register */
// #define HIT_THRESHOLD_REG               (0x2e << MEM_MAP_SHIFT)

/** Hit Count register */
// #define HIT_COUNT_REG                   (0x2f << MEM_MAP_SHIFT)

/* 16 bit Fifo Data register */
#define FIFO_DATA_REG (0x50 << MEM_MAP_SHIFT) // Not used in FW and SW (16bit)

/** Dacs Set 1 register */
// #define DACS_SET_1_REG                  (0x65 << MEM_MAP_SHIFT)

/** Dacs Set 2 register */
// #define DACS_SET_2_REG                  (0x66 << MEM_MAP_SHIFT)

/** Dacs Set 3 register */
// #define DACS_SET_3_REG                  (0x67 << MEM_MAP_SHIFT)

/* Set Delay 64 bit register */
#define SET_DELAY_LSB_REG (0x68 << MEM_MAP_SHIFT)
#define SET_DELAY_MSB_REG (0x69 << MEM_MAP_SHIFT)

/* Get Delay 64 bit register */
#define GET_DELAY_LSB_REG (0x6a << MEM_MAP_SHIFT)
#define GET_DELAY_MSB_REG (0x6b << MEM_MAP_SHIFT)

/* Set Triggers 64 bit register */
#define SET_TRAINS_LSB_REG (0x6c << MEM_MAP_SHIFT)
#define SET_TRAINS_MSB_REG (0x6d << MEM_MAP_SHIFT)

/* Get Triggers 64 bit register */
#define GET_TRAINS_LSB_REG (0x6e << MEM_MAP_SHIFT)
#define GET_TRAINS_MSB_REG (0x6f << MEM_MAP_SHIFT)

/* Set Frames 64 bit register */
#define SET_FRAMES_LSB_REG (0x70 << MEM_MAP_SHIFT)
#define SET_FRAMES_MSB_REG (0x71 << MEM_MAP_SHIFT)

/* Get Frames 64 bit register */
#define GET_FRAMES_LSB_REG (0x72 << MEM_MAP_SHIFT)
#define GET_FRAMES_MSB_REG (0x73 << MEM_MAP_SHIFT)

/* Set Period 64 bit register */
#define SET_PERIOD_LSB_REG (0x74 << MEM_MAP_SHIFT)
#define SET_PERIOD_MSB_REG (0x75 << MEM_MAP_SHIFT)

/* Get Period 64 bit register */
#define GET_PERIOD_LSB_REG (0x76 << MEM_MAP_SHIFT)
#define GET_PERIOD_MSB_REG (0x77 << MEM_MAP_SHIFT)

/* Set Exptime 64 bit register */
#define SET_EXPTIME_LSB_REG (0x78 << MEM_MAP_SHIFT)
#define SET_EXPTIME_MSB_REG (0x79 << MEM_MAP_SHIFT)

/* Get Exptime 64 bit register */
#define GET_EXPTIME_LSB_REG (0x7a << MEM_MAP_SHIFT)
#define GET_EXPTIME_MSB_REG (0x7b << MEM_MAP_SHIFT)

/* Set Gates 64 bit register */
// #define SET_GATES_LSB_REG               (0x7c << MEM_MAP_SHIFT)
// #define SET_GATES_MSB_REG               (0x7d << MEM_MAP_SHIFT)

/* Set Gates 64 bit register */
// #define GET_GATES_LSB_REG               (0x7e << MEM_MAP_SHIFT)
// #define GET_GATES_MSB_REG               (0x7f << MEM_MAP_SHIFT)

/* Dark Image starting address */
#define DARK_IMAGE_REG (0x81 << MEM_MAP_SHIFT)

/* Gain Image starting address */
#define GAIN_IMAGE_REG (0x82 << MEM_MAP_SHIFT)

/* Counter Block Memory starting address */
#define COUNTER_MEMORY_REG (0x85 << MEM_MAP_SHIFT)
