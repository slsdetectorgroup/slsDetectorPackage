/*
 * mythen3Server_defs.h
 *
 *  Created on: Jan 24, 2013
 *      Author: l_maliakal_d, changed my Marie A.
 */

#ifndef SLSDETECTORSERVER_DEFS_H_
#define SLSDETECTORSERVER_DEFS_H_

#include "sls_detector_defs.h"
#include <stdint.h>

/** This is only an example file!!! */



#define GOODBYE 			(-200)
enum DACINDEX			    {vIpre, vIbias, Vrf, VrfSh, vIinSh, VdcSh, Vth2, VPL, Vth1, Vth3, Vtrim, casSh, cas, vIbiasSh, vIcin, VPH, NC, vIpreOut, V_D, V_CHIP, V_C, V_B, V_A, V_IO, V_LIM}; // Mythen 3.01
enum PWRINDEX               {PWR_IO, PWR_A, PWR_B, PWR_C, PWR_D, PWR_CHIP=-1, PWR_LIMIT=-1};
enum CLKINDEX               {RUN_CLK_C, ADC_CLK_C, SYNC_CLK_C, DBIT_CLK_C};

#define DEFAULT_DAC_VALS    {   2150,	/* vIpre 	*/ \
							    1200,	/* vIbias 	*/ \
							    900,	/* Vrf 		*/ \
							    1050,	/* VrfSh	*/ \
							    1400,	/* vIinSh	*/ \
							    655, 	/* VdcSh	*/ \
							    850, 	/* Vth2		*/ \
							    1400,	/* VPL		*/ \
							    850,	/* Vth1		*/ \
							    850,	/* Vth3		*/ \
							    2294,	/* Vtrim	*/ \
							    983,	/* casSh	*/ \
                                1474,	/* cas 		*/ \
							    1200, 	/* vIbiasSh	*/ \
							    1600,	/* vIcin	*/ \
							    1520,	/* VPH		*/ \
							    0,		/* NC 		*/ \
							    1000	/* vIpreOut	*/ \
							    0       /* V_D      */ \
							    0       /* V_CHIP   */ \
							    0       /* V_C      */ \
							    1335    /* V_B      */ \
							    1335    /* V_A      */ \
							    1350    /* V_IO     */ \
						    };

#define DEFAULT_DAC_NAMES  {    "vIpre",    \
						        "vIbias",   \
						        "Vrf",      \
						        "VrfSh",    \
						        "vIinSh",   \
						        "VdcSh",    \
						        "Vth2",     \
						        "VPL",      \
						        "Vth1",     \
						        "Vth3",     \
						        "Vtrim",    \
						        "casSh",    \
						        "cas",      \
						        "vIbiasSh", \
						        "vIcin",    \
                                "VPH",      \
                                "NC",       \
						        "vIpreOut"  \
						        "v_d"       \
						        "v_chip"    \
						        "v_c"       \
						        "v_b"       \
						        "v_a"       \
						        "v_io"      \
						    };

/*Hardware Definitions */
#define NMAXMOD             (1)
#define NMOD                (1)
#define NCHAN               (32)
#define NCHIP               (1)
#define NADC                (0)
#define NDAC                (24)
#define NDAC_PER_SET        (8)


#define NPWR                (5)
#define MAX_DACVOLTVAL      (2500) //mV
#define MAX_DACVAL          (4096) // dac val
#define MAX_VCHIPVAL        (2700) //mV /** name ???? */
#define MIN_VCHIP_OFSTVAL   (200) //mV /** name ???? */
#define MIN_VCHIP_VAL       (600) //mV /** name ???? */


/** Default Parameters */
#define DEFAULT_NUM_FRAMES          (1)
#define DEFAULT_NUM_CYCLES          (1)
#define DEFAULT_EXPTIME             (200*1000)       //ns
#define DEFAULT_PERIOD              (1*1000*1000)   //ns
#define DEFAULT_DELAY               (0)
#define DEFAULT_HIGH_VOLTAGE        (0)
#define DEFAULT_TIMING_MODE         (AUTO_TIMING)


/* Defines in the Firmware */
#define FIX_PATT_VAL        (0xACDC1980)

/* LTC2620 DAC DEFINES */
#define LTC2620_DAC_CMD_OFST        (20)
#define LTC2620_DAC_CMD_MSK         (0x0000000F << LTC2620_DAC_CMD_OFST)
#define LTC2620_DAC_ADDR_OFST       (16)
#define LTC2620_DAC_ADDR_MSK        (0x0000000F << LTC2620_DAC_ADDR_OFST)
#define LTC2620_DAC_DATA_OFST       (4)
#define LTC2620_DAC_DATA_MSK        (0x00000FFF << LTC2620_DAC_DATA_OFST)

#define LTC2620_DAC_CMD_WRITE       (0x00000000 << LTC2620_DAC_CMD_OFST)
#define LTC2620_DAC_CMD_SET         (0x00000003 << LTC2620_DAC_CMD_OFST)
#define LTC2620_DAC_CMD_POWER_DOWN  (0x00000004 << LTC2620_DAC_CMD_OFST)
#define LTC2620_DAC_NUMBITS         (24)

/** PLL Reconfiguration Registers */
//https://www.altera.com/documentation/mcn1424769382940.html
#define PLL_MODE_REG                (0x00)
#define PLL_STATUS_REG              (0x01)
#define PLL_START_REG               (0x02)
#define PLL_N_COUNTER_REG           (0x03)
#define PLL_M_COUNTER_REG           (0x04)
#define PLL_C_COUNTER_REG           (0x05)
#define PLL_PHASE_SHIFT_REG         (0x06)

