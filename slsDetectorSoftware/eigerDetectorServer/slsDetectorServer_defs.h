/*
 * slsDetectorServer_defs.h
 *
 *  Created on: Jan 24, 2013
 *      Author: l_maliakal_d
 */

#ifndef SLSDETECTORSERVER_DEFS_H_
#define SLSDETECTORSERVER_DEFS_H_

//#include "sls_detector_defs.h"
#include <stdint.h>

#define GOODBYE 		-200

#define FEB_PORT		43210
#define BEB_PORT		43212



#define FIRMWAREREV		0xcaba	   //temporary should be in firmware


#define NCHAN 			256*256
#define NCHIP 			4
#define NDAC 			16
#define NADC			0
#define NMAXMODX  		1
#define NMAXMODY 		1
#define NMAXMOD 		NMAXMODX*NMAXMODY
#define NCHANS 			NCHAN*NCHIP*NMAXMOD
#define NDACS 			NDAC*NMAXMOD


#define DYNAMIC_RANGE	16


enum detDacIndex{SVP,VTR,VRF,VRS,SVN,VTGSTV,VCMP_LL,VCMP_LR,CAL,VCMP_RL,RXB_RB,RXB_LB,VCMP_RR,VCP,VCN,VIS,VTHRESHOLD};

#endif /* SLSDETECTORSERVER_DEFS_H_ */
