// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once
#include "RegisterDefs.h"
#include "sls/sls_detector_defs.h"

#define REQRD_FRMWRE_VRSN_BOARD2 0x444445 // 1.0 pcb (version = 010)
#define REQRD_FRMWRE_VRSN        0x230522 // 2.0 pcb (version = 011)

#define NUM_HARDWARE_VERSIONS (2)
#define HARDWARE_VERSION_NUMBERS                                               \
    { 0x2, 0x3 }
#define HARDWARE_VERSION_NAMES                                                 \
    { "1.0", "2.0" }

#define ID_FILE            ("detid_moench.txt")
#define LINKED_SERVER_NAME "moenchDetectorServer"

#define CTRL_SRVR_INIT_TIME_US (300 * 1000)

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
#define DEFAULT_SPEED                 (FULL_SPEED)
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
#define ACQ_TIME_MIN_CLOCK        (2)
#define ASIC_FILTER_MAX_RES_VALUE (1)
#define MAX_SELECT_CHIP10_VAL     (63)

#define MAX_PHASE_SHIFTS (240)
#define BIT16_MASK       (0xFFFF)

// pipeline
#define ADC_PORT_INVERT_VAL (0x55555555)

#define SAMPLE_ADC_FULL_SPEED                                                  \
    (SAMPLE_ADC_SAMPLE_0_VAL + SAMPLE_ADC_DECMT_FACTOR_0_VAL) // 0x0
#define ADC_PHASE_DEG_FULL_SPEED (140)
#define ADC_OFST_FULL_SPEED_VAL  (0xf)

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
    J_VB_COMP,
    J_VDD_PROT,
    J_VIN_COM,
    J_VREF_PRECH,
    J_VB_PIXBUF,
    J_VB_DS,
    J_VREF_DS,
    J_VREF_COMP
};
#define DAC_NAMES                                                              \
    "vb_comp", "vdd_prot", "vin_com", "vref_prech", "vb_pixbuf", "vb_ds",      \
        "vref_ds", "vref_comp"

#define DEFAULT_DAC_VALS                                                       \
    {                                                                          \
        1220, /* J_VB_COMP */                                                  \
        3000, /* J_VDD_PROT */                                                 \
        1053, /* J_VIN_COM */                                                  \
        1450, /* J_VREF_PRECH */                                               \
        750,  /* J_VB_PIXBUF */                                                \
        1000, /* J_VB_DS */                                                    \
        480,  /* J_VREF_DS */                                                  \
        420   /* J_VREF_COMP */                                                \
    };

enum MASTERINDEX { MASTER_HARDWARE, OW_MASTER, OW_SLAVE };
#define MASTER_NAMES "hardware", "master", "slave"

#define NUMSETTINGS (0)

enum NETWORKINDEX { TXN_FRAME, FLOWCTRL_10G };
enum CLKINDEX { RUN_CLK, ADC_CLK, NUM_CLOCKS };
#define CLK_NAMES "run", "adc", "dbit"
