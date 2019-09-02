// stuff from Carlos
#pragma once

/* Definitions for FPGA*/
#define MEM_MAP_SHIFT 1

<<<<<<< Updated upstream



=======
/* FPGA Version register */
#define FPGA_VERSION_REG      			(0x00 << MEM_MAP_SHIFT)

#define FPGA_COMPILATION_DATE_OFST		(0)
#define FPGA_COMPILATION_DATE_MSK		(0x00FFFFFF << FPGA_COMPILATION_DATE_OFST)
#define DETECTOR_TYPE_OFST   			(24)
#define DETECTOR_TYPE_MSK   			(0x000000FF << DETECTOR_TYPE_OFST)


#ifdef VIRTUAL // until firmware is ready ----------------------------------
>>>>>>> Stashed changes
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
