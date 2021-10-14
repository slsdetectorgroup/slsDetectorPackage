// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once

#include <inttypes.h>

/**
 * Set Defines
 * @param driverfname driver file name
 */
void ASIC_Driver_SetDefines(char *driverfname);

/**
 * Set value
 * @param index asic index
 * @param length number of bytes of the buffer
 * @param buffer buffer
 * @return OK or FAIL
 */
int ASIC_Driver_Set(int index, int length, char *buffer);