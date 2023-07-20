// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once

#include <inttypes.h>

#if defined(JUNGFRAUD)
/**
 * Set Defines
 * @param creg control register
 * @param preg parameter register
 * @param rprmsk reconfig parameter reset mask
 * @param wpmsk write parameter mask
 * @param prmsk pll reset mask
 * @param amsk address mask
 * @param aofst address offset
 * @param wd2msk write parameter mask for pll for dbit clock (Jungfrau only)
 * @param clk2Index clkIndex of second pll (Jungfrau only)
 */
void ALTERA_PLL_SetDefines(uint32_t creg, uint32_t preg, uint32_t rprmsk,
                           uint32_t wpmsk, uint32_t prmsk, uint32_t amsk,
                           int aofst, uint32_t wd2msk, int clk2Index);
#else
/**
 * Set Defines
 * @param creg control register
 * @param preg parameter register
 * @param rprmsk reconfig parameter reset mask
 * @param wpmsk write parameter mask
 * @param prmsk pll reset mask
 * @param amsk address mask
 * @param aofst address offset
 */
void ALTERA_PLL_SetDefines(uint32_t creg, uint32_t preg, uint32_t rprmsk,
                           uint32_t wpmsk, uint32_t prmsk, uint32_t amsk,
                           int aofst);
#endif

/**
 * Reset only PLL
 */
void ALTERA_PLL_ResetPLL();

/**
 * Reset PLL Reconfiguration and PLL
 */
void ALTERA_PLL_ResetPLLAndReconfiguration();

/**
 * Set PLL Reconfig register
 * @param reg register
 * @param val value
 * @param useDefaultWRMask only jungfrau for dbit clk (clkindex1, use
 * second WR mask)
 */
void ALTERA_PLL_SetPllReconfigReg(uint32_t reg, uint32_t val,
                                  int useSecondWRMask);

/**
 * Write Phase Shift
 * @param phase phase shift
 * @param clkIndex clock index
 * @param pos 1 if up down direction of shift is positive, else 0
 */
void ALTERA_PLL_SetPhaseShift(int32_t phase, int clkIndex, int pos);

/**
 * Set PLL mode register to polling mode
 */
void ALTERA_PLL_SetModePolling();
/**
 * Calculate and write output frequency
 * @param clkIndex clock index
 * @param pllVCOFreqMhz PLL VCO Frequency in Mhz
 * @param value frequency to set to
 * @param frequency set
 */
int ALTERA_PLL_SetOuputFrequency(int clkIndex, int pllVCOFreqMhz, int value);
