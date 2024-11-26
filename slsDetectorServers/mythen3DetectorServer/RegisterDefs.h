// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once

// clang-format off
#define REG_OFFSET (4)

/* Base addresses 0x1804 0000 ---------------------------------------------*/

/* Reconfiguration core for system pll */
#define BASE_SYSTEM_PLL                 (0x0800) // 0x1804_0800 - 0x1804_0FFF

/* Clock Generation */
#define BASE_CLK_GENERATION             (0x1000) // 0x1804_1000 - 0x1804_XXXX //TODO

/* Base addresses 0x1806 0000 ---------------------------------------------*/
/* General purpose control and status registers */
#define BASE_CONTROL                    (0x0000) // 0x1806_0000 - 0x1806_00FF
// https://git.psi.ch/sls_detectors_firmware/mythen_III_mcb/blob/master/code/hdl/ctrl/ctrl.vhd

/* ASIC Digital Interface. Data recovery core */
#define BASE_ADIF                       (0x0110) // 0x1806_0110 - 0x1806_011F
// https://git.psi.ch/sls_detectors_firmware/vhdl_library/blob/xxx/adif/adif_ctrl.vhd

/* Formatting of data core */
#define BASE_FMT                        (0x0120) // 0x1806_0120 - 0x1806_012F

/* Packetizer */
#define BASE_PKT                        (0x0130) // 0x1806_0130 - 0x1806_013F
// https://git.psi.ch/sls_detectors_firmware/mythen_III_mcb/blob/master/code/hdl/pkt/pkt_ctrl.vhd

/** Pipeline (Timing Rec) */
#define BASE_PIPELINE                   (0x0140) // 0x1806_0140 - 0x1806_014F
// https://git.psi.ch/sls_detectors_firmware/vhdl_library/blob/xxx/MythenIIITriggerBoard/timingReceier.vhd

/* ASIC Exposure Control */
#define BASE_ASIC_EXP                   (0x0180) // 0x1806_0180 - 0x1806_01BF

/* Pattern control and status */
#define BASE_PATTERN_CONTROL            (0x00200) // 0x1806_0200 - 0x1806_02FF
// https://git.psi.ch/sls_detectors_firmware/vhdl_library/blob/xxx/pattern_flow/pattern_flow_ctrl.vhd

/* Flow control and status */
#define BASE_FLOW_CONTROL               (0x00400) // 0x1806_0400 - 0x1806_04FF
// https://git.psi.ch/sls_detectors_firmware/vhdl_library/blob/qsys/flow/flow_ctrl.vhd

/** ASIC Readout Control */
#define BASE_ASIC_RDO                   (0x00500) // 0x1806_0500 - 0x1806_050F
// https://git.psi.ch/sls_detectors_firmware/mythen_III_mcb/blob/master/code/hdl/asic_rdo/asic_rdo.vhd

/** Dead time Free Controller */
#define BASE_DEADTIME_FREE_CTRL         (0x00580) // 0x1806_0580 - 0x1806_0587
// https://git.psi.ch/sls_detectors_firmware/mythen_III_mcb/blob/master/code/hdl/DeadTimeFreeController/DeadTimeFreeCtrl.vhd

/* UDP datagram generator */
#define BASE_UDP_RAM                    (0x01000) // 0x1806_1000 - 0x1806_1FFF

/* Pattern RAM. Pattern table */
#define BASE_PATTERN_RAM                (0x10000) // 0x1807_0000 - 0x1807_FFFF


/* Clock Generation registers
 * ------------------------------------------------------*/
#define PLL_RESET_REG                   (0x00 * REG_OFFSET + BASE_CLK_GENERATION)

#define PLL_RESET_READOUT_OFST          (0)
#define PLL_RESET_READOUT_MSK           (0x00000001 << PLL_RESET_READOUT_OFST)
#define PLL_RESET_SYSTEM_OFST           (1)
#define PLL_RESET_SYSTEM_MSK            (0x00000001 << PLL_RESET_SYSTEM_OFST)

/* Control registers --------------------------------------------------*/

/* Module Control Board Serial Number Register */
#define MCB_SERIAL_NO_REG               (0x00 * REG_OFFSET + BASE_CONTROL)

#define MCB_SERIAL_NO_VRSN_OFST         (16)
#define MCB_SERIAL_NO_VRSN_MSK          (0x0000001F << MCB_SERIAL_NO_VRSN_OFST)

/* FPGA Version register */
#define FPGA_VERSION_REG                (0x01 * REG_OFFSET + BASE_CONTROL)

