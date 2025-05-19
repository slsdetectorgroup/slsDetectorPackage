// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once
#include "RegisterDefs.h"
#include "sls/sls_detector_defs.h"

#define MIN_REQRD_VRSN_T_RD_API 0x181130
#define REQRD_FRMWR_VRSN        0x230705

#define NUM_HARDWARE_VERSIONS (1)
#define HARDWARE_VERSION_NUMBERS                                               \
    { 0x3f }
#define HARDWARE_VERSION_NAMES                                                 \
    { "5.1" }

#define LINKED_SERVER_NAME "ctbDetectorServer"

#define CTRL_SRVR_INIT_TIME_US (2 * 1000 * 1000)

/* Hardware Definitions */
#define NCHAN                   (40)
#define NCHAN_ANALOG            (32)
#define NCHAN_DIGITAL           (64)
#define NCHAN_TRANSCEIVER       (4)
#define NBITS_PER_TRANSCEIVER   (64)
#define NCHIP                   (1)
#define NDAC                    (24)
#define NPWR                    (6)
#define NDAC_ONLY               (NDAC - NPWR)
#define DYNAMIC_RANGE           (16)
#define NUM_BYTES_PER_PIXEL     (DYNAMIC_RANGE / 8)
#define CLK_FREQ                (156.25) // MHz
#define I2C_POWER_VIO_DEVICE_ID (0x40)
#define I2C_POWER_VA_DEVICE_ID  (0x41)
#define I2C_POWER_VB_DEVICE_ID  (0x42)
#define I2C_POWER_VC_DEVICE_ID  (0x43)
#define I2C_POWER_VD_DEVICE_ID  (0x44)
#define I2C_SHUNT_RESISTER_OHMS (0.005)

/** Default Parameters */
#define DEFAULT_DATA_BYTES            (NCHIP * NCHAN * NUM_BITS_PER_PIXEL)
#define DEFAULT_STARTING_FRAME_NUMBER (1)
#define DEFAULT_NUM_SAMPLES           (1)
#define DEFAULT_NUM_FRAMES            (1)
#define DEFAULT_EXPTIME               (0)
#define DEFAULT_NUM_CYCLES            (1)
#define DEFAULT_PERIOD                (1 * 1000 * 1000) // ns
#define DEFAULT_DELAY                 (0)
#define DEFAULT_HIGH_VOLTAGE          (0)
#define DEFAULT_VLIMIT                (-100)
#define DEFAULT_TIMING_MODE           (AUTO_TIMING)
#define DEFAULT_TX_UDP_PORT           (0x7e9a)
#define DEFAULT_RUN_CLK               (200) // 40
#define DEFAULT_ADC_CLK               (40)  // 20
#define DEFAULT_SYNC_CLK              (40)  // 20
#define DEFAULT_DBIT_CLK              (200)
#define DEFAULT_TRANSCEIVER_MASK      (0x3)

#define MAX_TRANSCEIVER_MASK    (0xF)
#define MAX_TRANSCEIVER_SAMPLES (0xFFFF)

#define UDP_HEADER_MAX_FRAME_VALUE (0xFFFFFFFFFFFF)

#define HIGHVOLTAGE_MIN (60)
#define HIGHVOLTAGE_MAX (200) // min dac val
#define DAC_MIN_MV      (0)
#define DAC_MAX_MV      (2500)
#define VCHIP_MIN_MV    (1673)
#define VCHIP_MAX_MV    (2668) // min dac val
#define POWER_RGLTR_MIN (636)
#define POWER_RGLTR_MAX                                                        \
    (2638) // min dac val (not vchip-max) because of dac conversions
#define VCHIP_POWER_INCRMNT (200)
#define VIO_MIN_MV          (1200) // for fpga to function

/* Defines in the Firmware */
#define DIGITAL_IO_DELAY_MAXIMUM_PS                                            \
    ((OUTPUT_DELAY_0_OTPT_STTNG_MSK >> OUTPUT_DELAY_0_OTPT_STTNG_OFST) *       \
     OUTPUT_DELAY_0_OTPT_STTNG_STEPS)
#define MAX_PHASE_SHIFTS_STEPS (8)

#define WAIT_TME_US_FR_ACQDONE_REG                                             \
    (100) // wait time in us after acquisition done to ensure there is no data
          // in fifo
#define WAIT_TIME_US_PLL           (10 * 1000)
#define WAIT_TIME_US_STP_ACQ       (100)
#define WAIT_TIME_CONFIGURE_MAC    (2 * 1000 * 1000)
#define WAIT_TIME_PATTERN_READ     (10)
#define WAIT_TIME_1US_FOR_LOOP_CNT (50) // around 30 is 1 us in blackfin

/* MSB & LSB DEFINES */
#define MSB_OF_64_BIT_REG_OFST (32)
#define LSB_OF_64_BIT_REG_OFST (0)
#define BIT32_MSK              (0xFFFFFFFF)
#define BIT16_MASK             (0xFFFF)

#define MAXIMUM_ADC_CLK  (65)
#define PLL_VCO_FREQ_MHZ (800)

/* Struct Definitions */
typedef struct udp_header_struct {
    uint32_t udp_destmac_msb;
    uint16_t udp_srcmac_msb;
    uint16_t udp_destmac_lsb;
    uint32_t udp_srcmac_lsb;
    uint8_t ip_tos;
    uint8_t ip_ihl : 4, ip_ver : 4;
    uint16_t udp_ethertype;
    uint16_t ip_identification;
    uint16_t ip_totallength;
    uint8_t ip_protocol;
    uint8_t ip_ttl;
    uint16_t ip_fragmentoffset : 13, ip_flags : 3;
    uint16_t ip_srcip_msb;
    uint16_t ip_checksum;
    uint16_t ip_destip_msb;
    uint16_t ip_srcip_lsb;
    uint16_t udp_srcport;
    uint16_t ip_destip_lsb;
    uint16_t udp_checksum;
    uint16_t udp_destport;
} udp_header;

#define IP_HEADER_SIZE             (20)
#define UDP_IP_HEADER_LENGTH_BYTES (28)

/* Enums */
enum ADCINDEX {
    V_PWR_IO,
    V_PWR_A,
    V_PWR_B,
    V_PWR_C,
    V_PWR_D,
    I_PWR_IO,
    I_PWR_A,
    I_PWR_B,
    I_PWR_C,
    I_PWR_D,
    S_ADC0,
    S_ADC1,
    S_ADC2,
    S_ADC3,
    S_ADC4,
    S_ADC5,
    S_ADC6,
    S_ADC7,
    S_TMP
};
enum DACINDEX {
    D0,
    D1,
    D2,
    D3,
    D4,
    D5,
    D6,
    D7,
    D8,
    D9,
    D10,
    D11,
    D12,
    D13,
    D14,
    D15,
    D16,
    D17,
    D_PWR_D,
    D_PWR_CHIP,
    D_PWR_C,
    D_PWR_B,
    D_PWR_A,
    D_PWR_IO
};
enum CLKINDEX { RUN_CLK, ADC_CLK, SYNC_CLK, DBIT_CLK, NUM_CLOCKS };
#define CLK_NAMES "run", "adc", "sync", "dbit"
