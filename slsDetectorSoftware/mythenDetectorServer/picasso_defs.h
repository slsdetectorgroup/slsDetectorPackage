#ifndef SERVER_DEFS_H
#define SERVER_DEFS_H

#include "sls_detector_defs.h"

#include <stdint.h> 


// Hardware definitions

#define NCHAN 128
#define NCHIP 12 //10 modified for PICASSO
#define NMAXMODX  24
#define NMAXMODY 1
#define NMAXMOD NMAXMODX*NMAXMODY
#define NDAC 6
#define NADC 0

#define NCHANS NCHAN*NCHIP*NMAXMOD
#define NDACS NDAC*NMAXMOD

#define NTRIMBITS 6
#define NCOUNTBITS 24

//#define TRIM_DR ((2**NTRIMBITS)-1)
//#define COUNT_DR ((2**NCOUNTBITS)-1) 
#define TRIM_DR (((int)pow(2,NTRIMBITS))-1)
#define COUNT_DR (((int)pow(2,NCOUNTBITS))-1)


#define ALLMOD 0xffff
#define ALLFIFO 0xffff

#ifdef VIRTUAL
#define DEBUGOUT
#endif

#define CLK_FREQ 100E+6


#define THIS_SOFTWARE_VERSION 0x20090205
#endif
