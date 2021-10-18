// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "AD9252.h"
#include "blackfin.h"
#include "clogger.h"
#include "commonServerFunctions.h" // blackfin.h, ansi.h

/* AD9252 ADC DEFINES */
#define AD9252_ADC_NUMBITS (24)

// default value is 0xF
#define AD9252_DEV_IND_2_REG (0x04)
#define AD9252_CHAN_H_OFST   (0)
#define AD9252_CHAN_H_MSK    (0x00000001 << AD9252_CHAN_H_OFST)
#define AD9252_CHAN_G_OFST   (1)
#define AD9252_CHAN_G_MSK    (0x00000001 << AD9252_CHAN_G_OFST)
#define AD9252_CHAN_F_OFST   (2)
#define AD9252_CHAN_F_MSK    (0x00000001 << AD9252_CHAN_F_OFST)
#define AD9252_CHAN_E_OFST   (3)
#define AD9252_CHAN_E_MSK    (0x00000001 << AD9252_CHAN_E_OFST)

// default value is 0x0F
#define AD9252_DEV_IND_1_REG    (0x05)
#define AD9252_CHAN_D_OFST      (0)
#define AD9252_CHAN_D_MSK       (0x00000001 << AD9252_CHAN_D_OFST)
#define AD9252_CHAN_C_OFST      (1)
#define AD9252_CHAN_C_MSK       (0x00000001 << AD9252_CHAN_C_OFST)
#define AD9252_CHAN_B_OFST      (2)
#define AD9252_CHAN_B_MSK       (0x00000001 << AD9252_CHAN_B_OFST)
#define AD9252_CHAN_A_OFST      (3)
#define AD9252_CHAN_A_MSK       (0x00000001 << AD9252_CHAN_A_OFST)
#define AD9252_CLK_CH_DCO_OFST  (4)
#define AD9252_CLK_CH_DCO_MSK   (0x00000001 << AD9252_CLK_CH_DCO_OFST)
#define AD9252_CLK_CH_IFCO_OFST (5)
#define AD9252_CLK_CH_IFCO_MSK  (0x00000001 << AD9252_CLK_CH_IFCO_OFST)

// default value is 0x00
#define AD9252_POWER_MODE_REG      (0x08)
#define AD9252_POWER_INTERNAL_OFST (0)
#define AD9252_POWER_INTERNAL_MSK  (0x00000007 << AD9252_POWER_INTERNAL_OFST)
#define AD9252_INT_CHIP_RUN_VAL                                                \
    ((0x0 << AD9252_POWER_INTERNAL_OFST) & AD9252_POWER_INTERNAL_MSK)
#define AD9252_INT_FULL_PWR_DWN_VAL                                            \
    ((0x1 << AD9252_POWER_INTERNAL_OFST) & AD9252_POWER_INTERNAL_MSK)
#define AD9252_INT_STANDBY_VAL                                                 \
    ((0x2 << AD9252_POWER_INTERNAL_OFST) & AD9252_POWER_INTERNAL_MSK)
#define AD9252_INT_RESET_VAL                                                   \
    ((0x3 << AD9252_POWER_INTERNAL_OFST) & AD9252_POWER_INTERNAL_MSK)

// default value is 0x0
#define AD9252_TEST_MODE_REG (0x0D)
#define AD9252_OUT_TEST_OFST (0)
#define AD9252_OUT_TEST_MSK  (0x0000000F << AD9252_OUT_TEST_OFST)
#define AD9252_TST_OFF_VAL   ((0x0 << AD9252_OUT_TEST_OFST) & AD9252_OUT_TEST_MSK)
#define AD9252_TST_MDSCL_SHRT_VAL                                              \
    ((0x1 << AD9252_OUT_TEST_OFST) & AD9252_OUT_TEST_MSK)
#define AD9252_TST_PSTV_FS_VAL                                                 \
    ((0x2 << AD9252_OUT_TEST_OFST) & AD9252_OUT_TEST_MSK)
#define AD9252_TST_NGTV_FS_VAL                                                 \
    ((0x3 << AD9252_OUT_TEST_OFST) & AD9252_OUT_TEST_MSK)
#define AD9252_TST_ALTRNTNG_CHKRBRD_VAL                                        \
    ((0x4 << AD9252_OUT_TEST_OFST) & AD9252_OUT_TEST_MSK)
#define AD9252_TST_PN_23_SQNC_VAL                                              \
    ((0x5 << AD9252_OUT_TEST_OFST) & AD9252_OUT_TEST_MSK)
#define AD9252_TST_PN_9_SQNC__VAL                                              \
    ((0x6 << AD9252_OUT_TEST_OFST) & AD9252_OUT_TEST_MSK)
#define AD9252_TST_1_0_WRD_TGGL_VAL                                            \
    ((0x7 << AD9252_OUT_TEST_OFST) & AD9252_OUT_TEST_MSK)
