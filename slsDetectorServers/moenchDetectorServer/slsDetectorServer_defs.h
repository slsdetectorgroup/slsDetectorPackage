// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once
#include "RegisterDefs.h"
#include "sls/sls_detector_defs.h"

#define REQRD_FRMWRE_VRSN_BOARD2 0x444445 // 1.0 pcb (version = 010)
#define REQRD_FRMWRE_VRSN        0x231026 // 2.0 pcb (version = 011)

#define NUM_HARDWARE_VERSIONS (2)
#define HARDWARE_VERSION_NUMBERS                                               \
    { 0x2, 0x3 }
#define HARDWARE_VERSION_NAMES                                                 \
    { "1.0", "2.0" }

#define ID_FILE            ("detid_moench.txt")
#define LINKED_SERVER_NAME "moenchDetectorServer"

#ifdef VIRTUAL
#define CTRL_SRVR_INIT_TIME_US (2 * 1000 * 1000)
#else
#define CTRL_SRVR_INIT_TIME_US (300 * 1000)
#endif

/* Hardware Definitions */
#define NCHAN               (400 * 400)
#define NCHIP               (1)
#define NDAC                (8)
#define DYNAMIC_RANGE       (16)
#define NUM_BYTES_PER_PIXEL (DYNAMIC_RANGE / 8)
#define DATA_BYTES          (NCHIP * NCHAN * NUM_BYTES_PER_PIXEL)
#define CLK_RUN             (40) // MHz
#define ADC_CLK_INDEX       (0)

/** Default Parameters */
#define DEFAULT_NUM_FRAMES            (1)
#define DEFAULT_STARTING_FRAME_NUMBER (1)
#define DEFAULT_NUM_CYCLES            (1)
#define DEFAULT_EXPTIME               (10 * 1000)       // ns
#define DEFAULT_PERIOD                (2 * 1000 * 1000) // ns
#define DEFAULT_DELAY                 (0)
#define DEFAULT_HIGH_VOLTAGE          (0)
#define DEFAULT_TIMING_MODE           (AUTO_TIMING)
#define DEFAULT_SETTINGS              (G4_HIGHGAIN)
#define DEFAULT_TX_UDP_PORT           (0x7e9a)
#define DEFAULT_TMP_THRSHLD           (65 * 1000) // milli degree Celsius
#define DEFAULT_FLIP_ROWS             (0)
#define DEFAULT_SPEED                 (HALF_SPEED)
#define DEFAULT_PARALLEL_ENABLE       (0)

#define HIGHVOLTAGE_MIN     (60)
#define HIGHVOLTAGE_MAX     (200)
#define DAC_MIN_MV          (0)
#define DAC_MAX_MV          (2500)
#define MAX_FILTER_CELL_VAL (12)

#define READ_N_ROWS_MULTIPLE (16) // 400 rows/50packets * 2 interfaces
#define MIN_ROWS_PER_READOUT (16)
#define MAX_ROWS_PER_READOUT (400)
#define ROWS_PER_PACKET      (8)

/* Defines in the Firmware */
#define MAX_TIMESLOT_VAL          (0x1F)
#define MAX_THRESHOLD_TEMP_VAL    (127999) // millidegrees
#define ASIC_FILTER_MAX_RES_VALUE (1)
#define MAX_SELECT_CHIP10_VAL     (63)

#define MAX_PHASE_SHIFTS (200)
#define BIT16_MASK       (0xFFFF)

#define ADC_DECMT_QUARTER_SPEED (0x3)
#define ADC_DECMT_HALF_SPEED    (0x1)
#define ADC_DECMT_FULL_SPEED    (0x0)

#define ADC_PHASE_DEG_QUARTER_SPEED (0)
#define ADC_PHASE_DEG_HALF_SPEED    (0)
#define ADC_PHASE_DEG_FULL_SPEED    (150)

#define ADC_OFST_QUARTER_SPEED (0x12)
#define ADC_OFST_HALF_SPEED    (0x12)
#define ADC_OFST_FULL_SPEED    (0x12)

// pipeline
#define ADC_PORT_INVERT_VAL (0x55555555)

#define SAMPLE_ADC_FULL_SPEED                                                  \
    (SAMPLE_ADC_SAMPLE_0_VAL + SAMPLE_ADC_DECMT_FACTOR_0_VAL) // 0x0

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
enum ADCINDEX { TEMP_FPGA, TEMP_ADC };
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
        1280, /* MO_VIPRE_CDS */                                               \
        900   /* MO_IBIAS_SFP */                                               \
    };

enum MASTERINDEX { MASTER_HARDWARE, OW_MASTER, OW_SLAVE };
#define MASTER_NAMES "hardware", "master", "slave"

#define NUMSETTINGS (0)

enum NETWORKINDEX { TXN_FRAME, FLOWCTRL_10G };
enum CLKINDEX { RUN_CLK, ADC_CLK, NUM_CLOCKS };
#define CLK_NAMES "run", "adc", "dbit"