#define FPGA_COMPILATION_DATE_OFST      (0)
#define FPGA_COMPILATION_DATE_MSK       (0x00FFFFFF << FPGA_COMPILATION_DATE_OFST)
#define DETECTOR_TYPE_OFST              (24)
#define DETECTOR_TYPE_MSK               (0x000000FF << DETECTOR_TYPE_OFST)

/* API Version Register */
#define API_VERSION_REG                 (0x02 * REG_OFFSET + BASE_CONTROL)

#define API_VERSION_OFST                (0)
#define API_VERSION_MSK                 (0x00FFFFFF << API_VERSION_OFST)
#define API_VERSION_DETECTOR_TYPE_OFST  (24) // Not used in software
#define API_VERSION_DETECTOR_TYPE_MSK   (0x000000FF << API_VERSION_DETECTOR_TYPE_OFST) // Not used in software

/* Fix pattern register */
#define FIX_PATT_REG                    (0x03 * REG_OFFSET + BASE_CONTROL)
#define FIX_PATT_VAL                    (0xACDC2019)

/* Status register */
#define STATUS_REG                      (0x04 * REG_OFFSET + BASE_CONTROL)

/* Look at me register, read only */
#define LOOK_AT_ME_REG                  (0x05 * REG_OFFSET + BASE_CONTROL) // Not used in firmware or software, good to play with

#define SYSTEM_STATUS_REG               (0x06 * REG_OFFSET + BASE_CONTROL) 

#define SYSTEM_STATUS_R_PLL_LCKD_OFST   (1)
#define SYSTEM_STATUS_R_PLL_LCKD_MSK    (0x00000001 << SYSTEM_STATUS_R_PLL_LCKD_OFST)
#define SYSTEM_STATUS_RDO_PLL_LCKD_OFST (2)
#define SYSTEM_STATUS_RDO_PLL_LCKD_MSK  (0x00000001 << SYSTEM_STATUS_RDO_PLL_LCKD_OFST)
#define SYSTEM_STATUS_SLV_BRD_DTCT_OFST (31)
#define SYSTEM_STATUS_SLV_BRD_DTCT_MSK  (0x00000001 << SYSTEM_STATUS_SLV_BRD_DTCT_OFST)


#define MOD_ID_REG                      (0x0B * REG_OFFSET + BASE_CONTROL)

#define MOD_ID_OFST                     (0)
#define MOD_ID_MSK                      (0x0000FFFF << MOD_ID_OFST)


/* Config RW regiseter */
#define CONFIG_REG                      (0x20 * REG_OFFSET + BASE_CONTROL)

#define CONFIG_COUNTERS_ENA_OFST        (0)
#define CONFIG_COUNTERS_ENA_MSK         (0x00000007 << CONFIG_COUNTERS_ENA_OFST)
#define CONFIG_COUNTER_1_ENA_VAL        ((0x1 << CONFIG_COUNTERS_ENA_OFST) & CONFIG_COUNTERS_ENA_MSK)
#define CONFIG_COUNTER_2_ENA_VAL        ((0x2 << CONFIG_COUNTERS_ENA_OFST) & CONFIG_COUNTERS_ENA_MSK)
#define CONFIG_COUNTER_3_ENA_VAL        ((0x4 << CONFIG_COUNTERS_ENA_OFST) & CONFIG_COUNTERS_ENA_MSK)
#define CONFIG_DYNAMIC_RANGE_OFST       (4)
#define CONFIG_DYNAMIC_RANGE_MSK        (0x00000003 << CONFIG_DYNAMIC_RANGE_OFST)
#define CONFIG_DYNAMIC_RANGE_1_VAL      ((0x0 << CONFIG_DYNAMIC_RANGE_OFST) & CONFIG_DYNAMIC_RANGE_MSK)
#define CONFIG_DYNAMIC_RANGE_8_VAL      ((0x1 << CONFIG_DYNAMIC_RANGE_OFST) & CONFIG_DYNAMIC_RANGE_MSK)
#define CONFIG_DYNAMIC_RANGE_16_VAL     ((0x2 << CONFIG_DYNAMIC_RANGE_OFST) & CONFIG_DYNAMIC_RANGE_MSK)
#define CONFIG_DYNAMIC_RANGE_24_VAL     ((0x3 << CONFIG_DYNAMIC_RANGE_OFST) & CONFIG_DYNAMIC_RANGE_MSK)
#define CONFIG_TRIGGER_ENA_OFST         (8)
#define CONFIG_TRIGGER_ENA_MSK          (0x00000001 << CONFIG_TRIGGER_ENA_OFST)


/* Control RW register */
#define CONTROL_REG                     (0x21 * REG_OFFSET + BASE_CONTROL)

