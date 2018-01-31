/*
 * mythen3Server_defs.h
 *
 *  Created on: Jan 24, 2013
 *      Author: l_maliakal_d, changed my Marie A.
 */

#ifndef SLSDETECTORSERVER_DEFS_H_
#define SLSDETECTORSERVER_DEFS_H_

#include "sls_detector_defs.h"
#include <stdint.h>

/** This is only an example file!!! */



#define GOODBYE 					 		(-200)
enum ADCINDEX				{TEMP_FPGA, TEMP_ADC};
enum DACINDEX					 			{vIpre, vIbias, Vrf, VrfSh, vIinSh, VdcSh, Vth2, VPL, Vth1, Vth3, Vtrim, casSh, cas, vIbiasSh, vIcin, VPH, NC, vIpreOut}; // Mythen 3.01
#define DEFAULT_DAC_VALS { 	2150,	/* vIpre 	*/ \
							1200,	/* vIbias 	*/ \
							900,	/* Vrf 		*/ \
							1050,	/* VrfSh	*/ \
							1400,	/* vIinSh	*/ \
							655, 	/* VdcSh	*/ \
							850, 	/* Vth2		*/ \
							1400,	/* VPL		*/ \
							850,	/* Vth1		*/ \
							850,	/* Vth3		*/ \
							2294,	/* Vtrim	*/ \
							983,	/* casSh	*/ \
							1474,	/* cas 		*/ \
							1200, 	/* vIbiasSh	*/ \
							1600,	/* vIcin	*/ \
							1520,	/* VPH		*/ \
							0,		/* NC 		*/ \
							1000	/* vIpreOut	*/ \
						};

/*Hardware Definitions
#define NMAXMOD 					 		(1)
#define NMOD 						 		(1)
#define NCHAN 						 		(256 * 256)
#define NCHIP 						 		(4)
#define NADC						 		(0)
#define NDAC 						 		(16)
#define NGAIN						 		(0)
#define NOFFSET						 		(0)
*/

/** Default Parameters */
#define DEFAULT_EXPTIME				(10*1000)		//ns

/* Hardware Definitions */
//#define NMAXMOD 					(1)
#define NMOD 						(1)
//#define NCHAN 						(256 * 256)
//#define NCHIP 						(8)
//#define NADC						(0)
#define NDAC 						(8)
#define NDAC_OLDBOARD				(16)
#define DYNAMIC_RANGE				(16)
#define NUM_BITS_PER_PIXEL			(DYNAMIC_RANGE / 8)
#define DATA_BYTES					(NCHIP * NCHAN * NUM_BITS_PER_PIXEL)
#define IP_PACKETSIZE				(0x2052)
#define CLK_RUN						(40)	/* MHz */
#define CLK_SYNC					(20)	/* MHz */


// Hardware definitions

#define NCHAN 36
#define NCHIP 1
#define NADC 9 //

/* #ifdef CTB */
/* #define NDAC 24 */
/* #define NPWR 5 */
/* #else */
/* #define NDAC 16 */
/* #define NPWR 0 */
/* #endif */
#define DAC_CMD_OFF 20

#define NMAXMODX  1
#define NMAXMODY 1
#define NMAXMOD (NMAXMODX*NMAXMODY)

#define NCHANS (NCHAN*NCHIP*NMAXMOD)
#define NDACS (NDAC*NMAXMOD)

#endif /* SLSDETECTORSERVER_DEFS_H_ */
