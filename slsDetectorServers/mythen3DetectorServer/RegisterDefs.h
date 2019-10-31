#pragma once

/* Base addresses */
#define REG_OFFSET                      (4)
#define BASE_CONTROL                    (0x0000)
#define BASE_PATTERN_CONTROL            (0x00200)
#define BASE_UDP_RAM                    (0x01000) 
#define BASE_PATTERN_RAM                (0x10000)


/* Control registers --------------------------------------------------*/

/* Module Control Board Serial Number Register */
#define MCB_SERIAL_NO_REG               (0x00 * REG_OFFSET + BASE_CONTROL)

/* FPGA Version register */
#define FPGA_VERSION_REG                (0x01 * REG_OFFSET + BASE_CONTROL)

#define FPGA_COMPILATION_DATE_OFST		(0)
#define FPGA_COMPILATION_DATE_MSK		(0x00FFFFFF << FPGA_COMPILATION_DATE_OFST)
#define DETECTOR_TYPE_OFST   			(24)
#define DETECTOR_TYPE_MSK   			(0x000000FF << DETECTOR_TYPE_OFST)


/* API Version Register */
#define API_VERSION_REG                 (0x02 * REG_OFFSET + BASE_CONTROL)

#define API_VERSION_OFST                (0)
#define API_VERSION_MSK                 (0x00FFFFFF << API_VERSION_OFST)
#define API_VERSION_DETECTOR_TYPE_OFST  (24)                                            //Not used in software
#define API_VERSION_DETECTOR_TYPE_MSK   (0x000000FF << API_VERSION_DETECTOR_TYPE_OFST)  //Not used in software

/* Fix pattern register */
#define FIX_PATT_REG             	    (0x03 * REG_OFFSET + BASE_CONTROL)
#define FIX_PATT_VAL                    (0xACDC2019)

/* Status register */
#define STATUS_REG                      (0x04 * REG_OFFSET + BASE_CONTROL)

#ifdef VIRTUAL // until firmware is ready
#define RUN_BUSY_OFST					(0)
#define RUN_BUSY_MSK      				(0x00000001 << RUN_BUSY_OFST)
#endif

/* Look at me register, read only */
#define LOOK_AT_ME_REG          		(0x05 * REG_OFFSET + BASE_CONTROL)	//Not used in firmware or software, good to play with

#define SYSTEM_STATUS_REG               (0x06 * REG_OFFSET + BASE_CONTROL)  //Not used in software

/* Config RW regiseter */
#define CONFIG_REG                      (0x20 * REG_OFFSET + BASE_CONTROL)

/* Control RW register */ // assumed for MY3
#define CONTROL_REG           			(0x21 * REG_OFFSET + BASE_CONTROL)

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
// #define CONTROL_MMRY_RST_OFST		    (12)
// #define CONTROL_MMRY_RST_MSK		    (0x00000001 << CONTROL_MMRY_RST_OFST)
#define CONTROL_CLR_ACQSTN_FIFO_OFST    (14)
#define CONTROL_CLR_ACQSTN_FIFO_MSK		(0x00000001 << CONTROL_CLR_ACQSTN_FIFO_OFST)

/* Pattern IO Control 64 bit register */
#define PATTERN_IO_CTRL_LSB_REG         (0x22 * REG_OFFSET + BASE_CONTROL)   
#define PATTERN_IO_CTRL_MSB_REG         (0x23 * REG_OFFSET + BASE_CONTROL)   

#define DTA_OFFSET_REG                  (0x24 * REG_OFFSET + BASE_CONTROL)


/* Pattern Control registers --------------------------------------------------*/

/* Pattern status Register*/
#define PAT_STATUS_REG                  (0x00 * REG_OFFSET  + BASE_PATTERN_CONTROL)

/* Delay left 64bit Register */
#define GET_DELAY_LSB_REG               (0x02 * REG_OFFSET + BASE_PATTERN_CONTROL)
#define GET_DELAY_MSB_REG               (0x03 * REG_OFFSET + BASE_PATTERN_CONTROL)

/* Triggers left 64bit Register */
#define GET_CYCLES_LSB_REG              (0x04 * REG_OFFSET + BASE_PATTERN_CONTROL)
#define GET_CYCLES_MSB_REG              (0x05 * REG_OFFSET + BASE_PATTERN_CONTROL)

/* Frames left 64bit Register */
#define GET_FRAMES_LSB_REG              (0x06 * REG_OFFSET + BASE_PATTERN_CONTROL)
#define GET_FRAMES_MSB_REG              (0x07 * REG_OFFSET + BASE_PATTERN_CONTROL)

