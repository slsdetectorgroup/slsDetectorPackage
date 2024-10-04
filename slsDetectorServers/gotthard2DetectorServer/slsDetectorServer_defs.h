// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once
#include "sls/sls_detector_defs.h"

#define REQRD_FRMWRE_VRSN (0x241003)
#define KERNEL_DATE_VRSN  "Mon May 10 18:00:21 CEST 2021"
#define ID_FILE           "detid_gotthard2.txt"

#define NUM_HARDWARE_VERSIONS    (2)
#define HARDWARE_VERSION_NUMBERS {0x0, 0x2};
#define HARDWARE_VERSION_NAMES                                                 \
    { "1.0", "1.2" }

#define LINKED_SERVER_NAME "gotthard2DetectorServer"

#ifdef VIRTUAL
#define CTRL_SRVR_INIT_TIME_US (3 * 1000 * 1000)
#else
#define CTRL_SRVR_INIT_TIME_US (300 * 1000)
#endif

/* Hardware Definitions */
#define NCHAN                       (128)
#define NCHIP                       (10)
#define NDAC                        (16)
#define NADC                        (32)
#define ONCHIP_NDAC                 (7)
#define DYNAMIC_RANGE               (16)
#define HV_SOFT_MAX_VOLTAGE         (500)
#define HV_HARD_MAX_VOLTAGE         (530)
#define HV_DRIVER_FILE_NAME         ("/etc/devlinks/hvdac")
#define DAC_DRIVER_FILE_NAME        ("/etc/devlinks/dac")
#define ONCHIP_DAC_DRIVER_FILE_NAME ("/etc/devlinks/chipdac")
#define TYPE_FILE_NAME              ("/etc/devlinks/type")
#ifdef VIRTUAL
#define TEMPERATURE_FILE_NAME ("/tmp/temp.txt")
#else
#define TEMPERATURE_FILE_NAME ("/sys/class/hwmon/hwmon0/temp1_input")
#endif
#define CONFIG_FILE                           ("config_gotthard2.txt")
#define DAC_MIN_MV                            (0)
#define DAC_MAX_MV                            (2048)
#define ONCHIP_DAC_MAX_VAL                    (0x3FF)
#define ADU_MAX_VAL                           (0xFFF)
#define ADU_MAX_BITS                          (12)
#define MAX_FRAMES_IN_BURST_MODE              (2720)
#define TYPE_GOTTHARD2_MODULE_VAL             (536)
#define TYPE_GOTTHARD2_25UM_MASTER_HD1_V1_VAL (683)
#define TYPE_GOTTHARD2_25UM_SLAVE_HDI_V1_VAL  (704)
#define TYPE_GOTTHARD2_25UM_MASTER_HD1_V2_VAL (723)
#define TYPE_GOTTHARD2_25UM_SLAVE_HDI_V2_VAL  (747)
#define TYPE_GOTTHARD2_MODULE_VAL             (536)
#define TYPE_TOLERANCE                        (10)
#define TYPE_NO_MODULE_STARTING_VAL           (800)
#define INITIAL_STARTUP_WAIT                  (1 * 1000 * 1000)

#define WAIT_HIGH_VOLTAGE_SETTLE_TIME_S (10) // 10s

/** Default Parameters */
#define DEFAULT_BURST_MODE          (BURST_INTERNAL)
#define DEFAULT_FILTER_RESISTOR     (0)
#define DEFAILT_CDS_GAIN            (0)
#define DEFAULT_FRAME_NUMBER        (1)
#define DEFAULT_NUM_FRAMES          (1)
#define DEFAULT_NUM_CYCLES          (1)
#define DEFAULT_NUM_BURSTS          (1)
#define DEFAULT_EXPTIME             (0) // 0 ms (220ns in firmware)
#define DEFAULT_PERIOD              (0) // 0 ms
#define DEFAULT_DELAY_AFTER_TRIGGER (0)
#define DEFAULT_BURST_PERIOD        (0)
#define DEFAULT_HIGH_VOLTAGE        (0)
#define DEFAULT_TIMING_MODE         (AUTO_TIMING)
#define DEFAULT_SETTINGS            (DYNAMICGAIN)
#define DEFAULT_CURRENT_SOURCE      (0)
#define DEFAULT_TIMING_SOURCE       (TIMING_INTERNAL)
#define DEFAULT_ALGORITHM           (ALG_HITS)

