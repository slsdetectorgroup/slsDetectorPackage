#pragma once
#include "sls_detector_defs.h"
#include "RegisterDefs.h"


#define GOODBYE 					(-200)
#define MIN_REQRD_VRSN_T_RD_API     0x180314
#define REQRD_FRMWR_VRSN            0x180314

#define PROGRAMMING_MODE            (0x2)
#define CTRL_SRVR_INIT_TIME_US      (300 * 1000)

/* Struct Definitions */
typedef struct ip_header_struct {
	uint16_t     ip_len;
	uint8_t      ip_tos;
	uint8_t      ip_ihl:4 ,ip_ver:4;
	uint16_t     ip_offset:13,ip_flag:3;
	uint16_t     ip_ident;
	uint16_t     ip_chksum;
	uint8_t      ip_protocol;
	uint8_t      ip_ttl;
	uint32_t     ip_sourceip;
	uint32_t     ip_destip;
} ip_header;

/* Enums */
enum CLKINDEX               {RUN_CLK, ADC_CLK, SYNC_CLK, DBIT_CLK, NUM_CLOCKS};
enum ADCINDEX				{V_PWR_IO, V_PWR_A, V_PWR_B, V_PWR_C, V_PWR_D, I_PWR_IO, I_PWR_A, I_PWR_B, I_PWR_C, I_PWR_D};
enum DACINDEX               {D0, D1, D2, D3, D4, D5, D6, D7, D8, D9,
                             D10, D11, D12, D13, D14, D15, D16, D17,
                             D_PWR_D, D_PWR_CHIP, D_PWR_C, D_PWR_B, D_PWR_A, D_PWR_IO};

/* Hardware Definitions */
#define NCHAN 						(36)
#define NCHAN_ANALOG                (32)
#define NCHAN_DIGITAL               (4)
#define NCHIP 						(1)
#define NDAC 						(8)
#define DYNAMIC_RANGE               (16)
#define NUM_BYTES_PER_PIXEL         (DYNAMIC_RANGE / 8)
#define CLK_FREQ					(156.25)	/* MHz */

/** Default Parameters */
#define DEFAULT_DATA_BYTES          (NCHIP * NCHAN * NUM_BITS_PER_PIXEL)
#define DEFAULT_NUM_SAMPLES         (1)
#define DEFAULT_NUM_FRAMES			(100 * 1000 * 1000)
#define DEFAULT_NUM_CYCLES			(1)
#define DEFAULT_PERIOD				(1 * 1000 * 1000)	//ns
#define DEFAULT_DELAY				(0)
#define DEFAULT_HIGH_VOLTAGE		(0)
#define DEFAULT_VLIMIT              (-100)
#define DEFAULT_TIMING_MODE			(AUTO_TIMING)
#define DEFAULT_TX_UDP_PORT			(0x7e9a)

#define HIGHVOLTAGE_MIN             (60)
#define HIGHVOLTAGE_MAX             (200)
#define DAC_MIN_MV                  (0)
#define DAC_MAX_MV                  (2500)

/* Defines in the Firmware */
#define WAIT_TME_US_FR_LK_AT_ME_REG (100) // wait time in us after acquisition done to ensure there is no data in fifo
#define WAIT_TIME_US_PLL            (10 * 1000)
#define WAIT_TIME_US_STP_ACQ        (100)
#define WAIT_TIME_CONFIGURE_MAC     (500 * 1000)

/* MSB & LSB DEFINES */
#define MSB_OF_64_BIT_REG_OFST      (32)
#define LSB_OF_64_BIT_REG_OFST      (0)
#define BIT_32_MSK                  (0xFFFFFFFF)

#define IP_PACKETSIZE               (0x2032)
#define ADC_PORT_INVERT_VAL   		(0x453b2593) //FIXME: a default value?
#define MAXIMUM_ADC_CLK             (40)
#define PLL_VCO_FREQ_MHZ            (400)

/** PLL Reconfiguration Registers */
//https://www.altera.com/documentation/mcn1424769382940.html
#define PLL_MODE_REG 				    (0x00)

#define PLL_MODE_WT_RQUST_VAL           (0)
#define PLL_MODE_PLLNG_MD_VAL           (1)

#define PLL_STATUS_REG 				    (0x01)
#define PLL_START_REG 				    (0x02)
#define PLL_N_COUNTER_REG 			    (0x03)
#define PLL_M_COUNTER_REG 			    (0x04)
#define PLL_C_COUNTER_REG 			    (0x05)

#define PLL_C_COUNTER_LW_CNT_OFST       (0)
#define PLL_C_COUNTER_LW_CNT_MSK        (0x000000FF << PLL_C_COUNTER_LW_CNT_OFST)
#define PLL_C_COUNTER_HGH_CNT_OFST      (8)
#define PLL_C_COUNTER_HGH_CNT_MSK       (0x000000FF << PLL_C_COUNTER_HGH_CNT_OFST)
/* total_div = lw_cnt + hgh_cnt */
#define PLL_C_COUNTER_BYPSS_ENBL_OFST   (16)
#define PLL_C_COUNTER_BYPSS_ENBL_MSK    (0x00000001 << PLL_C_COUNTER_BYPSS_ENBL_OFST)
/* if bypss_enbl = 0, fout = f(vco)/total_div; else fout = f(vco) (c counter is bypassed) */
#define PLL_C_COUNTER_ODD_DVSN_OFST     (17)
#define PLL_C_COUNTER_ODD_DVSN_MSK      (0x00000001 << PLL_C_COUNTER_ODD_DVSN_OFST)
/** if odd_dvsn = 0 (even), duty cycle = hgh_cnt/ total_div; else duty cycle = (hgh_cnt - 0.5) / total_div */
#define PLL_C_COUNTER_SLCT_OFST         (18)
#define PLL_C_COUNTER_SLCT_MSK          (0x0000001F << PLL_C_COUNTER_SLCT_OFST)

