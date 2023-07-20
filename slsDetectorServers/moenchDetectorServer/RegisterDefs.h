// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once
// clang-format off

/* Definitions for FPGA*/
#define MEM_MAP_SHIFT 1

/* FPGA Version register */
#define FPGA_VERSION_REG                    (0x00 << MEM_MAP_SHIFT)

#define FPGA_COMPILATION_DATE_OFST          (0)
#define FPGA_COMPILATION_DATE_MSK           (0x00FFFFFF << FPGA_COMPILATION_DATE_OFST)
#define DETECTOR_TYPE_OFST                  (24)
#define DETECTOR_TYPE_MSK                   (0x000000FF << DETECTOR_TYPE_OFST)

/* Fix pattern register */
#define FIX_PATT_REG                        (0x01 << MEM_MAP_SHIFT)

#define FIX_PATT_VAL                        (0xACDC2014)

/* Status register */
#define STATUS_REG                          (0x02 << MEM_MAP_SHIFT)

#define RUN_BUSY_OFST                       (0)
#define RUN_BUSY_MSK                        (0x00000001 << RUN_BUSY_OFST)
#define WAITING_FOR_TRIGGER_OFST            (3)
#define WAITING_FOR_TRIGGER_MSK             (0x00000001 << WAITING_FOR_TRIGGER_OFST)
#define DELAYBEFORE_OFST                    (4) // delay before acq and after trigger
#define DELAYBEFORE_MSK                     (0x00000001 << DELAYBEFORE_OFST)
#define DELAYAFTER_OFST                     (5) // delay after acq until next frame
#define DELAYAFTER_MSK                      (0x00000001 << DELAYAFTER_OFST)
#define EXPOSING_OFST                       (6)
#define EXPOSING_MSK                        (0x00000001 << EXPOSING_OFST)
#define STOPPED_OFST                        (15)
#define STOPPED_MSK                         (0x00000001 << STOPPED_OFST)
#define FRAME_BUSY_OFST                     (18)
#define FRAME_BUSY_MSK                      (0x00000001 << FRAME_BUSY_OFST)
#define ADC_DESERON_OFST                    (19)
#define ADC_DESERON_MSK                     (0x00000001 << ADC_DESERON_OFST)
#define RUNMACHINE_BUSY_OFST                (17)
#define RUNMACHINE_BUSY_MSK                 (0x00000001 << RUNMACHINE_BUSY_OFST)

/* System Status register */
#define SYSTEM_STATUS_REG                   (0x04 << MEM_MAP_SHIFT) // Not used in software

#define RECONFIG_PLL_LCK_OFST               (3) 
#define RECONFIG_PLL_LCK_MSK                (0x00000001 << RECONFIG_PLL_LCK_OFST)
#define PLL_A_LCK_OFST                      (4)
#define PLL_A_LCK_MSK                       (0x00000001 << PLL_A_LCK_OFST)

/* Module Control Board Serial Number Register */
#define MOD_SERIAL_NUM_REG                  (0x0A << MEM_MAP_SHIFT)

#define HARDWARE_SERIAL_NUM_OFST            (0)
#define HARDWARE_SERIAL_NUM_MSK             (0x000000FF << HARDWARE_SERIAL_NUM_OFST)
#define HARDWARE_VERSION_NUM_OFST           (16)
#define HARDWARE_VERSION_NUM_MSK            (0x0000003F << HARDWARE_VERSION_NUM_OFST)

/* API Version Register */
#define API_VERSION_REG                     (0x0F << MEM_MAP_SHIFT)

#define API_VERSION_OFST                    (0)
#define API_VERSION_MSK                     (0x00FFFFFF << API_VERSION_OFST)

/* Time from Start 64 bit register */
#define TIME_FROM_START_LSB_REG             (0x10 << MEM_MAP_SHIFT)
#define TIME_FROM_START_MSB_REG             (0x11 << MEM_MAP_SHIFT)

/* Get Delay 64 bit register */
#define GET_DELAY_LSB_REG                   (0x12 << MEM_MAP_SHIFT) // different kind of delay
#define GET_DELAY_MSB_REG                   (0x13 << MEM_MAP_SHIFT) // different kind of delay

/* Get Triggers 64 bit register */
#define GET_CYCLES_LSB_REG                  (0x14 << MEM_MAP_SHIFT)
#define GET_CYCLES_MSB_REG                  (0x15 << MEM_MAP_SHIFT)

/* Get Frames 64 bit register */
#define GET_FRAMES_LSB_REG                  (0x16 << MEM_MAP_SHIFT)
#define GET_FRAMES_MSB_REG                  (0x17 << MEM_MAP_SHIFT)

