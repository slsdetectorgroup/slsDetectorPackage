// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once
#include "RegisterDefs.h"
#include "sls/sls_detector_defs.h"

#define MIN_REQRD_VRSN_T_RD_API 0x180314
#define REQRD_FRMWR_VRSN        0x220825

#define NUM_HARDWARE_VERSIONS (1)
#define HARDWARE_VERSION_NUMBERS {0x1}
#define HARDWARE_VERSION_NAMES {"1.0"}

#define LINKED_SERVER_NAME "moenchDetectorServer"

#define CTRL_SRVR_INIT_TIME_US (2 * 1000 * 1000)

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
enum DACINDEX {
    MO_VBP_COLBUF,
    MO_VIPRE,
    MO_VIN_CM,
    MO_VB_SDA,
    MO_VCASC_SFP,
    MO_VOUT_CM,
    MO_VIPRE_CDS,
    MO_IBIAS_SFP
};
#define DAC_NAMES                                                              \
    "vbp_colbuf", "vipre", "vin_cm", "vb_sda", "vcasc_sfp", "vout_cm",         \
        "vipre_cds", "ibias_sfp"
#define DEFAULT_DAC_VALS                                                       \
    {                                                                          \
        1300, /* MO_VBP_COLBUF */                                              \
        1000, /* MO_VIPRE */                                                   \
        1400, /* MO_VIN_CM */                                                  \
        680,  /* MO_VB_SDA */                                                  \
        1428, /* MO_VCASC_SFP */                                               \
        1200, /* MO_VOUT_CM */                                                 \
        800,  /* MO_VIPRE_CDS */                                               \
        900   /* MO_IBIAS_SFP */                                               \
    };

enum CLKINDEX { RUN_CLK, ADC_CLK, SYNC_CLK, DBIT_CLK, NUM_CLOCKS };
#define CLK_NAMES "run", "adc", "sync", "dbit"

/* Hardware Definitions */
#define NCHAN               (32)
#define NCHIP               (1)
#define NDAC                (8)
#define DYNAMIC_RANGE       (16)
#define NUM_BYTES_PER_PIXEL (DYNAMIC_RANGE / 8)
#define CLK_FREQ            (156.25) /* MHz */
#define NSAMPLES_PER_ROW    (25)
#define NCHANS_PER_ADC      (25)

/** Default Parameters */
#define DEFAULT_PATTERN_FILE          ("DefaultPattern_moench.txt")
#define DEFAULT_STARTING_FRAME_NUMBER (1)
#define DEFAULT_DATA_BYTES            (NCHIP * NCHAN * NUM_BITS_PER_PIXEL)
#define DEFAULT_NUM_SAMPLES           (5000)
#define DEFAULT_EXPTIME               (0)
#define DEFAULT_NUM_FRAMES            (1)
#define DEFAULT_NUM_CYCLES            (1)
#define DEFAULT_PERIOD                (1 * 1000 * 1000) // ns
#define DEFAULT_DELAY                 (0)
#define DEFAULT_HIGH_VOLTAGE          (0)
#define DEFAULT_VLIMIT                (-100)
#define DEFAULT_TIMING_MODE           (AUTO_TIMING)
#define DEFAULT_TX_UDP_PORT           (0x7e9a)

#define DEFAULT_RUN_CLK_AT_STARTUP  (200) // 40
#define DEFAULT_ADC_CLK_AT_STARTUP  (40)  // 20
#define DEFAULT_SYNC_CLK_AT_STARTUP (40)  // 20
#define DEFAULT_DBIT_CLK_AT_STARTUP (200)

#define DEFAULT_RUN_CLK       (40)
#define DEFAULT_ADC_CLK       (20)
#define DEFAULT_DBIT_CLK      (40)
#define DEFAULT_ADC_PHASE_DEG (30)

#define DEFAULT_PIPELINE (15)
#define DEFAULT_SETTINGS (G4_HIGHGAIN)

#define UDP_HEADER_MAX_FRAME_VALUE (0xFFFFFFFFFFFF)

// settings
#define DEFAULT_PATMASK               (0x00000C800000800AULL)
#define G1_HIGHGAIN_PATSETBIT         (0x00000C0000008008ULL)
#define G1_LOWGAIN_PATSETBIT          (0x0000040000008000ULL)
#define G2_HIGHCAP_HIGHGAIN_PATSETBIT (0x0000080000000008ULL)
#define G2_HIGHCAP_LOWGAIN_PATSETBIT  (0x0000000000000000ULL)
#define G2_LOWCAP_HIGHGAIN_PATSETBIT  (0x00000C800000800AULL)
#define G2_LOWCAP_LOWGAIN_PATSETBIT   (0x0000048000008002ULL)
#define G4_HIGHGAIN_PATSETBIT         (0x000008800000000AULL)
#define G4_LOWGAIN_PATSETBIT          (0x0000008000000002ULL)

#define HIGHVOLTAGE_MIN (60)
#define HIGHVOLTAGE_MAX (200) // min dac val
#define DAC_MIN_MV      (0)
#define DAC_MAX_MV      (2500)

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

#define ADC_PORT_INVERT_VAL (0x4a342593)
#define MAXIMUM_ADC_CLK     (20)
#define PLL_VCO_FREQ_MHZ    (800)
