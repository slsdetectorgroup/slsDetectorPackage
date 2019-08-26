#pragma once

#include <inttypes.h>

/**
 * Set Defines
 * @param reg spi register
 * @param cmsk chip select mask
 * @param clkmsk clock output mask
 * @param dmsk digital output mask
 * @param dofst digital output offset
 */
void AD9252_SetDefines(uint32_t reg, uint32_t cmsk, uint32_t clkmsk, uint32_t dmsk, int dofst);

/**
 * Disable SPI
 */
void AD9252_Disable();

/**
 * Set SPI reg value
 * @param codata value to be set
 */
void AD9252_Set(int addr, int val);

/**
 * Configure
 */
void AD9252_Configure();
