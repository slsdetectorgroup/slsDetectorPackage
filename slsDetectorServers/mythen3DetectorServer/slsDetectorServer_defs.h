#pragma once
#include "sls_detector_defs.h"

#define REQRD_FRMWRE_VRSN 0x190000

#define CTRL_SRVR_INIT_TIME_US (300 * 1000)

/* Hardware Definitions */
#define NCOUNTERS                   (3)
#define MAX_COUNTER_MSK             (0x7)
#define NCHAN_1_COUNTER             (128)
#define NCHAN                       (128 * NCOUNTERS)
#define NCHIP                       (10)
#define NDAC                        (16)
#define HV_SOFT_MAX_VOLTAGE         (200)
#define HV_HARD_MAX_VOLTAGE         (530)
#define HV_DRIVER_FILE_NAME         ("/etc/devlinks/hvdac")
#define DAC_DRIVER_FILE_NAME        ("/etc/devlinks/dac")
#define TYPE_FILE_NAME              ("/etc/devlinks/type")
#define DAC_MAX_MV                  (2048)
#define TYPE_MYTHEN3_MODULE_VAL     (93)
#define TYPE_TOLERANCE              (10)
#define TYPE_NO_MODULE_STARTING_VAL (800)

/** Default Parameters */
#define DEFAULT_DYNAMIC_RANGE       (24)
#define DEFAULT_NUM_FRAMES          (1)
#define DEFAULT_NUM_CYCLES          (1)
#define DEFAULT_EXPTIME             (100 * 1000 * 1000) // ns
#define DEFAULT_PERIOD              (2 * 1000 * 1000)   // ns
#define DEFAULT_DELAY_AFTER_TRIGGER (0)
#define DEFAULT_HIGH_VOLTAGE        (0)
#define DEFAULT_TIMING_MODE         (AUTO_TIMING)
#define DEFAULT_READOUT_C0          (10) //(125000000) // rdo_clk, 125 MHz
#define DEFAULT_READOUT_C1          (10) //(125000000) // rdo_x2_clk, 125 MHz
#define DEFAULT_SYSTEM_C0           (5)  //(250000000) // run_clk, 250 MHz
#define DEFAULT_SYSTEM_C1           (10) //(125000000) // chip_clk, 125 MHz
#define DEFAULT_SYSTEM_C2           (10) //(125000000) // sync_clk, 125 MHz

/* Firmware Definitions */
#define IP_HEADER_SIZE          (20)
#define FIXED_PLL_FREQUENCY     (020000000)  // 20MHz
#define READOUT_PLL_VCO_FREQ_HZ (1250000000) // 1.25GHz
#define SYSTEM_PLL_VCO_FREQ_HZ  (1250000000) // 1.25GHz
#define MAX_PATTERN_LENGTH      (0x2000)     // maximum number of words (64bit)

/** Other Definitions */
#define BIT16_MASK (0xFFFF)

/* Enums */
enum DACINDEX {
    M_CASSH,
    M_VTH2,
    M_VRFSH,
    M_VRFSHNPOL,
    M_VIPRE_OUT,
    M_VTH3,
    M_VTH1,
    M_VICIN,
    M_CAS,
    M_VRF,
    M_VPL,
    M_VIPRE,
    M_VIINSH,
    M_VPH,
    M_VTRIM,
    M_VDCSH
};
#define DAC_NAMES                                                              \
    "vcassh", "vth2", "vshaper", "vshaperneg", "vipre_out", "vth3", "vth1",    \
        "vicin", "vcas", "vpreamp", "vpl", "vipre", "viinsh", "vph", "vtrim",  \
        "vdcsh"
#define DEFAULT_DAC_VALS                                                       \
    {                                                                          \
        1200, /* casSh */                                                      \
        2800, /* Vth2 */                                                       \
        1280, /* VrfSh */                                                      \
        2800, /* VrfShNpol */                                                  \
        1220, /* vIpreOut */                                                   \
        2800, /* Vth3 */                                                       \
        2800, /* Vth1 */                                                       \
        1708, /* vIcin */                                                      \
        1800, /* cas */                                                        \
        1100, /* Vrf */                                                        \
        1100, /* VPL */                                                        \
        2624, /* vIpre */                                                      \
        1708, /* vIinSh */                                                     \
        1712, /* VPH */                                                        \
        2800, /* vTrim */                                                      \
        800   /* VdcSh */                                                      \
    };
enum CLKINDEX {
    READOUT_C0,
    READOUT_C1,
    SYSTEM_C0,
    SYSTEM_C1,
    SYSTEM_C2,
    NUM_CLOCKS
};
#define CLK_NAMES                                                              \
    "READOUT_C0", "READOUT_C1", "SYSTEM_C0", "SYSTEM_C1", "SYSTEM_C2"
enum PLLINDEX { READOUT_PLL, SYSTEM_PLL };

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
#define PACKETS_PER_FRAME          (2)
