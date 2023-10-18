// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once
#include "sls/sls_detector_defs.h"

#define LINKED_SERVER_NAME "eigerDetectorServer"

#define NUM_HARDWARE_VERSIONS    (2)
#define HARDWARE_VERSION_NUMBERS {0x0, 0x1};
#define HARDWARE_VERSION_NAMES                                                 \
    { "FX70T", "FX30T" }

#define REQUIRED_FIRMWARE_VERSION (32)
// virtual ones renamed for consistency
// real ones keep previous name for compatibility (already in production)
#ifdef VIRTUAL
#define ID_FILE "detid_eiger.txt"
#else
#define ID_FILE "detid.txt"
#endif
#define CONFIG_FILE            ("config_eiger.txt")
#define WAIT_STOP_SERVER_START (1 * 1000 * 1000)

#define STATUS_IDLE    0
#define STATUS_RUNNING 1
#define STATUS_ERROR   2

/* Enums */
enum DACINDEX {
    E_VSVP,
    E_VTRIM,
    E_VRPREAMP,
    E_VRSHAPER,
    E_VSVN,
    E_VTGSTV,
    E_VCMP_LL,
    E_VCMP_LR,
    E_VCAL,
    E_VCMP_RL,
    E_RXB_RB,
    E_RXB_LB,
    E_VCMP_RR,
    E_VCP,
    E_VCN,
    E_VISHAPER,
    E_VTHRESHOLD
};
#define DAC_NAMES                                                              \
    "VSvP", "Vtrim", "Vrpreamp", "Vrshaper", "VSvN", "Vtgstv", "Vcmp_ll",      \
        "Vcmp_lr", "Vcal", "Vcmp_rl", "rxb_rb", "rxb_lb", "Vcmp_rr", "Vcp",    \
        "Vcn", "Vishaper", "Vthreshold"
#define DEFAULT_DAC_VALS                                                       \
    {                                                                          \
        0,    /* VSvP		*/                                                      \
        2480, /* Vtrim		*/                                                     \
        3300, /* Vrpreamp		*/                                                  \
        1400, /* Vrshaper		*/                                                  \
        4000, /* VSvN		*/                                                      \
        2556, /* Vtgstv	*/                                                     \
        1000, /* Vcmp_ll	*/                                                    \
        1000, /* Vcmp_lr	*/                                                    \
        0,    /* Vcal		*/                                                      \
        1000, /* Vcmp_rl	*/                                                    \
        1100, /* rxb_rb	*/                                                     \
        1100, /* rxb_lb	*/                                                     \
        1000, /* Vcmp_rr	*/                                                    \
        1000, /* Vcp		*/                                                       \
        2000, /* Vcn		*/                                                       \
        1550  /* Vishaper		*/                                                  \
    };
enum ADCINDEX {
    TEMP_FPGAEXT,
    TEMP_10GE,
    TEMP_DCDC,
    TEMP_SODL,
    TEMP_SODR,
    TEMP_FPGA,
    TEMP_FPGAFEBL,
    TEMP_FPGAFEBR
};

#define ADC_NAMES                                                              \
    "FPGA EXT", "10GE", "DCDC", "SODL", "SODR", "FPGA", "FPGA_FL", "FPGA_FR"

enum NETWORKINDEX { TXN_LEFT, TXN_RIGHT, TXN_FRAME, FLOWCTRL_10G };
enum ROINDEX { E_PARALLEL, E_NON_PARALLEL };
enum CLKINDEX { RUN_CLK, NUM_CLOCKS };
enum TOPINDEX { TOP_HARDWARE, OW_TOP, OW_BOTTOM };
#define TOP_NAMES "hardware", "top", "bottom"
enum MASTERINDEX { MASTER_HARDWARE, OW_MASTER, OW_SLAVE };
#define MASTER_NAMES "hardware", "master", "slave"

#define CLK_NAMES "run"

/* Hardware Definitions */
#define NCHAN (256 * 256)
#define NCHIP (4)
#define NDAC  (16)

#define TEN_GIGA_BUFFER_SIZE         (4112)
#define ONE_GIGA_BUFFER_SIZE         (1040)
#define TEN_GIGA_CONSTANT            (4)
#define ONE_GIGA_CONSTANT            (16)
#define NORMAL_HIGHVOLTAGE_INPUTPORT "/sys/class/hwmon/hwmon5/device/in0_input"
#define NORMAL_HIGHVOLTAGE_OUTPUTPORT                                          \
    "/sys/class/hwmon/hwmon5/device/out0_output"
#define SPECIAL9M_HIGHVOLTAGE_PORT       "/dev/ttyS1"
#define SPECIAL9M_HIGHVOLTAGE_BUFFERSIZE (16)
#define DEFAULT_UDP_SOURCE_PORT          (0xE185)

/** Default Parameters */
#define DEFAULT_NUM_FRAMES            (1)
#define DEFAULT_STARTING_FRAME_NUMBER (1)
#define DEFAULT_NUM_CYCLES            (1)
#define DEFAULT_EXPTIME               (1E9) // ns
#define DEFAULT_PERIOD                (1E9) // ns
#define DEFAULT_DELAY                 (0)
#define DEFAULT_HIGH_VOLTAGE          (0)
#define DEFAULT_SETTINGS              (DYNAMICGAIN)
#define DEFAULT_SUBFRAME_EXPOSURE     (2621440) // 2.6ms
#define DEFAULT_SUBFRAME_DEADTIME     (0)
#define DEFAULT_DYNAMIC_RANGE         (16)

#define DEFAULT_PARALLEL_MODE           (1)
#define DEFAULT_READOUT_OVERFLOW32_MODE (0)
#define DEFAULT_CLK_SPEED               (FULL_SPEED)
#define DEFAULT_IO_DELAY                (650)
#define DEFAULT_TIMING_MODE             (AUTO_TIMING)
#define DEFAULT_PHOTON_ENERGY           (-1)
#define DEFAULT_RATE_CORRECTION         (0)
#define DEFAULT_EXT_GATING_ENABLE       (0)
#define DEFAULT_EXT_GATING_POLARITY     (1) // positive
#define DEFAULT_TEST_MODE               (0)
#define DEFAULT_HIGH_VOLTAGE            (0)

#define MAX_TRIMBITS_VALUE (63)

#define MIN_ROWS_PER_READOUT    (1)
#define MAX_ROWS_PER_READOUT    (256)
#define MAX_PACKETS_PER_REQUEST (256)

#define UDP_HEADER_MAX_FRAME_VALUE (0xFFFFFFFFFFFF)

#define BIT16_MASK (0xFFFF)
#define BIT32_MSK  (0xFFFFFFFF)

#define DAC_MIN_MV (0)
#define DAC_MAX_MV (2048)
#define LTC2620_MIN_VAL                                                        \
    (0) // including LTC defines instead of LTC262.h (includes bit banging and
        // blackfin read and write)
#define LTC2620_MAX_VAL (4095) // 12 bits
#define DAC_MAX_STEPS   (4096)

#define MAX_SUBFRAME_EXPOSURE_VAL_IN_10NS                                      \
    (0x1FFFFFFF) // 29 bit register for max subframe exposure value

#define SLAVE_HIGH_VOLTAGE_READ_VAL (-999)
#define HIGH_VOLTAGE_TOLERANCE      (5)
