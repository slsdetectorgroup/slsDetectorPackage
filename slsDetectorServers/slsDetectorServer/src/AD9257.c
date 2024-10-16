// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "AD9257.h"
#include "blackfin.h"
#include "clogger.h"
#include "commonServerFunctions.h" // blackfin.h, ansi.h
#include "sls/sls_detector_defs.h"

/* AD9257 ADC DEFINES */
#define AD9257_ADC_NUMBITS (24)

// default value is 0xF
#define AD9257_DEV_IND_2_REG (0x04)
#define AD9257_CHAN_H_OFST   (0)
#define AD9257_CHAN_H_MSK    (0x00000001 << AD9257_CHAN_H_OFST)
#define AD9257_CHAN_G_OFST   (1)
#define AD9257_CHAN_G_MSK    (0x00000001 << AD9257_CHAN_G_OFST)
#define AD9257_CHAN_F_OFST   (2)
#define AD9257_CHAN_F_MSK    (0x00000001 << AD9257_CHAN_F_OFST)
#define AD9257_CHAN_E_OFST   (3)
#define AD9257_CHAN_E_MSK    (0x00000001 << AD9257_CHAN_E_OFST)

// default value is 0x3F
#define AD9257_DEV_IND_1_REG    (0x05)
#define AD9257_CHAN_D_OFST      (0)
#define AD9257_CHAN_D_MSK       (0x00000001 << AD9257_CHAN_D_OFST)
#define AD9257_CHAN_C_OFST      (1)
#define AD9257_CHAN_C_MSK       (0x00000001 << AD9257_CHAN_C_OFST)
#define AD9257_CHAN_B_OFST      (2)
#define AD9257_CHAN_B_MSK       (0x00000001 << AD9257_CHAN_B_OFST)
#define AD9257_CHAN_A_OFST      (3)
#define AD9257_CHAN_A_MSK       (0x00000001 << AD9257_CHAN_A_OFST)
#define AD9257_CLK_CH_DCO_OFST  (4)
#define AD9257_CLK_CH_DCO_MSK   (0x00000001 << AD9257_CLK_CH_DCO_OFST)
#define AD9257_CLK_CH_IFCO_OFST (5)
#define AD9257_CLK_CH_IFCO_MSK  (0x00000001 << AD9257_CLK_CH_IFCO_OFST)

// default value is 0x00
#define AD9257_POWER_MODE_REG      (0x08)
#define AD9257_POWER_INTERNAL_OFST (0)
#define AD9257_POWER_INTERNAL_MSK  (0x00000003 << AD9257_POWER_INTERNAL_OFST)
#define AD9257_INT_CHIP_RUN_VAL                                                \
    ((0x0 << AD9257_POWER_INTERNAL_OFST) & AD9257_POWER_INTERNAL_MSK)
#define AD9257_INT_FULL_PWR_DWN_VAL                                            \
    ((0x1 << AD9257_POWER_INTERNAL_OFST) & AD9257_POWER_INTERNAL_MSK)
#define AD9257_INT_STANDBY_VAL                                                 \
    ((0x2 << AD9257_POWER_INTERNAL_OFST) & AD9257_POWER_INTERNAL_MSK)
#define AD9257_INT_RESET_VAL                                                   \
    ((0x3 << AD9257_POWER_INTERNAL_OFST) & AD9257_POWER_INTERNAL_MSK)
#define AD9257_POWER_EXTERNAL_OFST (5)
#define AD9257_POWER_EXTERNAL_MSK  (0x00000001 << AD9257_POWER_EXTERNAL_OFST)
#define AD9257_EXT_FULL_POWER_VAL                                              \
    ((0x0 << AD9257_POWER_EXTERNAL_OFST) & AD9257_POWER_EXTERNAL_MSK)
#define AD9257_EXT_STANDBY_VAL                                                 \
    ((0x1 << AD9257_POWER_EXTERNAL_OFST) & AD9257_POWER_EXTERNAL_MSK)

