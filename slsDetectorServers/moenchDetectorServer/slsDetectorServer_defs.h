#pragma once
#include "sls_detector_defs.h"
#include "RegisterDefs.h"


#define MIN_REQRD_VRSN_T_RD_API     0x180314
#define REQRD_FRMWR_VRSN            0x180314

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
enum CLKINDEX               {RUN_CLK, ADC_CLK, SYNC_CLK, DBIT_CLK, NUM_CLOCKS};
enum DACINDEX               {D0, D1, D2, D3, D4, D5, D6, D7};

/* Hardware Definitions */
#define NCHAN 						(32)
#define NCHIP 						(1)
#define NDAC 						(8)
#define DYNAMIC_RANGE               (16)
#define NUM_BYTES_PER_PIXEL         (DYNAMIC_RANGE / 8)
#define CLK_FREQ					(156.25)	/* MHz */

/** Default Parameters */
#define DEFAULT_DATA_BYTES          (NCHIP * NCHAN * NUM_BITS_PER_PIXEL)
#define DEFAULT_NUM_SAMPLES         (1)
#define DEFAULT_EXPTIME				(0)
#define DEFAULT_NUM_FRAMES			(100 * 1000 * 1000)
#define DEFAULT_NUM_CYCLES			(1)
#define DEFAULT_PERIOD				(1 * 1000 * 1000)	//ns
#define DEFAULT_DELAY				(0)
#define DEFAULT_HIGH_VOLTAGE		(0)
#define DEFAULT_VLIMIT              (-100)
#define DEFAULT_TIMING_MODE			(AUTO_TIMING)
#define DEFAULT_TX_UDP_PORT			(0x7e9a)
#define DEFAULT_RUN_CLK             (40)
#define DEFAULT_ADC_CLK             (20)
#define DEFAULT_SYNC_CLK            (20)
#define DEFAULT_DBIT_CLK            (200)

#define HIGHVOLTAGE_MIN             (60)
#define HIGHVOLTAGE_MAX             (200)
#define DAC_MIN_MV                  (0)
#define DAC_MAX_MV                  (2500)

/* Defines in the Firmware */
#define MAX_PATTERN_LENGTH  		(0xFFFF)
#define DIGITAL_IO_DELAY_MAXIMUM_PS	((OUTPUT_DELAY_0_OTPT_STTNG_MSK >> OUTPUT_DELAY_0_OTPT_STTNG_OFST) * OUTPUT_DELAY_0_OTPT_STTNG_STEPS)
#define MAX_PHASE_SHIFTS_STEPS		(8)

#define WAIT_TME_US_FR_ACQDONE_REG 	(100) // wait time in us after acquisition done to ensure there is no data in fifo
#define WAIT_TIME_US_PLL            (10 * 1000)
#define WAIT_TIME_US_STP_ACQ        (100)
#define WAIT_TIME_CONFIGURE_MAC     (500 * 1000)
#define WAIT_TIME_PATTERN_READ     	(10)
#define WAIT_TIME_FIFO_RD_STROBE  	(10)

/* MSB & LSB DEFINES */
#define MSB_OF_64_BIT_REG_OFST      (32)
#define LSB_OF_64_BIT_REG_OFST      (0)
#define BIT_32_MSK                  (0xFFFFFFFF)

#define IP_PACKETSIZE               (0x2032)
#define ADC_PORT_INVERT_VAL   		(0x453b2593) //FIXME: a default value?
#define MAXIMUM_ADC_CLK             (40)
#define PLL_VCO_FREQ_MHZ            (800)

