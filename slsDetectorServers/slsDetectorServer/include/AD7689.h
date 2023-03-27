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

/** @returns temperature in °C */
int AD7689_GetTemperature();

/** @returns channel voltage in mV */
int AD7689_GetChannel(int ichan);

void AD7689_Configure();
void AD7689_ConvPulse();
void AD7689_Set(uint32_t codata);
uint32_t AD7689_Get();