#define DEFAULT_READOUT_C0 (8)  //(108333336) // rdo_clk, 144 MHz
#define DEFAULT_READOUT_C1 (8)  //(108333336) // rdo_x2_clk, 144 MHz
#define DEFAULT_SYSTEM_C0  (5)  //(144444448) // run_clk, 144 MHz
#define DEFAULT_SYSTEM_C1  (10) //(72222224) // chip_clk, 72 MHz
#define DEFAULT_SYSTEM_C2  (5)  //(144444448) // sync_clk, 144 MHz
#define DEFAULT_SYSTEM_C3  (5)  //(144444448) // str_clk, 144 MHz

#define DEFAULT_CNTNS_SYSTEM_C0 (10) // (72222224) run_clk, 72 MHz
#define DEFAULT_CNTNS_SYSTEM_C1 (20) // (36111112) chip_clk, 36 MHz
#define DEFAULT_CNTNS_SYSTEM_C2 (10) // (72222224) sync_clk, 72 MHz

#define DEFAULT_BURST_SYSTEM_C0 (5)  // (144444448) run_clk, 144 MHz
#define DEFAULT_BURST_SYSTEM_C1 (10) // (72222224) chip_clk, 72 MHz
#define DEFAULT_BURST_SYSTEM_C2 (5)  // (144444448) sync_clk, 144 MHz

#define DEFAULT_READOUT_SPEED    (G2_108MHZ)
#define SPEED_144_CLKDIV_0       (6)
#define SPEED_144_CLKDIV_1       (6)
#define SPEED_144_CLKPHASE_DEG_1 (122) // 125 not possible
#define SPEED_144_DBIT_PIPELINE  (1)
#define SPEED_108_CLKDIV_0       (8)
#define SPEED_108_CLKDIV_1       (8)
#define SPEED_108_CLKPHASE_DEG_1 (268) // 270 not possible
#define SPEED_108_DBIT_PIPELINE  (1)

/* Firmware Definitions */
#define FIXED_PLL_FREQUENCY     (20000000)  // 20MHz
#define INT_SYSTEM_C0_FREQUENCY (144000000) // 144 MHz
#define READOUT_PLL_VCO_FREQ_HZ (866666688) // 866 MHz
#define SYSTEM_PLL_VCO_FREQ_HZ  (722222224) // 722 MHz

#define DEFAULT_CLK1_PHASE_DEG    (270)
#define DEFAULT_DBIT_PIPELINE     (1)
#define DEFAULT_ASIC_DOUT_RDY_SRC (0x5)
#define DEFAULT_ASIC_DOUT_RDY_DLY (0x3)

#define GAIN_VAL_OFST (12)
#define GAIN_VAL_MSK  (0x3 << GAIN_VAL_OFST)

#define VETO_DATA_SIZE (160)
typedef struct {
    uint64_t frameNumber;
    uint64_t bunchId;
} veto_header;

/** Other Definitions */
#define BIT16_MASK (0xFFFF)

/* Enums */
enum DACINDEX {
    G2_VREF_H_ADC,   /* 0 */
    G2_DAC_UNUSED,   /* 1 */
    G2_VB_COMP_FE,   /* 2 */
    G2_VB_COMP_ADC,  /* 3 */
    G2_VCOM_CDS,     /* 4 */
    G2_VREF_RSTORE,  /* 5 */
    G2_VB_OPA_1ST,   /* 6 */
    G2_VREF_COMP_FE, /* 7 */
    G2_VCOM_ADC1,    /* 8 */
    G2_VREF_PRECH,   /* 9 */
    G2_VREF_L_ADC,   /* 10 */
    G2_VREF_CDS,     /* 11 */
    G2_VB_CS,        /* 12 */
    G2_VB_OPA_FD,    /* 13 */
    G2_DAC_UNUSED2,  /* 14 */
    G2_VCOM_ADC2     /* 15*/
};
#define DAC_NAMES                                                              \
    "vref_h_adc", "dac_unused", "vb_comp_fe", "vb_comp_adc", "vcom_cds",       \
        "vref_rstore", "vb_opa_1st", "vref_comp_fe", "vcom_adc1",              \
        "vref_prech", "vref_l_adc", "vref_cds", "vb_cs", "vb_opa_fd",          \
        "dac_unused2", "vcom_adc2"