// default value is 0x0
#define AD9257_TEST_MODE_REG (0x0D)
#define AD9257_OUT_TEST_OFST (0)
#define AD9257_OUT_TEST_MSK  (0x0000000F << AD9257_OUT_TEST_OFST)
#define AD9257_TST_OFF_VAL   ((0x0 << AD9257_OUT_TEST_OFST) & AD9257_OUT_TEST_MSK)
#define AD9257_TST_MDSCL_SHRT_VAL                                              \
    ((0x1 << AD9257_OUT_TEST_OFST) & AD9257_OUT_TEST_MSK)
#define AD9257_TST_PSTV_FS_VAL                                                 \
    ((0x2 << AD9257_OUT_TEST_OFST) & AD9257_OUT_TEST_MSK)
#define AD9257_TST_NGTV_FS_VAL                                                 \
    ((0x3 << AD9257_OUT_TEST_OFST) & AD9257_OUT_TEST_MSK)
#define AD9257_TST_ALTRNTNG_CHKRBRD_VAL                                        \
    ((0x4 << AD9257_OUT_TEST_OFST) & AD9257_OUT_TEST_MSK)
#define AD9257_TST_PN_23_SQNC_VAL                                              \
    ((0x5 << AD9257_OUT_TEST_OFST) & AD9257_OUT_TEST_MSK)
#define AD9257_TST_PN_9_SQNC__VAL                                              \
    ((0x6 << AD9257_OUT_TEST_OFST) & AD9257_OUT_TEST_MSK)
#define AD9257_TST_1_0_WRD_TGGL_VAL                                            \
    ((0x7 << AD9257_OUT_TEST_OFST) & AD9257_OUT_TEST_MSK)
#define AD9257_TST_USR_INPT_VAL                                                \
    ((0x8 << AD9257_OUT_TEST_OFST) & AD9257_OUT_TEST_MSK)
#define AD9257_TST_1_0_BT_TGGL_VAL                                             \
    ((0x9 << AD9257_OUT_TEST_OFST) & AD9257_OUT_TEST_MSK)
#define AD9257_TST_1_x_SYNC_VAL                                                \
    ((0xa << AD9257_OUT_TEST_OFST) & AD9257_OUT_TEST_MSK)
#define AD9257_TST_1_BIT_HGH_VAL                                               \
    ((0xb << AD9257_OUT_TEST_OFST) & AD9257_OUT_TEST_MSK)
#define AD9257_TST_MXD_BT_FRQ_VAL                                              \
    ((0xc << AD9257_OUT_TEST_OFST) & AD9257_OUT_TEST_MSK)
#define AD9257_TST_RST_SHRT_GN_OFST (4)
#define AD9257_TST_RST_SHRT_GN_MSK  (0x00000001 << AD9257_TST_RST_SHRT_GN_OFST)
#define AD9257_TST_RST_LNG_GN_OFST  (5)
#define AD9257_TST_RST_LNG_GN_MSK   (0x00000001 << AD9257_TST_RST_LNG_GN_OFST)
#define AD9257_USER_IN_MODE_OFST    (6)
#define AD9257_USER_IN_MODE_MSK     (0x00000003 << AD9257_USER_IN_MODE_OFST)
#define AD9257_USR_IN_SNGL_VAL                                                 \
    ((0x0 << AD9257_USER_IN_MODE_OFST) & AD9257_USER_IN_MODE_MSK)
#define AD9257_USR_IN_ALTRNT_VAL                                               \
    ((0x1 << AD9257_USER_IN_MODE_OFST) & AD9257_USER_IN_MODE_MSK)
#define AD9257_USR_IN_SNGL_ONC_VAL                                             \
    ((0x2 << AD9257_USER_IN_MODE_OFST) & AD9257_USER_IN_MODE_MSK)
#define AD9257_USR_IN_ALTRNT_ONC_VAL                                           \
    ((0x3 << AD9257_USER_IN_MODE_OFST) & AD9257_USER_IN_MODE_MSK)

// default value is 0x01
#define AD9257_OUT_MODE_REG    (0x14)
#define AD9257_OUT_FORMAT_OFST (0)
#define AD9257_OUT_FORMAT_MSK  (0x00000001 << AD9257_OUT_FORMAT_OFST)
#define AD9257_OUT_BINARY_OFST_VAL                                             \
    ((0x0 << AD9257_OUT_FORMAT_OFST) & AD9257_OUT_FORMAT_MSK)
