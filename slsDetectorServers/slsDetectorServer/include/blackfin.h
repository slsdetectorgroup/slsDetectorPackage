// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once

#include <inttypes.h>
#include <sys/types.h>

/** enable support for ARDY signal on interface to FPGA
 * needed to properly translate avalon_mm_waitrequest in the CTB firmware
 * https://www.analog.com/media/en/dsp-documentation/processor-manuals/bf537_hwr_Rev3.2.pdf
 * page 274
 * */
#define BFIN_EBIU_AMBCTL1_B2_ARDY_ENA_OFST (0)
#define BFIN_EBIU_AMBCTL1_B2_ARDY_ENA_MSK                                      \
    (1 << BFIN_EBIU_AMBCTL1_B2_ARDY_ENA_OFST)
#define BFIN_EBIU_AMBCTL1_B2_ARDY_POL_OFST (1)
#define BFIN_EBIU_AMBCTL1_B2_ARDY_POL_MSK                                      \
    (1 << BFIN_EBIU_AMBCTL1_B2_ARDY_POL_OFST)

#define BFIN_AMC_ACCESS_EXTENSION_ENA_VAL                                      \
    (BFIN_EBIU_AMBCTL1_B2_ARDY_ENA_MSK | BFIN_EBIU_AMBCTL1_B2_ARDY_POL_MSK)
#define BFIN_AMC_ACCESS_EXTENSION_FNAME                                        \
    "/sys/kernel/debug/blackfin/ebiu_amc/EBIU_AMBCTL1"

/** I2C defines */
#define I2C_CLOCK_MHZ (131.25)

/* 16 bit register */
void bus_w16(u_int32_t offset, u_int16_t data);
u_int16_t bus_r16(u_int32_t offset);

/** 32 bit register */
void bus_w(u_int32_t offset, u_int32_t data);
u_int32_t bus_r(u_int32_t offset);

/**
 * Force FPGA writes to be aligned and in order
 * by writing 32 bit words to the bus
 * @returns FAIL if size is not a non-zero multiple of sizeof(uint32_t), else OK
 */
int bus_w_block(u_int32_t offset, const void *src, size_t size);

/** legacy: to be replaced at some point (returns int64_t via tcp) */
int64_t get64BitReg(int aLSB, int aMSB);
int64_t set64BitReg(int64_t value, int aLSB, int aMSB);

uint64_t getU64BitReg(int aLSB, int aMSB);
void setU64BitReg(uint64_t value, int aLSB, int aMSB);

/**  literal register address value provided by client */
u_int32_t readRegister(u_int32_t offset);
void writeRegister(u_int32_t offset, u_int32_t data);
u_int32_t readRegister16(u_int32_t offset);
void writeRegister16(u_int32_t offset, u_int32_t data);

int mapCSP0(void);
