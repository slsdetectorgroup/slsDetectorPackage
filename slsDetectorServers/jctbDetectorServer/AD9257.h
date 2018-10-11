#ifndef AD9257_H
#define AD9257_H

#include "ansi.h"

#include "commonServerFunctions.h"
#include <stdio.h>

/* AD9257 ADC DEFINES */
#define AD9257_ADC_NUMBITS			(24)

#define AD9257_DEV_IND_2_REG		(0x04)
#define AD9257_CHAN_H_OFST			(0)
#define AD9257_CHAN_H_MSK			(0x00000001 << AD9257_CHAN_H_OFST)
#define AD9257_CHAN_G_OFST			(1)
#define AD9257_CHAN_G_MSK			(0x00000001 << AD9257_CHAN_G_OFST)
#define AD9257_CHAN_F_OFST			(2)
#define AD9257_CHAN_F_MSK			(0x00000001 << AD9257_CHAN_F_OFST)
#define AD9257_CHAN_E_OFST			(3)
#define AD9257_CHAN_E_MSK			(0x00000001 << AD9257_CHAN_E_OFST)

#define AD9257_DEV_IND_1_REG		(0x05)
#define AD9257_CHAN_D_OFST			(0)
#define AD9257_CHAN_D_MSK			(0x00000001 << AD9257_CHAN_D_OFST)
#define AD9257_CHAN_C_OFST			(1)
#define AD9257_CHAN_C_MSK			(0x00000001 << AD9257_CHAN_C_OFST)
#define AD9257_CHAN_B_OFST			(2)
#define AD9257_CHAN_B_MSK			(0x00000001 << AD9257_CHAN_B_OFST)
#define AD9257_CHAN_A_OFST			(3)
#define AD9257_CHAN_A_MSK			(0x00000001 << AD9257_CHAN_A_OFST)
#define AD9257_CLK_CH_DCO_OFST		(4)
#define AD9257_CLK_CH_DCO_MSK		(0x00000001 << AD9257_CLK_CH_DCO_OFST)
#define AD9257_CLK_CH_IFCO_OFST		(5)
#define AD9257_CLK_CH_IFCO_MSK		(0x00000001 << AD9257_CLK_CH_IFCO_OFST)

#define AD9257_POWER_MODE_REG		(0x08)
#define AD9257_POWER_INTERNAL_OFST	(0)
#define AD9257_POWER_INTERNAL_MSK	(0x00000003 << AD9257_POWER_INTERNAL_OFST)
#define AD9257_INT_RESET_VAL		(0x3)
#define AD9257_INT_CHIP_RUN_VAL		(0x0)
#define AD9257_POWER_EXTERNAL_OFST	(5)
#define AD9257_POWER_EXTERNAL_MSK	(0x00000001 << AD9257_POWER_EXTERNAL_OFST)
#define AD9257_EXT_FULL_POWER_VAL	(0x0)
#define AD9257_EXT_STANDBY_VAL		(0x1)

#define AD9257_OUT_MODE_REG			(0x14)
#define AD9257_OUT_FORMAT_OFST		(0)
#define AD9257_OUT_FORMAT_MSK		(0x00000001 << AD9257_OUT_FORMAT_OFST)
#define AD9257_OUT_BINARY_OFST_VAL	(0)
#define AD9257_OUT_TWOS_COMPL_VAL	(1)
#define AD9257_OUT_LVDS_OPT_OFST	(6)
#define AD9257_OUT_LVDS_OPT_MSK		(0x00000001 << AD9257_OUT_LVDS_OPT_OFST)
#define AD9257_OUT_LVDS_ANSI_VAL	(0)
#define AD9257_OUT_LVDS_IEEE_VAL	(1)

#define AD9257_OUT_PHASE_REG		(0x16)
#define AD9257_OUT_CLK_OFST			(0)
#define AD9257_OUT_CLK_MSK			(0x0000000F << AD9257_OUT_CLK_OFST)
#define AD9257_OUT_CLK_60_VAL		(0x1)
#define AD9257_IN_CLK_OFST			(4)
#define AD9257_IN_CLK_MSK			(0x00000007 << AD9257_IN_CLK_OFST)
#define AD9257_IN_CLK_0_VAL			(0x0)

