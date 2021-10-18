// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once

#include <inttypes.h>

/**
 * Configure the I2C core,
 * Enable core and
 * Calibrate the calibration register for current readout
 * @param creg control register (defined in RegisterDefs.h)
 * @param sreg status register (defined in RegisterDefs.h)
 * @param rreg rx data fifo register (defined in RegisterDefs.h)
 * @param rlvlreg rx data fifo level register (defined in RegisterDefs.h)
 * @param slreg scl low count register (defined in RegisterDefs.h)
 * @param shreg scl high count register (defined in RegisterDefs.h)
 * @param sdreg sda hold register (defined in RegisterDefs.h)
 * @param treg transfer command fifo register (defined in RegisterDefs.h)
 */
void I2C_ConfigureI2CCore(uint32_t creg, uint32_t sreg, uint32_t rreg,
                          uint32_t rlvlreg, uint32_t slreg, uint32_t shreg,
                          uint32_t sdreg, uint32_t treg);

/**
 * Read register
 * @param deviceId device Id
 * @param addr register address
 * @returns value read from register
 */
uint32_t I2C_Read(uint32_t devId, uint32_t addr);

/**
 * Write register (16 bit value)
 * @param deviceId device Id
 * @param addr register address
 * @param data data to be written (16 bit)
 */
void I2C_Write(uint32_t devId, uint32_t addr, uint16_t data);
