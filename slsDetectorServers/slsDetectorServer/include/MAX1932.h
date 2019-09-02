#pragma once

#include <inttypes.h>

/**
 * Set Defines
 * @param reg spi register
 * @param cmsk chip select mask
 * @param clkmsk clock output mask
 * @param dmsk digital output mask
 * @param dofst digital output offset
 * @param minMV minimum voltage determined by hardware
 * @param maxMV maximum voltage determined by hardware
 */
void MAX1932_SetDefines(uint32_t reg, uint32_t cmsk, uint32_t clkmsk, uint32_t dmsk, int dofst,
        int minMV, int maxMV);

/**
 * Disable SPI
 */
void MAX1932_Disable();

/**
 * Set value
 * @param val value to set
 * @return OK or FAIL
 */
int MAX1932_Set (int val) ;



