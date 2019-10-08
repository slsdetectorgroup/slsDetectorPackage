// stuff from Carlos
#pragma once

/* Definitions for FPGA*/
#define REG_OFFSET                      (4)
#define BASE_CONTROL                    (0x0)
#define BASE_PATTERN_CONTROL            (0x200)
#define BASE_PATTERN_RAM                (0x10000)

#define BASE_UDP_RAM                    (0x1000) // fix it 

/* Basic detector FPGA registers --------------------------------------------------*/
/* Module Control Board Serial Number Register */
#define MCB_SERIAL_NO_REG               (0x000 * REG_OFFSET + BASE_CONTROL)

/* FPGA Version register */
#define FPGA_VERSION_REG                (0x001 * REG_OFFSET + BASE_CONTROL)

#define FPGA_COMPILATION_DATE_OFST		(0)
#define FPGA_COMPILATION_DATE_MSK		(0x00FFFFFF << FPGA_COMPILATION_DATE_OFST)
#define DETECTOR_TYPE_OFST   			(24)
#define DETECTOR_TYPE_MSK   			(0x000000FF << DETECTOR_TYPE_OFST)


/* API Version Register */
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

#ifdef VIRTUAL // until firmware is ready ----------------------------------
#define RUN_BUSY_OFST					(0)
#define RUN_BUSY_MSK      				(0x00000001 << RUN_BUSY_OFST)
#endif

/* Look at me register, read only */
#define LOOK_AT_ME_REG          		(0x005 * REG_OFFSET + BASE_CONTROL)	//Not used in firmware or software, good to play with

/* Control RW register */ // assumed for MY3
#define CONTROL_REG           			(0x021 * REG_OFFSET + BASE_CONTROL)

#define CONTROL_STRT_ACQSTN_OFST       	(0)
#define CONTROL_STRT_ACQSTN_MSK			(0x00000001 << CONTROL_STRT_ACQSTN_OFST)
#define CONTROL_STP_ACQSTN_OFST			(1)
#define CONTROL_STP_ACQSTN_MSK			(0x00000001 << CONTROL_STP_ACQSTN_OFST)
#define CONTROL_RN_BSY_OFST             (2) // assumed for MY3
#define CONTROL_RN_BSY_MSK              (0x00000001 << CONTROL_RN_BSY_OFST)
#define CONTROL_STRT_EXPSR_OFST         (6)
#define CONTROL_STRT_EXPSR_MSK          (0x00000001 << CONTROL_STRT_EXPSR_OFST)
#define CONTROL_CRE_RST_OFST			(10)
#define CONTROL_CRE_RST_MSK			    (0x00000001 << CONTROL_CRE_RST_OFST)
#define CONTROL_PRPHRL_RST_OFST		    (11)										// Only GBE10?
#define CONTROL_PRPHRL_RST_MSK		    (0x00000001 << CONTROL_PRPHRL_RST_OFST)
#define CONTROL_MMRY_RST_OFST		    (12)
#define CONTROL_MMRY_RST_MSK		    (0x00000001 << CONTROL_MMRY_RST_OFST)
#define CONTROL_CLR_ACQSTN_FIFO_OFST    (15)
#define CONTROL_CLR_ACQSTN_FIFO_MSK		(0x00000001 << CONTROL_CLR_ACQSTN_FIFO_OFST)


#define DTA_OFFSET_REG                  (0x104 * REG_OFFSET + BASE_CONTROL)

/* Pattern Control FPGA registers --------------------------------------------------*/
/* Pattern status Register*/
#define PAT_STATUS_REG                  (0x000 * REG_OFFSET  + BASE_PATTERN_CONTROL)

/* Delay left 64bit Register */
#define GET_DELAY_LSB_REG               (0x0002 * REG_OFFSET + BASE_PATTERN_CONTROL)
#define GET_DELAY_MSB_REG               (0x0003 * REG_OFFSET + BASE_PATTERN_CONTROL)

