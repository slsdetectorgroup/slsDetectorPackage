// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once
// clang-format off
#define REG_OFFSET (4)

/* Base addresses 0x1804 0000 ---------------------------------------------*/
/* Reconfiguration core for readout pll */
#define BASE_READOUT_PLL    (0x0000) // 0x1804_0000 - 0x1804_07FF
/* Reconfiguration core for system pll */
#define BASE_SYSTEM_PLL     (0x0800) // 0x1804_0800 - 0x1804_0FFF
/* Clock Generation */
#define BASE_CLK_GENERATION (0x1000) // 0x1804_1000 - 0x1804_XXXX //TODO

/* Base addresses 0x1806 0000 ---------------------------------------------*/
/* General purpose control and status registers */
#define BASE_CONTROL        (0x0000) // 0x1806_0000 - 0x1806_00FF
// https://git.psi.ch/sls_detectors_firmware/gotthard_II_mcb/blob/master/code/hdl/ctrl/ctrl.vhd

/* ASIC Control */
#define BASE_ASIC           (0x0100) // 0x1806_0100 - 0x1806_011F
// https://git.psi.ch/sls_detectors_firmware/gotthard_II_mcb/blob/master/code/hdl/asic/asic_ctrl.vhd

/* ASIC Digital Interface. Data recovery core */
#define BASE_ADIF           (0x0120) // 0x1806_0120 - 0x1806_012F
// https://git.psi.ch/sls_detectors_firmware/gotthard_II_mcb/blob/master/code/hdl/adif/adif_ctrl.vhd

/* Formatting of data core */
#define BASE_FMT            (0x0130) // 0x1806_0130 - 0x1806_013F

/* Packetizer */
#define BASE_PKT            (0x0140) // 0x1806_0140 - 0x1806_014F
// https://git.psi.ch/sls_detectors_firmware/gotthard_II_mcb/blob/master/code/hdl/pkt/pkt_ctrl.vhd

/* Flow control and status registers */
#define BASE_FLOW_CONTROL   (0x00200) // 0x1806_0200 - 0x1806_02FF
// https://git.psi.ch/sls_detectors_firmware/vhdl_library/blob/f37608230b4721661f29aacc20124555705ee705/flow/flow_ctrl.vhd

/** Veto processing core */
#define BASE_VETO_PRCSSNG   (0x0300)    // 0x1806_0300 - 0x1806_03FF?
// https://git.psi.ch/sls_detectors_firmware/gotthard_II_mcb/blob/master/code/hdl/veto/veto_ctrl.vhd

/* UDP datagram generator */
#define BASE_UDP_RAM        (0x01000) // 0x1806_1000 - 0x1806_1FFF


/* Clock Generation registers
 * ------------------------------------------------------*/
#define PLL_RESET_REG                       (0x00 * REG_OFFSET + BASE_CLK_GENERATION)

#define PLL_RESET_READOUT_OFST              (0)
#define PLL_RESET_READOUT_MSK               (0x00000001 << PLL_RESET_READOUT_OFST)
#define PLL_RESET_SYSTEM_OFST               (1)
#define PLL_RESET_SYSTEM_MSK                (0x00000001 << PLL_RESET_SYSTEM_OFST)

/* Control registers --------------------------------------------------*/

/* Module Control Board Serial Number register */
#define MCB_SERIAL_NO_REG                   (0x00 * REG_OFFSET + BASE_CONTROL)

#define MCB_SERIAL_NO_VRSN_OFST             (16)
#define MCB_SERIAL_NO_VRSN_MSK              (0x0000001F << MCB_SERIAL_NO_VRSN_OFST)

/* FPGA Version register */
#define FPGA_VERSION_REG                    (0x01 * REG_OFFSET + BASE_CONTROL)

#define FPGA_COMPILATION_DATE_OFST          (0)
#define FPGA_COMPILATION_DATE_MSK           (0x00FFFFFF << FPGA_COMPILATION_DATE_OFST)
#define DETECTOR_TYPE_OFST                  (24)
#define DETECTOR_TYPE_MSK                   (0x000000FF << DETECTOR_TYPE_OFST)

/* API Version register */
#define API_VERSION_REG                     (0x02 * REG_OFFSET + BASE_CONTROL)

#define API_VERSION_OFST                    (0)
#define API_VERSION_MSK                     (0x00FFFFFF << API_VERSION_OFST)
#define API_VERSION_DETECTOR_TYPE_OFST      (24) // Not used in software
#define API_VERSION_DETECTOR_TYPE_MSK       (0x000000FF << API_VERSION_DETECTOR_TYPE_OFST) // Not used in software

