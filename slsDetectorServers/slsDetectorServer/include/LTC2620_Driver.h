// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once

#include <inttypes.h>

void LTC2620_D_SetDefines(int hardMinV, int hardMaxV, char *driverfname,
                          int numdacs, int numpowers);
int LTC2620_D_GetMaxNumSteps();
int LTC2620_D_GetPowerDownValue();
int LTC2620_D_GetMinInput();
int LTC2620_D_GetMaxInput();

/**
 * Convert voltage to dac units
 * @param voltage value in mv
 * @param dacval pointer to value converted to dac units
 * @returns FAIL when voltage outside limits, OK if conversion successful
 */
int LTC2620_D_VoltageToDac(int voltage, int *dacval);

/**
 * Convert dac units to voltage
 * @param dacval dac units
 * @param voltage pointer to value converted to mV
 * @returns FAIL when voltage outside limits, OK if conversion successful
 */
int LTC2620_D_DacToVoltage(int dacval, int *voltage);

/** for all dacs including power regulators to write dac value to file */
int LTC2620_D_WriteDACValue(int dacnum, int dacvalue, char *dacname);

/**
 * Set value for dac only
 * @param dacval if val is in mV, returns dac units set
 */
int LTC2620_D_SetDACValue(int dacnum, int val, int mV, char *dacname,
                          int *dacval);