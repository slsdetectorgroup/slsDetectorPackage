#pragma once
#include "sls_detector_defs.h"

#define REQRD_FRMWRE_VRSN     		(0x190000)

#define CTRL_SRVR_INIT_TIME_US      (300 * 1000)

/* Hardware Definitions */
#define NCHAN 						(128)
#define NCHIP 						(10)
#define NDAC 						(16)
#define DYNAMIC_RANGE               (16) 

#define HV_SOFT_MAX_VOLTAGE         (200)
#define HV_HARD_MAX_VOLTAGE         (530)
#define HV_DRIVER_FILE_NAME         ("/etc/devlinks/hvdac")

/** Default Parameters */
#define DEFAULT_NUM_FRAMES			(1)
#define DEFAULT_NUM_CYCLES			(1)
#define DEFAULT_EXPTIME				(1 * 1000 * 1000)	        // 1 ms
#define DEFAULT_PERIOD				(1 * 1000 * 1000 * 1000)	// 1 s
#define DEFAULT_HIGH_VOLTAGE		(0)
#define DEFAULT_RUN_CLK             (125) 
#define DEFAULT_TICK_CLK            (20) // will be fixed later. Not configurable
#define DEFAULT_SAMPLING_CLK        (80)

#define DEFAULT_TX_UDP_PORT			(0x7e9a)

/* Firmware Definitions */
#define IP_HEADER_SIZE              (20)

/** Other Definitions */
#define BIT16_MASK					(0xFFFF)

/* Enums */
enum DACINDEX				        {DAC0};
enum CLKINDEX                       {RUN_CLK, TICK_CLK, SAMPLING_CLK, NUM_CLOCKS};

/* Struct Definitions */
typedef struct udp_header_struct {
	uint32_t	udp_destmac_msb;
	uint16_t	udp_srcmac_msb;
	uint16_t	udp_destmac_lsb;
	uint32_t	udp_srcmac_lsb;
	uint8_t		ip_tos;
	uint8_t		ip_ihl: 4, ip_ver: 4;
	uint16_t	udp_ethertype;
	uint16_t	ip_identification;
	uint16_t	ip_totallength;
	uint8_t		ip_protocol;
	uint8_t		ip_ttl;
	uint16_t	ip_fragmentoffset: 13, ip_flags: 3;
	uint16_t	ip_srcip_msb;
	uint16_t	ip_checksum;
	uint16_t	ip_destip_msb;
	uint16_t	ip_srcip_lsb;
	uint16_t	udp_srcport;
	uint16_t	ip_destip_lsb;
	uint16_t	udp_checksum;
	uint16_t	udp_destport;
} udp_header;
#define UDP_IP_HEADER_LENGTH_BYTES	(28)