#define AD9257_VREF_REG				(0x18)
#define AD9257_VREF_OFST			(0)
#define AD9257_VREF_MSK				(0x00000003 << AD9257_VREF_OFST)
#define AD9257_VREF_1_33_VAL		(0x2)

#define AD9257_TEST_MODE_REG		(0x0D)
#define AD9257_OUT_TEST_OFST		(0)
#define AD9257_OUT_TEST_MSK			(0x0000000F << AD9257_OUT_TEST_OFST)
#define AD9257_NONE_VAL				(0x0)
#define AD9257_MIXED_BIT_FREQ_VAL	(0xC)
#define AD9257_TEST_RESET_SHORT_GEN	(4)
#define AD9257_TEST_RESET_LONG_GEN	(5)
#define AD9257_USER_IN_MODE_OFST	(6)
#define AD9257_USER_IN_MODE_MSK		(0x00000003 << AD9257_USER_IN_MODE_OFST)


void setAdc(int addr, int val) {

	u_int32_t codata;
	codata = val + (addr << 8);
	printf(" Setting ADC SPI Register. Wrote 0x%04x at 0x%04x\n", val, addr);
	serializeToSPI(ADC_SPI_REG, codata, ADC_SERIAL_CS_OUT_MSK, AD9257_ADC_NUMBITS,
			ADC_SERIAL_CLK_OUT_MSK, ADC_SERIAL_DATA_OUT_MSK, ADC_SERIAL_DATA_OUT_OFST);
}

void prepareADC(){
	printf("\n\nPreparing ADC ... \n");

	//power mode reset
	printf("power mode reset:\n");
	setAdc(AD9257_POWER_MODE_REG,
			(AD9257_INT_RESET_VAL << AD9257_POWER_INTERNAL_OFST) & AD9257_POWER_INTERNAL_MSK);

	//power mode chip run
	 printf("power mode chip run:\n");
	setAdc(AD9257_POWER_MODE_REG,
			(AD9257_INT_CHIP_RUN_VAL << AD9257_POWER_INTERNAL_OFST) & AD9257_POWER_INTERNAL_MSK);

	//output clock phase
	 printf("output clock phase:\n");
	setAdc(AD9257_OUT_PHASE_REG,
			(AD9257_OUT_CLK_60_VAL << AD9257_OUT_CLK_OFST) & AD9257_OUT_CLK_MSK);

	// lvds-iee reduced , binary offset
	 printf("lvds-iee reduced, binary offset:\n");
	setAdc(AD9257_OUT_MODE_REG,
			(AD9257_OUT_LVDS_IEEE_VAL << AD9257_OUT_LVDS_OPT_OFST) & AD9257_OUT_LVDS_OPT_MSK);

	// all devices on chip to receive next command
	 printf("all devices on chip to receive next command:\n");
	setAdc(AD9257_DEV_IND_2_REG,
			AD9257_CHAN_H_MSK | AD9257_CHAN_G_MSK | AD9257_CHAN_F_MSK | AD9257_CHAN_E_MSK);
	setAdc(AD9257_DEV_IND_1_REG,
			AD9257_CHAN_D_MSK | AD9257_CHAN_C_MSK | AD9257_CHAN_B_MSK | AD9257_CHAN_A_MSK |
			AD9257_CLK_CH_DCO_MSK | AD9257_CLK_CH_IFCO_MSK);

	// vref 1.33
	 printf("vref 1.33:\n");
	setAdc(AD9257_VREF_REG,
			(AD9257_VREF_1_33_VAL << AD9257_VREF_OFST) & AD9257_VREF_MSK);

	// no test mode
	 printf("no test mode:\n");
	setAdc(AD9257_TEST_MODE_REG,
			(AD9257_NONE_VAL << AD9257_OUT_TEST_OFST) & AD9257_OUT_TEST_MSK);

#ifdef TESTADC
	printf("***************************************** *******\n");
	printf("******* PUTTING ADC IN TEST MODE!!!!!!!!! *******\n");
	printf("***************************************** *******\n");
	// mixed bit frequency test mode
	 printf("mixed bit frequency test mode:\n");
	setAdc(AD9257_TEST_MODE_REG,
			(AD9257_MIXED_BIT_FREQ_VAL << AD9257_OUT_TEST_OFST) & AD9257_OUT_TEST_MSK);
#endif
}

#endif	//AD9257_H
