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
#define HV_SOFT_MAX_VOLTAGE         (500)
#define HV_HARD_MAX_VOLTAGE         (530)
#define HV_DRIVER_FILE_NAME         ("/etc/devlinks/hvdac")
#define DAC_DRIVER_FILE_NAME        ("/etc/devlinks/dac")
#define TYPE_FILE_NAME              ("/etc/devlinks/type")
#define DAC_MAX_MV                  (2048)
#define TYPE_MYTHEN3_MODULE_VAL     (93)
#define TYPE_TOLERANCE              (10)
#define TYPE_NO_MODULE_STARTING_VAL (800)
#define MAX_EXT_SIGNALS             (8)

/** Default Parameters */
#define DEFAULT_INTERNAL_GATES           (1)
#define DEFAULT_EXTERNAL_GATES           (1)
#define DEFAULT_DYNAMIC_RANGE            (24)
#define DEFAULT_NUM_FRAMES               (1)
#define DEFAULT_NUM_CYCLES               (1)
#define DEFAULT_GATE_WIDTH               (100 * 1000 * 1000) // ns
#define DEFAULT_GATE_DELAY               (0)
#define DEFAULT_PERIOD                   (2 * 1000 * 1000) // ns
#define DEFAULT_DELAY_AFTER_TRIGGER      (0)
#define DEFAULT_HIGH_VOLTAGE             (0)
#define DEFAULT_TIMING_MODE              (AUTO_TIMING)
#define DEFAULT_READOUT_C0               (10) //(125000000) // rdo_clk, 125 MHz
#define DEFAULT_READOUT_C1               (10) //(125000000) // rdo_x2_clk, 125 MHz
#define DEFAULT_SYSTEM_C0                (4)  //(250000000) // run_clk, 250 MHz
#define DEFAULT_SYSTEM_C1                (8) //(125000000) // chip_clk, 125 MHz
#define DEFAULT_SYSTEM_C2                (8) //(125000000) // sync_clk, 125 MHz
#define DEFAULT_ASIC_LATCHING_NUM_PULSES (10)
#define DEFAULT_MSTR_OTPT_P1_NUM_PULSES  (20)

/* Firmware Definitions */
#define IP_HEADER_SIZE          (20)
#define FIXED_PLL_FREQUENCY     (020000000)  // 20MHz
#define READOUT_PLL_VCO_FREQ_HZ (1250000000) // 1.25GHz
#define SYSTEM_PLL_VCO_FREQ_HZ  (1000000000) // 1GHz
#define MAX_PATTERN_LENGTH      (0x2000)     // maximum number of words (64bit)

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
        "vtrim", "vdcsh"
#define DEFAULT_DAC_VALS                                                       \
    {                                                                          \
        1200, /* casSh */                                                      \
        2800, /* Vth2 */                                                       \
        1280, /* Vrshaper */                                                   \
        2800, /* Vrshaper_n */                                                 \
        1220, /* vIpreOut */                                                   \
        2800, /* Vth3 */                                                       \
        2800, /* Vth1 */                                                       \
        1708, /* vIcin */                                                      \
        1800, /* cas */                                                        \
        1100, /* Vrpreamp */                                                   \
        1100, /* Vcal_n */                                                     \
        2624, /* vIpre */                                                      \
        1708, /* vishaper */                                                   \
        1712, /* Vcal_p */                                                     \
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

/** Signal Definitions */
#define SIGNAL_TBLoad_1    (0)
#define SIGNAL_TBLoad_2    (1)
#define SIGNAL_TBLoad_3    (2)
#define SIGNAL_TBLoad_4    (3)
#define SIGNAL_TBLoad_5    (4)
#define SIGNAL_TBLoad_6    (5)
#define SIGNAL_TBLoad_7    (6)
#define SIGNAL_TBLoad_8    (7)
#define SIGNAL_TBLoad_9    (8)
#define SIGNAL_TBLoad_10   (9)
#define SIGNAL_AnaMode     (10)
#define SIGNAL_CHSserialIN (11)
#define SIGNAL_READOUT     (12)
#define SIGNAL_pulse       (13)
#define SIGNAL_EN1         (14)
#define SIGNAL_EN2         (15)
#define SIGNAL_EN3         (16)
#define SIGNAL_clk         (17)
#define SIGNAL_SRmode      (18)
#define SIGNAL_serialIN    (19)
#define SIGNAL_STO         (20)
#define SIGNAL_STATLOAD    (21)
#define SIGNAL_resStorage  (22)
#define SIGNAL_resCounter  (23)
#define SIGNAL_CHSclk      (24)
#define SIGNAL_exposing    (25)