/* Get Period 64 bit register tT = T x 50 ns */
#define GET_PERIOD_LSB_REG                  (0x18 << MEM_MAP_SHIFT)
#define GET_PERIOD_MSB_REG                  (0x19 << MEM_MAP_SHIFT)

/** Get Temperature */
#define GET_TEMPERATURE_TMP112_REG          (0x1c << MEM_MAP_SHIFT) // (after multiplying by 625) in 10ths of
                            // millidegrees of TMP112

#define TEMPERATURE_VALUE_BIT               (0)
#define TEMPERATURE_VALUE_MSK               (0x000007FF << TEMPERATURE_VALUE_BIT)
#define TEMPERATURE_POLARITY_BIT            (11)
#define TEMPERATURE_POLARITY_MSK            (0x00000001 << TEMPERATURE_POLARITY_BIT)

/* Get Frames from Start 64 bit register (frames from last reset using
 * CONTROL_CRST) */
#define FRAMES_FROM_START_LSB_REG           (0x22 << MEM_MAP_SHIFT)
#define FRAMES_FROM_START_MSB_REG           (0x23 << MEM_MAP_SHIFT)

/* Get Next Frame Number */
#define GET_NEXT_FRAME_NUMBER_LSB_REG       (0x24 << MEM_MAP_SHIFT)
#define GET_NEXT_FRAME_NUMBER_MSB_REG       (0x25 << MEM_MAP_SHIFT)

/* Measurement Time 64 bit register (timestamp at a frame start until reset)*/
#define START_FRAME_TIME_LSB_REG            (0x26 << MEM_MAP_SHIFT)
#define START_FRAME_TIME_MSB_REG            (0x27 << MEM_MAP_SHIFT)

/* SPI (Serial Peripheral Interface) Register */
#define SPI_REG                             (0x40 << MEM_MAP_SHIFT)

#define SPI_DAC_SRL_DGTL_OTPT_OFST          (0)
#define SPI_DAC_SRL_DGTL_OTPT_MSK           (0x00000001 << SPI_DAC_SRL_DGTL_OTPT_OFST)
#define SPI_DAC_SRL_CLK_OTPT_OFST           (1)
#define SPI_DAC_SRL_CLK_OTPT_MSK            (0x00000001 << SPI_DAC_SRL_CLK_OTPT_OFST)
#define SPI_DAC_SRL_CS_OTPT_OFST            (2)
#define SPI_DAC_SRL_CS_OTPT_MSK             (0x00000001 << SPI_DAC_SRL_CS_OTPT_OFST)
#define SPI_HV_SRL_DGTL_OTPT_OFST           (8)
#define SPI_HV_SRL_DGTL_OTPT_MSK            (0x00000001 << SPI_HV_SRL_DGTL_OTPT_OFST)
#define SPI_HV_SRL_CLK_OTPT_OFST            (9)
#define SPI_HV_SRL_CLK_OTPT_MSK             (0x00000001 << SPI_HV_SRL_CLK_OTPT_OFST)
#define SPI_HV_SRL_CS_OTPT_OFST             (10)
#define SPI_HV_SRL_CS_OTPT_MSK              (0x00000001 << SPI_HV_SRL_CS_OTPT_OFST)

/* ADC SPI (Serial Peripheral Interface) Register */
#define ADC_SPI_REG                         (0x41 << MEM_MAP_SHIFT)

#define ADC_SPI_SRL_CLK_OTPT_OFST           (0)
#define ADC_SPI_SRL_CLK_OTPT_MSK            (0x00000001 << ADC_SPI_SRL_CLK_OTPT_OFST)
#define ADC_SPI_SRL_DT_OTPT_OFST            (1)
#define ADC_SPI_SRL_DT_OTPT_MSK             (0x00000001 << ADC_SPI_SRL_DT_OTPT_OFST)
#define ADC_SPI_SRL_CS_OTPT_OFST            (2)
#define ADC_SPI_SRL_CS_OTPT_MSK             (0x0000000F << ADC_SPI_SRL_CS_OTPT_OFST)

/* ADC offset Register */
#define ADC_OFST_REG                        (0x42 << MEM_MAP_SHIFT)

#define ADC_OFFSET_OFST                     (0)
#define ADC_OFFSET_MSK                      (0x0000001F << ADC_OFFSET_OFST)

/* ADC Port Invert Register */
#define ADC_PORT_INVERT_REG                 (0x43 << MEM_MAP_SHIFT)