#define AD9252_TST_USR_INPT_VAL                                                \
    ((0x8 << AD9252_OUT_TEST_OFST) & AD9252_OUT_TEST_MSK)
#define AD9252_TST_1_0_BT_TGGL_VAL                                             \
    ((0x9 << AD9252_OUT_TEST_OFST) & AD9252_OUT_TEST_MSK)
#define AD9252_TST_1_x_SYNC_VAL                                                \
    ((0xa << AD9252_OUT_TEST_OFST) & AD9252_OUT_TEST_MSK)
#define AD9252_TST_1_BIT_HGH_VAL                                               \
    ((0xb << AD9252_OUT_TEST_OFST) & AD9252_OUT_TEST_MSK)
#define AD9252_TST_MXD_BT_FRQ_VAL                                              \
    ((0xc << AD9252_OUT_TEST_OFST) & AD9252_OUT_TEST_MSK)
#define AD9252_TST_RST_SHRT_GN_OFST (4)
#define AD9252_TST_RST_SHRT_GN_MSK  (0x00000001 << AD9252_TST_RST_SHRT_GN_OFST)
#define AD9252_TST_RST_LNG_GN_OFST  (5)
#define AD9252_TST_RST_LNG_GN_MSK   (0x00000001 << AD9252_TST_RST_LNG_GN_OFST)
#define AD9252_USER_IN_MODE_OFST    (6)
#define AD9252_USER_IN_MODE_MSK     (0x00000003 << AD9252_USER_IN_MODE_OFST)
#define AD9252_USR_IN_SNGL_VAL                                                 \
    ((0x0 << AD9252_USER_IN_MODE_OFST) & AD9252_USER_IN_MODE_MSK)
#define AD9252_USR_IN_ALTRNT_VAL                                               \
    ((0x1 << AD9252_USER_IN_MODE_OFST) & AD9252_USER_IN_MODE_MSK)
#define AD9252_USR_IN_SNGL_ONC_VAL                                             \
    ((0x2 << AD9252_USER_IN_MODE_OFST) & AD9252_USER_IN_MODE_MSK)
#define AD9252_USR_IN_ALTRNT_ONC_VAL                                           \
    ((0x3 << AD9252_USER_IN_MODE_OFST) & AD9252_USER_IN_MODE_MSK)

// default value is 0x00
#define AD9252_OUT_MODE_REG    (0x14)
#define AD9252_OUT_FORMAT_OFST (0)
#define AD9252_OUT_FORMAT_MSK  (0x00000003 << AD9252_OUT_FORMAT_OFST)
#define AD9252_OUT_BINARY_OFST_VAL                                             \
    ((0x0 << AD9252_OUT_FORMAT_OFST) & AD9252_OUT_FORMAT_MSK)
#define AD9252_OUT_TWOS_COMPL_VAL                                              \
    ((0x1 << AD9252_OUT_FORMAT_OFST) & AD9252_OUT_FORMAT_MSK)
#define AD9252_OUT_OTPT_INVRT_OFST (2)
#define AD9252_OUT_OTPT_INVRT_MSK  (0x00000001 << AD9252_OUT_OTPT_INVRT_OFST)
#define AD9252_OUT_LVDS_OPT_OFST   (6)
#define AD9252_OUT_LVDS_OPT_MSK    (0x00000001 << AD9252_OUT_LVDS_OPT_OFST)
#define AD9252_OUT_LVDS_ANSI_VAL                                               \
    ((0x0 << AD9252_OUT_LVDS_OPT_OFST) & AD9252_OUT_LVDS_OPT_MSK)
#define AD9252_OUT_LVDS_IEEE_VAL                                               \
    ((0x1 << AD9252_OUT_LVDS_OPT_OFST) & AD9252_OUT_LVDS_OPT_MSK)

// default value is 0x3
#define AD9252_OUT_PHASE_REG (0x16)
#define AD9252_OUT_CLK_OFST  (0)
#define AD9252_OUT_CLK_MSK   (0x0000000F << AD9252_OUT_CLK_OFST)
#define AD9252_OUT_CLK_0_VAL ((0x0 << AD9252_OUT_CLK_OFST) & AD9252_OUT_CLK_MSK)
#define AD9252_OUT_CLK_60_VAL                                                  \
    ((0x1 << AD9252_OUT_CLK_OFST) & AD9252_OUT_CLK_MSK)
#define AD9252_OUT_CLK_120_VAL                                                 \
    ((0x2 << AD9252_OUT_CLK_OFST) & AD9252_OUT_CLK_MSK)
#define AD9252_OUT_CLK_180_VAL                                                 \
    ((0x3 << AD9252_OUT_CLK_OFST) & AD9252_OUT_CLK_MSK)
#define AD9252_OUT_CLK_300_VAL                                                 \
    ((0x5 << AD9252_OUT_CLK_OFST) & AD9252_OUT_CLK_MSK)
