#pragma once
#include "sls_detector_defs.h"


#define CTRL_SRVR_INIT_TIME_US      (300 * 1000)

/* Hardware Definitions */
#define NCHAN 						(128)
#define NCHIP 						(10)
#define NDAC 						(16)
#define HV_SOFT_MAX_VOLTAGE         (200)
#define HV_HARD_MAX_VOLTAGE         (530)
#define HV_DRIVER_FILE_NAME         ("/etc/devlinks/hvdac")

/** Default Parameters */
#define DEFAULT_NUM_FRAMES			(1)
#define DEFAULT_NUM_CYCLES			(1)
#define DEFAULT_EXPTIME				(100*1000*1000)		//ns
#define DEFAULT_PERIOD              (2*1000*1000)	    //ns 
#define DEFAULT_DELAY_AFTER_TRIGGER (0)
#define DEFAULT_HIGH_VOLTAGE        (0)
#define DEFAULT_RUN_CLK             (125) 
#define DEFAULT_TICK_CLK            (20) // will be fixed later. Not configurable
#define DEFAULT_SAMPLING_CLK        (80)

/* Enums */
enum CLKINDEX   {RUN_CLK, TICK_CLK, SAMPLING_CLK, NUM_CLOCKS};
enum DACINDEX   {VIBIASSH, VTRIM, VIPRE, VRFSHNPOL, VTH1, VIPREOUT, VRF, VTH2, CAS, CASSH, VPL, VDCSH, VICIN, VICINSH, VICBIAS, VPH, VTH3, VRFSH};
#define DEFAULT_DAC_VALS    {1200,  /* vIbiasSh */  \
                            2300,   /* vTrim */     \
                            2150,   /* vIpre */     \
                            2300,   /* VrfShNpol */ \
                            900,    /* Vth1 */      \
                            1000,   /* vIpreOut */  \
                            900,    /* Vrf */       \
                            900,    /* Vth2 */      \
                            1474,   /* cas */       \
                            983,    /* casSh */     \
                            900,    /* VPL */       \
                            655,    /* VdcSh */     \
                            1400,   /* vIcin */     \
                            1400,   /* vIcinSh */   \
                            1200,   /* vIcbias */   \
                            960,    /* VPH */       \
                            900,    /* Vth3 */      \
                            1100    /* VrfSh */     \
                            };


/* Defines in the Firmware */
#define MAX_PATTERN_LENGTH  		(0x8192) // maximum number of words (64bit)
