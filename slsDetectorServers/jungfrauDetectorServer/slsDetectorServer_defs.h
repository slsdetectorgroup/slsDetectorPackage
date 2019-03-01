#pragma once
#include "sls_detector_defs.h"
#include "RegisterDefs.h"


#define GOODBYE 					(-200)
#define MIN_REQRD_VRSN_T_RD_API     0x171220
#define REQRD_FRMWR_VRSN            0x181206 // temp bug fix from last version, timing mode is backwards compatible

#define PROGRAMMING_MODE            (0x2)
#define BOARD_JUNGFRAU_TYPE         (8)
#define CTRL_SRVR_INIT_TIME_US      (300 * 1000)

/* Struct Definitions */
typedef struct ip_header_struct {
	uint16_t     ip_len;
	uint8_t      ip_tos;
	uint8_t      ip_ihl:4 ,ip_ver:4;
	uint16_t     ip_offset:13,ip_flag:3;
	uint16_t     ip_ident;
	uint16_t     ip_chksum;
	uint8_t      ip_protocol;
	uint8_t      ip_ttl;
	uint32_t     ip_sourceip;
	uint32_t     ip_destip;
} ip_header;

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
enum NETWORKINDEX           { TXN_FRAME };

/* Hardware Definitions */
#define NCHAN 						(256 * 256)
#define NCHIP 						(8)
#define NDAC 						(8)
#define NDAC_OLDBOARD				(16)
#define DYNAMIC_RANGE				(16)
#define NUM_BYTES_PER_PIXEL			(DYNAMIC_RANGE / 8)
#define DATA_BYTES					(NCHIP * NCHAN * NUM_BYTES_PER_PIXEL)
#define IP_PACKETSIZE				(0x2052)
#define CLK_RUN						(40)	/* MHz */
#define CLK_SYNC					(20)	/* MHz */

/** Default Parameters */
#define DEFAULT_NUM_FRAMES			(100*1000*1000)
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

#define SAMPLE_ADC_HALF_SPEED	 	(SAMPLE_DECMT_FACTOR_2_VAL + SAMPLE_DGTL_SAMPLE_0_VAL + SAMPLE_ADC_DECMT_FACTOR_0_VAL + SAMPLE_ADC_SAMPLE_0_VAL)	/* 0x1000 */
#define SAMPLE_ADC_QUARTER_SPEED 	(SAMPLE_DECMT_FACTOR_4_VAL + SAMPLE_DGTL_SAMPLE_8_VAL + SAMPLE_ADC_DECMT_FACTOR_1_VAL + SAMPLE_ADC_SAMPLE_0_VAL)	/* 0x2810 */
#define CONFIG_HALF_SPEED           (CONFIG_TDMA_DISABLE_VAL + CONFIG_HALF_SPEED_20MHZ_VAL + CONFIG_OPRTN_MDE_1_X_10GBE_VAL)
#define CONFIG_QUARTER_SPEED        (CONFIG_TDMA_DISABLE_VAL + CONFIG_QUARTER_SPEED_10MHZ_VAL + CONFIG_OPRTN_MDE_1_X_10GBE_VAL)
#define ADC_OFST_HALF_SPEED_VAL		(0x1f) //(0x20)
#define ADC_OFST_QUARTER_SPEED_VAL	(0x0f) //(0x0f)
#define ADC_PHASE_HALF_SPEED 		(0x2D) //45
#define ADC_PHASE_QUARTER_SPEED 	(0x2D) //45
#define ADC_PORT_INVERT_VAL         (0x453b2a9c)

/* MSB & LSB DEFINES */
#define MSB_OF_64_BIT_REG_OFST		(32)
#define LSB_OF_64_BIT_REG_OFST		(0)
#define BIT_32_MSK					(0xFFFFFFFF)