#define PLL_SHIFT_NUM_SHIFTS_OFST   (0)
#define PLL_SHIFT_NUM_SHIFTS_MSK    (0x0000FFFF << PLL_SHIFT_NUM_SHIFTS_OFST)

#define PLL_SHIFT_CNT_SELECT_OFST   (16)
#define PLL_SHIFT_CNT_SELECT_MSK    (0x0000001F << PLL_SHIFT_CNT_SELECT_OFST)
#define PLL_SHIFT_CNT_SLCT_C0_VAL   ((0x0 << PLL_SHIFT_CNT_SELECT_OFST) & PLL_SHIFT_CNT_SELECT_MSK)
#define PLL_SHIFT_CNT_SLCT_C1_VAL   ((0x1 << PLL_SHIFT_CNT_SELECT_OFST) & PLL_SHIFT_CNT_SELECT_MSK)
#define PLL_SHIFT_CNT_SLCT_C2_VAL   ((0x2 << PLL_SHIFT_CNT_SELECT_OFST) & PLL_SHIFT_CNT_SELECT_MSK)
#define PLL_SHIFT_CNT_SLCT_C3_VAL   ((0x3 << PLL_SHIFT_CNT_SELECT_OFST) & PLL_SHIFT_CNT_SELECT_MSK)
#define PLL_SHIFT_CNT_SLCT_C4_VAL   ((0x4 << PLL_SHIFT_CNT_SELECT_OFST) & PLL_SHIFT_CNT_SELECT_MSK)
#define PLL_SHIFT_CNT_SLCT_C5_VAL   ((0x5 << PLL_SHIFT_CNT_SELECT_OFST) & PLL_SHIFT_CNT_SELECT_MSK)
#define PLL_SHIFT_CNT_SLCT_C6_VAL   ((0x6 << PLL_SHIFT_CNT_SELECT_OFST) & PLL_SHIFT_CNT_SELECT_MSK)
#define PLL_SHIFT_CNT_SLCT_C7_VAL   ((0x7 << PLL_SHIFT_CNT_SELECT_OFST) & PLL_SHIFT_CNT_SELECT_MSK)
#define PLL_SHIFT_CNT_SLCT_C8_VAL   ((0x8 << PLL_SHIFT_CNT_SELECT_OFST) & PLL_SHIFT_CNT_SELECT_MSK)
#define PLL_SHIFT_CNT_SLCT_C9_VAL   ((0x9 << PLL_SHIFT_CNT_SELECT_OFST) & PLL_SHIFT_CNT_SELECT_MSK)
#define PLL_SHIFT_CNT_SLCT_C10_VAL  ((0x10 << PLL_SHIFT_CNT_SELECT_OFST) & PLL_SHIFT_CNT_SELECT_MSK)
#define PLL_SHIFT_CNT_SLCT_C11_VAL  ((0x11 << PLL_SHIFT_CNT_SELECT_OFST) & PLL_SHIFT_CNT_SELECT_MSK)
#define PLL_SHIFT_CNT_SLCT_C12_VAL  ((0x12 << PLL_SHIFT_CNT_SELECT_OFST) & PLL_SHIFT_CNT_SELECT_MSK)
#define PLL_SHIFT_CNT_SLCT_C13_VAL  ((0x13 << PLL_SHIFT_CNT_SELECT_OFST) & PLL_SHIFT_CNT_SELECT_MSK)
#define PLL_SHIFT_CNT_SLCT_C14_VAL  ((0x14 << PLL_SHIFT_CNT_SELECT_OFST) & PLL_SHIFT_CNT_SELECT_MSK)
#define PLL_SHIFT_CNT_SLCT_C15_VAL  ((0x15 << PLL_SHIFT_CNT_SELECT_OFST) & PLL_SHIFT_CNT_SELECT_MSK)
#define PLL_SHIFT_CNT_SLCT_C16_VAL  ((0x16 << PLL_SHIFT_CNT_SELECT_OFST) & PLL_SHIFT_CNT_SELECT_MSK)
#define PLL_SHIFT_CNT_SLCT_C17_VAL  ((0x17 << PLL_SHIFT_CNT_SELECT_OFST) & PLL_SHIFT_CNT_SELECT_MSK)

#define PLL_SHIFT_UP_DOWN_OFST      (21)
#define PLL_SHIFT_UP_DOWN_MSK       (0x00000001 << PLL_SHIFT_UP_DOWN_OFST)
#define PLL_SHIFT_UP_DOWN_NEG_VAL   ((0x0 << PLL_SHIFT_UP_DOWN_OFST) & PLL_SHIFT_UP_DOWN_MSK)
#define PLL_SHIFT_UP_DOWN_POS_VAL   ((0x1 << PLL_SHIFT_UP_DOWN_OFST) & PLL_SHIFT_UP_DOWN_MSK)

#define PLL_K_COUNTER_REG           (0x07)
#define PLL_BANDWIDTH_REG           (0x08)
#define PLL_CHARGEPUMP_REG          (0x09)
#define PLL_VCO_DIV_REG             (0x1c)
#define PLL_MIF_REG                 (0x1f)

#define PLL_VCO_FREQ_MHZ 400


#endif /* SLSDETECTORSERVER_DEFS_H_ */
