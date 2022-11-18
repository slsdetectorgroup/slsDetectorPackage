// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "ALTERA_PLL_CYCLONE10.h"
#include "clogger.h"
#include "nios.h"
#include "sls/sls_detector_defs.h"

#include <unistd.h> // usleep

/* Altera PLL CYCLONE 10 DEFINES */

/** PLL Reconfiguration Registers */
// https://www.intel.com/content/dam/www/programmable/us/en/pdfs/literature/an/an728.pdf

// c counter (C0-C8 (+1 to base address))
#define ALTERA_PLL_C10_C_COUNTER_BASE_REG (0x0C0)

#define ALTERA_PLL_C10_C_COUNTER_MAX_DIVIDER_VAL (512)
#define ALTERA_PLL_C10_C_COUNTER_LW_CNT_OFST     (0)
#define ALTERA_PLL_C10_C_COUNTER_LW_CNT_MSK                                    \
    (0x000000FF << ALTERA_PLL_C10_C_COUNTER_LW_CNT_OFST)
#define ALTERA_PLL_C10_C_COUNTER_HGH_CNT_OFST (8)
#define ALTERA_PLL_C10_C_COUNTER_HGH_CNT_MSK                                   \
    (0x000000FF << ALTERA_PLL_C10_C_COUNTER_HGH_CNT_OFST)
/* total_div = lw_cnt + hgh_cnt */
#define ALTERA_PLL_C10_C_COUNTER_BYPSS_ENBL_OFST (16)
#define ALTERA_PLL_C10_C_COUNTER_BYPSS_ENBL_MSK                                \
    (0x00000001 << ALTERA_PLL_C10_C_COUNTER_BYPSS_ENBL_OFST)
/* if bypss_enbl = 0, fout = f(vco)/total_div; else fout = f(vco) (c counter is
 * bypassed) */
#define ALTERA_PLL_C10_C_COUNTER_ODD_DVSN_OFST (17)
#define ALTERA_PLL_C10_C_COUNTER_ODD_DVSN_MSK                                  \
    (0x00000001 << ALTERA_PLL_C10_C_COUNTER_ODD_DVSN_OFST)
/** if odd_dvsn = 0 (even), duty cycle = hgh_cnt/ total_div; else duty cycle =
 * (hgh_cnt - 0.5) / total_div */

// dynamic phase shift (C0-C8 (+1 to base address), 0xF for all counters)
#define ALTERA_PLL_C10_PHASE_SHIFT_BASE_REG (0x100)

#define ALTERA_PLL_C10_MAX_SHIFTS_PER_OPERATION (7)
#define ALTERA_PLL_C10_SHIFT_NUM_SHIFTS_OFST    (0)
#define ALTERA_PLL_C10_SHIFT_NUM_SHIFTS_MSK                                    \
    (0x00000007 << ALTERA_PLL_C10_SHIFT_NUM_SHIFTS_OFST)

#define ALTERA_PLL_C10_SHIFT_UP_DOWN_OFST (3)
#define ALTERA_PLL_C10_SHIFT_UP_DOWN_MSK                                       \
    (0x00000001 << ALTERA_PLL_C10_SHIFT_UP_DOWN_OFST)
#define ALTERA_PLL_C10_SHIFT_UP_DOWN_NEG_VAL                                   \
    ((0x0 << ALTERA_PLL_C10_SHIFT_UP_DOWN_OFST) &                              \
     ALTERA_PLL_C10_SHIFT_UP_DOWN_MSK)
#define ALTERA_PLL_C10_SHIFT_UP_DOWN_POS_VAL                                   \
    ((0x1 << ALTERA_PLL_C10_SHIFT_UP_DOWN_OFST) &                              \
     ALTERA_PLL_C10_SHIFT_UP_DOWN_MSK)

#define ALTERA_PLL_C10_PHASE_SHIFT_STEP_OF_VCO (8)
#define ALTERA_PLL_C10_WAIT_TIME_US            (1 * 1000) // 1 ms