#define CONTROL_STRT_ACQSTN_OFST        (0)
#define CONTROL_STRT_ACQSTN_MSK         (0x00000001 << CONTROL_STRT_ACQSTN_OFST)
#define CONTROL_STP_ACQSTN_OFST         (1)
#define CONTROL_STP_ACQSTN_MSK          (0x00000001 << CONTROL_STP_ACQSTN_OFST)
#define CONTROL_STRT_PATTERN_OFST       (2)
#define CONTROL_STRT_PATTERN_MSK        (0x00000001 << CONTROL_STRT_PATTERN_OFST)
#define CONTROL_STRT_READOUT_OFST       (3) 
#define CONTROL_STRT_READOUT_MSK        (0x00000001 << CONTROL_STRT_READOUT_OFST)
#define CONTROL_STRT_SW_TRIGGER_OFST    (4)
#define CONTROL_STRT_SW_TRIGGER_MSK     (0x00000001 << CONTROL_STRT_SW_TRIGGER_OFST)
#define CONTROL_CRE_RST_OFST            (10)
#define CONTROL_CRE_RST_MSK             (0x00000001 << CONTROL_CRE_RST_OFST)
#define CONTROL_PRPHRL_RST_OFST         (11) // Only GBE10?
#define CONTROL_PRPHRL_RST_MSK          (0x00000001 << CONTROL_PRPHRL_RST_OFST)
#define CONTROL_CLR_ACQSTN_FIFO_OFST    (15)
#define CONTROL_CLR_ACQSTN_FIFO_MSK     (0x00000001 << CONTROL_CLR_ACQSTN_FIFO_OFST)
#define CONTROL_PWR_CHIP_OFST           (31)
#define CONTROL_PWR_CHIP_MSK            (0x00000001 << CONTROL_PWR_CHIP_OFST)

/* Pattern IO Control 64 bit register */
#define PATTERN_IO_CTRL_LSB_REG         (0x22 * REG_OFFSET + BASE_CONTROL)
#define PATTERN_IO_CTRL_MSB_REG         (0x23 * REG_OFFSET + BASE_CONTROL)

#define DTA_OFFSET_REG                  (0x24 * REG_OFFSET + BASE_CONTROL)


/* Formatting for adif core -----------------------------------------------*/
#define ADIF_CONFIG_REG                 (0x00 * REG_OFFSET + BASE_ADIF)

#define ADIF_ADDTNL_OFST_OFST           (0)
#define ADIF_ADDTNL_OFST_MSK            (0x00000003 << ADIF_ADDTNL_OFST_OFST)
#define ADIF_PIPELINE_OFST              (4)
#define ADIF_PIPELINE_MSK               (0x0000000F << ADIF_PIPELINE_OFST)


/* Formatting for data core -----------------------------------------------*/
#define FMT_CONFIG_REG                  (0x00 * REG_OFFSET + BASE_FMT)

#define FMT_CONFIG_TXN_DELAY_OFST       (0)
#define FMT_CONFIG_TXN_DELAY_MSK        (0x00FFFFFF << FMT_CONFIG_TXN_DELAY_OFST)


/* Packetizer -------------------------------------------------------------*/

/* Packetizer Config Register */
#define PKT_CONFIG_REG                  (0x00 * REG_OFFSET + BASE_PKT)

#define PKT_CONFIG_NRXR_MAX_OFST        (0)
#define PKT_CONFIG_NRXR_MAX_MSK         (0x0000003F << PKT_CONFIG_NRXR_MAX_OFST)
#define PKT_CONFIG_RXR_START_ID_OFST    (8)
#define PKT_CONFIG_RXR_START_ID_MSK     (0x0000003F << PKT_CONFIG_RXR_START_ID_OFST)
#define PKT_CONFIG_1G_INTERFACE_OFST    (16)
#define PKT_CONFIG_1G_INTERFACE_MSK     (0x00000001 << PKT_CONFIG_1G_INTERFACE_OFST)

#define PKT_FRAG_REG                    (0x01 * REG_OFFSET + BASE_PKT)

#define PKT_FRAG_1G_N_DSR_PER_PKT_OFST  (0)
#define PKT_FRAG_1G_N_DSR_PER_PKT_MSK   (0x0000003F << PKT_FRAG_1G_N_DSR_PER_PKT_OFST)
#define PKT_FRAG_10G_N_DSR_PER_PKT_OFST (8)
#define PKT_FRAG_10G_N_DSR_PER_PKT_MSK  (0x0000003F << PKT_FRAG_10G_N_DSR_PER_PKT_OFST)
#define PKT_FRAG_1G_NUM_PACKETS_OFST    (16)
#define PKT_FRAG_1G_NUM_PACKETS_MSK     (0x0000003F << PKT_FRAG_1G_NUM_PACKETS_OFST)
#define PKT_FRAG_10G_NUM_PACKETS_OFST   (24)
#define PKT_FRAG_10G_NUM_PACKETS_MSK    (0x0000003F << PKT_FRAG_10G_NUM_PACKETS_OFST)

