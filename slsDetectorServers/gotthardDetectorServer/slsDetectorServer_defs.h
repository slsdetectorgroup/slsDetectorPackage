#pragma once
#include "sls_detector_defs.h"
#include <stdlib.h>

#define GOODBYE 					(-200)

/* Enums */
enum ADCINDEX				        {TEMP_FPGA, TEMP_ADC};
enum DACINDEX				        {VREF_DS, VCASCN_PB, VCASCP_PB, VOUT_CM, VCASC_OUT, VIN_CM, VREF_COMP, IB_TESTC};
#define DEFAULT_DAC_VALS   	        { 	                        \
                                    660,    /* VREF_DS */       \
                                    650,    /* VCASCN_PB */     \
                                    1480,   /* VCASCP_PB */     \
                                    1520,   /* VOUT_CM */       \
                                    1320,   /* VCASC_OUT */     \
                                    1350,   /* VIN_CM */        \
                                    350,    /* VREF_COMP */     \
                                    2001    /* IB_TESTC */      \
                                    };

/* for 25 um */
#define CONFIG_FILE                 "config.txt"

/* Hardware Definitions */
#define NCHAN 						(128)
#define NCHIP 						(10)
#define NDAC 						(8)
#define NCHIPS_PER_ADC              (2)
#define DYNAMIC_RANGE				(16)
#define NUM_BITS_PER_PIXEL			(DYNAMIC_RANGE / 8)
#define DATA_BYTES					(NCHIP * NCHAN * NUM_BITS_PER_PIXEL)
#define CLK_FREQ					(32.007729)	/* MHz */

/** Firmware Definitions */
#define IP_PACKET_SIZE_NO_ROI       (NCHIP * (NCHAN / 2) * 2 + 14 + 20)     // 2 packets, so divide by 2
#define IP_PACKET_SIZE_ROI          (NCHIPS_PER_ADC * NCHAN * 2 + 14 + 20)

#define UDP_PACKETSIZE_NO_ROI       (NCHIP * (NCHAN / 2) * 2 + 4 + 8 + 2)  // 2 packets, so divide by 2
#define UDP_PACKETSIZE_ROI          (NCHIPS_PER_ADC * NCHAN * 2 + 4 + 8 + 2)

/** Default Parameters */
#define DEFAULT_NUM_FRAMES			(1)
#define DEFAULT_NUM_CYCLES			(1)
#define DEFAULT_EXPTIME				(1 * 1000 * 1000)	        // 1 ms
#define DEFAULT_PERIOD				(1 * 1000 * 1000 * 1000)	// 1 s
#define DEFAULT_DELAY				(0)
#define DEFAULT_SETTINGS            (DYNAMICGAIN)
#define DEFAULT_TIMING_MODE         (AUTO_TIMING)
#define DEFAULT_TRIGGER_MODE        (TRIGGER_IN_RISING_EDGE)
#define DEFAULT_HIGH_VOLTAGE		(0)
#define DEFAULT_PHASE_SHIFT         (120)
#define DEFAULT_TX_UDP_PORT         (0xE185)

/** ENEt conf structs */
typedef struct mac_header_struct{
    u_int8_t    mac_dest_mac2;
    u_int8_t    mac_dest_mac1;
    u_int8_t    mac_dummy1;
    u_int8_t    mac_dummy2;
    u_int8_t    mac_dest_mac6;
    u_int8_t    mac_dest_mac5;
    u_int8_t    mac_dest_mac4;
    u_int8_t    mac_dest_mac3;
    u_int8_t    mac_src_mac4;
    u_int8_t    mac_src_mac3;
    u_int8_t    mac_src_mac2;
    u_int8_t    mac_src_mac1;
    u_int16_t   mac_ether_type;
    u_int8_t    mac_src_mac6;
    u_int8_t    mac_src_mac5;
} mac_header;

typedef struct ip_header_struct {
    u_int16_t     ip_len;
    u_int8_t      ip_tos;
    u_int8_t      ip_ihl:4 ,ip_ver:4;
    u_int16_t     ip_offset:13,ip_flag:3;
    u_int16_t     ip_ident;
    u_int16_t     ip_chksum;
    u_int8_t      ip_protocol;
    u_int8_t      ip_ttl;
    u_int32_t     ip_sourceip;
    u_int32_t     ip_destip;
} ip_header;

typedef struct udp_header_struct{
    u_int16_t   udp_destport;
    u_int16_t   udp_srcport;
    u_int16_t   udp_chksum;
    u_int16_t   udp_len;
} udp_header;

typedef struct mac_conf_struct{
    mac_header  mac;
    ip_header   ip;
    udp_header  udp;
    u_int32_t   npack;
    u_int32_t   lpack;
    u_int32_t   npad;
    u_int32_t   cdone;
} mac_conf;

typedef struct tse_conf_struct{
    u_int32_t   rev;                    //0x0
    u_int32_t   scratch;
    u_int32_t   command_config;
    u_int32_t   mac_0;                  //0x3
    u_int32_t   mac_1;
    u_int32_t   frm_length;
    u_int32_t   pause_quant;
    u_int32_t   rx_section_empty;       //0x7
    u_int32_t   rx_section_full;
    u_int32_t   tx_section_empty;
    u_int32_t   tx_section_full;
    u_int32_t   rx_almost_empty;        //0xB
    u_int32_t   rx_almost_full;
    u_int32_t   tx_almost_empty;
    u_int32_t   tx_almost_full;
    u_int32_t   mdio_addr0;             //0xF
    u_int32_t   mdio_addr1;
}tse_conf;

