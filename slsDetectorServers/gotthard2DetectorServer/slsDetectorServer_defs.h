#pragma once
#include "sls_detector_defs.h"
#include "RegisterDefs.h"


#define CTRL_SRVR_INIT_TIME_US      (300 * 1000)


/* Enums */
enum DACINDEX				{DAC0};

/* Hardware Definitions */
#define NCHAN 						(128)
#define NCHIP 						(10)
#define NDAC 						(16)
#define TEMP_CLK					(20)	/* MHz */
#define HV_SOFT_MAX_VOLTAGE         (200)
#define HV_HARD_MAX_VOLTAGE         (530)
#define HV_DRIVER_FILE_NAME         ("/root/devlinks/hvdac")

/** Default Parameters */
#define DEFAULT_NUM_FRAMES			(1)
#define DEFAULT_NUM_CYCLES			(1)
#define DEFAULT_EXPTIME				(1 * 1000 * 1000)	        // 1 ms
#define DEFAULT_PERIOD				(1 * 1000 * 1000 * 1000)	// 1 s
#define DEFAULT_HIGH_VOLTAGE		(0)