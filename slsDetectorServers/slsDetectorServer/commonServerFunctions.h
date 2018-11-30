#pragma once

#include "blackfin.h"

void SPIChipSelect (uint32_t* valw, uint32_t addr,  uint32_t csmask) {

    // start point
    (*valw) = 0xffffffff;          // old board compatibility (not using specific bits)
    bus_w (addr, (*valw));

    // chip sel bar down
    (*valw) &= ~csmask;            /* todo with test: done a bit different, not with previous value */
    bus_w (addr, (*valw));
}


void SPIChipDeselect (uint32_t* valw, uint32_t addr,  uint32_t csmask, uint32_t clkmask) {
    // chip sel bar up
    (*valw) |= csmask; /* todo with test: not done for spi */
    bus_w (addr, (*valw));

    //clk down
    (*valw) &= ~clkmask;
    bus_w (addr, (*valw));

    // stop point = start point of course
    (*valw) = 0xffffffff;              // old board compatibility (not using specific bits)
    bus_w (addr, (*valw));
}

void sendDataToSPI (uint32_t* valw, uint32_t addr, uint32_t val, int numbitstosend, uint32_t clkmask, uint32_t digoutmask, int digofset) {
    int i = 0;
    for (i = 0; i < numbitstosend; ++i) {

        // clk down
        (*valw) &= ~clkmask;
        bus_w (addr, (*valw));

        // write data (i)
        (*valw) = (((*valw) & ~digoutmask) +                                      // unset bit
                (((val >> (numbitstosend - 1 - i)) & 0x1) << digofset));    // each bit from val starting from msb
        bus_w (addr, (*valw));

        // clk up
        (*valw) |= clkmask ;
        bus_w (addr, (*valw));
    }
}


void serializeToSPI(uint32_t addr, uint32_t val, uint32_t csmask, int numbitstosend, uint32_t clkmask, uint32_t digoutmask, int digofset) {
    if (numbitstosend == 16) {
        FILE_LOG(logDEBUG1, ("Writing to SPI Register: 0x%04x\n", val));
    } else {
        FILE_LOG(logDEBUG1, ("Writing to SPI Register: 0x%08x\n", val));
    }
    uint32_t valw;

    SPIChipSelect (&valw, addr, csmask);

    sendDataToSPI(&valw, addr, val, numbitstosend, clkmask, digoutmask, digofset);

    SPIChipDeselect(&valw, addr, csmask, clkmask);
}
