/*
 * registers.h
 *
 *  Created on: Jan 24, 2013
 *      Author: l_maliakal_d
 */

#ifndef REGISTERS_H_
#define REGISTERS_H_

#include "sls_detector_defs.h"

#define CSP0 		0xC4100000
#define MEM_SIZE 	0xFFFFFFF


#define STATUS_REG			  0x0a000

#define SET_FRAMES_LSB_REG    0x10000
#define SET_FRAMES_MSB_REG    0x11000
#define GET_FRAMES_LSB_REG    0x12000
#define GET_FRAMES_MSB_REG    0x13000

#define SET_EXPTIME_LSB_REG   0x14000
#define SET_EXPTIME_MSB_REG   0x15000
#define GET_EXPTIME_LSB_REG   0x16000
#define GET_EXPTIME_MSB_REG   0x17000

#define SET_GATES_LSB_REG     0x18000
#define SET_GATES_MSB_REG     0x19000
#define GET_GATES_LSB_REG     0x1a000
#define GET_GATES_MSB_REG     0x1b000

#define SET_PERIOD_LSB_REG    0x1c000
#define SET_PERIOD_MSB_REG    0x1d000
#define GET_PERIOD_LSB_REG    0x1e000
#define GET_PERIOD_MSB_REG    0x1f000

#define SET_DELAY_LSB_REG     0x20000
#define SET_DELAY_MSB_REG     0x21000
#define GET_DELAY_LSB_REG     0x22000
#define GET_DELAY_MSB_REG     0x23000

#define SET_TRAINS_LSB_REG    0x24000
#define SET_TRAINS_MSB_REG    0x25000
#define GET_TRAINS_LSB_REG    0x26000
#define GET_TRAINS_MSB_REG    0x27000





#endif /* REGISTERS_H_ */