/* Period left 64bit Register */
#define GET_PERIOD_LSB_REG              (0x08 * REG_OFFSET + BASE_PATTERN_CONTROL)
#define GET_PERIOD_MSB_REG              (0x09 * REG_OFFSET + BASE_PATTERN_CONTROL)

/* Time from Start 64 bit register */
#define TIME_FROM_START_LSB_REG   		(0x0A * REG_OFFSET + BASE_PATTERN_CONTROL)
#define TIME_FROM_START_MSB_REG   		(0x0B * REG_OFFSET + BASE_PATTERN_CONTROL)

/* Get Frames from Start 64 bit register (frames from last reset using CONTROL_CRST) */
#define FRAMES_FROM_START_LSB_REG	    (0x0C * REG_OFFSET + BASE_PATTERN_CONTROL)
#define FRAMES_FROM_START_MSB_REG 	    (0x0D * REG_OFFSET + BASE_PATTERN_CONTROL)

/* Measurement Time 64 bit register (timestamp at a frame start until reset)*/
#define START_FRAME_TIME_LSB_REG		(0x0E * REG_OFFSET + BASE_PATTERN_CONTROL)
#define START_FRAME_TIME_MSB_REG 		(0x0F * REG_OFFSET + BASE_PATTERN_CONTROL)

/* Delay 64bit Write-register */
#define SET_DELAY_LSB_REG               (0x22 * REG_OFFSET + BASE_PATTERN_CONTROL)
#define SET_DELAY_MSB_REG               (0x23 * REG_OFFSET + BASE_PATTERN_CONTROL)

/* Cylces 64bit Write-register */
#define SET_CYCLES_LSB_REG              (0x24 * REG_OFFSET + BASE_PATTERN_CONTROL)
#define SET_CYCLES_MSB_REG              (0x25 * REG_OFFSET + BASE_PATTERN_CONTROL)

/* Frames 64bit Write-register */
#define SET_FRAMES_LSB_REG              (0x26 * REG_OFFSET + BASE_PATTERN_CONTROL)
#define SET_FRAMES_MSB_REG              (0x27 * REG_OFFSET + BASE_PATTERN_CONTROL)

/* Period 64bit Write-register */
#define SET_PERIOD_LSB_REG              (0x28 * REG_OFFSET + BASE_PATTERN_CONTROL)
#define SET_PERIOD_MSB_REG              (0x29 * REG_OFFSET + BASE_PATTERN_CONTROL)

/* External Signal register */
#define EXT_SIGNAL_REG        			(0x30 * REG_OFFSET + BASE_PATTERN_CONTROL)

/* Trigger Delay 64 bit register */
#define SET_TRIGGER_DELAY_LSB_REG       (0x32 * REG_OFFSET + BASE_PATTERN_CONTROL)
#define SET_TRIGGER_DELAY_MSB_REG       (0x33 * REG_OFFSET + BASE_PATTERN_CONTROL)

/* Pattern Limit RW Register */
#define PATTERN_LIMIT_REG               (0x40 * REG_OFFSET + BASE_PATTERN_CONTROL)

#define PATTERN_LIMIT_STRT_OFST       	(0)
#define PATTERN_LIMIT_STRT_MSK        	(0x00001FFF << PATTERN_LIMIT_STRT_OFST)
#define PATTERN_LIMIT_STP_OFST        	(16)
#define PATTERN_LIMIT_STP_MSK         	(0x00001FFF << PATTERN_LIMIT_STP_OFST)

/** Pattern Mask 64 bit RW regiser */
#define PATTERN_MASK_LSB_REG            (0x42 * REG_OFFSET + BASE_PATTERN_CONTROL)
#define PATTERN_MASK_MSB_REG            (0x43 * REG_OFFSET + BASE_PATTERN_CONTROL)

/** Pattern Set 64 bit RW regiser */
#define PATTERN_SET_LSB_REG             (0x44 * REG_OFFSET + BASE_PATTERN_CONTROL)
#define PATTERN_SET_MSB_REG             (0x45 * REG_OFFSET + BASE_PATTERN_CONTROL)

/* Pattern Wait Timer 0 64bit RW Register */
#define PATTERN_WAIT_TIMER_0_LSB_REG    (0x60 * REG_OFFSET + BASE_PATTERN_CONTROL)
#define PATTERN_WAIT_TIMER_0_MSB_REG    (0x61 * REG_OFFSET + BASE_PATTERN_CONTROL)

/* Pattern Wait 0 RW Register*/
#define PATTERN_WAIT_0_ADDR_REG         (0x62 * REG_OFFSET + BASE_PATTERN_CONTROL)

