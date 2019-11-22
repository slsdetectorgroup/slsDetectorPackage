#pragma once


#define REG_OFFSET                      (4)


/* Base addresses 0x1804 0000 ---------------------------------------------*/
/* Reconfiguration core for readout pll */
#define BASE_READOUT_PLL                (0x0000) // 0x1804_0000 - 0x1804_07FF
/* Reconfiguration core for system pll */
#define BASE_SYSTEM_PLL                 (0x0800) // 0x1804_0800 - 0x1804_0FFF
/* Clock Generation */
#define BASE_CLK_GENERATION             (0x1000) // 0x1804_1000 - 0x1804_XXXX //TODO

/* Base addresses 0x1806 0000 ---------------------------------------------*/
/* General purpose control and status registers */
#define BASE_CONTROL                    (0x0000)
/* Acquisition? TODO */
#define BASE_ACQUISITION                (0x0200) 
/* UDP datagram generator */
#define BASE_UDP_RAM                    (0x01000) // 0x1806_1000 - 0x1806_1FFF


/* Clock Generation registers ------------------------------------------------------*/
#define PLL_RESET_REG                   (0x00 * REG_OFFSET + BASE_CLK_GENERATION)

#define PLL_RESET_READOUT_OFST          (0)
#define PLL_RESET_READOUT_MSK           (0x00000001 << PLL_RESET_READOUT_OFST)
#define PLL_RESET_SYSTEM_OFST           (1)
#define PLL_RESET_SYSTEM_MSK            (0x00000001 << PLL_RESET_SYSTEM_OFST)


/* Control registers --------------------------------------------------*/

/* Module Control Board Serial Number register */
#define MCB_SERIAL_NO_REG               (0x00 * REG_OFFSET + BASE_CONTROL)

#define MCB_SERIAL_NO_VRSN_OFST			(16)
#define MCB_SERIAL_NO_VRSN_MSK      	(0x0000001F << MCB_SERIAL_NO_VRSN_OFST)

/* FPGA Version register */
#define FPGA_VERSION_REG                (0x01 * REG_OFFSET + BASE_CONTROL)

#define FPGA_COMPILATION_DATE_OFST		(0)
#define FPGA_COMPILATION_DATE_MSK		(0x00FFFFFF << FPGA_COMPILATION_DATE_OFST)
#define DETECTOR_TYPE_OFST   			(24)
#define DETECTOR_TYPE_MSK   			(0x000000FF << DETECTOR_TYPE_OFST)

/* API Version register */
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

#ifdef VIRTUAL
#define RUN_BUSY_OFST					(0)
#define RUN_BUSY_MSK      				(0x00000001 << RUN_BUSY_OFST)
#endif

/* Look at me read only register */
#define LOOK_AT_ME_REG          		(0x05 * REG_OFFSET + BASE_CONTROL)	

/* System status register */
#define SYSTEM_STATUS_REG          		(0x06 * REG_OFFSET + BASE_CONTROL)	

/* Config RW regiseter */
#define CONFIG_REG                      (0x20 * REG_OFFSET + BASE_CONTROL)

/* Control RW register */ 
#define CONTROL_REG           			(0x21 * REG_OFFSET + BASE_CONTROL)

#define CONTROL_STRT_ACQSTN_OFST       	(0)
#define CONTROL_STRT_ACQSTN_MSK			(0x00000001 << CONTROL_STRT_ACQSTN_OFST)
#define CONTROL_STP_ACQSTN_OFST			(1)
#define CONTROL_STP_ACQSTN_MSK			(0x00000001 << CONTROL_STP_ACQSTN_OFST)
#define CONTROL_CRE_RST_OFST			(10)
#define CONTROL_CRE_RST_MSK			    (0x00000001 << CONTROL_CRE_RST_OFST)
#define CONTROL_PRPHRL_RST_OFST		    (11)										// Only GBE10?
#define CONTROL_PRPHRL_RST_MSK		    (0x00000001 << CONTROL_PRPHRL_RST_OFST)
#define CONTROL_CLR_ACQSTN_FIFO_OFST    (15)
#define CONTROL_CLR_ACQSTN_FIFO_MSK		(0x00000001 << CONTROL_CLR_ACQSTN_FIFO_OFST)

/* Pattern IO Control 64 bit register */
#define PATTERN_IO_CTRL_LSB_REG         (0x22 * REG_OFFSET + BASE_CONTROL)   
#define PATTERN_IO_CTRL_MSB_REG         (0x23 * REG_OFFSET + BASE_CONTROL)   

/** DTA Offset Register */
#define DTA_OFFSET_REG                  (0x24 * REG_OFFSET + BASE_CONTROL)




/* Acquisition registers --------------------------------------------------*/
//TODO
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