#define ADC_PORT_INVERT_ADC_0_OFST          (0)
#define ADC_PORT_INVERT_ADC_0_MSK           (0x000000FF << ADC_PORT_INVERT_ADC_0_OFST)
#define ADC_PORT_INVERT_ADC_1_OFST          (8)
#define ADC_PORT_INVERT_ADC_1_MSK           (0x000000FF << ADC_PORT_INVERT_ADC_1_OFST)
#define ADC_PORT_INVERT_ADC_2_OFST          (16)
#define ADC_PORT_INVERT_ADC_2_MSK           (0x000000FF << ADC_PORT_INVERT_ADC_2_OFST)
#define ADC_PORT_INVERT_ADC_3_OFST          (24)
#define ADC_PORT_INVERT_ADC_3_MSK           (0x000000FF << ADC_PORT_INVERT_ADC_3_OFST)

/** Read N Rows Register */
#define READ_N_ROWS_REG                     (0x44 << MEM_MAP_SHIFT)

#define READ_N_ROWS_NUM_ROWS_OFST           (0)
#define READ_N_ROWS_NUM_ROWS_MSK            (0x0000003F << READ_N_ROWS_NUM_ROWS_OFST)
#define READ_N_ROWS_ENBL_OFST               (7)
#define READ_N_ROWS_ENBL_MSK                (0x00000001 << READ_N_ROWS_ENBL_OFST)


/* Configuration Register */
#define CONFIG_REG                          (0x4D << MEM_MAP_SHIFT)

// if 0, outer is the primary interface
// bottom via port 0 (outer)
#define CONFIG_OPRTN_MDE_2_X_10GbE_OFST     (16)
#define CONFIG_OPRTN_MDE_2_X_10GbE_MSK      (0x00000001 << CONFIG_OPRTN_MDE_2_X_10GbE_OFST) 
#define CONFIG_INNR_PRIMRY_INTRFCE_OFST     (17)
#define CONFIG_INNR_PRIMRY_INTRFCE_MSK      (0x00000001 << CONFIG_INNR_PRIMRY_INTRFCE_OFST)
#define CONFIG_READOUT_SPEED_OFST           (20)
#define CONFIG_READOUT_SPEED_MSK            (0x00000003 << CONFIG_READOUT_SPEED_OFST)
#define CONFIG_QUARTER_SPEED_10MHZ_VAL      ((0x0 << CONFIG_READOUT_SPEED_OFST) & CONFIG_READOUT_SPEED_MSK)
#define CONFIG_HALF_SPEED_20MHZ_VAL         ((0x1 << CONFIG_READOUT_SPEED_OFST) & CONFIG_READOUT_SPEED_MSK)
#define CONFIG_FULL_SPEED_40MHZ_VAL         ((0x2 << CONFIG_READOUT_SPEED_OFST) & CONFIG_READOUT_SPEED_MSK)
#define CONFIG_TDMA_ENABLE_OFST             (24)
#define CONFIG_TDMA_ENABLE_MSK              (0x00000001 << CONFIG_TDMA_ENABLE_OFST)
#define CONFIG_TDMA_TIMESLOT_OFST           (25) // 1ms
#define CONFIG_TDMA_TIMESLOT_MSK            (0x0000001F << CONFIG_TDMA_TIMESLOT_OFST)
#define CONFIG_BOTTOM_INVERT_STREAM_OFST    (30)
#define CONFIG_BOTTOM_INVERT_STREAM_MSK     (0x00000001 << CONFIG_BOTTOM_INVERT_STREAM_OFST)
#define CONFIG_ETHRNT_FLW_CNTRL_OFST        (31)
#define CONFIG_ETHRNT_FLW_CNTRL_MSK         (0x00000001 << CONFIG_ETHRNT_FLW_CNTRL_OFST)

/* External Signal Register */
#define EXT_SIGNAL_REG                      (0x4E << MEM_MAP_SHIFT)

#define EXT_SIGNAL_OFST                     (0)
#define EXT_SIGNAL_MSK                      (0x00000001 << EXT_SIGNAL_OFST)
#define EXT_SYNC_OFST                       (4)
#define EXT_SYNC_MSK                        (0x00000001 << EXT_SYNC_OFST)

/* Control Register */
#define CONTROL_REG                         (0x4F << MEM_MAP_SHIFT)