/* Module Coordinates Register */
#define COORD_0_REG                     (0x02 * REG_OFFSET + BASE_PKT)
#define COORD_ROW_OFST                  (0)
#define COORD_ROW_MSK                   (0x0000FFFF << COORD_ROW_OFST)
#define COORD_COL_OFST                  (16)
#define COORD_COL_MSK                   (0x0000FFFF << COORD_COL_OFST)

/* Module ID Register */
#define COORD_1_REG                     (0x03 * REG_OFFSET + BASE_PKT)
#define COORD_RESERVED_OFST             (0)
#define COORD_RESERVED_MSK              (0x0000FFFF << COORD_RESERVED_OFST)
#define COORD_ID_OFST                   (16) // Not connected in firmware TODO
#define COORD_ID_MSK                    (0x0000FFFF << COORD_ID_OFST) // Not connected in firmware TODO

/* Pipeline -------------------------------------------------------------*/

/** DINF1 Master Input Register */
#define DINF1_REG                       (0x00 * REG_OFFSET + BASE_PIPELINE)

#define DINF1_TRIGGER_BYPASS_OFST       (0)
#define DINF1_TRIGGER_BYPASS_MSK        (0x00000001 << DINF1_TRIGGER_BYPASS_OFST)
#define DINF1_BYPASS_GATE_OFST          (1)
#define DINF1_BYPASS_GATE_MSK           (0x00000007 << DINF1_BYPASS_GATE_OFST)
#define DINF1_INVERSION_OFST            (4)
#define DINF1_INVERSION_MSK             (0x0000000F << DINF1_INVERSION_OFST)
#define DINF1_RISING_TRIGGER_OFST       (8)
#define DINF1_RISING_TRIGGER_MSK        (0x00000001 << DINF1_RISING_TRIGGER_OFST)
#define DINF1_RISING_GATE_OFST          (9)
#define DINF1_RISING_GATE_MSK           (0x00000007 << DINF1_RISING_GATE_OFST)
#define DINF1_FALLING_OFST              (12)
#define DINF1_FALLING_MSK               (0x0000000F << DINF1_FALLING_OFST)

/** DOUTIF1 Master Ouput Register */
#define DOUTIF1_REG                     (0x01 * REG_OFFSET + BASE_PIPELINE)

#define DOUTIF1_TRIGGER_BYPASS_OFST     (0)
#define DOUTIF1_TRIGGER_BYPASS_MSK      (0x00000001 << DOUTIF1_TRIGGER_BYPASS_OFST)
#define DOUTIF1_BYPASS_GATE_OFST        (1)
#define DOUTIF1_BYPASS_GATE_MSK         (0x00000007 << DOUTIF1_BYPASS_GATE_OFST)
#define DOUTIF1_INVERSION_OFST          (4)
#define DOUTIF1_INVERSION_MSK           (0x0000000F << DOUTIF1_INVERSION_OFST)
#define DOUTIF1_RISING_TRIGGER_OFST     (8)
#define DOUTIF1_RISING_TRIGGER_MSK      (0x00000001 << DOUTIF1_RISING_TRIGGER_OFST)
#define DOUTIF1_RISING_GATE_OFST        (9)
#define DOUTIF1_RISING_GATE_MSK         (0x00000007 << DOUTIF1_RISING_GATE_OFST)
#define DOUTIF1_FALLING_OFST            (12)
#define DOUTIF1_FALLING_MSK             (0x0000000F << DOUTIF1_FALLING_OFST)

/** DINF2 Slave Input Register */
#define DINF2_REG                       (0x02 * REG_OFFSET + BASE_PIPELINE)

#define DINF2_BYPASS_OFST               (0)
#define DINF2_BYPASS_MSK                (0x0000000F << DINF2_BYPASS_OFST)
#define DINF2_INVERSION_OFST            (4)
#define DINF2_INVERSION_MSK             (0x0000000F << DINF2_INVERSION_OFST)
#define DINF2_RISING_OFST               (8)
#define DINF2_RISING_MSK                (0x0000000F << DINF2_RISING_OFST)
#define DINF2_FALLING_OFST              (12)
#define DINF2_FALLING_MSK               (0x0000000F << DINF2_FALLING_OFST)

/* Pulse length after rising edge TODO (maybe fix a value for port 1 later )*/
#define DOUTIF_RISING_LNGTH_REG         (0x03 * REG_OFFSET + BASE_PIPELINE)

