// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once

#include <inttypes.h>
#include <sys/types.h>

void bus_w(u_int32_t offset, u_int32_t data);
u_int32_t bus_r(u_int32_t offset);
uint64_t getU64BitReg(int aLSB, int aMSB);
void setU64BitReg(uint64_t value, int aLSB, int aMSB);
u_int32_t readRegister(u_int32_t offset);
u_int32_t writeRegister(u_int32_t offset, u_int32_t data);
int mapCSP0(void);
u_int32_t *Arm_getUDPBaseAddress();