#define PATTERN_WAIT_0_ADDR_OFST        (0)
#define PATTERN_WAIT_0_ADDR_MSK         (0x00001FFF << PATTERN_WAIT_0_ADDR_OFST)

/* Pattern Loop 0 Iteration RW Register */
#define PATTERN_LOOP_0_ITERATION_REG    (0x63 * REG_OFFSET  + BASE_PATTERN_CONTROL)

/* Pattern Loop 0 Address RW Register */
#define PATTERN_LOOP_0_ADDR_REG         (0x64 * REG_OFFSET  + BASE_PATTERN_CONTROL)

#define PATTERN_LOOP_0_ADDR_STRT_OFST   (0)
#define PATTERN_LOOP_0_ADDR_STRT_MSK    (0x00001FFF << PATTERN_LOOP_0_ADDR_STRT_OFST)
#define PATTERN_LOOP_0_ADDR_STP_OFST    (16)
#define PATTERN_LOOP_0_ADDR_STP_MSK     (0x00001FFF << PATTERN_LOOP_0_ADDR_STP_OFST)

/* Pattern Wait Timer 1 64bit RW Register */
#define PATTERN_WAIT_TIMER_1_LSB_REG    (0x65 * REG_OFFSET + BASE_PATTERN_CONTROL)
#define PATTERN_WAIT_TIMER_1_MSB_REG    (0x66 * REG_OFFSET + BASE_PATTERN_CONTROL)

/* Pattern Wait 1 RW Register*/
#define PATTERN_WAIT_1_ADDR_REG         (0x67 * REG_OFFSET + BASE_PATTERN_CONTROL)

#define PATTERN_WAIT_1_ADDR_OFST        (0)
#define PATTERN_WAIT_1_ADDR_MSK         (0x00001FFF << PATTERN_WAIT_1_ADDR_OFST)

/* Pattern Loop 1 Iteration RW Register */
#define PATTERN_LOOP_1_ITERATION_REG    (0x68 * REG_OFFSET + BASE_PATTERN_CONTROL)

/* Pattern Loop 1 Address RW Register */
#define PATTERN_LOOP_1_ADDR_REG         (0x69 * REG_OFFSET + BASE_PATTERN_CONTROL)

#define PATTERN_LOOP_1_ADDR_STRT_OFST   (0)
#define PATTERN_LOOP_1_ADDR_STRT_MSK    (0x00001FFF << PATTERN_LOOP_1_ADDR_STRT_OFST)
#define PATTERN_LOOP_1_ADDR_STP_OFST    (16)
#define PATTERN_LOOP_1_ADDR_STP_MSK     (0x00001FFF << PATTERN_LOOP_1_ADDR_STP_OFST)

/* Pattern Wait Timer 2 64bit RW Register */
#define PATTERN_WAIT_TIMER_2_LSB_REG    (0x6A * REG_OFFSET + BASE_PATTERN_CONTROL)
#define PATTERN_WAIT_TIMER_2_MSB_REG    (0x6B * REG_OFFSET + BASE_PATTERN_CONTROL)

/* Pattern Wait 2 RW Register*/
#define PATTERN_WAIT_2_ADDR_REG         (0x6C * REG_OFFSET + BASE_PATTERN_CONTROL)

#define PATTERN_WAIT_2_ADDR_OFST        (0)
#define PATTERN_WAIT_2_ADDR_MSK         (0x00001FFF << PATTERN_WAIT_2_ADDR_OFST)

/* Pattern Loop 2 Iteration RW Register */
#define PATTERN_LOOP_2_ITERATION_REG    (0x6D * REG_OFFSET + BASE_PATTERN_CONTROL)

/* Pattern Loop 0 Address RW Register */
#define PATTERN_LOOP_2_ADDR_REG         (0x6E * REG_OFFSET + BASE_PATTERN_CONTROL)

#define PATTERN_LOOP_2_ADDR_STRT_OFST   (0)
#define PATTERN_LOOP_2_ADDR_STRT_MSK    (0x00001FFF << PATTERN_LOOP_2_ADDR_STRT_OFST)
#define PATTERN_LOOP_2_ADDR_STP_OFST    (16)
#define PATTERN_LOOP_2_ADDR_STP_MSK     (0x00001FFF << PATTERN_LOOP_2_ADDR_STP_OFST)


/* Pattern RAM registers --------------------------------------------------*/

/* Register of first word */
#define PATTERN_STEP0_LSB_REG           (0x0 * REG_OFFSET + BASE_PATTERN_RAM)
#define PATTERN_STEP0_MSB_REG           (0x1 * REG_OFFSET + BASE_PATTERN_RAM)