#define DOUTIF_RISING_LNGTH_PORT_1_OFST (0)
#define DOUTIF_RISING_LNGTH_PORT_1_MSK  (0x000000FF << DOUTIF_RISING_LNGTH_PORT_1_OFST)
#define DOUTIF_RISING_LNGTH_PORT_2_OFST (8)
#define DOUTIF_RISING_LNGTH_PORT_2_MSK  (0x000000FF << DOUTIF_RISING_LNGTH_PORT_2_OFST)
#define DOUTIF_RISING_LNGTH_PORT_3_OFST (16)
#define DOUTIF_RISING_LNGTH_PORT_3_MSK  (0x000000FF << DOUTIF_RISING_LNGTH_PORT_3_OFST)
#define DOUTIF_RISING_LNGTH_PORT_4_OFST (24)
#define DOUTIF_RISING_LNGTH_PORT_4_MSK  (0x000000FF << DOUTIF_RISING_LNGTH_PORT_4_OFST)


/* ASIC Exposure Control registers
 * --------------------------------------------------*/

/** ASIC Exposure Status register */
#define ASIC_EXP_STATUS_REG             (0x00 * REG_OFFSET + BASE_ASIC_EXP)

#define ASIC_EXP_STAT_GATE_SRC_EXT_OFST (0)
#define ASIC_EXP_STAT_GATE_SRC_EXT_MSK  (0x00000001 << ASIC_EXP_STAT_GATE_SRC_EXT_OFST)
#define ASIC_EXP_STAT_STO_LNGTH_OFST    (16)
#define ASIC_EXP_STAT_STO_LNGTH_MSK     (0x000000FF << ASIC_EXP_STAT_STO_LNGTH_OFST)
#define ASIC_EXP_STAT_RSCNTR_LNGTH_OFST (24)
#define ASIC_EXP_STAT_RSCNTR_LNGTH_MSK  (0x000000FF << ASIC_EXP_STAT_RSCNTR_LNGTH_OFST)

/** Gate 0 width register */
#define ASIC_EXP_GATE_0_WIDTH_LSB_REG   (0x01 * REG_OFFSET + BASE_ASIC_EXP)
#define ASIC_EXP_GATE_0_WIDTH_MSB_REG   (0x02 * REG_OFFSET + BASE_ASIC_EXP)

/** Gate 1 width register */
#define ASIC_EXP_GATE_1_WIDTH_LSB_REG   (0x03 * REG_OFFSET + BASE_ASIC_EXP)
#define ASIC_EXP_GATE_1_WIDTH_MSB_REG   (0x04 * REG_OFFSET + BASE_ASIC_EXP)

/** Gate 2 width register */
#define ASIC_EXP_GATE_2_WIDTH_LSB_REG   (0x05 * REG_OFFSET + BASE_ASIC_EXP)
#define ASIC_EXP_GATE_2_WIDTH_MSB_REG   (0x06 * REG_OFFSET + BASE_ASIC_EXP)

/** Gate 0 delay register */
#define ASIC_EXP_GATE_0_DELAY_LSB_REG   (0x07 * REG_OFFSET + BASE_ASIC_EXP)
#define ASIC_EXP_GATE_0_DELAY_MSB_REG   (0x08 * REG_OFFSET + BASE_ASIC_EXP)

/** Gate 1 delay register */
#define ASIC_EXP_GATE_1_DELAY_LSB_REG   (0x09 * REG_OFFSET + BASE_ASIC_EXP)
#define ASIC_EXP_GATE_1_DELAY_MSB_REG   (0x0A * REG_OFFSET + BASE_ASIC_EXP)

/** Gate 2 delay register */
#define ASIC_EXP_GATE_2_DELAY_LSB_REG   (0x0B * REG_OFFSET + BASE_ASIC_EXP)
#define ASIC_EXP_GATE_2_DELAY_MSB_REG   (0x0C * REG_OFFSET + BASE_ASIC_EXP)

/** Gate period register */
#define ASIC_EXP_GATE_PERIOD_LSB_REG    (0x0D * REG_OFFSET + BASE_ASIC_EXP)
#define ASIC_EXP_GATE_PERIOD_MSB_REG    (0x0E * REG_OFFSET + BASE_ASIC_EXP)

/** Number of Internal Gates register */
#define ASIC_EXP_INT_GATE_NUMBER_REG    (0x0F * REG_OFFSET + BASE_ASIC_EXP)

/** Number of Internal Gates register */
#define ASIC_EXP_EXT_GATE_NUMBER_REG    (0x10 * REG_OFFSET + BASE_ASIC_EXP)

