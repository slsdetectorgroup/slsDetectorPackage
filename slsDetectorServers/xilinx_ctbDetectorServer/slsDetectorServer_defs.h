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
#define NCHAN                   (40)
#define NCHAN_ANALOG            (32)
#define NCHAN_DIGITAL           (64)
#define NCHAN_TRANSCEIVER       (4)
#define NBITS_PER_TRANSCEIVER   (64)
#define NCHIP                   (1)
#define NDAC                    (3)

#define DYNAMIC_RANGE       (16)
#define NUM_BYTES_PER_PIXEL     (DYNAMIC_RANGE / 8)


enum ADCINDEX { V_PWR_IO };
enum DACINDEX { D0 };

/** Default Parameters */
#define DEFAULT_NUM_FRAMES  (1)
#define DEFAULT_NUM_CYCLES  (1)
#define DEFAULT_TIMING_MODE (AUTO_TIMING)
#define DEFAULT_EXPTIME     (0)
#define DEFAULT_PERIOD      (300 * 1000) // 300us
#define DEFAULT_READOUT_MODE (TRANSCEIVER_ONLY)
#define DEFAULT_READOUT_MODE_STR "transceiver_only"
#define DEFAULT_TRANSCEIVER_MASK (0x3) // TODO: check
#define DEFAULT_NUM_ASAMPLES (1)
#define DEFAULT_NUM_DSAMPLES (1)
#define DEFAULT_NUM_TSAMPLES (200)

#define MAX_TRANSCEIVER_MASK    (0xF)
#define MAX_TRANSCEIVER_SAMPLES (0x1FFF)

#define MAX_ANALOG_SAMPLES (0x3FFF)
#define MAX_DIGITAL_SAMPLES (0x3FFF)

#define TICK_CLK (20)  // MHz (trig_timeFromStart, frametime, timeFromStart)
#define RUN_CLK  (100) // MHz (framesFromStart, c_swTrigger, run, waitForTrigger, starting, acquiring, waitForPeriod, internalStop, c_framesFromSTart_reset, s_start, c_stop, triggerEnable, period, frames, cycles, delay)

/* Defines in the Firmware */
#define WAIT_TIME_PATTERN_READ (10)
#define WAIT_TIME_OUT_0US_TIMES     (35000)


#define BIT32_MSK       (0xFFFFFFFF)
#define BIT16_MASK      (0xFFFF)

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


