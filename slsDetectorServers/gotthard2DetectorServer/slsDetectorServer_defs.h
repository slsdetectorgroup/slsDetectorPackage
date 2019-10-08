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
#define DAC_DRIVER_FILE_NAME        ("/etc/devlinks/dac")
#define DAC_MAX_MV                  (2048)

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
#define  DAC_NAMES					"VREF_H_ADC", "DAC_UNUSED", "VB_COMP_FE", "VB_COMP_ADC", "VCOM_CDS", "VREF_RESTORE", "VB_OPA_1ST", "VREF_COMP_FE", "VCOM_ADC1", "VREF_PRECH", "VREF_L_ADC", "VREF_CDS", "VB_CS", "VB_OPA_FD", "DAC_UNUSED2", "VCOM_ADC2"

/* Enums */
enum DACINDEX				        {G_VREF_H_ADC, /* 0	*/		\
									G_DAC_UNUSED,  /* 1 */ 		\
									G_VB_COMP_FE,  /* 2 */		\
        							G_VB_COMP_ADC, /* 3 */		\
        							G_VCOM_CDS,    /* 4 */		\
        							G_VREF_RESTORE,/* 5 */		\
        							G_VB_OPA_1ST,  /* 6 */		\
        							G_VREF_COMP_FE,/* 7 */		\
        							G_VCOM_ADC1,   /* 8 */		\
        							G_VREF_PRECH,  /* 9 */ 		\
        							G_VREF_L_ADC,  /* 10 */ 	\
        							G_VREF_CDS,    /* 11 */		\
        							G_VB_CS,       /* 12 */		\
        							G_VB_OPA_FD,   /* 13 */		\
		  							G_DAC_UNUSED2, /* 14 */		\
        							G_VCOM_ADC2    /* 15*/		\
									};  
#define DEFAULT_DAC_VALS   			{2099, 		/* 0 (1050 mV) VREF_H_ADC*/ 	\
									0, 			/* 1 (0 mV) DAC_UNUSED*/ 		\
									0, 			/* 2 (0 mV) VB_COMP_FE*/   		\
        							0,			/* 3 (0 mV) VB_COMP_ADC*/  		\
        							1400, 		/* 4 (700 mV) VCOM_CDS*/   		\
        							640, 		/* 5 (320 mV) VREF_RESTORE*/	\
        							0, 			/* 6 (0 mV) VB_OPA_1ST*/    	\
        							0, 			/* 7 (0 mV) VREF_COMP_FE*/ 		\
        							1400, 		/* 8 (700 mV) VCOM_ADC1*/  		\
        							1720, 		/* 9 (860 mV) VREF_PRECH*/ 		\
        							700, 		/* 10 (350 mV) VREF_L_ADC*/		\
        							1200, 		/* 11 (600 mV) VREF_CDS*/  		\
        							2799, 		/* 12 (1400 mV) VB_CS*/   		\
        							0, 			/* 13 (0 mV) VB_OPA_FD*/  		\
		  							0,			/* 14 (0 mV) DAC_UNUSED2*/		\
        							1400 		/* 15 (700 mV) VCOM_ADC2*/		\
									};
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