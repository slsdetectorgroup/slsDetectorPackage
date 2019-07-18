#pragma once
#include "sls_detector_defs.h"
#include "RegisterDefs.h"


#define MIN_REQRD_VRSN_T_RD_API     0x171220
#define REQRD_FRMWR_VRSN            0x190708

#define CTRL_SRVR_INIT_TIME_US      (300 * 1000)

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

#define IP_HEADER_SIZE	20


/* Enums */
enum CLK_SPEED_INDEX		{FULL_SPEED, HALF_SPEED, QUARTER_SPEED};
enum ADCINDEX				{TEMP_FPGA, TEMP_ADC};
enum DACINDEX				{VB_COMP, VDD_PROT, VIN_COM, VREF_PRECH, VB_PIXBUF, VB_DS, VREF_DS, VREF_COMP };
#define DEFAULT_DAC_VALS   	{ 	1220,	/* VB_COMP */		\
								3000,	/* VDD_PROT */		\
								1053,	/* VIN_COM */		\
								1450,	/* VREF_PRECH */	\
								750,	/* VB_PIXBUF */		\
								1000,	/* VB_DS */			\
								480,	/* VREF_DS */		\
								420		/* VREF_COMP */		\
							};
enum NETWORKINDEX           { TXN_FRAME, FLOWCTRL_10G };

/* Hardware Definitions */
#define NCHAN 						(256 * 256)
#define NCHIP 						(8)
#define NDAC 						(8)
#define NDAC_OLDBOARD				(16)
#define DYNAMIC_RANGE				(16)
#define NUM_BYTES_PER_PIXEL			(DYNAMIC_RANGE / 8)
#define DATA_BYTES					(NCHIP * NCHAN * NUM_BYTES_PER_PIXEL)
#define CLK_RUN						(40)	/* MHz */
#define CLK_SYNC					(20)	/* MHz */

/** Default Parameters */
#define DEFAULT_NUM_FRAMES			(100*1000*1000)
#define DEFAULT_STARTING_FRAME_NUMBER (1)
#define DEFAULT_NUM_CYCLES			(1)
#define DEFAULT_EXPTIME				(10*1000)		//ns
#define DEFAULT_PERIOD				(2*1000*1000)	//ns
#define DEFAULT_DELAY				(0)
#define DEFAULT_HIGH_VOLTAGE		(0)
#define DEFAULT_TIMING_MODE			(AUTO_TIMING)
#define DEFAULT_SETTINGS			(DYNAMICGAIN)
#define DEFAULT_TX_UDP_PORT			(0x7e9a)
#define DEFAULT_TMP_THRSHLD         (65*1000) //milli degree Celsius
#define DEFAULT_NUM_STRG_CLLS       (0)
#define DEFAULT_STRG_CLL_STRT       (0xf)
#define DEFAULT_STRG_CLL_DLY		(0)

#define HIGHVOLTAGE_MIN             (60)
#define HIGHVOLTAGE_MAX             (200)
#define DAC_MIN_MV                  (0)
#define DAC_MAX_MV                  (2500)

/* Defines in the Firmware */
#define MAX_TIMESLOT_VAL            (0x1F)
#define MAX_THRESHOLD_TEMP_VAL      (127999) //millidegrees
#define MAX_STORAGE_CELL_VAL        (15) //0xF
#define MAX_STORAGE_CELL_DLY_NS_VAL	((ASIC_CTRL_EXPSRE_TMR_MSK >> ASIC_CTRL_EXPSRE_TMR_OFST) * ASIC_CTRL_EXPSRE_TMR_STEPS)
#define ACQ_TIME_MIN_CLOCK          (2)

#define BOARD_VERSION_2_VAL			(0x3F)

#define SAMPLE_ADC_FULL_SPEED		(SAMPLE_ADC_SAMPLE_0_VAL + SAMPLE_ADC_DECMT_FACTOR_0_VAL + SAMPLE_DGTL_SAMPLE_3_VAL + SAMPLE_DECMT_FACTOR_FULL_VAL) // 0x300
#define SAMPLE_ADC_HALF_SPEED	 	(SAMPLE_ADC_SAMPLE_0_VAL + SAMPLE_ADC_DECMT_FACTOR_1_VAL + SAMPLE_DGTL_SAMPLE_6_VAL + SAMPLE_DECMT_FACTOR_HALF_VAL) // 0x1610
#define SAMPLE_ADC_QUARTER_SPEED 	(SAMPLE_ADC_SAMPLE_0_VAL + SAMPLE_ADC_DECMT_FACTOR_3_VAL + SAMPLE_DGTL_SAMPLE_11_VAL + SAMPLE_DECMT_FACTOR_QUARTER_VAL)	// 0x2b30 

#define ADC_OFST_FULL_SPEED_VAL		(0xF)
#define ADC_OFST_HALF_SPEED_VAL		(0xB)
#define ADC_OFST_QUARTER_SPEED_VAL	(0x7) 

#define ADC_PHASE_FULL_SPEED 		(0x1E) //30
#define ADC_PHASE_HALF_SPEED 		(0x1E) //30
#define ADC_PHASE_QUARTER_SPEED 	(0x1E) //30

#define ADC_PORT_INVERT_VAL         (0x5A5A5A5A)//(0x453b2a9c)
#define MAX_PHASE_SHIFTS			(160)


#define BIT16_MASK					(0xFFFF)
#define UDP_IP_HEADER_LENGTH_BYTES	(28)