#define AD9257_OUT_TWOS_COMPL_VAL                                              \
    ((0x1 << AD9257_OUT_FORMAT_OFST) & AD9257_OUT_FORMAT_MSK)
#define AD9257_OUT_OTPT_INVRT_OFST (2)
#define AD9257_OUT_OTPT_INVRT_MSK  (0x00000001 << AD9257_OUT_OTPT_INVRT_OFST)
#define AD9257_OUT_LVDS_OPT_OFST   (6)
#define AD9257_OUT_LVDS_OPT_MSK    (0x00000001 << AD9257_OUT_LVDS_OPT_OFST)
#define AD9257_OUT_LVDS_ANSI_VAL                                               \
    ((0x0 << AD9257_OUT_LVDS_OPT_OFST) & AD9257_OUT_LVDS_OPT_MSK)
#define AD9257_OUT_LVDS_IEEE_VAL                                               \
    ((0x1 << AD9257_OUT_LVDS_OPT_OFST) & AD9257_OUT_LVDS_OPT_MSK)

// default value is 0x3
#define AD9257_OUT_PHASE_REG (0x16)
#define AD9257_OUT_CLK_OFST  (0)
#define AD9257_OUT_CLK_MSK   (0x0000000F << AD9257_OUT_CLK_OFST)
#define AD9257_OUT_CLK_0_VAL ((0x0 << AD9257_OUT_CLK_OFST) & AD9257_OUT_CLK_MSK)
#define AD9257_OUT_CLK_60_VAL                                                  \
    ((0x1 << AD9257_OUT_CLK_OFST) & AD9257_OUT_CLK_MSK)
#define AD9257_OUT_CLK_120_VAL                                                 \
    ((0x2 << AD9257_OUT_CLK_OFST) & AD9257_OUT_CLK_MSK)
#define AD9257_OUT_CLK_180_VAL                                                 \
    ((0x3 << AD9257_OUT_CLK_OFST) & AD9257_OUT_CLK_MSK)
#define AD9257_OUT_CLK_240_VAL                                                 \
    ((0x4 << AD9257_OUT_CLK_OFST) & AD9257_OUT_CLK_MSK)
#define AD9257_OUT_CLK_300_VAL                                                 \
    ((0x5 << AD9257_OUT_CLK_OFST) & AD9257_OUT_CLK_MSK)
#define AD9257_OUT_CLK_360_VAL                                                 \
    ((0x6 << AD9257_OUT_CLK_OFST) & AD9257_OUT_CLK_MSK)
#define AD9257_OUT_CLK_420_VAL                                                 \
    ((0x7 << AD9257_OUT_CLK_OFST) & AD9257_OUT_CLK_MSK)
#define AD9257_OUT_CLK_480_VAL                                                 \
    ((0x8 << AD9257_OUT_CLK_OFST) & AD9257_OUT_CLK_MSK)
#define AD9257_OUT_CLK_540_VAL                                                 \
    ((0x9 << AD9257_OUT_CLK_OFST) & AD9257_OUT_CLK_MSK)
#define AD9257_OUT_CLK_600_VAL                                                 \
    ((0xa << AD9257_OUT_CLK_OFST) & AD9257_OUT_CLK_MSK)
#define AD9257_OUT_CLK_660_VAL                                                 \
    ((0xb << AD9257_OUT_CLK_OFST) & AD9257_OUT_CLK_MSK)
#define AD9257_IN_CLK_OFST  (4)
#define AD9257_IN_CLK_MSK   (0x00000007 << AD9257_IN_CLK_OFST)
#define AD9257_IN_CLK_0_VAL ((0x0 << AD9257_IN_CLK_OFST) & AD9257_IN_CLK_MSK)
#define AD9257_IN_CLK_1_VAL ((0x1 << AD9257_IN_CLK_OFST) & AD9257_IN_CLK_MSK)
#define AD9257_IN_CLK_2_VAL ((0x2 << AD9257_IN_CLK_OFST) & AD9257_IN_CLK_MSK)
#define AD9257_IN_CLK_3_VAL ((0x3 << AD9257_IN_CLK_OFST) & AD9257_IN_CLK_MSK)
#define AD9257_IN_CLK_4_VAL ((0x4 << AD9257_IN_CLK_OFST) & AD9257_IN_CLK_MSK)
#define AD9257_IN_CLK_5_VAL ((0x5 << AD9257_IN_CLK_OFST) & AD9257_IN_CLK_MSK)
#define AD9257_IN_CLK_6_VAL ((0x6 << AD9257_IN_CLK_OFST) & AD9257_IN_CLK_MSK)
#define AD9257_IN_CLK_7_VAL ((0x7 << AD9257_IN_CLK_OFST) & AD9257_IN_CLK_MSK)