/* Cycles left 64bit Register */
#define GET_CYCLES_LSB_REG              (0x0004 * REG_OFFSET + BASE_PATTERN_CONTROL)
#define GET_CYCLES_MSB_REG              (0x0005 * REG_OFFSET + BASE_PATTERN_CONTROL)

/* Frames left 64bit Register */
#define GET_FRAMES_LSB_REG              (0x0006 * REG_OFFSET + BASE_PATTERN_CONTROL)
#define GET_FRAMES_MSB_REG              (0x0007 * REG_OFFSET + BASE_PATTERN_CONTROL)

/* Period left 64bit Register */
#define GET_PERIOD_LSB_REG              (0x0008 * REG_OFFSET + BASE_PATTERN_CONTROL)
#define GET_PERIOD_MSB_REG              (0x0009 * REG_OFFSET + BASE_PATTERN_CONTROL)


/* Delay 64bit Write-register */
#define SET_DELAY_LSB_REG               (0x0102 * REG_OFFSET + BASE_PATTERN_CONTROL)
#define SET_DELAY_MSB_REG               (0x0103 * REG_OFFSET + BASE_PATTERN_CONTROL)

/* Cylces 64bit Write-register */
#define SET_CYCLES_LSB_REG              (0x0104 * REG_OFFSET + BASE_PATTERN_CONTROL)
#define SET_CYCLES_MSB_REG              (0x0105 * REG_OFFSET + BASE_PATTERN_CONTROL)

/* Frames 64bit Write-register */
#define SET_FRAMES_LSB_REG              (0x0106 * REG_OFFSET + BASE_PATTERN_CONTROL)
#define SET_FRAMES_MSB_REG              (0x0107 * REG_OFFSET + BASE_PATTERN_CONTROL)

/* Period 64bit Write-register */
#define SET_PERIOD_LSB_REG              (0x0108 * REG_OFFSET + BASE_PATTERN_CONTROL)
#define SET_PERIOD_MSB_REG              (0x0109 * REG_OFFSET + BASE_PATTERN_CONTROL)


/* Pattern Control FPGA registers --------------------------------------------------*/

// /* Pattern IO Control 64 bit RW Register
//  * Each bit configured as output(1)/ input(0) */
// #define PATTERN_IO_CNTRL_LSB_REG        (0x88 + BASE_CONTROL)
// #define PATTERN_IO_CNTRL_MSB_REG        (0x8C + BASE_CONTROL)

/* Pattern Limit RW Register */
#define PATTERN_LIMIT_REG               (0x1000 * REG_OFFSET + BASE_PATTERN_CONTROL)

#define PATTERN_LIMIT_STRT_OFST       	(0)
#define PATTERN_LIMIT_STRT_MSK        	(0x00001FFF << PATTERN_LIMIT_STRT_OFST)
#define PATTERN_LIMIT_STP_OFST        	(16)
#define PATTERN_LIMIT_STP_MSK         	(0x00001FFF << PATTERN_LIMIT_STP_OFST)

/* Pattern Wait Timer 0 64bit RW Register */
#define PATTERN_WAIT_TIMER_0_LSB_REG    (0x1100 * REG_OFFSET + BASE_PATTERN_CONTROL)
#define PATTERN_WAIT_TIMER_0_MSB_REG    (0x1101 * REG_OFFSET + BASE_PATTERN_CONTROL)

/* Pattern Wait 0 RW Register*/
#define PATTERN_WAIT_0_ADDR_REG         (0x1102 * REG_OFFSET + BASE_PATTERN_CONTROL)

#define PATTERN_WAIT_0_ADDR_OFST        (0)
#define PATTERN_WAIT_0_ADDR_MSK         (0x00001FFF << PATTERN_WAIT_0_ADDR_OFST)

/* Pattern Loop 0 Iteration RW Register */
#define PATTERN_LOOP_0_ITERATION_REG    (0x1103 * REG_OFFSET  + BASE_PATTERN_CONTROL) // patnloop

/* Pattern Loop 0 Address RW Register */
#define PATTERN_LOOP_0_ADDR_REG         (0x1104 * REG_OFFSET  + BASE_PATTERN_CONTROL)

