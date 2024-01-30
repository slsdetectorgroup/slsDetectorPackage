// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once

#include <inttypes.h>

/**
 * Set Defines
 * @param hardMaxV maximum hardware limit
 * @param driverfname driver file name
 * @param numdacs number of dacs
 */
void LTC2620_D_SetDefines(int hardMaxV, char *driverfname, int numdacs, int numdevices, int startingDeviceIndex);

int LTC2620_D_GetMaxNumSteps();
int LTC2620_D_GetPowerDownValue();

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

/**
 * Set value
 * @param dacnum dac index
 * @param val value to set
 * @param mV 1 for mv, else 0
 * @paam dacname dac name
 * @param dacval pointer to dac value
 * @return OK or FAIL
 */
int LTC2620_D_SetDACValue(int dacnum, int val, int mV, char *dacname,
                          int *dacval);