enum ONCHIP_DACINDEX {
    G2_VCHIP_COMP_FE,     /* 0 */
    G2_VCHIP_OPA_1ST,     /* 1 */
    G2_VCHIP_OPA_FD,      /* 2 */
    G2_VCHIP_COMP_ADC,    /* 3 */
    G2_VCHIP_UNUSED,      /* 4 */
    G2_VCHIP_REF_COMP_FE, /* 5 */
    G2_VCHIP_CS           /* 6 */
};
#define ONCHIP_DAC_NAMES                                                       \
    "vchip_comp_fe", "vchip_opa_1st", "vchip_opa_fd", "vchip_comp_adc",        \
        "vchip_unused", "vchip_ref_comp_fe", "vchip_cs"

enum CLKINDEX {
    READOUT_C0,
    READOUT_C1,
    SYSTEM_C0,
    SYSTEM_C1,
    SYSTEM_C2,
    SYSTEM_C3,
    NUM_CLOCKS
};
#define CLK_NAMES                                                              \
    "READOUT_C0", "READOUT_C1", "SYSTEM_C0", "SYSTEM_C1", "SYSTEM_C2",         \
        "SYSTEM_C3"

enum ADCINDEX { TEMP_FPGA };

enum PLLINDEX { READOUT_PLL, SYSTEM_PLL };

enum MASTERINDEX { MASTER_HARDWARE, OW_MASTER, OW_SLAVE };
#define MASTER_NAMES "hardware (master/slave)", "master", "slave"

/** Chip Definitions */
#define ASIC_ADDR_MAX_BITS       (4)
#define ASIC_CURRENT_INJECT_ADDR (0x9)
#define ASIC_VETO_REF_ADDR       (0xA)
#define ASIC_CONF_ADC_ADDR       (0xB)
#define ASIC_CONF_GLOBAL_SETT    (0xC)

#define ASIC_GAIN_MAX_BITS        (2)
#define ASIC_GAIN_MSK             (0x3)
#define ASIC_G0_VAL               ((0x0 & ASIC_GAIN_MSK) << ADU_MAX_BITS)
#define ASIC_G1_VAL               ((0x1 & ASIC_GAIN_MSK) << ADU_MAX_BITS)
#define ASIC_G2_VAL               ((0x3 & ASIC_GAIN_MSK) << ADU_MAX_BITS)
#define ASIC_CONTINUOUS_MODE_MSK  (0x7)
#define ASIC_ADC_MAX_BITS         (7)
#define ASIC_ADC_MAX_VAL          (0x7F)
#define ASIC_GLOBAL_SETT_MAX_BITS (6)

#define ASIC_EXT_MEMCTRL_OFST     (0)
#define ASIC_EXT_MEMCTRL_MSK      (0x1 << ASIC_EXT_MEMCTRL_OFST)
#define ASIC_EXT_TIMING_OFST      (1)
#define ASIC_EXT_TIMING_MSK       (0x1 << ASIC_EXT_TIMING_OFST)
#define ASIC_CONT_MODE_OFST       (2)
#define ASIC_CONT_MODE_MSK        (0x1 << ASIC_CONT_MODE_OFST)
#define ASIC_FILTER_OFST          (3)
#define ASIC_FILTER_MSK           (0x3 << ASIC_FILTER_OFST)
#define ASIC_FILTER_MAX_RES_VALUE (3)
#define ASIC_CDS_GAIN_OFST        (5)
#define ASIC_CDS_GAIN_MSK         (0x1 << ASIC_CDS_GAIN_OFST)

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