/* Pattern Control registers
 * --------------------------------------------------*/

/* Pattern status Register*/
#define PAT_STATUS_REG                  (0x00 * REG_OFFSET + BASE_PATTERN_CONTROL)
#define PAT_STATUS_RUN_BUSY_OFST        (0)
#define PAT_STATUS_RUN_BUSY_MSK         (0x00000001 << PAT_STATUS_RUN_BUSY_OFST)

/* Pattern Limit RW Register */
#define PATTERN_LIMIT_REG               (0x40 * REG_OFFSET + BASE_PATTERN_CONTROL)

#define PATTERN_LIMIT_STRT_OFST         (0)
#define PATTERN_LIMIT_STRT_MSK          (0x00001FFF << PATTERN_LIMIT_STRT_OFST)
#define PATTERN_LIMIT_STP_OFST          (16)
#define PATTERN_LIMIT_STP_MSK           (0x00001FFF << PATTERN_LIMIT_STP_OFST)

/** Pattern Mask 64 bit RW regiser */
#define PATTERN_MASK_LSB_REG            (0x42 * REG_OFFSET + BASE_PATTERN_CONTROL)
#define PATTERN_MASK_MSB_REG            (0x43 * REG_OFFSET + BASE_PATTERN_CONTROL)

/** Pattern Set 64 bit RW regiser */
#define PATTERN_SET_LSB_REG             (0x44 * REG_OFFSET + BASE_PATTERN_CONTROL)
#define PATTERN_SET_MSB_REG             (0x45 * REG_OFFSET + BASE_PATTERN_CONTROL)

/* Pattern Wait Timer 0 64bit RW Register */
#define PATTERN_WAIT_TIMER_0_LSB_REG    (0x60 * REG_OFFSET + BASE_PATTERN_CONTROL)
#define PATTERN_WAIT_TIMER_0_MSB_REG    (0x61 * REG_OFFSET + BASE_PATTERN_CONTROL)

/* Pattern Wait 0 RW Register*/
#define PATTERN_WAIT_0_ADDR_REG         (0x62 * REG_OFFSET + BASE_PATTERN_CONTROL)

#define PATTERN_WAIT_0_ADDR_OFST        (0)
#define PATTERN_WAIT_0_ADDR_MSK         (0x00001FFF << PATTERN_WAIT_0_ADDR_OFST)

/* Pattern Loop 0 Iteration RW Register */
#define PATTERN_LOOP_0_ITERATION_REG    (0x63 * REG_OFFSET + BASE_PATTERN_CONTROL)

/* Pattern Loop 0 Address RW Register */
#define PATTERN_LOOP_0_ADDR_REG         (0x64 * REG_OFFSET + BASE_PATTERN_CONTROL)

#define PATTERN_LOOP_0_ADDR_STRT_OFST   (0)
#define PATTERN_LOOP_0_ADDR_STRT_MSK    (0x00001FFF << PATTERN_LOOP_0_ADDR_STRT_OFST)
#define PATTERN_LOOP_0_ADDR_STP_OFST    (16)
#define PATTERN_LOOP_0_ADDR_STP_MSK     (0x00001FFF << PATTERN_LOOP_0_ADDR_STP_OFST)

/* Pattern Wait Timer 1 64bit RW Register */
#define PATTERN_WAIT_TIMER_1_LSB_REG    (0x65 * REG_OFFSET + BASE_PATTERN_CONTROL)
#define PATTERN_WAIT_TIMER_1_MSB_REG    (0x66 * REG_OFFSET + BASE_PATTERN_CONTROL)

/* Pattern Wait 1 RW Register*/
#define PATTERN_WAIT_1_ADDR_REG         (0x67 * REG_OFFSET + BASE_PATTERN_CONTROL)

#define PATTERN_WAIT_1_ADDR_OFST        (0)
#define PATTERN_WAIT_1_ADDR_MSK         (0x00001FFF << PATTERN_WAIT_1_ADDR_OFST)

/* Pattern Loop 1 Iteration RW Register */
#define PATTERN_LOOP_1_ITERATION_REG    (0x68 * REG_OFFSET + BASE_PATTERN_CONTROL)

/* Pattern Loop 1 Address RW Register */
#define PATTERN_LOOP_1_ADDR_REG         (0x69 * REG_OFFSET + BASE_PATTERN_CONTROL)

#define PATTERN_LOOP_1_ADDR_STRT_OFST   (0)
#define PATTERN_LOOP_1_ADDR_STRT_MSK    (0x00001FFF << PATTERN_LOOP_1_ADDR_STRT_OFST)
#define PATTERN_LOOP_1_ADDR_STP_OFST    (16)
#define PATTERN_LOOP_1_ADDR_STP_MSK     (0x00001FFF << PATTERN_LOOP_1_ADDR_STP_OFST)

