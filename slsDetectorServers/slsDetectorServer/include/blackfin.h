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

/**
 * Write into a 16 bit register
 * @param offset address offset
 * @param data 16 bit data
 */
void bus_w16(u_int32_t offset, u_int16_t data);

/**
 * Read from a 16 bit register
 * @param offset address offset
 * @retuns 16 bit data read
 */
u_int16_t bus_r16(u_int32_t offset);

/**
 * Write into a 32 bit register
 * @param offset address offset
 * @param data 32 bit data
 */
void bus_w(u_int32_t offset, u_int32_t data);

/**
 * Read from a 32 bit register
 * @param offset address offset
 * @retuns 32 bit data read
 */
u_int32_t bus_r(u_int32_t offset);

/**
 * Read from a 64 bit register
 * @param aLSB LSB offset address
 * @param aMSB MSB offset address
 * @returns 64 bit data read
 */
int64_t get64BitReg(int aLSB, int aMSB);

/**
 * Write into a 64 bit register
 * @param value 64 bit data
 * @param aLSB LSB offset address
 * @param aMSB MSB offset address
 * @returns 64 bit data read
 */
int64_t set64BitReg(int64_t value, int aLSB, int aMSB);

/**
 * Read unsigned 64 bit from a 64 bit register
 * @param aLSB LSB offset address
 * @param aMSB MSB offset address
 * @returns unsigned 64 bit data read
 */
uint64_t getU64BitReg(int aLSB, int aMSB);

/**
 * Write unsigned 64 bit into a 64 bit register
 * @param value unsigned 64 bit data
 * @param aLSB LSB offset address
 * @param aMSB MSB offset address
 */
void setU64BitReg(uint64_t value, int aLSB, int aMSB);

/**
 * Read from a 32 bit register (literal register value provided by client)
 * @param offset address offset
 * @retuns 32 bit data read
 */
u_int32_t readRegister(u_int32_t offset);

/**
 * Write into a 32 bit register (literal register value provided by client)
 * @param offset address offset
 * @param data 32 bit data
 */
void writeRegister(u_int32_t offset, u_int32_t data);

/**
 * Read from a 16 bit register (literal register value provided by client)
 * @param offset address offset
 * @retuns 16 bit data read
 */
u_int32_t readRegister16(u_int32_t offset);

/**
 * Write into a 16 bit register (literal register value provided by client)
 * @param offset address offset
 * @param data 16 bit data
 */
void writeRegister16(u_int32_t offset, u_int32_t data);

/**
 * Get base address for memory copy
 */
uint32_t *Blackfin_getBaseAddress();
/**
 * Map FPGA
 */
int mapCSP0(void);
