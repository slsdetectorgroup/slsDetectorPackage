// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once

#include <inttypes.h>

#ifdef JUNGFRAUD
void AD9257_Set_Jungfrau_Hardware_Version_1_0(int val);
#endif

/**
 * Set Defines
 * @param reg spi register
 * @param cmsk chip select mask
 * @param clkmsk clock output mask
 * @param dmsk digital output mask
 * @param dofst digital output offset
 */
void AD9257_SetDefines(uint32_t reg, uint32_t cmsk, uint32_t clkmsk,
                       uint32_t dmsk, int dofst);

/**
 * Disable SPI
 */
void AD9257_Disable();

/**
 * Get vref voltage
 */
int AD9257_GetVrefVoltage(int mV);

/**
 * Set vref voltage
 * @param val voltage to be set (0 for 1.0V, 1 for 1.14V, 2 for 1.33V, 3
 * for 1.6V, 4 for 2.0V
 * @returns ok or fail
 */
int AD9257_SetVrefVoltage(int val, int mV);

/**
 * Set SPI reg value
 * @param codata value to be set
 */
void AD9257_Set(int addr, int val);

/**
 * Configure
 */
void AD9257_Configure();