#pragma once
#include "sls_detector_defs.h"
#include "RegisterDefs.h"


#define CTRL_SRVR_INIT_TIME_US      (300 * 1000)

/* Hardware Definitions */
#define NCHAN 						(128)
#define NCHIP 						(10)
#define NDAC 						(16)
#define TEMP_CLK					(20)	/* MHz */
#define HIGHVOLTAGE_HARD_MAX        (500)   // given by hardware DAC6571

/** Default Parameters */
#define DEFAULT_NUM_FRAMES			(1)
#define DEFAULT_NUM_CYCLES			(1)
#define DEFAULT_EXPTIME				(100*1000*1000)		//ns
#define DEFAULT_PERIOD              (2*1000*1000)	    //ns 
#define DEFAULT_HIGH_VOLTAGE        (0)

#define HIGHVOLTAGE_SOFT_MAX        (200)   // in Software for Mythen3