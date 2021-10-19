// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once

#include <inttypes.h>

/**
 * Set defines
 * @param regofst register offset
 * @param baseaddr0 base address of pll 0
 * @param baseaddr1 base address of pll 1
 * @param resetreg0 reset register of pll 0
 * @param resetreg1 reset register of pll 1
 * @param resetmsk reset mask of pll 0
 * @param resetms1 reset mask of pll 1
 * @param vcofreq0 vco frequency of pll 0
 * @param vcofreq1 vco frequency of pll 1
 */
void ALTERA_PLL_C10_SetDefines(int regofst, uint32_t baseaddr0,
                               uint32_t baseaddr1, uint32_t resetreg,
                               uint32_t resetmsk0, uint32_t resetmsk1,
#ifdef MYTHEN3D
                               uint32_t statusreg, uint32_t statusmsk0,
                               uint32_t statusmsk1,
#endif
                               int vcofreq0, int vcofreq1);

/**
 * Get Max Clock Divider
 */
int ALTERA_PLL_C10_GetMaxClockDivider();

/**
 * Get VCO frequency based on pll Index
 * @param pllIndex pll index
 * @returns VCO frequency
 */
int ALTERA_PLL_C10_GetVCOFrequency(int pllIndex);

/**
 * Get maximum phase shift steps of vco frequency
 * @returns max phase shift steps of vco
 */
int ALTERA_PLL_C10_GetMaxPhaseShiftStepsofVCO();

/**
 * Start reconfiguration
 * @param pllIndex pll index
 */
void ALTERA_PLL_C10_Reconfigure(int pllIndex);

/**
 * Reset pll
 * @param pllIndex pll index
 */
void ALTERA_PLL_C10_ResetPLL(int pllIndex);

/**
 * Set Phase Shift
 * @param pllIndex pll index
 * @param clkIndex clock index
 * @param phase phase shift
 * @param pos 1 if up down direction of shift is positive, else 0
 */
void ALTERA_PLL_C10_SetPhaseShift(int pllIndex, int clkIndex, int phase,
                                  int pos);

/**
 * Calculate and write output frequency
 * @param pllIndex pll index
 * @param clkIndex clock index
 * @param value clock divider to set to
 */
void ALTERA_PLL_C10_SetOuputClockDivider(int pllIndex, int clkIndex, int value);