#define PLL_PHASE_SHIFT_REG			    (0x06)

#define PLL_SHIFT_NUM_SHIFTS_OFST	    (0)
#define PLL_SHIFT_NUM_SHIFTS_MSK	    (0x0000FFFF << PLL_SHIFT_NUM_SHIFTS_OFST)

#define PLL_SHIFT_CNT_SELECT_OFST	    (16)
#define PLL_SHIFT_CNT_SELECT_MSK	    (0x0000001F << PLL_SHIFT_CNT_SELECT_OFST)
#define PLL_SHIFT_CNT_SLCT_C0_VAL	    ((0x0 << PLL_SHIFT_CNT_SELECT_OFST) & PLL_SHIFT_CNT_SELECT_MSK)
#define PLL_SHIFT_CNT_SLCT_C1_VAL	    ((0x1 << PLL_SHIFT_CNT_SELECT_OFST) & PLL_SHIFT_CNT_SELECT_MSK)
#define PLL_SHIFT_CNT_SLCT_C2_VAL	    ((0x2 << PLL_SHIFT_CNT_SELECT_OFST) & PLL_SHIFT_CNT_SELECT_MSK)
#define PLL_SHIFT_CNT_SLCT_C3_VAL	    ((0x3 << PLL_SHIFT_CNT_SELECT_OFST) & PLL_SHIFT_CNT_SELECT_MSK)
#define PLL_SHIFT_CNT_SLCT_C4_VAL	    ((0x4 << PLL_SHIFT_CNT_SELECT_OFST) & PLL_SHIFT_CNT_SELECT_MSK)
#define PLL_SHIFT_CNT_SLCT_C5_VAL	    ((0x5 << PLL_SHIFT_CNT_SELECT_OFST) & PLL_SHIFT_CNT_SELECT_MSK)
#define PLL_SHIFT_CNT_SLCT_C6_VAL	    ((0x6 << PLL_SHIFT_CNT_SELECT_OFST) & PLL_SHIFT_CNT_SELECT_MSK)
#define PLL_SHIFT_CNT_SLCT_C7_VAL	    ((0x7 << PLL_SHIFT_CNT_SELECT_OFST) & PLL_SHIFT_CNT_SELECT_MSK)
#define PLL_SHIFT_CNT_SLCT_C8_VAL	    ((0x8 << PLL_SHIFT_CNT_SELECT_OFST) & PLL_SHIFT_CNT_SELECT_MSK)
#define PLL_SHIFT_CNT_SLCT_C9_VAL	    ((0x9 << PLL_SHIFT_CNT_SELECT_OFST) & PLL_SHIFT_CNT_SELECT_MSK)
#define PLL_SHIFT_CNT_SLCT_C10_VAL	    ((0x10 << PLL_SHIFT_CNT_SELECT_OFST) & PLL_SHIFT_CNT_SELECT_MSK)
#define PLL_SHIFT_CNT_SLCT_C11_VAL	    ((0x11 << PLL_SHIFT_CNT_SELECT_OFST) & PLL_SHIFT_CNT_SELECT_MSK)
#define PLL_SHIFT_CNT_SLCT_C12_VAL	    ((0x12 << PLL_SHIFT_CNT_SELECT_OFST) & PLL_SHIFT_CNT_SELECT_MSK)
#define PLL_SHIFT_CNT_SLCT_C13_VAL	    ((0x13 << PLL_SHIFT_CNT_SELECT_OFST) & PLL_SHIFT_CNT_SELECT_MSK)
#define PLL_SHIFT_CNT_SLCT_C14_VAL	    ((0x14 << PLL_SHIFT_CNT_SELECT_OFST) & PLL_SHIFT_CNT_SELECT_MSK)
#define PLL_SHIFT_CNT_SLCT_C15_VAL	    ((0x15 << PLL_SHIFT_CNT_SELECT_OFST) & PLL_SHIFT_CNT_SELECT_MSK)
#define PLL_SHIFT_CNT_SLCT_C16_VAL	    ((0x16 << PLL_SHIFT_CNT_SELECT_OFST) & PLL_SHIFT_CNT_SELECT_MSK)
#define PLL_SHIFT_CNT_SLCT_C17_VAL	    ((0x17 << PLL_SHIFT_CNT_SELECT_OFST) & PLL_SHIFT_CNT_SELECT_MSK)

#define PLL_SHIFT_UP_DOWN_OFST		    (21)
#define PLL_SHIFT_UP_DOWN_MSK		    (0x00000001 << PLL_SHIFT_UP_DOWN_OFST)
#define PLL_SHIFT_UP_DOWN_NEG_VAL	    ((0x0 << PLL_SHIFT_UP_DOWN_OFST) & PLL_SHIFT_UP_DOWN_MSK)
#define PLL_SHIFT_UP_DOWN_POS_VAL	    ((0x1 << PLL_SHIFT_UP_DOWN_OFST) & PLL_SHIFT_UP_DOWN_MSK)

#define PLL_K_COUNTER_REG 			    (0x07)
#define PLL_BANDWIDTH_REG 			    (0x08)
#define PLL_CHARGEPUMP_REG 			    (0x09)
#define PLL_VCO_DIV_REG 			    (0x1c)
#define PLL_MIF_REG 				    (0x1f)