#define CONTROL_START_ACQ_OFST              (0)
#define CONTROL_START_ACQ_MSK               (0x00000001 << CONTROL_START_ACQ_OFST)
#define CONTROL_STOP_ACQ_OFST               (1)
#define CONTROL_STOP_ACQ_MSK                (0x00000001 << CONTROL_STOP_ACQ_OFST)
#define CONTROL_SOFTWARE_TRIGGER_OFST       (2)
#define CONTROL_SOFTWARE_TRIGGER_MSK        (0x00000001 << CONTROL_SOFTWARE_TRIGGER_OFST)
#define CONTROL_CORE_RST_OFST               (10) // flow, dac driver, 10GbE
#define CONTROL_CORE_RST_MSK                (0x00000001 << CONTROL_CORE_RST_OFST)
#define CONTROL_PERIPHERAL_RST_OFST         (11) // 10GBE
#define CONTROL_PERIPHERAL_RST_MSK          (0x00000001 << CONTROL_PERIPHERAL_RST_OFST)
#define CONTROL_ACQ_FIFO_CLR_OFST           (14)
#define CONTROL_ACQ_FIFO_CLR_MSK            (0x00000001 << CONTROL_ACQ_FIFO_CLR_OFST)
#define CONTROL_MASTER_OFST                 (15)
#define CONTROL_MASTER_MSK                  (0x00000001 << CONTROL_MASTER_OFST)
#define CONTROL_RX_ADDTNL_ENDPTS_NUM_OFST   (20)
#define CONTROL_RX_ADDTNL_ENDPTS_NUM_MSK    (0x0000003F << CONTROL_RX_ADDTNL_ENDPTS_NUM_OFST)
#define CONTROL_RX_ENDPTS_START_OFST        (26)
#define CONTROL_RX_ENDPTS_START_MSK         (0x0000003F << CONTROL_RX_ENDPTS_START_OFST)

/* Reconfiguratble PLL Paramater Register */
#define PLL_PARAM_REG                       (0x50 << MEM_MAP_SHIFT)

/* Reconfiguratble PLL Control Regiser */
#define PLL_CNTRL_REG                       (0x51 << MEM_MAP_SHIFT)

#define PLL_CNTRL_RCNFG_PRMTR_RST_OFST      (0) // parameter reset
#define PLL_CNTRL_RCNFG_PRMTR_RST_MSK       (0x00000001 << PLL_CNTRL_RCNFG_PRMTR_RST_OFST) // parameter reset
#define PLL_CNTRL_WR_PRMTR_OFST             (2)
#define PLL_CNTRL_WR_PRMTR_MSK              (0x00000001 << PLL_CNTRL_WR_PRMTR_OFST)
#define PLL_CNTRL_PLL_RST_OFST              (3)
#define PLL_CNTRL_PLL_RST_MSK               (0x00000001 << PLL_CNTRL_PLL_RST_OFST)
#define PLL_CNTRL_ADDR_OFST                 (16)
#define PLL_CNTRL_ADDR_MSK                  (0x0000003F << PLL_CNTRL_ADDR_OFST)

/* Sample Register */
#define SAMPLE_REG                          (0x59 << MEM_MAP_SHIFT)

#define SAMPLE_ADC_SAMPLE_SEL_OFST          (0)
#define SAMPLE_ADC_SAMPLE_SEL_MSK           (0x00000007 << SAMPLE_ADC_SAMPLE_SEL_OFST)
#define SAMPLE_ADC_SAMPLE_0_VAL             ((0x0 << SAMPLE_ADC_SAMPLE_SEL_OFST) & SAMPLE_ADC_SAMPLE_SEL_MSK)
#define SAMPLE_ADC_SAMPLE_1_VAL             ((0x1 << SAMPLE_ADC_SAMPLE_SEL_OFST) & SAMPLE_ADC_SAMPLE_SEL_MSK)
#define SAMPLE_ADC_SAMPLE_2_VAL             ((0x2 << SAMPLE_ADC_SAMPLE_SEL_OFST) & SAMPLE_ADC_SAMPLE_SEL_MSK)
#define SAMPLE_ADC_SAMPLE_3_VAL             ((0x3 << SAMPLE_ADC_SAMPLE_SEL_OFST) & SAMPLE_ADC_SAMPLE_SEL_MSK)
#define SAMPLE_ADC_SAMPLE_4_VAL             ((0x4 << SAMPLE_ADC_SAMPLE_SEL_OFST) & SAMPLE_ADC_SAMPLE_SEL_MSK)
#define SAMPLE_ADC_SAMPLE_5_VAL             ((0x5 << SAMPLE_ADC_SAMPLE_SEL_OFST) & SAMPLE_ADC_SAMPLE_SEL_MSK)
#define SAMPLE_ADC_SAMPLE_6_VAL             ((0x6 << SAMPLE_ADC_SAMPLE_SEL_OFST) & SAMPLE_ADC_SAMPLE_SEL_MSK)
#define SAMPLE_ADC_SAMPLE_7_VAL             ((0x7 << SAMPLE_ADC_SAMPLE_SEL_OFST) & SAMPLE_ADC_SAMPLE_SEL_MSK)
// Decimation = ADF + 1
#define SAMPLE_ADC_DECMT_FACTOR_OFST        (4)
#define SAMPLE_ADC_DECMT_FACTOR_MSK         (0x00000007 << SAMPLE_ADC_DECMT_FACTOR_OFST)
#define SAMPLE_ADC_DECMT_FACTOR_0_VAL       ((0x0 << SAMPLE_ADC_DECMT_FACTOR_OFST) & SAMPLE_ADC_DECMT_FACTOR_MSK)
#define SAMPLE_ADC_DECMT_FACTOR_1_VAL       ((0x1 << SAMPLE_ADC_DECMT_FACTOR_OFST) & SAMPLE_ADC_DECMT_FACTOR_MSK)
#define SAMPLE_ADC_DECMT_FACTOR_2_VAL       ((0x2 << SAMPLE_ADC_DECMT_FACTOR_OFST) & SAMPLE_ADC_DECMT_FACTOR_MSK)
#define SAMPLE_ADC_DECMT_FACTOR_3_VAL       ((0x3 << SAMPLE_ADC_DECMT_FACTOR_OFST) & SAMPLE_ADC_DECMT_FACTOR_MSK)
#define SAMPLE_ADC_DECMT_FACTOR_4_VAL       ((0x4 << SAMPLE_ADC_DECMT_FACTOR_OFST) & SAMPLE_ADC_DECMT_FACTOR_MSK)
#define SAMPLE_ADC_DECMT_FACTOR_5_VAL       ((0x5 << SAMPLE_ADC_DECMT_FACTOR_OFST) & SAMPLE_ADC_DECMT_FACTOR_MSK)
#define SAMPLE_ADC_DECMT_FACTOR_6_VAL       ((0x6 << SAMPLE_ADC_DECMT_FACTOR_OFST) & SAMPLE_ADC_DECMT_FACTOR_MSK)
#define SAMPLE_ADC_DECMT_FACTOR_7_VAL       ((0x7 << SAMPLE_ADC_DECMT_FACTOR_OFST) & SAMPLE_ADC_DECMT_FACTOR_MSK)