int ALTERA_PLL_C10_Reg_offset = 0x0;
const int ALTERA_PLL_C10_NUM = 2;
uint32_t ALTERA_PLL_C10_BaseAddress[2] = {0x0, 0x0};
uint32_t ALTERA_PLL_C10_Reset_Reg = 0x0;
uint32_t ALTERA_PLL_C10_Reset_Msk[2] = {0x0, 0x0};
#ifdef MYTHEN3D
uint32_t ALTERA_PLL_C10_Locked_Status_Reg = 0x0;
uint32_t ALTERA_PLL_C10_Locked_Status_Msk[2] = {0x0, 0x0};
#endif
int ALTERA_PLL_C10_VCO_FREQ[2] = {0, 0};

void ALTERA_PLL_C10_SetDefines(int regofst, uint32_t baseaddr0,
                               uint32_t baseaddr1, uint32_t resetreg,
                               uint32_t resetmsk0, uint32_t resetmsk1,
#ifdef MYTHEN3D
                               uint32_t statusreg, uint32_t statusmsk0,
                               uint32_t statusmsk1,
#endif
                               int vcofreq0, int vcofreq1) {
    ALTERA_PLL_C10_Reg_offset = regofst;
    ALTERA_PLL_C10_BaseAddress[0] = baseaddr0;
    ALTERA_PLL_C10_BaseAddress[1] = baseaddr1;
    ALTERA_PLL_C10_Reset_Reg = resetreg;
    ALTERA_PLL_C10_Reset_Msk[0] = resetmsk0;
    ALTERA_PLL_C10_Reset_Msk[1] = resetmsk1;

    // pll locked status reg and msk only used for m3 now
#ifdef MYTHEN3D
    ALTERA_PLL_C10_Locked_Status_Reg = statusreg;
    ALTERA_PLL_C10_Locked_Status_Msk[0] = statusmsk0;
    ALTERA_PLL_C10_Locked_Status_Msk[1] = statusmsk1;
#endif
    ALTERA_PLL_C10_VCO_FREQ[0] = vcofreq0;
    ALTERA_PLL_C10_VCO_FREQ[1] = vcofreq1;
}

int ALTERA_PLL_C10_GetMaxClockDivider() {
    return ALTERA_PLL_C10_C_COUNTER_MAX_DIVIDER_VAL;
}

int ALTERA_PLL_C10_GetVCOFrequency(int pllIndex) {
    return ALTERA_PLL_C10_VCO_FREQ[pllIndex];
}

int ALTERA_PLL_C10_GetMaxPhaseShiftStepsofVCO() {
    return ALTERA_PLL_C10_PHASE_SHIFT_STEP_OF_VCO;
}

void ALTERA_PLL_C10_Reconfigure(int pllIndex) {
    LOG(logINFO, ("\tReconfiguring PLL %d\n", pllIndex));

    // write anything to base address to start reconfiguring
    LOG(logDEBUG1, ("\tWriting 1 to base address 0x%x to start reconfiguring\n",
                    ALTERA_PLL_C10_BaseAddress[pllIndex]));
    bus_w_csp1(ALTERA_PLL_C10_BaseAddress[pllIndex], 0x1);
    usleep(ALTERA_PLL_C10_WAIT_TIME_US);
}

void ALTERA_PLL_C10_ResetPLL(int pllIndex) {
    uint32_t resetreg = ALTERA_PLL_C10_Reset_Reg;
    uint32_t resetmsk = ALTERA_PLL_C10_Reset_Msk[pllIndex];
    LOG(logINFO, ("Resetting PLL %d\n", pllIndex));
    bus_w_csp1(resetreg, bus_r_csp1(resetreg) | resetmsk);
    usleep(ALTERA_PLL_C10_WAIT_TIME_US);

#ifndef VIRTUAL
#ifdef MYTHEN3D
    uint32_t statusreg = ALTERA_PLL_C10_Locked_Status_Reg;
    uint32_t statusmsk = ALTERA_PLL_C10_Locked_Status_Msk[pllIndex];
    // wait for pll locked bit to go high
    while (!(bus_r(statusreg) & statusmsk)) {
        usleep(ALTERA_PLL_C10_WAIT_TIME_US);
        LOG(logWARNING, ("Still waiting for PLL %d recovery\n", pllIndex));
    }
    LOG(logINFO, ("Reset success for PLL %d\n", pllIndex));
#endif
#endif
}

