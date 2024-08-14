// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once

#include <inttypes.h>

void DAC6571_SetDefines(int hardMaxV, char *driverfname);
int DAC6571_Set(int val);
int DAC6571_Get(int *retval);