/** Chip Power Register */
#define CHIP_POWER_REG                      (0x5E << MEM_MAP_SHIFT)

#define CHIP_POWER_ENABLE_OFST              (0)
#define CHIP_POWER_ENABLE_MSK               (0x00000001 << CHIP_POWER_ENABLE_OFST)
#define CHIP_POWER_STATUS_OFST              (1)
#define CHIP_POWER_STATUS_MSK               (0x00000001 << CHIP_POWER_STATUS_OFST)

/** Temperature Control Register */
#define TEMP_CTRL_REG                       (0x5F << MEM_MAP_SHIFT)

#define TEMP_CTRL_PROTCT_THRSHLD_OFST       (0)
#define TEMP_CTRL_PROTCT_THRSHLD_MSK        (0x000007FF << TEMP_CTRL_PROTCT_THRSHLD_OFST)
#define TEMP_CTRL_PROTCT_ENABLE_OFST        (16)
#define TEMP_CTRL_PROTCT_ENABLE_MSK         (0x00000001 << TEMP_CTRL_PROTCT_ENABLE_OFST)
// set when temp higher than over threshold, write 1 to clear it
#define TEMP_CTRL_OVR_TMP_EVNT_OFST         (31)
#define TEMP_CTRL_OVR_TMP_EVNT_MSK          (0x00000001 << TEMP_CTRL_OVR_TMP_EVNT_OFST)

/* Set Delay 64 bit register */
#define SET_DELAY_LSB_REG                   (0x60 << MEM_MAP_SHIFT) // different kind of delay
#define SET_DELAY_MSB_REG                   (0x61 << MEM_MAP_SHIFT) // different kind of delay

/* Set Triggers 64 bit register */
#define SET_CYCLES_LSB_REG                  (0x62 << MEM_MAP_SHIFT)
#define SET_CYCLES_MSB_REG                  (0x63 << MEM_MAP_SHIFT)

/* Set Frames 64 bit register */
#define SET_FRAMES_LSB_REG                  (0x64 << MEM_MAP_SHIFT)
#define SET_FRAMES_MSB_REG                  (0x65 << MEM_MAP_SHIFT)

/* Set Period 64 bit register tT = T x 50 ns */
#define SET_PERIOD_LSB_REG                  (0x66 << MEM_MAP_SHIFT)
#define SET_PERIOD_MSB_REG                  (0x67 << MEM_MAP_SHIFT)

/* Set Exptime 64 bit register eEXP = Exp x 25 ns */
#define SET_EXPTIME_LSB_REG                 (0x68 << MEM_MAP_SHIFT)
#define SET_EXPTIME_MSB_REG                 (0x69 << MEM_MAP_SHIFT)

/* Set next Frame number 64 bit register */
#define SET_NEXT_FRAME_NUMBER_LSB_REG       (0x6A << MEM_MAP_SHIFT)
#define SET_NEXT_FRAME_NUMBER_MSB_REG       (0x6B << MEM_MAP_SHIFT)