/* Fix pattern register */
#define FIX_PATT_REG                        (0x03 * REG_OFFSET + BASE_CONTROL)
#define FIX_PATT_VAL                        (0xACDC2019)

/* Status register */
#define STATUS_REG                          (0x04 * REG_OFFSET + BASE_CONTROL)

/* System status register */
#define SYSTEM_STATUS_REG                   (0x05 * REG_OFFSET + BASE_CONTROL)

/* Config RW regiseter */
#define CONFIG_REG                          (0x08 * REG_OFFSET + BASE_CONTROL)

#define CONFIG_VETO_ENBL_OFST               (0)
#define CONFIG_VETO_ENBL_MSK                (0x00000001 << CONFIG_VETO_ENBL_OFST)
#define CONFIG_VETO_CH_LLL_ALG_OFST         (8)
#define CONFIG_VETO_CH_LLL_ALG_MSK          (0x00000007 << CONFIG_VETO_CH_LLL_ALG_OFST)
#define CONFIG_VETO_CH_LLL_ENBL_OFST        (11)
#define CONFIG_VETO_CH_LLL_ENBL_MSK         (0x00000001 << CONFIG_VETO_CH_LLL_ENBL_OFST)
#define CONFIG_VETO_CH_10GBE_ALG_OFST       (12)
#define CONFIG_VETO_CH_10GBE_ALG_MSK        (0x00000007 << CONFIG_VETO_CH_10GBE_ALG_OFST)
#define CONFIG_VETO_CH_10GBE_ENBL_OFST      (15)
#define CONFIG_VETO_CH_10GBE_ENBL_MSK       (0x00000001 << CONFIG_VETO_CH_10GBE_ENBL_OFST)
#define CONFIG_SLAVE_OFST                   (16)
#define CONFIG_SLAVE_MSK                    (0x00000001 << CONFIG_SLAVE_OFST)
#define CONFIG_HDI_SLAVE_OFST               (17)
#define CONFIG_HDI_SLAVE_MSK                (0x00000001 << CONFIG_HDI_SLAVE_OFST)
#define CONFIG_HDI_VERSION_OFST             (18)
#define CONFIG_HDI_VERSION_MSK              (0x0000001F << CONFIG_HDI_VERSION_OFST)
#define CONFIG_HDI_25UM_OFST                (23)
#define CONFIG_HDI_25UM_MSK                 (0x00000001 << CONFIG_HDI_25UM_OFST)
#define CONFIG_FLIP_DATA_STREAM_OFST        (24)
#define CONFIG_FLIP_DATA_STREAM_MSK         (0x00000001 << CONFIG_FLIP_DATA_STREAM_OFST)
#define ALGORITHM_HITS_VAL                  (0x0)
#define ALGORITHM_RAW_VAL                   (0x7)

/* Control RW register */
#define CONTROL_REG                         (0x09 * REG_OFFSET + BASE_CONTROL)

#define CONTROL_STRT_ACQSTN_OFST            (0)
#define CONTROL_STRT_ACQSTN_MSK             (0x00000001 << CONTROL_STRT_ACQSTN_OFST)
#define CONTROL_STP_ACQSTN_OFST             (1)
#define CONTROL_STP_ACQSTN_MSK              (0x00000001 << CONTROL_STP_ACQSTN_OFST)
#define CONTROL_CRE_RST_OFST                (10)
#define CONTROL_CRE_RST_MSK                 (0x00000001 << CONTROL_CRE_RST_OFST)
#define CONTROL_PRPHRL_RST_OFST             (11) // Only GBE10?
#define CONTROL_PRPHRL_RST_MSK              (0x00000001 << CONTROL_PRPHRL_RST_OFST)
#define CONTROL_CLR_ACQSTN_FIFO_OFST        (15)
#define CONTROL_CLR_ACQSTN_FIFO_MSK         (0x00000001 << CONTROL_CLR_ACQSTN_FIFO_OFST)
#define CONTROL_TIMING_SOURCE_EXT_OFST      (17)
#define CONTROL_TIMING_SOURCE_EXT_MSK       (0x00000001 << CONTROL_TIMING_SOURCE_EXT_OFST)
#define CONTROL_PWR_CHIP_OFST               (31)
#define CONTROL_PWR_CHIP_MSK                (0x00000001 << CONTROL_PWR_CHIP_OFST)

/** DTA Offset Register */
#define DTA_OFFSET_REG                      (0x0A * REG_OFFSET + BASE_CONTROL)