// default value is 0x4
#define AD9257_VREF_REG         (0x18)
#define AD9257_VREF_OFST        (0)
#define AD9257_VREF_MSK         (0x00000007 << AD9257_VREF_OFST)
#define AD9257_VREF_DEFAULT_VAL (AD9257_VREF_2_0_VAL)
#define AD9257_VREF_1_0_VAL     ((0x0 << AD9257_VREF_OFST) & AD9257_VREF_MSK)
#define AD9257_VREF_1_14_VAL    ((0x1 << AD9257_VREF_OFST) & AD9257_VREF_MSK)
#define AD9257_VREF_1_33_VAL    ((0x2 << AD9257_VREF_OFST) & AD9257_VREF_MSK)
#define AD9257_VREF_1_6_VAL     ((0x3 << AD9257_VREF_OFST) & AD9257_VREF_MSK)
#define AD9257_VREF_2_0_VAL     ((0x4 << AD9257_VREF_OFST) & AD9257_VREF_MSK)

// defines from the fpga
uint32_t AD9257_Reg = 0x0;
uint32_t AD9257_CsMask = 0x0;
uint32_t AD9257_ClkMask = 0x0;
uint32_t AD9257_DigMask = 0x0;
int AD9257_DigOffset = 0x0;
int AD9257_VrefVoltage = 0;

#ifdef JUNGFRAUD
int AD9257_is_Jungfrau_Hardware_Version_1_0 = 0;

void AD9257_Set_Jungfrau_Hardware_Version_1_0(int val) {
    AD9257_is_Jungfrau_Hardware_Version_1_0 = val;
}
#endif

void AD9257_SetDefines(uint32_t reg, uint32_t cmsk, uint32_t clkmsk,
                       uint32_t dmsk, int dofst) {
    AD9257_Reg = reg;
    AD9257_CsMask = cmsk;
    AD9257_ClkMask = clkmsk;
    AD9257_DigMask = dmsk;
    AD9257_DigOffset = dofst;
}

void AD9257_Disable() {
    bus_w(AD9257_Reg, (bus_r(AD9257_Reg) | AD9257_CsMask | AD9257_ClkMask) &
                          ~(AD9257_DigMask));
}

int AD9257_GetVrefVoltage(int mV) {
    if (mV == 0)
        return AD9257_VrefVoltage;
    switch (AD9257_VrefVoltage) {
    case 0:
        return 1000;
    case 1:
        return 1140;
    case 2:
        return 1330;
    case 3:
        return 1600;
    case 4:
        return 2000;
    default:
        LOG(logERROR, ("Could not convert Adc Vpp from mode to mV\n"));
        return -1;
    }
}

int AD9257_SetVrefVoltage(int val, int mV) {
    int mode = val;
    // convert to mode
    if (mV) {
        switch (val) {
        case 1000:
            mode = 0;
            break;
        case 1140:
            mode = 1;
            break;
        case 1330:
            mode = 2;
            break;
        case 1600:
            mode = 3;
            break;
        case 2000:
            mode = 4;
            break;
        // validation for mV
        default:
            LOG(logERROR, ("mv:%d doesnt exist\n", val));
            return FAIL;
        }
    }

    // validation for mode
    switch (mode) {
    case 0:
        LOG(logINFO, ("Setting ADC Vref to 1.0 V (Mode:%d)\n", mode));
        break;
    case 1:
        LOG(logINFO, ("Setting ADC Vref to 1.14 V (Mode:%d)\n", mode));
        break;
    case 2:
        LOG(logINFO, ("Setting ADC Vref to 1.33 V (Mode:%d)\n", mode));
        break;
    case 3:
        LOG(logINFO, ("Setting ADC Vref to 1.6 V (Mode:%d)\n", mode));
        break;
    case 4:
        LOG(logINFO, ("Setting ADC Vref to 2.0 V (Mode:%d)\n", mode));
        break;
    default:
        return FAIL;
    }
    // set vref voltage
    AD9257_Set(AD9257_VREF_REG, mode);
    AD9257_VrefVoltage = mode;
    return OK;
}