/* Trigger Delay 32 bit register */
#define SET_TRIGGER_DELAY_LSB_REG           (0x70 << MEM_MAP_SHIFT)
#define SET_TRIGGER_DELAY_MSB_REG           (0x71 << MEM_MAP_SHIFT)

/** Module row coordinates */
#define COORD_ROW_REG                       (0x7C << MEM_MAP_SHIFT)

#define COORD_ROW_OUTER_OFST                (0)
#define COORD_ROW_OUTER_MSK                 (0x0000FFFF << COORD_ROW_OUTER_OFST)
#define COORD_ROW_INNER_OFST                (16)
#define COORD_ROW_INNER_MSK                 (0x0000FFFF << COORD_ROW_INNER_OFST)

/** Module column coordinates */
#define COORD_COL_REG                       (0x7D << MEM_MAP_SHIFT)

#define COORD_COL_OUTER_OFST                (0)
#define COORD_COL_OUTER_MSK                 (0x0000FFFF << COORD_COL_OUTER_OFST)
#define COORD_COL_INNER_OFST                (16)
#define COORD_COL_INNER_MSK                 (0x0000FFFF << COORD_COL_INNER_OFST)

/** Module ID coordinates */
#define MOD_ID_REG                          (0x7E << MEM_MAP_SHIFT)

#define MOD_ID_OFST                         (0)
#define MOD_ID_MSK                          (0x0000FFFF << MOD_ID_OFST)

/* ASIC Control Register */
#define ASIC_CTRL_REG                       (0x7F << MEM_MAP_SHIFT)