/** Module ID Register */
#define MOD_ID_REG                          (0x0B * REG_OFFSET + BASE_CONTROL)

#define MOD_ID_OFST                         (0)
#define MOD_ID_MSK                          (0x0000FFFF << MOD_ID_OFST)

/** Set Next Frame Number Register */
#define FRAME_NUMBER_LSB_REG                (0x0C * REG_OFFSET + BASE_CONTROL)
#define FRAME_NUMBER_MSB_REG                (0x0D * REG_OFFSET + BASE_CONTROL)

/** Get Next Frame Number Register */
#define GET_FRAME_NUMBER_LSB_REG            (0x0E * REG_OFFSET + BASE_CONTROL)
#define GET_FRAME_NUMBER_MSB_REG            (0x0F * REG_OFFSET + BASE_CONTROL)

/* ASIC registers --------------------------------------------------*/

/* ASIC Config register */
#define ASIC_CONFIG_REG                     (0x00 * REG_OFFSET + BASE_ASIC)

#define ASIC_CONFIG_RUN_MODE_OFST           (0)
#define ASIC_CONFIG_RUN_MODE_MSK            (0x00000003 << ASIC_CONFIG_RUN_MODE_OFST)
#define ASIC_CONFIG_RUN_MODE_INT_BURST_VAL  ((0x0 << ASIC_CONFIG_RUN_MODE_OFST) & ASIC_CONFIG_RUN_MODE_MSK)
#define ASIC_CONFIG_RUN_MODE_EXT_BURST_VAL  ((0x1 << ASIC_CONFIG_RUN_MODE_OFST) & ASIC_CONFIG_RUN_MODE_MSK)
#define ASIC_CONFIG_RUN_MODE_INT_CONT_VAL   ((0x2 << ASIC_CONFIG_RUN_MODE_OFST) & ASIC_CONFIG_RUN_MODE_MSK)
#define ASIC_CONFIG_RUN_MODE_EXT_CONT_VAL   ((0x3 << ASIC_CONFIG_RUN_MODE_OFST) & ASIC_CONFIG_RUN_MODE_MSK)
#define ASIC_CONFIG_GAIN_OFST               (4)
#define ASIC_CONFIG_GAIN_MSK                (0x00000003 << ASIC_CONFIG_GAIN_OFST)
#define ASIC_CONFIG_DYNAMIC_GAIN_VAL        ((0x0 << ASIC_CONFIG_GAIN_OFST) & ASIC_CONFIG_GAIN_MSK)
#define ASIC_CONFIG_FIX_GAIN_1_VAL          ((0x1 << ASIC_CONFIG_GAIN_OFST) & ASIC_CONFIG_GAIN_MSK)
#define ASIC_CONFIG_FIX_GAIN_2_VAL          ((0x2 << ASIC_CONFIG_GAIN_OFST) & ASIC_CONFIG_GAIN_MSK)
#define ASIC_CONFIG_RESERVED_VAL            ((0x3 << ASIC_CONFIG_GAIN_OFST) & ASIC_CONFIG_GAIN_MSK)
#define ASIC_CONFIG_CURRENT_SRC_EN_OFST     (7)
#define ASIC_CONFIG_CURRENT_SRC_EN_MSK      (0x00000001 << ASIC_CONFIG_CURRENT_SRC_EN_OFST)
#define ASIC_CONFIG_NON_PARALLEL_RD_OFST    (8)
#define ASIC_CONFIG_NON_PARALLEL_RD_MSK     (0x00000001 << ASIC_CONFIG_NON_PARALLEL_RD_OFST)
#define ASIC_CONFIG_RST_DAC_OFST            (15)
#define ASIC_CONFIG_RST_DAC_MSK             (0x00000001 << ASIC_CONFIG_RST_DAC_OFST)
#define ASIC_CONFIG_DOUT_RDY_SRC_OFST       (16)
#define ASIC_CONFIG_DOUT_RDY_SRC_MSK        (0x0000000F << ASIC_CONFIG_DOUT_RDY_SRC_OFST)
#define ASIC_CONFIG_DOUT_RDY_DLY_OFST       (20)
#define ASIC_CONFIG_DOUT_RDY_DLY_MSK        (0x000000FF << ASIC_CONFIG_DOUT_RDY_DLY_OFST)
#define ASIC_CONFIG_DONE_OFST               (31)
#define ASIC_CONFIG_DONE_MSK                (0x00000001 << ASIC_CONFIG_DONE_OFST)

/* ASIC Internal Frames Register */
#define ASIC_INT_FRAMES_REG                 (0x01 * REG_OFFSET + BASE_ASIC)

