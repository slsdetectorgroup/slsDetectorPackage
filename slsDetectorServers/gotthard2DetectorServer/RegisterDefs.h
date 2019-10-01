#pragma once

/* Definitions for FPGA*/
#define BASE_CONTROL                    (0x000)
#define BASE_ACQUISITION                (0x200)
#define BASE_UDP_RAM                    (0x1000)

/* Module Control Board Serial Number register */
#define MCB_SERIAL_NO_REG               (0x00 + BASE_CONTROL)

/* FPGA Version register */
#define FPGA_VERSION_REG                (0x04 + BASE_CONTROL)

#define FPGA_COMPILATION_DATE_OFST		(0)
#define FPGA_COMPILATION_DATE_MSK		(0x00FFFFFF << FPGA_COMPILATION_DATE_OFST)
#define DETECTOR_TYPE_OFST   			(24)
#define DETECTOR_TYPE_MSK   			(0x000000FF << DETECTOR_TYPE_OFST)

/* API Version register */
#define API_VERSION_REG                 (0x08 + BASE_CONTROL)

#define API_VERSION_OFST                (0)
#define API_VERSION_MSK                 (0x00FFFFFF << API_VERSION_OFST)
#define API_VERSION_DETECTOR_TYPE_OFST  (24)                                            //Not used in software
#define API_VERSION_DETECTOR_TYPE_MSK   (0x000000FF << API_VERSION_DETECTOR_TYPE_OFST)  //Not used in software

/* Fix pattern register */
#define FIX_PATT_REG             	    (0x0D + BASE_CONTROL)
#define FIX_PATT_VAL                    (0xACDC2019)

/* Status register */
#define STATUS_REG                      (0x12 + BASE_CONTROL)

#ifdef VIRTUAL
#define RUN_BUSY_OFST					(0)
#define RUN_BUSY_MSK      				(0x00000001 << RUN_BUSY_OFST)
#endif

/* Look at me register */
#define LOOK_AT_ME_REG          		(0x16 + BASE_CONTROL)	



/* Pattern Control FPGA registers TODO --------------------------------------------------*/

/* Cycles left 64bit Register */
#define GET_CYCLES_LSB_REG              (0x10 + BASE_ACQUISITION)
#define GET_CYCLES_MSB_REG              (0x14 + BASE_ACQUISITION)

/* Frames left 64bit Register */
#define GET_FRAMES_LSB_REG              (0x18 + BASE_ACQUISITION)
#define GET_FRAMES_MSB_REG              (0x1C + BASE_ACQUISITION)

/* Delay 64bit Write-register */
#define SET_DELAY_LSB_REG               (0x88 + BASE_ACQUISITION)
#define SET_DELAY_MSB_REG               (0x8C + BASE_ACQUISITION)

/* Cylces 64bit Write-register */
#define SET_CYCLES_LSB_REG              (0x90 + BASE_ACQUISITION)
#define SET_CYCLES_MSB_REG              (0x94 + BASE_ACQUISITION)

/* Frames 64bit Write-register */
#define SET_FRAMES_LSB_REG              (0x98 + BASE_ACQUISITION)
#define SET_FRAMES_MSB_REG              (0x9C + BASE_ACQUISITION)

/* Period 64bit Write-register */
#define SET_PERIOD_LSB_REG              (0xA0 + BASE_ACQUISITION)
#define SET_PERIOD_MSB_REG              (0xA4 + BASE_ACQUISITION)

/* Exptime 64bit Write-register */
#define SET_EXPTIME_LSB_REG             (0xA8 + BASE_ACQUISITION)
#define SET_EXPTIME_MSB_REG             (0xBC + BASE_ACQUISITION)