// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once

#include <inttypes.h>
#include <stdbool.h>
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
 * Write `size` bytes from `src` to the FPGA as 32-bit words,
 * one at a time and in order.
 * @param fixedAddress true: every word goes to `offset` (e.g. a FIFO).
 *                     false: words go to consecutive addresses from `offset`.
 * @returns OK, or FAIL if `size` is 0 or not a multiple of 4 bytes.
 */
int bus_w_block(u_int32_t offset, const void *src, size_t size,
                bool fixedAddress);
int bus_r_block(u_int32_t offset, void *dest, size_t size, bool fixedAddress);

/**
 * Force FPGA writes to be aligned and in order
 * by writing 32 bit words to the bus (csp1: udp header)
 * @returns FAIL if size is not a non-zero multiple of sizeof(uint32_t), else OK
 */
int bus_w_csp1_block(const void *src, size_t size);

uint64_t getU64BitReg(int aLSB, int aMSB);
void setU64BitReg(uint64_t value, int aLSB, int aMSB);

/**  literal register address value provided by client */
u_int32_t readRegister(u_int32_t offset);
u_int32_t writeRegister(u_int32_t offset, u_int32_t data);

int mapCSP0(void);