#define ASIC_INT_FRAMES_OFST                (0)
#define ASIC_INT_FRAMES_MSK                 (0x00000FFF << ASIC_INT_FRAMES_OFST)

/* ASIC Period 64bit Register */
#define ASIC_INT_PERIOD_LSB_REG             (0x02 * REG_OFFSET + BASE_ASIC)
#define ASIC_INT_PERIOD_MSB_REG             (0x03 * REG_OFFSET + BASE_ASIC)

/* ASIC Exptime 64bit Register */
#define ASIC_INT_EXPTIME_LSB_REG            (0x04 * REG_OFFSET + BASE_ASIC)
#define ASIC_INT_EXPTIME_MSB_REG            (0x05 * REG_OFFSET + BASE_ASIC)

/* ASIC Exptime 64bit Register */
#define ASIC_CONT_FRAMES_LSB_REG            (0x06 * REG_OFFSET + BASE_ASIC)
#define ASIC_CONT_FRAMES_MSB_REG            (0x07 * REG_OFFSET + BASE_ASIC)


/* ADIF registers --------------------------------------------------*/

/* ADIF Config register */
#define ADIF_CONFIG_REG                     (0x00 * REG_OFFSET + BASE_ADIF)

#define ADIF_CONFIG_DBIT_PIPELINE_OFST      (4)
#define ADIF_CONFIG_DBIT_PIPELINE_MSK       (0x00000007 << ADIF_CONFIG_DBIT_PIPELINE_OFST)


/* Packetizer -------------------------------------------------------------*/

/* Packetizer Config Register */
#define PKT_CONFIG_REG                      (0x00 * REG_OFFSET + BASE_PKT)

#define PKT_CONFIG_NRXR_MAX_OFST            (0)
#define PKT_CONFIG_NRXR_MAX_MSK             (0x0000001F << PKT_CONFIG_NRXR_MAX_OFST)
#define PKT_CONFIG_RXR_START_ID_OFST        (8)
#define PKT_CONFIG_RXR_START_ID_MSK         (0x0000001F << PKT_CONFIG_RXR_START_ID_OFST)

/* Module Coordinates Register */
#define COORD_0_REG                         (0x02 * REG_OFFSET + BASE_PKT)
#define COORD_ROW_OFST                      (0)
#define COORD_ROW_MSK                       (0x0000FFFF << COORD_ROW_OFST)
#define COORD_COL_OFST                      (16)
#define COORD_COL_MSK                       (0x0000FFFF << COORD_COL_OFST)

/* Module ID Register */
#define COORD_1_REG                         (0x03 * REG_OFFSET + BASE_PKT)
#define COORD_RESERVED_OFST                 (0)
#define COORD_RESERVED_MSK                  (0x0000FFFF << COORD_RESERVED_OFST)
#define COORD_ID_OFST                       (16) // Not connected in firmware TODO
#define COORD_ID_MSK                        (0x0000FFFF << COORD_ID_OFST) // Not connected in firmware TODO

/* Flow control registers --------------------------------------------------*/

/* Flow status Register*/
#define FLOW_STATUS_REG                     (0x00 * REG_OFFSET + BASE_FLOW_CONTROL)

#define FLOW_STATUS_RUN_BUSY_OFST           (0)
#define FLOW_STATUS_RUN_BUSY_MSK            (0x00000001 << FLOW_STATUS_RUN_BUSY_OFST)
#define FLOW_STATUS_WAIT_FOR_TRGGR_OFST     (3)
#define FLOW_STATUS_WAIT_FOR_TRGGR_MSK      (0x00000001 << FLOW_STATUS_WAIT_FOR_TRGGR_OFST)
#define FLOW_STATUS_DLY_BFRE_TRGGR_OFST     (4)
#define FLOW_STATUS_DLY_BFRE_TRGGR_MSK      (0x00000001 << FLOW_STATUS_DLY_BFRE_TRGGR_OFST)
#define FLOW_STATUS_FIFO_FULL_OFST          (5)
#define FLOW_STATUS_FIFO_FULL_MSK           (0x00000001 << FLOW_STATUS_FIFO_FULL_OFST)
#define FLOW_STATUS_DLY_AFTR_TRGGR_OFST     (15)
#define FLOW_STATUS_DLY_AFTR_TRGGR_MSK      (0x00000001 << FLOW_STATUS_DLY_AFTR_TRGGR_OFST)
#define FLOW_STATUS_CSM_BUSY_OFST           (17)
#define FLOW_STATUS_CSM_BUSY_MSK            (0x00000001 << FLOW_STATUS_CSM_BUSY_OFST)

