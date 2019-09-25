// stuff from Carlos
#pragma once

/* Definitions for FPGA*/

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

/* Period left 64bit Register */
#define GET_PERIOD_LSB_REG              (0x20 + BASE_PATTERN_CONTROL)
#define GET_PERIOD_MSB_REG              (0x24 + BASE_PATTERN_CONTROL)


/* Delay 64bit Write-register */
#define SET_DELAY_LSB_REG               (0x88 + BASE_PATTERN_CONTROL)
#define SET_DELAY_MSB_REG               (0x8C + BASE_PATTERN_CONTROL)

/* Cylces 64bit Write-register */
#define SET_CYCLES_LSB_REG              (0x90 + BASE_PATTERN_CONTROL)
#define SET_CYCLES_MSB_REG              (0x94 + BASE_PATTERN_CONTROL)

/* Frames 64bit Write-register */
#define SET_FRAMES_LSB_REG              (0x98 + BASE_PATTERN_CONTROL)
#define SET_FRAMES_MSB_REG              (0x9C + BASE_PATTERN_CONTROL)

/* Period 64bit Write-register */
#define SET_PERIOD_LSB_REG              (0xA0 + BASE_PATTERN_CONTROL)
#define SET_PERIOD_MSB_REG              (0xA4 + BASE_PATTERN_CONTROL)


/* Pattern Control FPGA registers --------------------------------------------------*/

// /* Pattern IO Control 64 bit RW Register
//  * Each bit configured as output(1)/ input(0) */
// #define PATTERN_IO_CNTRL_LSB_REG        (0x88 + BASE_CONTROL)
// #define PATTERN_IO_CNTRL_MSB_REG        (0x8C + BASE_CONTROL)

/* Pattern Limit RW Register */
#define PATTERN_LIMIT_REG               (0x100 + BASE_PATTERN_CONTROL)

#define PATTERN_LIMIT_STRT_OFST       	(0)
#define PATTERN_LIMIT_STRT_MSK        	(0x00001FFF << PATTERN_LIMIT_STRT_OFST)
#define PATTERN_LIMIT_STP_OFST        	(16)
#define PATTERN_LIMIT_STP_MSK         	(0x00001FFF << PATTERN_LIMIT_STP_OFST)

/* Pattern Wait 0 RW Register*/
#define PATTERN_WAIT_0_ADDR_REG         (0x188 + BASE_PATTERN_CONTROL)

#define PATTERN_WAIT_0_ADDR_OFST        (0)
#define PATTERN_WAIT_0_ADDR_MSK         (0x00001FFF << PATTERN_WAIT_0_ADDR_OFST)

/* Pattern Wait 1 RW Register*/
#define PATTERN_WAIT_1_ADDR_REG         (0x19C + BASE_PATTERN_CONTROL)

#define PATTERN_WAIT_1_ADDR_OFST        (0)
#define PATTERN_WAIT_1_ADDR_MSK         (0x00001FFF << PATTERN_WAIT_1_ADDR_OFST)

/* Pattern Wait 2 RW Register*/
#define PATTERN_WAIT_2_ADDR_REG         (0x1B0 + BASE_PATTERN_CONTROL)

#define PATTERN_WAIT_2_ADDR_OFST        (0)
#define PATTERN_WAIT_2_ADDR_MSK         (0x00001FFF << PATTERN_WAIT_2_ADDR_OFST)

/* Pattern Wait Timer 0 64bit RW Register */
#define PATTERN_WAIT_TIMER_0_LSB_REG    (0x180 + BASE_PATTERN_CONTROL)
#define PATTERN_WAIT_TIMER_0_MSB_REG    (0x184 + BASE_PATTERN_CONTROL)

/* Pattern Wait Timer 1 64bit RW Register */
#define PATTERN_WAIT_TIMER_1_LSB_REG    (0x194 + BASE_PATTERN_CONTROL)
#define PATTERN_WAIT_TIMER_1_MSB_REG    (0x198 + BASE_PATTERN_CONTROL)

/* Pattern Wait Timer 2 64bit RW Register */
#define PATTERN_WAIT_TIMER_2_LSB_REG    (0x1A8 + BASE_PATTERN_CONTROL)
#define PATTERN_WAIT_TIMER_2_MSB_REG    (0x1AC + BASE_PATTERN_CONTROL)

/* Pattern Loop 0 Address RW Register */
#define PATTERN_LOOP_0_ADDR_REG         (0x190 + BASE_PATTERN_CONTROL)

#define PATTERN_LOOP_0_ADDR_STRT_OFST   (0)
#define PATTERN_LOOP_0_ADDR_STRT_MSK    (0x00001FFF << PATTERN_LOOP_0_ADDR_STRT_OFST)
#define PATTERN_LOOP_0_ADDR_STP_OFST    (16)
#define PATTERN_LOOP_0_ADDR_STP_MSK     (0x00001FFF << PATTERN_LOOP_0_ADDR_STP_OFST)

/* Pattern Loop 1 Address RW Register */
#define PATTERN_LOOP_1_ADDR_REG         (0x1A4 + BASE_PATTERN_CONTROL)

#define PATTERN_LOOP_1_ADDR_STRT_OFST   (0)
#define PATTERN_LOOP_1_ADDR_STRT_MSK    (0x00001FFF << PATTERN_LOOP_1_ADDR_STRT_OFST)
#define PATTERN_LOOP_1_ADDR_STP_OFST    (16)
#define PATTERN_LOOP_1_ADDR_STP_MSK     (0x00001FFF << PATTERN_LOOP_1_ADDR_STP_OFST)

/* Pattern Loop 0 Address RW Register */
#define PATTERN_LOOP_2_ADDR_REG         (0x1B8 + BASE_PATTERN_CONTROL)

#define PATTERN_LOOP_2_ADDR_STRT_OFST   (0)
#define PATTERN_LOOP_2_ADDR_STRT_MSK    (0x00001FFF << PATTERN_LOOP_2_ADDR_STRT_OFST)
#define PATTERN_LOOP_2_ADDR_STP_OFST    (16)
#define PATTERN_LOOP_2_ADDR_STP_MSK     (0x00001FFF << PATTERN_LOOP_2_ADDR_STP_OFST)

/* Pattern Loop 0 Iteration RW Register */
#define PATTERN_LOOP_0_ITERATION_REG    (0x18C + BASE_PATTERN_CONTROL) // patnloop

/* Pattern Loop 1 Iteration RW Register */
#define PATTERN_LOOP_1_ITERATION_REG    (0x1A0 + BASE_PATTERN_CONTROL) // patnloop

/* Pattern Loop 2 Iteration RW Register */
#define PATTERN_LOOP_2_ITERATION_REG    (0x1B4 + BASE_PATTERN_CONTROL) // patnloop

/* Register of first word */
#define PATTERN_STEP0_LSB_REG           (0x0 + BASE_PATTERN_RAM)
#define PATTERN_STEP0_MSB_REG           (0x4 + BASE_PATTERN_RAM)


#ifdef VIRTUAL // until firmware is ready ----------------------------------

#define RUN_BUSY_OFST					(0)
#define RUN_BUSY_MSK      				(0x00000001 << RUN_BUSY_OFST)

/* Set Exptime 64 bit register eEXP = Exp x 25 ns */
#define SET_EXPTIME_LSB_REG    			(0x08)
#define SET_EXPTIME_MSB_REG    			(0x09)

#endif