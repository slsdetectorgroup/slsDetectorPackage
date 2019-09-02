#pragma once

#include <inttypes.h>

/**
 * Set Defines
 * @param dofst digital output offset
 * @param softMaxV maximum software limit (permissible for this detector)
 * @param hardMaxV maximum hardware limit 
 * @param driverfname driver file name
 */
void DAC6571_SetDefines(int softMaxV, int hardMaxV, char* driverfname);

/**
 * Set value
 * @param val value to set
 * @return OK or FAIL
 */
int DAC6571_Set (int val) ;