#define ASIC_CTRL_PARALLEL_RD_OFST          (0)
#define ASIC_CTRL_PARALLEL_RD_MSK           (0x00000001 << ASIC_CTRL_PARALLEL_RD_OFST)
#define ASIC_CTRL_INTRFCE_CLK_PLRTY_OFST    (1)
#define ASIC_CTRL_INTRFCE_CLK_PLRTY_MSK     (0x00000001 << ASIC_CTRL_INTRFCE_CLK_PLRTY_OFST)
#define ASIC_CTRL_PULSETOP_OFST             (4)
#define ASIC_CTRL_PULSETOP_MSK              (0x00000001 << ASIC_CTRL_PULSETOP_OFST
#define ASIC_CTRL_PULSEBOT_OFST             (5)
#define ASIC_CTRL_PULSEBOT_MSK              (0x00000001 << ASIC_CTRL_PULSEBOT_OFST)
#define ASIC_CTRL_ENPRECHPREBOT_OFST        (6)
#define ASIC_CTRL_ENPRECHPREBOT_MSK         (0x00000001 << ASIC_CTRL_ENPRECHPREBOT_OFST)
#define ASIC_CTRL_DSG1_BOT_OFST             (7)
#define ASIC_CTRL_DSG1_BOT_MSK              (0x00000001 << ASIC_CTRL_DSG1_BOT_OFST)
#define ASIC_CTRL_BYPASSCDSBOT_OFST         (8)
#define ASIC_CTRL_BYPASSCDSBOT_MSK          (0x00000001 << ASIC_CTRL_BYPASSCDSBOT_OFST)
#define ASIC_CTRL_HG_BOT_OFST               (9)
#define ASIC_CTRL_HG_BOT_MSK                (0x00000001 << ASIC_CTRL_HG_BOT_OFST)
#define ASIC_CTRL_STO2TOP_OFST              (10)
#define ASIC_CTRL_STO2TOP_MSK               (0x00000001 << ASIC_CTRL_STO2TOP_OFST)
#define ASIC_CTRL_PRECHARGECONNECTBOT_OFST  (11)
#define ASIC_CTRL_PRECHARGECONNECTBOT_MSK   (0x00000001 << ASIC_CTRL_PRECHARGECONNECTBOT_OFST)
#define ASIC_CTRL_CONNCDSTOP_OFST           (12)
#define ASIC_CTRL_CONNCDSTOP_MSK            (0x00000001 << ASIC_CTRL_CONNCDSTOP_OFST)
#define ASIC_CTRL_BYPASSTOP_OFST            (13)
#define ASIC_CTRL_BYPASSTOP_MSK             (0x00000001 << ASIC_CTRL_BYPASSTOP_OFST)
#define ASIC_CTRL_STO1TOP_OFST              (14)
#define ASIC_CTRL_STO1TOP_MSK               (0x00000001 << ASIC_CTRL_STO1TOP_OFST)
#define ASIC_CTRL_DSG3_TOP_OFST             (15)
#define ASIC_CTRL_DSG3_TOP_MSK              (0x00000001 << ASIC_CTRL_DSG3_TOP_OFST)
#define ASIC_CTRL_S2DTESTEN_OFST            (16)
#define ASIC_CTRL_S2DTESTEN_MSK             (0x00000001 << ASIC_CTRL_S2DTESTEN_OFST)
#define ASIC_CTRL_BOTSRTESTBOT_OFST         (17)
#define ASIC_CTRL_BOTSRTESTBOT_MSK          (0x00000001 << ASIC_CTRL_BOTSRTESTBOT_OFST)
#define ASIC_CTRL_PRESETSSR_OFST            (18)
#define ASIC_CTRL_PRESETSSR_MSK             (0x00000001 << ASIC_CTRL_PRESETSSR_OFST)
#define ASIC_CTRL_PULSEOFFTOP_OFST          (19)
#define ASIC_CTRL_PULSEOFFTOP_MSK           (0x00000001 << ASIC_CTRL_PULSEOFFTOP_OFST)
#define ASIC_CTRL_PULSEOFFBOT_OFST          (20)
#define ASIC_CTRL_PULSEOFFBOT_MSK           (0x00000001 << ASIC_CTRL_PULSEOFFBOT_OFST)
#define ASIC_CTRL_CONNCDSBOT_OFST           (21)
#define ASIC_CTRL_CONNCDSBOT_MSK            (0x00000001 << ASIC_CTRL_CONNCDSBOT_OFST)
#define ASIC_CTRL_ENPRECHPRETOP_OFST        (22)
#define ASIC_CTRL_ENPRECHPRETOP_MSK         (0x00000001 << ASIC_CTRL_ENPRECHPRETOP_OFST)
#define ASIC_CTRL_BYPASSBOT_OFST            (23)
#define ASIC_CTRL_BYPASSBOT_MSK             (0x00000001 << ASIC_CTRL_BYPASSBOT_OFST)
#define ASIC_CTRL_DSG1_TOP_OFST             (24)
#define ASIC_CTRL_DSG1_TOP_MSK              (0x00000001 << ASIC_CTRL_DSG1_TOP_OFST)
#define ASIC_CTRL_STO1BOT_OFST              (25)
#define ASIC_CTRL_STO1BOT_MSK               (0x00000001 << ASIC_CTRL_STO1BOT_OFST)
#define ASIC_CTRL_BYPASSCDSTOP_OFST         (26)
#define ASIC_CTRL_BYPASSCDSTOP_MSK          (0x00000001 << ASIC_CTRL_BYPASSCDSTOP_OFST)
#define ASIC_CTRL_DSG3_BOT_OFST             (27)
#define ASIC_CTRL_DSG3_BOT_MSK              (0x00000001 << ASIC_CTRL_DSG3_BOT_OFST)
#define ASIC_CTRL_HG_TOP_OFST               (28)
#define ASIC_CTRL_HG_TOP_MSK                (0x00000001 << ASIC_CTRL_HG_TOP_OFST)
#define ASIC_CTRL_STO2BOT_OFST              (29)
#define ASIC_CTRL_STO2BOT_MSK               (0x00000001 << ASIC_CTRL_STO2BOT_OFST)
#define ASIC_CTRL_PRECHARGECONNECTTOP_OFST  (30)
#define ASIC_CTRL_PRECHARGECONNECTTOP_MSK   (0x00000001 << ASIC_CTRL_PRECHARGECONNECTTOP_OFST)
#define ASIC_CTRL_BOTSRTESTTOP_OFST         (31)
#define ASIC_CTRL_BOTSRTESTTOP_MSK          (0x00000001 << ASIC_CTRL_BOTSRTESTTOP_OFST)

#define ASIC_CTRL_DEFAULT_VAL               (ASIC_CTRL_DSG1_BOT_MSK | \
                                            ASIC_CTRL_HG_BOT_MSK | ASIC_CTRL_STO2TOP_MSK | \
                                            ASIC_CTRL_CONNCDSTOP_MSK | ASIC_CTRL_STO1TOP_MSK | \
                                            ASIC_CTRL_BOTSRTESTBOT_MSK | ASIC_CTRL_PULSEOFFTOP_MSK | \
                                            ASIC_CTRL_PULSEOFFBOT_MSK | ASIC_CTRL_CONNCDSBOT_MSK | \
                                            ASIC_CTRL_DSG1_TOP_MSK | ASIC_CTRL_STO1BOT_MSK | \
                                            ASIC_CTRL_HG_TOP_MSK | ASIC_CTRL_STO2BOT_MSK | \
                                            ASIC_CTRL_BOTSRTESTTOP_MSK)