#define AD9252_OUT_CLK_360_VAL                                                 \
    ((0x6 << AD9252_OUT_CLK_OFST) & AD9252_OUT_CLK_MSK)
#define AD9252_OUT_CLK_480_VAL                                                 \
    ((0x8 << AD9252_OUT_CLK_OFST) & AD9252_OUT_CLK_MSK)
#define AD9252_OUT_CLK_540_VAL                                                 \
    ((0x9 << AD9252_OUT_CLK_OFST) & AD9252_OUT_CLK_MSK)
#define AD9252_OUT_CLK_600_VAL                                                 \
    ((0xa << AD9252_OUT_CLK_OFST) & AD9252_OUT_CLK_MSK)
#define AD9252_OUT_CLK_660_VAL                                                 \
    ((0xb << AD9252_OUT_CLK_OFST) & AD9252_OUT_CLK_MSK) // 0xb - 0xf is 660

// defines from the fpga
uint32_t AD9252_Reg = 0x0;
uint32_t AD9252_CsMask = 0x0;
uint32_t AD9252_ClkMask = 0x0;
uint32_t AD9252_DigMask = 0x0;
int AD9252_DigOffset = 0x0;

void AD9252_SetDefines(uint32_t reg, uint32_t cmsk, uint32_t clkmsk,
                       uint32_t dmsk, int dofst) {
    AD9252_Reg = reg;
    AD9252_CsMask = cmsk;
    AD9252_ClkMask = clkmsk;
    AD9252_DigMask = dmsk;
    AD9252_DigOffset = dofst;
}

void AD9252_Disable() {
    bus_w(AD9252_Reg, (bus_r(AD9252_Reg) | AD9252_CsMask | AD9252_ClkMask) &
                          ~(AD9252_DigMask));
}

void AD9252_Set(int addr, int val) {

    u_int32_t codata;
    codata = val + (addr << 8);
    LOG(logINFO,
        ("\tSetting ADC SPI Register. Wrote 0x%04x at 0x%04x\n", val, addr));
    serializeToSPI(AD9252_Reg, codata, AD9252_CsMask, AD9252_ADC_NUMBITS,
                   AD9252_ClkMask, AD9252_DigMask, AD9252_DigOffset, 0);
}

void AD9252_Configure() {
    LOG(logINFOBLUE, ("Configuring ADC9252:\n"));

    // power mode reset
    LOG(logINFO, ("\tPower mode reset\n"));
    AD9252_Set(AD9252_POWER_MODE_REG, AD9252_INT_RESET_VAL);

    // power mode chip run
    LOG(logINFO, ("\tPower mode chip run\n"));
    AD9252_Set(AD9252_POWER_MODE_REG, AD9252_INT_CHIP_RUN_VAL);

    // binary offset
    LOG(logINFO, ("\tBinary offset\n"));
    AD9252_Set(AD9252_OUT_MODE_REG, AD9252_OUT_BINARY_OFST_VAL);

    // output clock phase
#ifdef GOTTHARDD
    LOG(logINFO, ("\tOutput clock phase is at default: 180\n"));
#else
    LOG(logINFO, ("\tOutput clock phase: 60\n"));
    AD9257_Set(AD9257_OUT_PHASE_REG, AD9257_OUT_CLK_60_VAL);
#endif

    // lvds-iee reduced , binary offset
    LOG(logINFO, ("\tLvds-iee reduced, binary offset\n"));
    AD9252_Set(AD9252_OUT_MODE_REG, AD9252_OUT_LVDS_IEEE_VAL);

    // all devices on chip to receive next command
    LOG(logINFO, ("\tAll devices on chip to receive next command\n"));
    AD9252_Set(AD9252_DEV_IND_2_REG, AD9252_CHAN_H_MSK | AD9252_CHAN_G_MSK |
                                         AD9252_CHAN_F_MSK | AD9252_CHAN_E_MSK);
    AD9252_Set(AD9252_DEV_IND_1_REG, AD9252_CHAN_D_MSK | AD9252_CHAN_C_MSK |
                                         AD9252_CHAN_B_MSK | AD9252_CHAN_A_MSK |
                                         AD9252_CLK_CH_DCO_MSK |
                                         AD9252_CLK_CH_IFCO_MSK);

    // no test mode
    LOG(logINFO, ("\tNo test mode\n"));
    AD9252_Set(AD9252_TEST_MODE_REG, AD9252_TST_OFF_VAL);

#ifdef TESTADC
    LOG(logINFOBLUE, ("Putting ADC in Test Mode!\n");
    // mixed bit frequency test mode
    LOG(logINFO, ("\tMixed bit frequency test mode\n"));
    AD9252_Set(AD9252_TEST_MODE_REG, AD9252_TST_MXD_BT_FRQ_VAL);
#endif
}
