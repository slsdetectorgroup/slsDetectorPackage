// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once

#include <inttypes.h>

/**
 * Configure the I2C core and Enable core
 * @param rOhm shunt resister value in Ohms (defined in
 * slsDetectorServer_defs.h)
 * @param creg control register (defined in RegisterDefs.h)
 * @param sreg status register (defined in RegisterDefs.h)
 * @param rreg rx data fifo register (defined in RegisterDefs.h)
 * @param rlvlreg rx data fifo level register (defined in RegisterDefs.h)
 * @param slreg scl low count register (defined in RegisterDefs.h)
 * @param shreg scl high count register (defined in RegisterDefs.h)
 * @param sdreg sda hold register (defined in RegisterDefs.h)
 * @param treg transfer command fifo register (defined in RegisterDefs.h)
 */
void INA226_ConfigureI2CCore(double rOhm, uint32_t creg, uint32_t sreg,
                             uint32_t rreg, uint32_t rlvlreg, uint32_t slreg,
                             uint32_t shreg, uint32_t sdreg, uint32_t treg);

/**
 * Calibrate resolution of current register
 * @param deviceId device Id (defined in slsDetectorServer_defs.h)
 */
void INA226_CalibrateCurrentRegister(uint32_t deviceId);

/**
 * Read voltage of device
 * @param deviceId device Id
 * @returns voltage in mV
 */
int INA226_ReadVoltage(uint32_t deviceId);

/**
 * Read current
 * @param deviceId device Id
 * @returns current in mA
 */
int INA226_ReadCurrent(uint32_t deviceId);