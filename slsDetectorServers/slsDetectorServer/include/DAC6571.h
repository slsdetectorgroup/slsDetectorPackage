// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once

#include <inttypes.h>

/**
 * Set Defines
 * @param hardMaxV maximum hardware limit
 * @param driverfname driver file name
 */
void DAC6571_SetDefines(int hardMaxV, char *driverfname);

/**
 * Set value
 * @param val value to set
 * @return OK or FAIL
 */
int DAC6571_Set(int val);
