#ifndef SERVER_DEFS_H
#define SERVER_DEFS_H

#include "sls_detector_defs.h"

#include <stdint.h> 


// Hardware definitions
#define NMAXMODY 	1
#define NMAXMODX  	1
#define NMAXMOD 	(NMAXMODX*NMAXMODY)
#define NCHAN 		(256*256)
#define NCHIP 		8
#define NADC		0
#define NDAC 		16
#define NCHANS 		(NCHAN*NCHIP*NMAXMOD)
#define NDACS 		(NDAC*NMAXMOD)


/**when moench readout tested with gotthard module*/
#define GOTTHARDNCHAN 128
#define GOTTHARDNCHIP 10


#define NTRIMBITS 6
#define NCOUNTBITS 24

#define NCHIPS_PER_ADC		2

//#define TRIM_DR ((2**NTRIMBITS)-1)
//#define COUNT_DR ((2**NCOUNTBITS)-1) 
#define TRIM_DR (((int)pow(2,NTRIMBITS))-1)
#define COUNT_DR (((int)pow(2,NCOUNTBITS))-1)


#define ALLMOD 0xffff
#define ALLFIFO 0xffff

#define GOTTHARD_ADCSYNC_VAL		0x32214
#define ADCSYNC_VAL   				0x02111
#define TOKEN_RESTART_DELAY			0x88000000
#define TOKEN_RESTART_DELAY_ROI     0x1b000000
#define TOKEN_TIMING_REV1           0x1f16
#define TOKEN_TIMING_REV2           0x1f0f

#define DEFAULT_PHASE_SHIFT		0 //	120
#define DEFAULT_IP_PACKETSIZE		0x0522
#define DEFAULT_UDP_PACKETSIZE		0x050E
#define ADC1_IP_PACKETSIZE			256*2+14+20
#define ADC1_UDP_PACKETSIZE			256*2+4+8+2

#ifdef VIRTUAL
#define DEBUGOUT
#endif

#define CLK_FREQ 156.25E+6
#define ADC_CLK_FREQ 32E+6


#endif
