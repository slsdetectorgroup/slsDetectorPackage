// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once
#include "sls/sls_detector_defs.h"

#define REQRD_FRMWRE_VRSN (0x241113)
#define KERNEL_DATE_VRSN  "Mon May 10 18:00:21 CEST 2021"
#define ID_FILE           "detid_mythen3.txt"

#define NUM_HARDWARE_VERSIONS    (2)
#define HARDWARE_VERSION_NUMBERS {0x0, 0x2};
#define HARDWARE_VERSION_NAMES                                                 \
    { "1.0", "1.2" }

#define LINKED_SERVER_NAME "mythen3DetectorServer"

#define CTRL_SRVR_INIT_TIME_US (300 * 1000)

/* Hardware Definitions */
#define NCOUNTERS            (3)
#define MAX_COUNTER_MSK      (0x7)
#define NCHAN_1_COUNTER      (128)
#define NCHAN                (128 * NCOUNTERS)
#define NCHIP                (10)
#define NCHAN_PER_MODULE     (NCHAN * NCHIP)
#define NDAC                 (16)
#define HV_SOFT_MAX_VOLTAGE  (500)
#define HV_HARD_MAX_VOLTAGE  (530)
#define HV_DRIVER_FILE_NAME  ("/etc/devlinks/hvdac")
#define DAC_DRIVER_FILE_NAME ("/etc/devlinks/dac")
#define TYPE_FILE_NAME       ("/etc/devlinks/type")
#ifdef VIRTUAL
#define TEMPERATURE_FILE_NAME ("/tmp/temp.txt")
#else
#define TEMPERATURE_FILE_NAME ("/sys/class/hwmon/hwmon0/temp1_input")
#endif
#define DAC_MIN_MV                  (0)
#define DAC_MAX_MV                  (2048)
#define TYPE_MYTHEN3_MODULE_VAL     (93)
#define TYPE_TOLERANCE              (5)
#define TYPE_NO_MODULE_STARTING_VAL (800)
#define MAX_EXT_SIGNALS             (8)

/** Default Parameters */
#define DEFAULT_PATTERN_FILE             ("DefaultPattern_mythen3.txt")
#define DEFAULT_INTERNAL_GATES           (1)
#define DEFAULT_EXTERNAL_GATES           (1)
#define DEFAULT_DYNAMIC_RANGE            (32)
#define DEFAULT_NUM_FRAMES               (1)
#define DEFAULT_NUM_CYCLES               (1)
#define DEFAULT_GATE_WIDTH               (100 * 1000 * 1000) // ns
#define DEFAULT_GATE_DELAY               (0)
#define DEFAULT_PERIOD                   (0) // ns
#define DEFAULT_DELAY_AFTER_TRIGGER      (0)
#define DEFAULT_HIGH_VOLTAGE             (0)
#define DEFAULT_TIMING_MODE              (AUTO_TIMING)
#define DEFAULT_SETTINGS                 (STANDARD)
#define DEFAULT_TRIMBIT_VALUE            (0)
#define DEFAULT_COUNTER_DISABLED_VTH_VAL (2800)
#define DEFAULT_READOUT_SPEED            (HALF_SPEED)

#define DEFAULT_SYSTEM_C0           (10) //(100000000) // run_clk, 100 MHz
#define DEFAULT_SYSTEM_C1           (6)  //(166666666) // str_clk, 166 MHz const
#define DEFAULT_SYSTEM_C2           (5)  //(200000000) // smp_clk, 200 MHz const
#define DEFAULT_TRIMMING_RUN_CLKDIV (40) // (25000000) // 25 MHz

#define FULL_SPEED_CLKDIV    (10) //(100000000) 100 MHz
#define HALF_SPEED_CLKDIV    (20) //( 50000000) 50 MHz
#define QUARTER_SPEED_CLKDIV (40) //( 25000000) 25 MHz

#define DEFAULT_ASIC_LATCHING_NUM_PULSES (10)
#define DEFAULT_MSTR_OTPT_P1_NUM_PULSES  (20)
#define DEFAULT_ADIF_PIPELINE_VAL        (8)
#define DEFAULT_ADIF_ADD_OFST_VAL        (0)

/* Firmware Definitions */
#define MAX_TIMESLOT_VAL       (0xFFFFFF)
#define IP_HEADER_SIZE         (20)
#define FIXED_PLL_FREQUENCY    (020000000)  // 20MHz
#define SYSTEM_PLL_VCO_FREQ_HZ (1000000000) // 1GHz
#define MAX_NUM_DESERIALIZERS  (40)

/** Other Definitions */
#define BIT16_MASK         (0xFFFF)
#define MAX_TRIMBITS_VALUE (63)

/* Enums */
enum DACINDEX {
    M_VCASSH,
    M_VTH2,
    M_VRSHAPER,
    M_VRSHAPER_N,
    M_VIPRE_OUT,
    M_VTH3,
    M_VTH1,
    M_VICIN,
    M_VCAS,
    M_VRPREAMP,
    M_VCAL_N,
    M_VIPRE,
    M_VISHAPER,
    M_VCAL_P,
    M_VTRIM,
    M_VDCSH,
    M_VTHRESHOLD
};
#define DAC_NAMES                                                              \
    "vcassh", "vth2", "vrshaper", "vrshaper_n", "vipre_out", "vth3", "vth1",   \
        "vicin", "vcas", "vrpreamp", "vcal_n", "vipre", "vishaper", "vcal_p",  \
        "vtrim", "vdcsh", "vthreshold"
#define DEFAULT_DAC_VALS                                                       \
    {                                                                          \
        1200, /* casSh */                                                      \
        2800, /* Vth2 */                                                       \
        1280, /* Vrshaper */                                                   \
        2800, /* Vrshaper_n */                                                 \
        1220, /* vIpreOut */                                                   \
        2800, /* Vth3 */                                                       \
        2800, /* Vth1 */                                                       \
        800,  /* vIcin */                                                      \
        1800, /* cas */                                                        \
        1100, /* Vrpreamp */                                                   \
        1100, /* Vcal_n */                                                     \
        2624, /* vIpre */                                                      \
        1708, /* vishaper */                                                   \
        1712, /* Vcal_p */                                                     \
        2800, /* vTrim */                                                      \
        800   /* VdcSh */                                                      \
    };

enum ADCINDEX { TEMP_FPGA };

#define NUMSETTINGS     (3)
#define NSPECIALDACS    (2)
#define SPECIALDACINDEX {M_VRPREAMP, M_VRSHAPER};
#define SPECIAL_DEFAULT_STANDARD_DAC_VALS                                      \
    { 1100, 1280 }
#define SPECIAL_DEFAULT_FAST_DAC_VALS                                          \
    { 300, 1500 }
#define SPECIAL_DEFAULT_HIGHGAIN_DAC_VALS                                      \
    { 1300, 1100 }

enum CLKINDEX { SYSTEM_C0, SYSTEM_C1, SYSTEM_C2, NUM_CLOCKS };
#define NUM_CLOCKS_TO_SET (1)

#define CLK_NAMES "SYSTEM_C0", "SYSTEM_C1", "SYSTEM_C2"

#define SYSTEM_PLL (1)

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
#define UDP_IP_HEADER_LENGTH_BYTES (28)
#define PACKETS_PER_FRAME_10G      (2)
#define PACKETS_PER_FRAME_1G       (20)