/* Pattern Wait Timer 2 64bit RW Register */
#define PATTERN_WAIT_TIMER_2_LSB_REG    (0x6A * REG_OFFSET + BASE_PATTERN_CONTROL)
#define PATTERN_WAIT_TIMER_2_MSB_REG    (0x6B * REG_OFFSET + BASE_PATTERN_CONTROL)

/* Pattern Wait 2 RW Register*/
#define PATTERN_WAIT_2_ADDR_REG         (0x6C * REG_OFFSET + BASE_PATTERN_CONTROL)

#define PATTERN_WAIT_2_ADDR_OFST        (0)
#define PATTERN_WAIT_2_ADDR_MSK         (0x00001FFF << PATTERN_WAIT_2_ADDR_OFST)

/* Pattern Loop 2 Iteration RW Register */
#define PATTERN_LOOP_2_ITERATION_REG    (0x6D * REG_OFFSET + BASE_PATTERN_CONTROL)

/* Pattern Loop 0 Address RW Register */
#define PATTERN_LOOP_2_ADDR_REG         (0x6E * REG_OFFSET + BASE_PATTERN_CONTROL)

#define PATTERN_LOOP_2_ADDR_STRT_OFST   (0)
#define PATTERN_LOOP_2_ADDR_STRT_MSK    (0x00001FFF << PATTERN_LOOP_2_ADDR_STRT_OFST)
#define PATTERN_LOOP_2_ADDR_STP_OFST    (16)
#define PATTERN_LOOP_2_ADDR_STP_MSK     (0x00001FFF << PATTERN_LOOP_2_ADDR_STP_OFST)

/* Pattern RAM registers --------------------------------------------------*/

/* Register of first word */
#define PATTERN_STEP0_LSB_REG           (0x0 * REG_OFFSET + BASE_PATTERN_RAM)
#define PATTERN_STEP0_MSB_REG           (0x1 * REG_OFFSET + BASE_PATTERN_RAM)


/* Flow Control registers
 * --------------------------------------------------*/

/* Flow status Register*/
#define FLOW_STATUS_REG                 (0x00 * REG_OFFSET + BASE_FLOW_CONTROL)

#define FLOW_STATUS_RUN_BUSY_OFST       (0)
#define FLOW_STATUS_RUN_BUSY_MSK        (0x00000001 << FLOW_STATUS_RUN_BUSY_OFST)
#define FLOW_STATUS_WAIT_FOR_TRGGR_OFST (3)
#define FLOW_STATUS_WAIT_FOR_TRGGR_MSK  (0x00000001 << FLOW_STATUS_WAIT_FOR_TRGGR_OFST)
#define FLOW_STATUS_DLY_BFRE_TRGGR_OFST (4)
#define FLOW_STATUS_DLY_BFRE_TRGGR_MSK  (0x00000001 << FLOW_STATUS_DLY_BFRE_TRGGR_OFST)
#define FLOW_STATUS_FIFO_FULL_OFST      (5)
#define FLOW_STATUS_FIFO_FULL_MSK       (0x00000001 << FLOW_STATUS_FIFO_FULL_OFST)
#define FLOW_STATUS_DLY_AFTR_TRGGR_OFST (15)
#define FLOW_STATUS_DLY_AFTR_TRGGR_MSK  (0x00000001 << FLOW_STATUS_DLY_AFTR_TRGGR_OFST)
#define FLOW_STATUS_CSM_BUSY_OFST       (17)
#define FLOW_STATUS_CSM_BUSY_MSK        (0x00000001 << FLOW_STATUS_CSM_BUSY_OFST)

/* Delay left 64bit Register */
#define GET_DELAY_LSB_REG               (0x02 * REG_OFFSET + BASE_FLOW_CONTROL)
#define GET_DELAY_MSB_REG               (0x03 * REG_OFFSET + BASE_FLOW_CONTROL)

/* Triggers left 64bit Register */
#define GET_CYCLES_LSB_REG              (0x04 * REG_OFFSET + BASE_FLOW_CONTROL)
#define GET_CYCLES_MSB_REG              (0x05 * REG_OFFSET + BASE_FLOW_CONTROL)

/* Frames left 64bit Register */
#define GET_FRAMES_LSB_REG              (0x06 * REG_OFFSET + BASE_FLOW_CONTROL)
#define GET_FRAMES_MSB_REG              (0x07 * REG_OFFSET + BASE_FLOW_CONTROL)

