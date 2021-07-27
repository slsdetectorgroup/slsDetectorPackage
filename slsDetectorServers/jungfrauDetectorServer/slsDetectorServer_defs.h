#pragma once
#include "RegisterDefs.h"
#include "sls/sls_detector_defs.h"

#define MIN_REQRD_VRSN_T_RD_API  0x171220
#define REQRD_FRMWRE_VRSN_BOARD2 0x210621 // 1.0 pcb (version = 010)
#define REQRD_FRMWRE_VRSN        0x210622 // 2.0 pcb (version = 011)

#define CTRL_SRVR_INIT_TIME_US (300 * 1000)

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
enum NETWORKINDEX { TXN_FRAME, FLOWCTRL_10G };
enum CLKINDEX { RUN_CLK, ADC_CLK, DBIT_CLK, NUM_CLOCKS };
#define CLK_NAMES "run", "adc", "dbit"

/* Hardware Definitions */
#define NCHAN               (256 * 256)
#define NCHIP               (8)
#define NDAC                (8)
#define DYNAMIC_RANGE       (16)
#define NUM_BYTES_PER_PIXEL (DYNAMIC_RANGE / 8)
#define DATA_BYTES          (NCHIP * NCHAN * NUM_BYTES_PER_PIXEL)
#define CLK_RUN             (40) // MHz
#define CLK_SYNC            (20) // MHz
#define ADC_CLK_INDEX       (1)
#define DBIT_CLK_INDEX      (0)
#define CONFIG_FILE         ("config_jungfrau.txt")

/** Default Parameters */
#define DEFAULT_NUM_FRAMES            (100 * 1000 * 1000)
#define DEFAULT_STARTING_FRAME_NUMBER (1)
#define DEFAULT_NUM_CYCLES            (1)
#define DEFAULT_EXPTIME               (10 * 1000)       // ns
#define DEFAULT_PERIOD                (2 * 1000 * 1000) // ns
#define DEFAULT_DELAY                 (0)
#define DEFAULT_HIGH_VOLTAGE          (0)
#define DEFAULT_TIMING_MODE           (AUTO_TIMING)
#define DEFAULT_SETTINGS              (DYNAMICGAIN)
#define DEFAULT_TX_UDP_PORT           (0x7e9a)
#define DEFAULT_TMP_THRSHLD           (65 * 1000) // milli degree Celsius
#define DEFAULT_NUM_STRG_CLLS         (0)
#define DEFAULT_STRG_CLL_STRT         (0xf)
#define DEFAULT_STRG_CLL_DLY          (0)

#define HIGHVOLTAGE_MIN (60)
#define HIGHVOLTAGE_MAX (200)
#define DAC_MIN_MV      (0)
#define DAC_MAX_MV      (2500)

/* Defines in the Firmware */
#define MAX_TIMESLOT_VAL            (0x1F)
#define MAX_THRESHOLD_TEMP_VAL      (127999) // millidegrees
#define MAX_STORAGE_CELL_VAL        (15)     // 0xF
#define MAX_STORAGE_CELL_DLY_NS_VAL (ASIC_CTRL_EXPSRE_TMR_MAX_VAL)
#define ACQ_TIME_MIN_CLOCK          (2)

#define MAX_PHASE_SHIFTS (240)
#define BIT16_MASK       (0xFFFF)

// pipeline
#define ADC_OFST_FULL_SPEED_VAL           (0x10) // 2.0 pcb
#define ADC_OFST_HALF_SPEED_VAL           (0x08) // 2.0 pcb
#define ADC_OFST_QUARTER_SPEED_VAL        (0x04) // 2.0 pcb
#define ADC_OFST_HALF_SPEED_BOARD2_VAL    (0x10) // 1.0 pcb (2 resistor network)
#define ADC_OFST_QUARTER_SPEED_BOARD2_VAL (0x08) // 1.0 pcb (2 resistor network)

#define ADC_PORT_INVERT_VAL        (0x5A5A5A5A)
#define ADC_PORT_INVERT_BOARD2_VAL (0x453b2a9c)

// 2.0 pcb
#define SAMPLE_ADC_FULL_SPEED                                                  \
    (SAMPLE_ADC_SAMPLE_0_VAL + SAMPLE_ADC_DECMT_FACTOR_0_VAL +                 \
     SAMPLE_DGTL_SAMPLE_1_VAL + SAMPLE_DECMT_FACTOR_FULL_VAL) // 0x100
#define SAMPLE_ADC_HALF_SPEED                                                  \
    (SAMPLE_ADC_SAMPLE_0_VAL + SAMPLE_ADC_DECMT_FACTOR_1_VAL +                 \
     SAMPLE_DGTL_SAMPLE_3_VAL + SAMPLE_DECMT_FACTOR_HALF_VAL) // 0x1310
#define SAMPLE_ADC_QUARTER_SPEED                                               \
    (SAMPLE_ADC_SAMPLE_0_VAL + SAMPLE_ADC_DECMT_FACTOR_3_VAL +                 \
     SAMPLE_DGTL_SAMPLE_6_VAL + SAMPLE_DECMT_FACTOR_QUARTER_VAL) // 0x2630
// 1.0 pcb (2 resistor network)
#define SAMPLE_ADC_HALF_SPEED_BOARD2                                           \
    (SAMPLE_ADC_SAMPLE_0_VAL + SAMPLE_ADC_DECMT_FACTOR_0_VAL +                 \
     SAMPLE_DGTL_SAMPLE_3_VAL + SAMPLE_DECMT_FACTOR_HALF_VAL) // 0x1300
#define SAMPLE_ADC_QUARTER_SPEED_BOARD2                                        \
    (SAMPLE_ADC_SAMPLE_0_VAL + SAMPLE_ADC_DECMT_FACTOR_1_VAL +                 \
     SAMPLE_DGTL_SAMPLE_6_VAL + SAMPLE_DECMT_FACTOR_QUARTER_VAL) // 0x2610

#define ADC_PHASE_FULL_SPEED           (150) // 2.0 pcb
#define ADC_PHASE_HALF_SPEED           (200) // 2.0 pcb
#define ADC_PHASE_QUARTER_SPEED        (200) // 2.0 pcb
#define ADC_PHASE_HALF_SPEED_BOARD2    (110)  // 1.0 pcb (2 resistor network)
#define ADC_PHASE_QUARTER_SPEED_BOARD2 (220)  // 1.0 pcb (2 resistor network)

#define DBIT_PHASE_FULL_SPEED           (85)  // 2.0 pcb
#define DBIT_PHASE_HALF_SPEED           (150) // 2.0 pcb
#define DBIT_PHASE_QUARTER_SPEED        (150) // 2.0 pcb
#define DBIT_PHASE_HALF_SPEED_BOARD2    (150) // 1.0 pcb (2 resistor network)
#define DBIT_PHASE_QUARTER_SPEED_BOARD2 (150) // 1.0 pcb (2 resistor network)