#define PATTERN_LOOP_0_ADDR_STRT_OFST   (0)
#define PATTERN_LOOP_0_ADDR_STRT_MSK    (0x00001FFF << PATTERN_LOOP_0_ADDR_STRT_OFST)
#define PATTERN_LOOP_0_ADDR_STP_OFST    (16)
#define PATTERN_LOOP_0_ADDR_STP_MSK     (0x00001FFF << PATTERN_LOOP_0_ADDR_STP_OFST)


/* Pattern Wait Timer 1 64bit RW Register */
#define PATTERN_WAIT_TIMER_1_LSB_REG    (0x1105 * REG_OFFSET + BASE_PATTERN_CONTROL)
#define PATTERN_WAIT_TIMER_1_MSB_REG    (0x1106 * REG_OFFSET + BASE_PATTERN_CONTROL)

/* Pattern Wait 1 RW Register*/
#define PATTERN_WAIT_1_ADDR_REG         (0x1107 * REG_OFFSET + BASE_PATTERN_CONTROL)

#define PATTERN_WAIT_1_ADDR_OFST        (0)
#define PATTERN_WAIT_1_ADDR_MSK         (0x00001FFF << PATTERN_WAIT_1_ADDR_OFST)

/* Pattern Loop 1 Iteration RW Register */
#define PATTERN_LOOP_1_ITERATION_REG    (0x1108 * REG_OFFSET + BASE_PATTERN_CONTROL) // patnloop

/* Pattern Loop 1 Address RW Register */
#define PATTERN_LOOP_1_ADDR_REG         (0x1109 * REG_OFFSET + BASE_PATTERN_CONTROL)

#define PATTERN_LOOP_1_ADDR_STRT_OFST   (0)
#define PATTERN_LOOP_1_ADDR_STRT_MSK    (0x00001FFF << PATTERN_LOOP_1_ADDR_STRT_OFST)
#define PATTERN_LOOP_1_ADDR_STP_OFST    (16)
#define PATTERN_LOOP_1_ADDR_STP_MSK     (0x00001FFF << PATTERN_LOOP_1_ADDR_STP_OFST)


/* Pattern Wait Timer 2 64bit RW Register */
#define PATTERN_WAIT_TIMER_2_LSB_REG    (0x110A * REG_OFFSET + BASE_PATTERN_CONTROL)
#define PATTERN_WAIT_TIMER_2_MSB_REG    (0x110B * REG_OFFSET + BASE_PATTERN_CONTROL)

/* Pattern Wait 2 RW Register*/
#define PATTERN_WAIT_2_ADDR_REG         (0x110C * REG_OFFSET + BASE_PATTERN_CONTROL)

#define PATTERN_WAIT_2_ADDR_OFST        (0)
#define PATTERN_WAIT_2_ADDR_MSK         (0x00001FFF << PATTERN_WAIT_2_ADDR_OFST)

/* Pattern Loop 2 Iteration RW Register */
#define PATTERN_LOOP_2_ITERATION_REG    (0x110D * REG_OFFSET + BASE_PATTERN_CONTROL) // patnloop

/* Pattern Loop 0 Address RW Register */
#define PATTERN_LOOP_2_ADDR_REG         (0x110E * REG_OFFSET + BASE_PATTERN_CONTROL)

#define PATTERN_LOOP_2_ADDR_STRT_OFST   (0)
#define PATTERN_LOOP_2_ADDR_STRT_MSK    (0x00001FFF << PATTERN_LOOP_2_ADDR_STRT_OFST)
#define PATTERN_LOOP_2_ADDR_STP_OFST    (16)
#define PATTERN_LOOP_2_ADDR_STP_MSK     (0x00001FFF << PATTERN_LOOP_2_ADDR_STP_OFST)

/* Register of first word */
#define PATTERN_STEP0_LSB_REG           (0x0 * REG_OFFSET + BASE_PATTERN_RAM)
#define PATTERN_STEP0_MSB_REG           (0x1 * REG_OFFSET + BASE_PATTERN_RAM)