void AD9257_Set(int addr, int val) {

    u_int32_t codata;
    codata = val + (addr << 8);
    LOG(logINFO,
        ("\tSetting ADC SPI Register. Wrote 0x%04x at 0x%04x\n", val, addr));
    serializeToSPI(AD9257_Reg, codata, AD9257_CsMask, AD9257_ADC_NUMBITS,
                   AD9257_ClkMask, AD9257_DigMask, AD9257_DigOffset, 0);
}

void AD9257_Configure() {
    LOG(logINFOBLUE, ("Configuring ADC9257:\n"));

    // power mode reset
    LOG(logINFO, ("\tPower mode reset\n"));
    AD9257_Set(AD9257_POWER_MODE_REG, AD9257_INT_RESET_VAL);

    // power mode chip run
    LOG(logINFO, ("\tPower mode chip run\n"));
    AD9257_Set(AD9257_POWER_MODE_REG, AD9257_INT_CHIP_RUN_VAL);

    // binary offset, lvds-iee reduced
    LOG(logINFO, ("\tBinary offset, Lvds-ieee reduced\n"));
    AD9257_Set(AD9257_OUT_MODE_REG,
               AD9257_OUT_BINARY_OFST_VAL | AD9257_OUT_LVDS_IEEE_VAL);

    // output clock phase
#ifdef JUNGFRAUD
    if (AD9257_is_Jungfrau_Hardware_Version_1_0) {
        LOG(logINFO, ("\tOutput clock phase: 120\n"));
        AD9257_Set(AD9257_OUT_PHASE_REG, AD9257_OUT_CLK_120_VAL);
    } else {
        LOG(logINFO, ("\tOutput clock phase: 180\n"));
        AD9257_Set(AD9257_OUT_PHASE_REG, AD9257_OUT_CLK_180_VAL);
    }
#else
    LOG(logINFO, ("\tOutput clock phase: 180\n"));
    AD9257_Set(AD9257_OUT_PHASE_REG, AD9257_OUT_CLK_180_VAL);
#endif

    // all devices on chip to receive next command
    LOG(logINFO, ("\tAll devices on chip to receive next command\n"));
    AD9257_Set(AD9257_DEV_IND_2_REG, AD9257_CHAN_H_MSK | AD9257_CHAN_G_MSK |
                                         AD9257_CHAN_F_MSK | AD9257_CHAN_E_MSK);

    AD9257_Set(AD9257_DEV_IND_1_REG, AD9257_CHAN_D_MSK | AD9257_CHAN_C_MSK |
                                         AD9257_CHAN_B_MSK | AD9257_CHAN_A_MSK |
                                         AD9257_CLK_CH_DCO_MSK |
                                         AD9257_CLK_CH_IFCO_MSK);

    // vref
#if defined(GOTTHARDD) || defined(MOENCHD)
    LOG(logINFO, ("\tVref default at 2.0\n"));
    AD9257_SetVrefVoltage(AD9257_VREF_DEFAULT_VAL, 0);
#else
    LOG(logINFO, ("\tVref 1.33\n"));
    AD9257_SetVrefVoltage(AD9257_VREF_1_33_VAL, 0);
#endif

    // no test mode
    LOG(logINFO, ("\tNo test mode\n"));
    AD9257_Set(AD9257_TEST_MODE_REG, AD9257_TST_OFF_VAL);

#ifdef TESTADC
        LOG(logINFOBLUE, ("Putting ADC in Test Mode!\n");
	// mixed bit frequency test mode
	LOG(logINFO, ("\tMixed bit frequency test mode\n"));
	AD9257_Set(AD9257_TEST_MODE_REG, AD9257_TST_MXD_BT_FRQ_VAL);
#endif
}
