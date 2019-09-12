// stuff from Carlos
#pragma once

/* Definitions for FPGA*/
#define MEM_MAP_SHIFT 0


#define BASE_CONTROL                    (0x0)
#define BASE_PATTERN_CONTROL            (0x200 )
#define BASE_PATTERN_RAM                (0x10000 )

/* Basic detector FPGA registers --------------------------------------------------*/
/* FPGA Version register */
#define FPGA_VERSION_REG                (0x04 + BASE_CONTROL)

#define FPGA_COMPILATION_DATE_OFST		(0)
#define FPGA_COMPILATION_DATE_MSK		(0x00FFFFFF << FPGA_COMPILATION_DATE_OFST)
#define DETECTOR_TYPE_OFST   			(24)
#define DETECTOR_TYPE_MSK   			(0x000000FF << DETECTOR_TYPE_OFST)


/* Module Control Board Serial Number Register */
#define MCB_SERIAL_NO_REG               (0x00 + BASE_CONTROL)

/* API Version Register */
#define API_VERSION_REG                 (0x08 + BASE_CONTROL)

/* Fix pattern register */
#define FIX_PATT_REG             	    (0x0C + BASE_CONTROL)
#define FIX_PATT_VAL                    (0xACDC2019)

/* Status register */
#define STATUS_REG                      (0x10 + BASE_CONTROL)

/* Look at me register */
#define LOOK_AT_ME_REG          		(0x14 + BASE_CONTROL)	//Not used in firmware or software, good to play with


/* Pattern Control FPGA registers --------------------------------------------------*/
/* Pattern status Register*/
#define PAT_STATUS_REG                  (0x00 + BASE_PATTERN_CONTROL)

/* Delay left 64bit Register */
#define GET_DELAY_LSB_REG               (0x08 + BASE_PATTERN_CONTROL)
#define GET_DELAY_MSB_REG               (0x0C + BASE_PATTERN_CONTROL)

/* Cycles left 64bit Register */
#define GET_CYCLES_LSB_REG              (0x10 + BASE_PATTERN_CONTROL)
#define GET_CYCLES_MSB_REG              (0x14 + BASE_PATTERN_CONTROL)

/* Frames left 64bit Register */
#define GET_FRAMES_LSB_REG              (0x18 + BASE_PATTERN_CONTROL)
#define GET_FRAMES_MSB_REG              (0x1C + BASE_PATTERN_CONTROL)


/* Delay 64bit Write-register */
#define SET_DELAY_LSB_REG               (0x88 + BASE_PATTERN_CONTROL)
#define SET_DELAY_MSB_REG               (0x8C + BASE_PATTERN_CONTROL)

/* Cylces 64bit Write-register */
#define SET_CYCLES_LSB_REG              (0x90 + BASE_PATTERN_CONTROL)
#define SET_CYCLES_MSB_REG              (0x94 + BASE_PATTERN_CONTROL)

/* Frames 64bit Write-register */
#define SET_FRAMES_LSB_REG              (0x98 + BASE_PATTERN_CONTROL)
#define SET_FRAMES_MSB_REG              (0x9C + BASE_PATTERN_CONTROL)



#ifdef VIRTUAL // until firmware is ready ----------------------------------
/* Status register */
#define STATUS_REG            			(0x01 << MEM_MAP_SHIFT)

#define RUN_BUSY_OFST					(0)
#define RUN_BUSY_MSK      				(0x00000001 << RUN_BUSY_OFST)

/* Set Cycles 64 bit register */
#define SET_CYCLES_LSB_REG    			(0x02 << MEM_MAP_SHIFT)
#define SET_CYCLES_MSB_REG    			(0x03 << MEM_MAP_SHIFT)

/* Set Frames 64 bit register */
#define SET_FRAMES_LSB_REG   			(0x04 << MEM_MAP_SHIFT)
#define SET_FRAMES_MSB_REG    			(0x05 << MEM_MAP_SHIFT)

/* Set Period 64 bit register tT = T x 50 ns */
#define SET_PERIOD_LSB_REG    			(0x06 << MEM_MAP_SHIFT)
#define SET_PERIOD_MSB_REG    			(0x07 << MEM_MAP_SHIFT)

/* Set Exptime 64 bit register eEXP = Exp x 25 ns */
#define SET_EXPTIME_LSB_REG    			(0x08 << MEM_MAP_SHIFT)
#define SET_EXPTIME_MSB_REG    			(0x09 << MEM_MAP_SHIFT)

/* Get Cycles 64 bit register */
#define GET_CYCLES_LSB_REG    			(0x0A << MEM_MAP_SHIFT)
#define GET_CYCLES_MSB_REG    			(0x0B << MEM_MAP_SHIFT)

/* Get Frames 64 bit register */
#define GET_FRAMES_LSB_REG   			(0x0C << MEM_MAP_SHIFT)
#define GET_FRAMES_MSB_REG    			(0x0D << MEM_MAP_SHIFT)
#endif