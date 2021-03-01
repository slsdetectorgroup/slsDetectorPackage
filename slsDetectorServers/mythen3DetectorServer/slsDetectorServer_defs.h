#pragma once
#include "sls/sls_detector_defs.h"

#define REQRD_FRMWRE_VRSN (0x210201)
#define KERNEL_DATE_VRSN  "Wed May 20 13:58:38 CEST 2020"
#define ID_FILE           "detid_mythen3.txt"

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
#define DEFAULT_PATTERN_FILE             ("DefaultPattern_mythen3.txt")
#define DEFAULT_INTERNAL_GATES           (1)
#define DEFAULT_EXTERNAL_GATES           (1)
#define DEFAULT_DYNAMIC_RANGE            (32)
#define DEFAULT_NUM_FRAMES               (1)
#define DEFAULT_NUM_CYCLES               (1)
#define DEFAULT_GATE_WIDTH               (100 * 1000 * 1000) // ns
#define DEFAULT_GATE_DELAY               (0)
#define DEFAULT_PERIOD                   (2 * 1000 * 1000) // ns
#define DEFAULT_DELAY_AFTER_TRIGGER      (0)
#define DEFAULT_HIGH_VOLTAGE             (0)
#define DEFAULT_TIMING_MODE              (AUTO_TIMING)
#define DEFAULT_SETTINGS                 (STANDARD)
#define DEFAULT_TRIMBIT_VALUE            (0)
#define DEFAULT_COUNTER_DISABLED_VTH_VAL (2800)

#define DEFAULT_STANDARD_VRPREAMP (1100)
#define DEFAULT_FAST_VRPREAMP     (300)
#define DEFAULT_HIGHGAIN_VRPREAMP (1300)
#define DEFAULT_STANDARD_VRSHAPER (1280)
#define DEFAULT_FAST_VRSHAPER     (1500)
#define DEFAULT_HIGHGAIN_VRSHAPER (900)

#define DEFAULT_READOUT_C0 (10) //(100000000) // rdo_clk, 100 MHz
#define DEFAULT_READOUT_C1 (10) //(100000000) // smp sample clk (x2), 100 MHz
#define DEFAULT_SYSTEM_C0  (10) //(100000000) // run_clk, 100 MHz
#define DEFAULT_SYSTEM_C1  (10) //(100000000) // sync_clk, 100 MHz
#define DEFAULT_SYSTEM_C2  (10) //(100000000) // str_clk, 100 MHz
#define DEFAULT_SYSTEM_C3  (5)  //(200000000) // smp_clk, 200 MHz
// (DEFAULT_SYSTEM_C3 only for timing receiver) should not be changed
#define DEFAULT_TRIMMING_RUN_CLKDIV (40) // (25000000) // 25 MHz

#define DEFAULT_ASIC_LATCHING_NUM_PULSES (10)
#define DEFAULT_MSTR_OTPT_P1_NUM_PULSES  (20)
#define DEFAULT_ADIF_PIPELINE_VAL        (8)
#define DEFAULT_ADIF_ADD_OFST_VAL        (0)

/* Firmware Definitions */
#define MAX_TIMESLOT_VAL        (0xFFFFFF)
#define IP_HEADER_SIZE          (20)
#define FIXED_PLL_FREQUENCY     (020000000)  // 20MHz
#define READOUT_PLL_VCO_FREQ_HZ (1000000000) // 1GHz
#define SYSTEM_PLL_VCO_FREQ_HZ  (1000000000) // 1GHz
#define MAX_NUM_DESERIALIZERS   (40)

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
    SYSTEM_C3,
    NUM_CLOCKS
};
#define CLK_NAMES                                                              \
    "READOUT_C0", "READOUT_C1", "SYSTEM_C0", "SYSTEM_C1", "SYSTEM_C2",         \
        "SYSTEM_C3"
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
#define PACKETS_PER_FRAME_10G      (2)
#define PACKETS_PER_FRAME_1G       (20)

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

//CHIP STARTUS REGISTER BITS
#define CSR_spypads  0
#define CSR_invpol  4
#define CSR_dpulse  5
#define CSR_interp  6
#define CSR_C10pre  7 //#default
#define CSR_pumprobe  8
#define CSR_apulse  9
#define CSR_C15sh  10 
#define CSR_C30sh  11 //#default
#define CSR_C50sh  12
#define CSR_C225ACsh  13 // Connects 225fF SHAPER AC cap (1: 225 to shaper, 225 to GND. 0: 450 to shaper) 
#define CSR_C15pre  14 

#define CSR_default  (1<<CSR_C10pre )|(1<< CSR_C30sh)
