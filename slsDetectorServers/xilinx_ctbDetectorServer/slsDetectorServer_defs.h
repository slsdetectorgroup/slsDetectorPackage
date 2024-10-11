// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once
#include "RegisterDefs.h"
#include "sls/sls_detector_defs.h"

#define REQRD_FRMWRE_VRSN (0x230710)
#define KERNEL_DATE_VRSN  "Wed Nov 29 17:32:14 CET 2023"

#define LINKED_SERVER_NAME "xilinx_ctbDetectorServer"

#define CTRL_SRVR_INIT_TIME_US (2 * 1000 * 1000)

/* Hardware Definitions */
#define NCHAN                 (40)
#define NCHAN_ANALOG          (32)
#define NCHAN_DIGITAL         (64)
#define NCHAN_TRANSCEIVER     (4)
#define NBITS_PER_TRANSCEIVER (64)
#define NCHIP                 (1)
#define NDAC                  (24)
#define NPWR                  (6)
#define NDAC_ONLY             (NDAC - NPWR)

#define DYNAMIC_RANGE       (16)
#define NUM_BYTES_PER_PIXEL (DYNAMIC_RANGE / 8)

#define MAIN_APP_FOLDER            "/root/apps/xilinx-ctb"
#define FIRMWARE_FILE              MAIN_APP_FOLDER "/XilinxCTB.bit"
#define DEVICE_TREE_OVERLAY_FILE   MAIN_APP_FOLDER "/pl.dtbo"
#define CURRENT_BOARD_LINKS_FOLDER MAIN_APP_FOLDER "/current_board_links"
#define IIO_DEVICE_FOLDER          MAIN_APP_FOLDER "/iio_device_links"

#define DEVICE_TREE_DST        "/sys/bus/iio/devices/iio:device"
#define DEVICE_NAME_LIST       "xilinx-ams", "ad7689", "dac@0", "dac@1", "dac@2"
#define DEVICE_TREE_API_FOLDER "/sys/kernel/config/device-tree/overlays/spidr"

#define DAC_DRIVER_FILE_NAME           CURRENT_BOARD_LINKS_FOLDER "/ao%d"
#define DAC_POWERDOWN_DRIVER_FILE_NAME CURRENT_BOARD_LINKS_FOLDER "/ao%d_pd"
#define SLOWADC_DRIVER_FILE_NAME       CURRENT_BOARD_LINKS_FOLDER "/ai%d"
#define TEMP_DRIVER_FILE_NAME          DEVICE_TREE_DST "0/in_temp7_input"

/** Default Parameters */
#define DEFAULT_NUM_FRAMES            (1)
#define DEFAULT_NUM_CYCLES            (1)
#define DEFAULT_TIMING_MODE           (AUTO_TIMING)
#define DEFAULT_EXPTIME               (0)
#define DEFAULT_PERIOD                (300 * 1000) // 300us
#define DEFAULT_READOUT_MODE          (TRANSCEIVER_ONLY)
#define DEFAULT_READOUT_MODE_STR      "transceiver_only"
#define DEFAULT_TRANSCEIVER_MASK      (0x3) // TODO: check
#define DEFAULT_NUM_ASAMPLES          (1)
#define DEFAULT_NUM_DSAMPLES          (1)
#define DEFAULT_NUM_TSAMPLES          (200)
#define DEFAULT_STARTING_FRAME_NUMBER (1)
#define DEFAULT_VLIMIT                (-100)
#define DEFAULT_DELAY                 (0)

#define MAX_TRANSCEIVER_MASK    (0xF)
#define MAX_TRANSCEIVER_SAMPLES (0x1FFF)

#define MAX_ANALOG_SAMPLES  (0x3FFF)
#define MAX_DIGITAL_SAMPLES (0x3FFF)

#define DAC_MIN_MV      (0)
#define DAC_MAX_MV      (2048)
#define POWER_RGLTR_MIN (1041)
#define POWER_RGLTR_MAX (2661)
#define VIO_MIN_MV      (1200) // for fpga to function

#define TICK_CLK (20) // MHz (trig_timeFromStart, frametime, timeFromStart)
#define RUN_CLK                                                                \
    (100) // MHz (framesFromStart, c_swTrigger, run, waitForTrigger, starting,
          // acquiring, waitForPeriod, internalStop, c_framesFromSTart_reset,
          // s_start, c_stop, triggerEnable, period, frames, cycles, delay)

/* Defines in the Firmware */
#define WAIT_TIME_PATTERN_READ  (10)
#define WAIT_TIME_OUT_0US_TIMES (35000) // 2s

#define BIT32_MSK  (0xFFFFFFFF)
#define BIT16_MASK (0xFFFF)

#define MAX_DATA_SIZE_IN_PACKET (8144)

/* Enum Definitions */
enum ADCINDEX {
    S_ADC0,
    S_ADC1,
    S_ADC2,
    S_ADC3,
    S_ADC4,
    S_ADC5,
    S_ADC6,
    S_ADC7,
    TEMP_FPGA
};
enum DACINDEX {
    D0,
    D1,
    D2,
    D3,
    D4,
    D5,
    D6,
    D7,
    D8,
    D9,
    D10,
    D11,
    D12,
    D13,
    D14,
    D15,
    D16,
    D17,
    D_PWR_D,
    D_PWR_EMPTY,
    D_PWR_IO,
    D_PWR_A,
    D_PWR_B,
    D_PWR_C
};

#define PWR_NAMES "D", "_unknown", "IO", "A", "B", "C"

/* Struct Definitions */
// For arm has to be multiple of 16
// We dont byteswap in the upd_gen so the order has to be different
typedef struct udp_header_struct {
    uint16_t udp_srcmac_msb;
    uint16_t udp_destmac_lsb;
    uint32_t udp_destmac_msb;
    uint8_t ip_tos;
    uint8_t ip_ihl : 4, ip_ver : 4;
    uint16_t udp_ethertype;
    uint32_t udp_srcmac_lsb;
    uint8_t ip_protocol;
    uint8_t ip_ttl;
    uint16_t ip_fragmentoffset : 13, ip_flags : 3;
    uint16_t ip_identification;
    uint16_t ip_totallength;
    uint16_t ip_destip_msb;
    uint16_t ip_srcip_lsb;
    uint16_t ip_srcip_msb;
    uint16_t ip_checksum;
    uint16_t udp_checksum;
    uint16_t udp_destport;
    uint16_t udp_srcport;
    uint16_t ip_destip_lsb;
    // padding
    uint32_t padding0;
    uint32_t padding1;
} udp_header;

#define IP_HEADER_SIZE             (20)
#define UDP_IP_HEADER_LENGTH_BYTES (28)
