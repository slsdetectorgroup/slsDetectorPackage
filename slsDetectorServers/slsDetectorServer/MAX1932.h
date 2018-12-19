#pragma once

#include "commonServerFunctions.h" // blackfin.h, ansi.h

/* MAX1932 HV DEFINES */

#define MAX1932_HV_NUMBITS          (8)
#define MAX1932_HV_DATA_OFST        (0)
#define MAX1932_HV_DATA_MSK         (0x000000FF << MAX1932_HV_DATA_OFST)

// on power up, dac = 0xff (1.25)

uint32_t MAX1932_Reg = 0x0;
uint32_t MAX1932_CsMask = 0x0;
uint32_t MAX1932_ClkMask = 0x0;
uint32_t MAX1932_DigMask = 0x0;
int MAX1932_DigOffset = 0x0;

/**
 * Set Defines
 * @param reg spi register
 * @param cmsk chip select mask
 * @param clkmsk clock output mask
 * @param dmsk digital output mask
 * @param dofst digital output offset
 */
void MAX1932_SetDefines(uint32_t reg, uint32_t cmsk, uint32_t clkmsk, uint32_t dmsk, int dofst) {
    MAX1932_Reg = reg;
    MAX1932_CsMask = cmsk;
    MAX1932_ClkMask = clkmsk;
    MAX1932_DigMask = dmsk;
    MAX1932_DigOffset = dofst;
}

/**
 * Disable SPI
 */
void MAX1932_Disable() {
    bus_w(MAX1932_Reg, bus_r(MAX1932_Reg)
            | MAX1932_CsMask
            | MAX1932_ClkMask
            &~(MAX1932_DigMask));
}

/**
 * Configure
 */
void MAX1932_Configure(){
    FILE_LOG(logINFOBLUE, ("Configuring MAX1932\n"));

}


/**
 * Set value
 * @param val value to set
 */
void MAX1932_Set (int val) {
    FILE_LOG(logDEBUG1, ("\tSetting high voltage to %d\n", val));

}



