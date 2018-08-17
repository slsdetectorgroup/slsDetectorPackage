/*
 * slsDetectorServer_defs.h
 *
 *  Created on: Jan 24, 2013
 *      Author: l_maliakal_d
 */

#ifndef SLSDETECTORSERVER_DEFS_H_
#define SLSDETECTORSERVER_DEFS_H_

#include "sls_detector_defs.h"
#include <stdint.h>

#define GOODBYE 					 		(-200)
#define REQUIRED_FIRMWARE_VERSION 	 		(22)
#define IDFILECOMMAND						"more /home/root/executables/detid.txt"

#define STATUS_IDLE		0
#define STATUS_RUNNING	1
#define STATUS_ERROR	2

/* Enums */
enum CLK_SPEED_INDEX						{FULL_SPEED, HALF_SPEED, QUARTER_SPEED};
enum DACINDEX					 			{SVP,VTR,VRF,VRS,SVN,VTGSTV,VCMP_LL,VCMP_LR,CAL,VCMP_RL,RXB_RB,RXB_LB,VCMP_RR,VCP,VCN,VIS,VTHRESHOLD};
#define DEFAULT_DAC_VALS	 	 			{	\
									 		0, 		/* SvP		*/	\
								 			2480, 	/* Vtr		*/	\
								 			3300, 	/* Vrf		*/	\
								 			1400, 	/* Vrs		*/	\
								 			4000, 	/* SvN		*/	\
								 			2556, 	/* Vtgstv	*/	\
								  			1000, 	/* Vcmp_ll	*/	\
								 			1000, 	/* Vcmp_lr	*/	\
								 			4000, 	/* cal		*/	\
									 		1000, 	/* Vcmp_rl	*/	\
									 		1100, 	/* rxb_rb	*/	\
									 		1100, 	/* rxb_lb	*/	\
									 		1000, 	/* Vcmp_rr	*/	\
									 		1000, 	/* Vcp		*/	\
									 		2000, 	/* Vcn		*/	\
									 		1550 	/* Vis		*/	\
									 		};
enum ADCINDEX						 		{TEMP_FPGAEXT, TEMP_10GE, TEMP_DCDC, TEMP_SODL, TEMP_SODR, TEMP_FPGA, TEMP_FPGAFEBL, TEMP_FPGAFEBR};
enum NETWORKINDEX				 			{TXN_LEFT, TXN_RIGHT, TXN_FRAME,FLOWCTRL_10G};


/* Hardware Definitions */
#define NMAXMOD 					 		(1)
#define NMOD 						 		(1)
#define NCHAN 						 		(256 * 256)
#define NCHIP 						 		(4)
#define NADC						 		(0)
#define NDAC 						 		(16)
#define NGAIN						 		(0)
#define NOFFSET						 		(0)


#define TEN_GIGA_BUFFER_SIZE 				(4112)
#define ONE_GIGA_BUFFER_SIZE 				(1040)
#define TEN_GIGA_CONSTANT	 				(4)
#define ONE_GIGA_CONSTANT 					(16)
#define NORMAL_HIGHVOLTAGE_INPUTPORT 		"/sys/class/hwmon/hwmon5/device/in0_input"
#define NORMAL_HIGHVOLTAGE_OUTPUTPORT 		"/sys/class/hwmon/hwmon5/device/out0_output"
#define SPECIAL9M_HIGHVOLTAGE_PORT 			"/dev/ttyS1"
#define SPECIAL9M_HIGHVOLTAGE_BUFFERSIZE 	(16)

/** Default Parameters */
#define DEFAULT_MOD_INDEX					(0)
#define DEFAULT_NUM_FRAMES					(1)
#define DEFAULT_NUM_CYCLES					(1)
#define DEFAULT_EXPTIME						(1E9)			//ns
#define DEFAULT_PERIOD						(1E9)			//ns
#define DEFAULT_DELAY						(0)
#define DEFAULT_HIGH_VOLTAGE				(0)
#define DEFAULT_SETTINGS					(DYNAMICGAIN)
#define DEFAULT_SUBFRAME_EXPOSURE 			(2621440) 		// 2.6ms
#define DEFAULT_SUBFRAME_DEADTIME			(0)
#define DEFAULT_DYNAMIC_RANGE				(16)

#define DEFAULT_READOUT_MODE				(NONPARALLEL)
#define DEFAULT_READOUT_STOREINRAM_MODE		(CONTINOUS_RO)
#define DEFAULT_READOUT_OVERFLOW32_MODE		(NOOVERFLOW)
#define DEFAULT_CLK_SPEED					(HALF_SPEED)
#define DEFAULT_IO_DELAY					(650)
#define DEFAULT_TIMING_MODE					(AUTO_TIMING)
#define DEFAULT_PHOTON_ENERGY				(-1)
#define DEFAULT_RATE_CORRECTION				(0)
#define DEFAULT_EXT_GATING_ENABLE			(0)
#define DEFAULT_EXT_GATING_POLARITY			(1)				//positive
#define DEFAULT_TEST_MODE					(0)
#define DEFAULT_HIGH_VOLTAGE				(0)


#define MAX_SUBFRAME_EXPOSURE_VAL_IN_10NS	(0x1FFFFFFF) /** 29 bit register for max subframe exposure value */

#define SLAVE_HIGH_VOLTAGE_READ_VAL			(-999)
#define HIGH_VOLTAGE_TOLERANCE				(5)


#endif /* SLSDETECTORSERVER_DEFS_H_ */