void ALTERA_PLL_C10_SetPhaseShift(int pllIndex, int clkIndex, int phase,
                                  int pos) {
    LOG(logINFO, ("\tC%d: Writing PLL %d Phase Shift [phase:%d, pos dir:%d]\n",
                  clkIndex, pllIndex, phase, pos));

    uint32_t addr = ALTERA_PLL_C10_BaseAddress[pllIndex] +
                    (ALTERA_PLL_C10_PHASE_SHIFT_BASE_REG + (int)clkIndex) *
                        ALTERA_PLL_C10_Reg_offset;
    int maxshifts = ALTERA_PLL_C10_MAX_SHIFTS_PER_OPERATION;

    // only 7 shifts at a time
    while (phase > 0) {
        int phaseToDo = (phase > maxshifts) ? maxshifts : phase;
        uint32_t value = (((phaseToDo << ALTERA_PLL_C10_SHIFT_NUM_SHIFTS_OFST) &
                           ALTERA_PLL_C10_SHIFT_NUM_SHIFTS_MSK) |
                          (pos ? ALTERA_PLL_C10_SHIFT_UP_DOWN_POS_VAL
                               : ALTERA_PLL_C10_SHIFT_UP_DOWN_NEG_VAL));
        LOG(logDEBUG1,
            ("\t[addr:0x%x, phaseTodo:%d phaseleft:%d phase word:0x%08x]\n",
             addr, phaseToDo, phase, value));
        bus_w_csp1(addr, value);

        ALTERA_PLL_C10_Reconfigure(pllIndex);
        phase -= phaseToDo;
    }
}

void ALTERA_PLL_C10_SetOuputClockDivider(int pllIndex, int clkIndex,
                                         int value) {
    LOG(logDEBUG1, ("\tC%d: Setting output clock divider for pll%d to %d\n",
                    clkIndex, pllIndex, value));

    // assume 50% duty cycle
    uint32_t low_count = value / 2;
    uint32_t high_count = low_count;
    uint32_t odd_division = 0;

    // odd division
    if (value > (int)(2 * low_count)) {
        ++high_count;
        odd_division = 1;
    }
    LOG(logINFO, ("\tC%d: Low:%d, High:%d, Odd:%d\n", clkIndex, low_count,
                  high_count, odd_division));

    // command to set output frequency
    uint32_t addr = ALTERA_PLL_C10_BaseAddress[pllIndex] +
                    (ALTERA_PLL_C10_C_COUNTER_BASE_REG + (int)clkIndex) *
                        ALTERA_PLL_C10_Reg_offset;
    uint32_t val = (((low_count << ALTERA_PLL_C10_C_COUNTER_LW_CNT_OFST) &
                     ALTERA_PLL_C10_C_COUNTER_LW_CNT_MSK) |
                    ((high_count << ALTERA_PLL_C10_C_COUNTER_HGH_CNT_OFST) &
                     ALTERA_PLL_C10_C_COUNTER_HGH_CNT_MSK) |
                    ((odd_division << ALTERA_PLL_C10_C_COUNTER_ODD_DVSN_OFST) &
                     ALTERA_PLL_C10_C_COUNTER_ODD_DVSN_MSK));
    LOG(logDEBUG1, ("\t[addr:0x%x, word:0x%08x]\n", addr, val));

    // write frequency
    bus_w_csp1(addr, val);

    ALTERA_PLL_C10_Reconfigure(pllIndex);

    // reset required to keep the phase relationships
    ALTERA_PLL_C10_ResetPLL(pllIndex);
}
