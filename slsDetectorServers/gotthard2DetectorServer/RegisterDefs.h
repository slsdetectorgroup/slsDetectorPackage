#pragma once

/* Definitions for FPGA*/
#define REG_OFFSET                      (4)

/* cspbase 0x1804 0000 */
#define BASE_READOUT_PLL                (0x000) 
#define BASE_SYSTEM_PLL                 (0x800)

#define READOUT_PLL_RESET_REG           (0x1 * REG_OFFSET + BASE_READOUT_PLL) //TODO

#define READOUT_PLL_RESET_OFST          (0)
#define READOUT_PLL_RESET_MSK           (0x00000001 << READOUT_PLL_RESET_OFST)

#define SYSTEM_PLL_RESET_REG            (0x1 * REG_OFFSET + BASE_SYSTEM_PLL) //TODO

#define SYSTEM_PLL_RESET_OFST           (0)
#define SYSTEM_PLL_RESET_MSK            (0x00000001 << SYSTEM_PLL_RESET_OFST)

#define READOUT_PLL_WAIT_REG            (0x2 * REG_OFFSET + BASE_READOUT_PLL) //TODO

#define READOUT_PLL_WAIT_OFST           (0)
#define READOUT_PLL_WAIT_MSK            (0x00000001 << READOUT_PLL_WAIT_OFST)

#define SYSTEM_PLL_WAIT_REG             (0x2 * REG_OFFSET + BASE_SYSTEM_PLL) //TODO

#define SYSTEM_PLL_WAIT_OFST            (0)
#define SYSTEM_PLL_WAIT_MSK             (0x00000001 << SYSTEM_PLL_WAIT_OFST)


/* cspbase 0x1806 0000 */
#define BASE_CONTROL                    (0x000)
#define BASE_ACQUISITION                (0x200) //???TODO
#define BASE_UDP_RAM                    (0x1000)

/* Module Control Board Serial Number register */
#define MCB_SERIAL_NO_REG               (0x000 * REG_OFFSET + BASE_CONTROL)

/* FPGA Version register */
#define FPGA_VERSION_REG                (0x001 * REG_OFFSET + BASE_CONTROL)

#define FPGA_COMPILATION_DATE_OFST		(0)
#define FPGA_COMPILATION_DATE_MSK		(0x00FFFFFF << FPGA_COMPILATION_DATE_OFST)
#define DETECTOR_TYPE_OFST   			(24)
#define DETECTOR_TYPE_MSK   			(0x000000FF << DETECTOR_TYPE_OFST)

/* API Version register */
#define API_VERSION_REG                 (0x002 * REG_OFFSET + BASE_CONTROL)

#define API_VERSION_OFST                (0)
#define API_VERSION_MSK                 (0x00FFFFFF << API_VERSION_OFST)
#define API_VERSION_DETECTOR_TYPE_OFST  (24)                                            //Not used in software
#define API_VERSION_DETECTOR_TYPE_MSK   (0x000000FF << API_VERSION_DETECTOR_TYPE_OFST)  //Not used in software

/* Fix pattern register */
#define FIX_PATT_REG             	    (0x003 * REG_OFFSET + BASE_CONTROL)
#define FIX_PATT_VAL                    (0xACDC2019)

/* Status register */
#define STATUS_REG                      (0x004 * REG_OFFSET + BASE_CONTROL)

#ifdef VIRTUAL
#define RUN_BUSY_OFST					(0)
#define RUN_BUSY_MSK      				(0x00000001 << RUN_BUSY_OFST)
#endif

/* Look at me read only register */
#define LOOK_AT_ME_REG          		(0x005 * REG_OFFSET + BASE_CONTROL)	

/** DTA Offset Register */
#define DTA_OFFSET_REG                  (0x104 * REG_OFFSET + BASE_CONTROL)




/* BASE_ACQUISITION FPGA registers TODO --------------------------------------------------*/

/* Triggers left 64bit Register */
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