#define ASIC_CTRL_HG_MSK                    (ASIC_CTRL_HG_TOP_MSK | ASIC_CTRL_HG_BOT_MSK)
#define ASIC_CTRL_DSG1_MSK                  (ASIC_CTRL_DSG1_TOP_MSK | ASIC_CTRL_DSG1_BOT_MSK)
#define ASIC_CTRL_DSG3_MSK                  (ASIC_CTRL_DSG3_TOP_MSK | ASIC_CTRL_DSG3_BOT_MSK)

#define SETTINGS_MSK                        (ASIC_CTRL_HG_MSK | ASIC_CTRL_DSG1_MSK | ASIC_CTRL_DSG3_MSK)
#define SETTINGS_G1_HG                      (ASIC_CTRL_HG_MSK | ASIC_CTRL_DSG3_MSK)                    
#define SETTINGS_G1_LG                      (ASIC_CTRL_DSG3_MSK)                        
#define SETTINGS_G2_HC_HG                   (ASIC_CTRL_HG_MSK)                         
#define SETTINGS_G2_HC_LG                   (0)                         
#define SETTINGS_G2_LC_HG                   (ASIC_CTRL_HG_MSK | ASIC_CTRL_DSG1_MSK | ASIC_CTRL_DSG3_MSK)                      
#define SETTINGS_G2_LC_LG                   (ASIC_CTRL_DSG1_MSK | ASIC_CTRL_DSG3_MSK)                      
#define SETTINGS_G4_HG                      (ASIC_CTRL_HG_MSK | ASIC_CTRL_DSG1_MSK)                      
#define SETTINGS_G4_LG                      (ASIC_CTRL_DSG1_MSK)                      




/* ADC 0 Deserializer Control */
#define ADC_DSRLZR_0_REG                    (0xF0 << MEM_MAP_SHIFT)
#define ADC_DSRLZR_0_RESET_ALGNMNT_OFST     (30)
#define ADC_DSRLZR_0_RESET_ALGNMNT_MSK      (0x00000001 << ADC_DSRLZR_0_RESET_ALGNMNT_OFST)
#define ADC_DSRLZR_0_RFRSH_ALGNMNT_OFST     (31) /* Refresh alignment */
#define ADC_DSRLZR_0_RFRSH_ALGNMNT_MSK      (0x00000001 << ADC_DSRLZR_0_RFRSH_ALGNMNT_OFST)

/* ADC 0 Deserializer Control */
#define ADC_DSRLZR_1_REG                    (0xF1 << MEM_MAP_SHIFT)
#define ADC_DSRLZR_1_RESET_ALGNMNT_OFST     (30)
#define ADC_DSRLZR_1_RESET_ALGNMNT_MSK      (0x00000001 << ADC_DSRLZR_1_RESET_ALGNMNT_OFST)
#define ADC_DSRLZR_1_RFRSH_ALGNMNT_OFST     (31)
#define ADC_DSRLZR_1_RFRSH_ALGNMNT_MSK      (0x00000001 << ADC_DSRLZR_1_RFRSH_ALGNMNT_OFST)

/* ADC 0 Deserializer Control */
#define ADC_DSRLZR_2_REG                    (0xF2 << MEM_MAP_SHIFT)
#define ADC_DSRLZR_2_RESET_ALGNMNT_OFST     (30)
#define ADC_DSRLZR_2_RESET_ALGNMNT_MSK      (0x00000001 << ADC_DSRLZR_2_RESET_ALGNMNT_OFST)
#define ADC_DSRLZR_2_RFRSH_ALGNMNT_OFST     (31)
#define ADC_DSRLZR_2_RFRSH_ALGNMNT_MSK      (0x00000001 << ADC_DSRLZR_2_RFRSH_ALGNMNT_OFST)

/* ADC 0 Deserializer Control */
#define ADC_DSRLZR_3_REG                    (0xF3 << MEM_MAP_SHIFT)
#define ADC_DSRLZR_3_RESET_ALGNMNT_OFST     (30)
#define ADC_DSRLZR_3_RESET_ALGNMNT_MSK      (0x00000001 << ADC_DSRLZR_3_RESET_ALGNMNT_OFST)
#define ADC_DSRLZR_3_RFRSH_ALGNMNT_OFST     (31)
#define ADC_DSRLZR_3_RFRSH_ALGNMNT_MSK      (0x00000001 << ADC_DSRLZR_3_RFRSH_ALGNMNT_OFST)

/* Round Robin  */
#define RXR_ENDPOINTS_MAX                   (64)
#define RXR_ENDPOINT_OUTER_START_REG        (0x1000 << MEM_MAP_SHIFT)
#define RXR_ENDPOINT_INNER_START_REG        (0x2000 << MEM_MAP_SHIFT)

#define RXR_ENDPOINT_OFST                   (0x10 << MEM_MAP_SHIFT)

// clang-format on
