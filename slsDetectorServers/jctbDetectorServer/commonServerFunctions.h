#ifndef COMMON_SERVER_FUNCTIONS_H
#define COMMON_SERVER_FUNCTIONS_H

#ifndef GOTTHARDD	//gotthard already had bus_w etc defined in its firmware_funcs.c (not yet made with common files)
#include "blackfin.h"
#endif

/* global variables */

void SPIChipSelect (u_int32_t* valw, u_int32_t addr,  u_int32_t csmask) {

    // start point
    (*valw) = 0xffffffff;          // old board compatibility (not using specific bits)
    bus_w (addr, (*valw));

    // chip sel bar down
    (*valw) &= ~csmask;            /* todo with test: done a bit different, not with previous value */
    bus_w (addr, (*valw));
}


void SPIChipDeselect (u_int32_t* valw, u_int32_t addr,  u_int32_t csmask, u_int32_t clkmask) {
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

void sendDataToSPI (u_int32_t* valw, u_int32_t addr, u_int32_t val, int numbitstosend, u_int32_t clkmask, u_int32_t digoutmask, int digofset) {
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


void serializeToSPI(u_int32_t addr, u_int32_t val, u_int32_t csmask, int numbitstosend, u_int32_t clkmask, u_int32_t digoutmask, int digofset) {
#ifdef VERBOSE
    if (numbitstosend == 16)
        printf("Writing to SPI Register: 0x%04x\n",val);
    else
        printf("Writing to SPI Register: 0x%08x\n", val);
#endif

    u_int32_t valw;

    SPIChipSelect (&valw, addr, csmask);

    sendDataToSPI(&valw, addr, val, numbitstosend, clkmask, digoutmask, digofset);

    SPIChipDeselect(&valw, addr, csmask, clkmask);
}

#endif	//COMMON_SERVER_FUNCTIONS_H
