#pragma once
#include "sls_detector_defs.h"

#define REQRD_FRMWRE_VRSN     		(0x190000)

#define CTRL_SRVR_INIT_TIME_US      (300 * 1000)

/* Hardware Definitions */
#define NCHAN 						(128)
#define NCHIP 						(10)
#define NDAC 						(16)
#define ONCHIP_NDAC 				(6)
#define DYNAMIC_RANGE               (16) 
#define HV_SOFT_MAX_VOLTAGE         (200)
#define HV_HARD_MAX_VOLTAGE         (530)
#define HV_DRIVER_FILE_NAME         ("/etc/devlinks/hvdac")
#define DAC_DRIVER_FILE_NAME        ("/etc/devlinks/dac")
#define ONCHIP_DAC_DRIVER_FILE_NAME ("/etc/devlinks/chipdac")
#define CONFIG_FILE                 ("config.txt")
#define DAC_MAX_MV                  (2048)
#define ONCHIP_DAC_MAX_VAL			(0x3FF)

/** Default Parameters */
#define DEFAULT_NUM_FRAMES			(1)
#define DEFAULT_NUM_CYCLES			(1)
#define DEFAULT_EXPTIME				(1 * 1000 * 1000)	        // 1 ms
#define DEFAULT_PERIOD				(1 * 1000 * 1000 * 1000)	// 1 s
#define DEFAULT_HIGH_VOLTAGE		(0)
#define DEFAULT_READOUT_C0          (144444448) // rdo_clk, 144 MHz  
#define DEFAULT_READOUT_C1          (288888896) // rdo_x2_clk, 288 MHz  
#define DEFAULT_SYSTEM_C0			(144444448) // run_clk, 144 MHz 
#define DEFAULT_SYSTEM_C1			(72222224) // chip_clk, 72 MHz 
#define DEFAULT_SYSTEM_C2			(18055556) // sync_clk, 18 MHz 
#define DEFAULT_SYSTEM_C3			(144444448) // str_clk, 144 MHz

/* Firmware Definitions */
#define IP_HEADER_SIZE              (20)
#define READOUT_PLL_VCO_FREQ_HZ     (866666688) // Hz 
#define SYSTEM_PLL_VCO_FREQ_HZ      (722222240) // Hz


/** Other Definitions */
#define BIT16_MASK					(0xFFFF)

/* Enums */
enum DACINDEX				        {G2_VREF_H_ADC, /* 0 */		\
									G2_DAC_UNUSED,  /* 1 */		\
									G2_VB_COMP_FE,  /* 2 */		\
        							G2_VB_COMP_ADC, /* 3 */		\
        							G2_VCOM_CDS,    /* 4 */		\
        							G2_VREF_RESTORE,/* 5 */		\
        							G2_VB_OPA_1ST,  /* 6 */		\
        							G2_VREF_COMP_FE,/* 7 */		\
        							G2_VCOM_ADC1,   /* 8 */		\
        							G2_VREF_PRECH,  /* 9 */		\
        							G2_VREF_L_ADC,  /* 10 */ 	\
        							G2_VREF_CDS,    /* 11 */	\
        							G2_VB_CS,       /* 12 */	\
        							G2_VB_OPA_FD,   /* 13 */	\
		  							G2_DAC_UNUSED2, /* 14 */	\
        							G2_VCOM_ADC2    /* 15*/		\
									};  
#define  DAC_NAMES					"vref_h_adc", "dac_unused", "vb_comp_fe", "vb_comp_adc", "vcom_cds", "vref_restore", "vb_opa_1st", "vref_comp_fe", "vcom_adc1", "vref_prech", "vref_l_adc", "vref_cds", "vb_cs", "vb_opa_fd", "dac_unused2", "vcom_adc2"
									
enum ONCHIP_DACINDEX				{G2_VCHIP_COMP_FE, 		/* 0 */		\
									G2_VCHIP_OPA_1ST,  		/* 1 */		\
									G2_VCHIP_OPA_FD,  		/* 2 */		\
        							G2_VCHIP_COMP_ADC, 		/* 3 */		\
        							G2_VCHIP_REF_COMP_FE,	/* 4 */		\
        							G2_VCHIP_CS				/* 5 */		\
									};  
#define  ONCHIP_DAC_NAMES			"vchip_comp_fe", "vchip_opa_1st", "vchip_opa_fd", "vchip_comp_adc", "vchip_ref_comp_fe", "vchip_cs"	


enum CLKINDEX                       {READOUT_C0, READOUT_C1, SYSTEM_C0, SYSTEM_C1, SYSTEM_C2, SYSTEM_C3, NUM_CLOCKS};
#define CLK_NAMES					"READOUT_C0", "READOUT_C1", "SYSTEM_C0", "SYSTEM_C1", "SYSTEM_C2", "SYSTEM_C3"

enum PLLINDEX						{READOUT_PLL, SYSTEM_PLL};

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