/* Delay left 64bit Register */
#define GET_DELAY_LSB_REG                   (0x02 * REG_OFFSET + BASE_FLOW_CONTROL)
#define GET_DELAY_MSB_REG                   (0x03 * REG_OFFSET + BASE_FLOW_CONTROL)

/* Triggers left 64bit Register */
#define GET_CYCLES_LSB_REG                  (0x04 * REG_OFFSET + BASE_FLOW_CONTROL)
#define GET_CYCLES_MSB_REG                  (0x05 * REG_OFFSET + BASE_FLOW_CONTROL)

/* Frames left 64bit Register */
#define GET_FRAMES_LSB_REG                  (0x06 * REG_OFFSET + BASE_FLOW_CONTROL)
#define GET_FRAMES_MSB_REG                  (0x07 * REG_OFFSET + BASE_FLOW_CONTROL)

/* Period left 64bit Register */
#define GET_PERIOD_LSB_REG                  (0x08 * REG_OFFSET + BASE_FLOW_CONTROL)
#define GET_PERIOD_MSB_REG                  (0x09 * REG_OFFSET + BASE_FLOW_CONTROL)

/* Time from Start 64 bit register */
#define TIME_FROM_START_LSB_REG             (0x0A * REG_OFFSET + BASE_FLOW_CONTROL)
#define TIME_FROM_START_MSB_REG             (0x0B * REG_OFFSET + BASE_FLOW_CONTROL)

/* Get Frames from Start 64 bit register (frames from last reset using
 * CONTROL_CRST) */
#define FRAMES_FROM_START_LSB_REG           (0x0C * REG_OFFSET + BASE_FLOW_CONTROL)
#define FRAMES_FROM_START_MSB_REG           (0x0D * REG_OFFSET + BASE_FLOW_CONTROL)

/* Measurement Time 64 bit register (timestamp at a frame start until reset)*/
#define START_FRAME_TIME_LSB_REG            (0x0E * REG_OFFSET + BASE_FLOW_CONTROL)
#define START_FRAME_TIME_MSB_REG            (0x0F * REG_OFFSET + BASE_FLOW_CONTROL)

/* Delay 64bit Write-register */
#define SET_DELAY_LSB_REG                   (0x22 * REG_OFFSET + BASE_FLOW_CONTROL)
#define SET_DELAY_MSB_REG                   (0x23 * REG_OFFSET + BASE_FLOW_CONTROL)

/* Cylces (also #bursts) 64bit Write-register */
#define SET_CYCLES_LSB_REG                  (0x24 * REG_OFFSET + BASE_FLOW_CONTROL)
#define SET_CYCLES_MSB_REG                  (0x25 * REG_OFFSET + BASE_FLOW_CONTROL)

/* Frames 64bit Write-register */
#define SET_FRAMES_LSB_REG                  (0x26 * REG_OFFSET + BASE_FLOW_CONTROL)
#define SET_FRAMES_MSB_REG                  (0x27 * REG_OFFSET + BASE_FLOW_CONTROL)

/* Period (also burst period) 64bit Write-register */
#define SET_PERIOD_LSB_REG                  (0x28 * REG_OFFSET + BASE_FLOW_CONTROL)
#define SET_PERIOD_MSB_REG                  (0x29 * REG_OFFSET + BASE_FLOW_CONTROL)

/* External Signal register */
#define EXT_SIGNAL_REG                      (0x30 * REG_OFFSET + BASE_FLOW_CONTROL)

#define EXT_SIGNAL_OFST                     (0)
#define EXT_SIGNAL_MSK                      (0x00000001 << EXT_SIGNAL_OFST)

/* Trigger Delay 64 bit register */
#define SET_TRIGGER_DELAY_LSB_REG           (0x32 * REG_OFFSET + BASE_FLOW_CONTROL)
#define SET_TRIGGER_DELAY_MSB_REG           (0x33 * REG_OFFSET + BASE_FLOW_CONTROL)


/* UDP datagram registers --------------------------------------------------*/
#define RXR_ENDPOINTS_MAX                   (32)
#define RXR_ENDPOINT_OFST                   (16 * REG_OFFSET)


/** Veto processing core  --------------------------------------------------*/
/** Mask Strip Registers (40) */
#define MASK_STRIP_START_REG                (0x00 * REG_OFFSET + BASE_VETO_PRCSSNG)
#define MASK_STRIP_NUM_REGS                 (40)

// clang-format on
