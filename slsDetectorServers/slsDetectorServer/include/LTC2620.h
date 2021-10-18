// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once

#include <inttypes.h>

/**
 * Set Defines
 * @param reg spi register
 * @param cmsk chip select mask
 * @param clkmsk clock output mask
 * @param dmsk digital output mask
 * @param dofst digital output offset
 * @param nd total number of dacs for this board (for dac channel and daisy
 * chain chip id)
 * @param minMV minimum voltage determined by hardware
 * @param maxMV maximum voltage determined by hardware
 */
void LTC2620_SetDefines(uint32_t reg, uint32_t cmsk, uint32_t clkmsk,
                        uint32_t dmsk, int dofst, int nd, int minMV, int maxMV);

/**
 * Disable SPI
 */
void LTC2620_Disable();

/**
 * Get power down value
 */
int LTC2620_GetPowerDownValue();

/**
 * Get minimum input value for dac
 */
int LTC2620_GetMinInput();

/**
 * Get maximum input value for dac
 */
int LTC2620_GetMaxInput();

/**
 * Get max number of steps
 */
int LTC2620_GetMaxNumSteps();

/**
 * Convert voltage to dac units
 * @param voltage value in mv
 * @param dacval pointer to value converted to dac units
 * @returns FAIL when voltage outside limits, OK if conversion successful
 */
int LTC2620_VoltageToDac(int voltage, int *dacval);

/**
 * Convert dac units to voltage
 * @param dacval dac units
 * @param voltage pointer to value converted to mV
 * @returns FAIL when voltage outside limits, OK if conversion successful
 */
int LTC2620_DacToVoltage(int dacval, int *voltage);

/**
 * Set a single chip (all non ctb detectors use this)
 * when max dac is 8
 * @param cmd command
 * @param data dac value to be set
 * @param dacaddr dac channel number in chip
 */
void LTC2620_SetSingle(int cmd, int data, int dacaddr);

/**
 * bit bang the data into all the chips daisy fashion
 * @param valw current value of register while bit banging
 * @param val data to be sent (data, dac addr and command)
 */
void LTC2620_SendDaisyData(uint32_t *valw, uint32_t val);

/**
 * Set a single chip (all non ctb detectors use this)
 * when max dac is 8
 * @param cmd command
 * @param data dac value to be set
 * @param dacaddr dac channel number in chip
 * @param chipIndex index of the chip
 */
void LTC2620_SetDaisy(int cmd, int data, int dacaddr, int chipIndex);

/**
 * Sets a single chip (LTC2620_SetSingle) or multiple chip (LTC2620_SetDaisy)
 * multiple chip is only for ctb where the multiple chips are connected in daisy
 * fashion
 * @param cmd command to send
 * @param data dac value to be set
 * @param dacaddr dac channel number for the chip
 * @param chipIndex the chip to be set
 */
void LTC2620_Set(int cmd, int data, int dacaddr, int chipIndex);

/**
 * Configure (obtains dacaddr, command and ichip and calls LTC2620_Set)
 */
void LTC2620_Configure();

/**
 * Set Dac (obtains dacaddr, command and ichip and calls LTC2620_Set)
 * @param dacnum dac number
 * @param data dac value to set
 */
void LTC2620_SetDAC(int dacnum, int data);

/**
 * Set dac in dac units or mV
 * @param dacnum dac index
 * @param val value in dac units or mV
 * @param mV 0 for dac units and 1 for mV unit
 * @param dacval pointer to value in dac units
 * @returns OK or FAIL for success of operation
 */
int LTC2620_SetDACValue(int dacnum, int val, int mV, int *dacval);