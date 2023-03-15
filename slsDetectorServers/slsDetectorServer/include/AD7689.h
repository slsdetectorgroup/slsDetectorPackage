// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once

#include <inttypes.h>

/**
 * Set Defines
 * @param reg spi register
 * @param roreg spi readout register
 * @param cmsk conversion mask
 * @param clkmsk clock output mask
 * @param dmsk digital output mask
 * @param dofst digital output offset
 */
void AD7689_SetDefines(uint32_t reg, uint32_t roreg, uint32_t cmsk,
                       uint32_t clkmsk, uint32_t dmsk, int dofst);

/**
 * Get temperature
 * @returns temperature in °C
 */
int AD7689_GetTemperature();

/**
 * Reads channels voltage
 * @param ichan channel number from 0 to 7
 * @returns channel voltage in mV
 */
int AD7689_GetChannel(int ichan);

/**
 * Configure
 */
void AD7689_Configure();

/**
 * Disable SPI
 */
void AD7689_Disable();

/**
 * Set SPI reg value
 * @param codata value to be set
 */
void AD7689_Set(uint32_t codata);

/**
 * Get SPI reg value
 * @returns SPI reg value
 */
uint16_t AD7689_Get();