/* Period left 64bit Register */
#define GET_PERIOD_LSB_REG              (0x08 * REG_OFFSET + BASE_FLOW_CONTROL)
#define GET_PERIOD_MSB_REG              (0x09 * REG_OFFSET + BASE_FLOW_CONTROL)

/* Time from Start 64 bit register */
#define TIME_FROM_START_LSB_REG         (0x0A * REG_OFFSET + BASE_FLOW_CONTROL)
#define TIME_FROM_START_MSB_REG         (0x0B * REG_OFFSET + BASE_FLOW_CONTROL)

/* Get Frames from Start 64 bit register (frames from last reset using
 * CONTROL_CRST) */
#define FRAMES_FROM_START_LSB_REG       (0x0C * REG_OFFSET + BASE_FLOW_CONTROL)
#define FRAMES_FROM_START_MSB_REG       (0x0D * REG_OFFSET + BASE_FLOW_CONTROL)

/* Measurement Time 64 bit register (timestamp at a frame start until reset)*/
#define START_FRAME_TIME_LSB_REG        (0x0E * REG_OFFSET + BASE_FLOW_CONTROL)
#define START_FRAME_TIME_MSB_REG        (0x0F * REG_OFFSET + BASE_FLOW_CONTROL)

/* Delay 64bit Write-register */
#define SET_DELAY_LSB_REG               (0x22 * REG_OFFSET + BASE_FLOW_CONTROL)
#define SET_DELAY_MSB_REG               (0x23 * REG_OFFSET + BASE_FLOW_CONTROL)

/* Cylces 64bit Write-register */
#define SET_CYCLES_LSB_REG              (0x24 * REG_OFFSET + BASE_FLOW_CONTROL)
#define SET_CYCLES_MSB_REG              (0x25 * REG_OFFSET + BASE_FLOW_CONTROL)

/* Frames 64bit Write-register */
#define SET_FRAMES_LSB_REG              (0x26 * REG_OFFSET + BASE_FLOW_CONTROL)
#define SET_FRAMES_MSB_REG              (0x27 * REG_OFFSET + BASE_FLOW_CONTROL)

/* Period 64bit Write-register */
#define SET_PERIOD_LSB_REG              (0x28 * REG_OFFSET + BASE_FLOW_CONTROL)
#define SET_PERIOD_MSB_REG              (0x29 * REG_OFFSET + BASE_FLOW_CONTROL)

/* Flow Trigger register (for all timing modes) */
#define FLOW_TRIGGER_REG                (0x30 * REG_OFFSET + BASE_FLOW_CONTROL)

#define FLOW_TRIGGER_OFST               (0)
#define FLOW_TRIGGER_MSK                (0x00000001 << FLOW_TRIGGER_OFST)

/* Trigger Delay 64 bit register */
#define SET_TRIGGER_DELAY_LSB_REG       (0x32 * REG_OFFSET + BASE_FLOW_CONTROL)
#define SET_TRIGGER_DELAY_MSB_REG       (0x33 * REG_OFFSET + BASE_FLOW_CONTROL)


/* ASIC Readout Control registers
 * --------------------------------------------------*/

/** ASIC Readout Status register */
#define ASIC_RDO_STATUS_REG             (0x00 * REG_OFFSET + BASE_ASIC_RDO)

#define ASIC_RDO_STATUS_BUSY_OFST       (1)
#define ASIC_RDO_STATUS_BUSY_MSK        (0x00000001 << ASIC_RDO_STATUS_BUSY_OFST)


/** ASIC Readout Res Storage Counter register */
#define ASIC_RDO_CONFIG_REG             (0x01 * REG_OFFSET + BASE_ASIC_RDO)

#define ASICRDO_CNFG_RESSTRG_LNGTH_OFST (0)
#define ASICRDO_CNFG_RESSTRG_LNGTH_MSK  (0x000000FF << ASICRDO_CNFG_RESSTRG_LNGTH_OFST)


/** Dead time Free Controller 
 * --------------------------------------------------*/

#define DEADTIME_CONFIG_REG             (0x00 * REG_OFFSET + BASE_DEADTIME_FREE_CTRL)

#define DEADTIME_FREE_MODE_ENBL_OFST    (0)
#define DEADTIME_FREE_MODE_ENBL_MSK     (0x00000001 << DEADTIME_FREE_MODE_ENBL_OFST)
#define DEADTIME_EARLY_EXP_FIN_ERR_OFST (4)
#define DEADTIME_EARLY_EXP_FIN_ERR_MSK  (0x00000001 << DEADTIME_EARLY_EXP_FIN_ERR_OFST)

/* UDP datagram registers --------------------------------------------------*/
#define RXR_ENDPOINT_OFST                   (16 * REG_OFFSET)

// clang-format on