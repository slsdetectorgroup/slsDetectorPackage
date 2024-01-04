// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once

#include <inttypes.h>
#include <sys/types.h>

int mapCSP0(void);
void bus_w(u_int32_t offset, u_int32_t data);
u_int32_t bus_r(u_int32_t offset);