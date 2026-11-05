// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once

#include <inttypes.h>
#include <sys/types.h>

/** csp0base */
void bus_w(u_int32_t offset, u_int32_t data);
u_int32_t bus_r(u_int32_t offset);

/** udp header */
void bus_w_csp1(u_int32_t offset, u_int32_t data);
u_int32_t bus_r_csp1(u_int32_t offset);

void bus_w_csp2(u_int32_t offset, u_int32_t data);
u_int32_t bus_r_csp2(u_int32_t offset);

/**
 * Force FPGA writes to be aligned and in order
 * by writing 32 bit words to the bus (csp1)
 * @returns FAIL if size is not a non-zero multiple of sizeof(uint32_t), else OK
 */
int bus_w_csp1_block(const void *src, size_t size);

uint64_t getU64BitReg(int aLSB, int aMSB);
void setU64BitReg(uint64_t value, int aLSB, int aMSB);

/**  literal register address value provided by client */
u_int32_t readRegister(u_int32_t offset);
u_int32_t writeRegister(u_int32_t offset, u_int32_t data);

int mapCSP0(void);
