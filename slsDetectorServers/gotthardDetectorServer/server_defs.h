#ifndef SERVER_DEFS_H
#define SERVER_DEFS_H

#include "sls_detector_defs.h"

#include <stdint.h> 


// Hardware definitions
#define NCHAN 128
#define NCHIP 10
#define NDAC 8
#define NADC 5
#define NCHANS NCHAN*NCHIP
#define NDACS NDAC
#define NCHIPS_PER_ADC		2

#define DYNAMIC_RANGE		16
#define DATA_BYTES			(NCHIP*NCHAN*2)

// for 25 um
#define CONFIG_FILE	"config.txt"



#define ADCSYNC_VAL   				0x32214
#define TOKEN_RESTART_DELAY			0x88000000
#define TOKEN_RESTART_DELAY_ROI     0x1b000000
#define TOKEN_TIMING_REV1           0x1f16
#define TOKEN_TIMING_REV2           0x1f0f

#define DEFAULT_PHASE_SHIFT			120
#define DEFAULT_IP_PACKETSIZE		0x0522
#define DEFAULT_UDP_PACKETSIZE		0x050E
#define ADC1_IP_PACKETSIZE			(256*2+14+20)
#define ADC1_UDP_PACKETSIZE			(256*2+4+8+2)

#define CLK_FREQ 32.007729

#define DAC_DR 1024
#define CONF_GAIN     {		\
		0,	/*standard gain*/ 	\
		0, 	/*fast gain*/ 		\
		0, 	/*high gain*/ 		\
		8, 	/*dynamic gain*/ 	\
		6, 	/*low gain*/ 		\
		2, 	/*medium gain*/ 	\
		1	/*very high gain*/ 	\
};
//dynamic gain confgain yet to be figured out-probably 8 or 16

// DAC definitions
enum dacsVal{VREF_DS, VCASCN_PB, VCASCP_PB, VOUT_CM, VCASC_OUT, VIN_CM, VREF_COMP, IB_TESTC,HIGH_VOLTAGE, CONFGAIN};
#define DEFAULT_DAC_VALS   	{		\
		660,	/* VREF_DS */			\
		650,	/* VCASCN_PB */		\
		1480,	/* VCASCP_PB */		\
		1520,	/* VOUT_CM */		\
		1320,	/* VCASC_OUT */		\
		1350,	/* VIN_CM */		\
		350,	/* VREF_COMP */		\
		2001	/* IB_TESTC */		\
};

//Register Definitions for temp,hv,dac gain
enum adcVals{TEMP_FPGA, TEMP_ADC};


#endif
