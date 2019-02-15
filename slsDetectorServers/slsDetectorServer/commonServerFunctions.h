#pragma once

#include "blackfin.h"

void SPIChipSelect (uint32_t* valw, uint32_t addr,  uint32_t csmask, uint32_t clkmask, uint32_t digoutmask) {
    FILE_LOG(logDEBUG2, ("SPI chip select. valw:0x%08x addr:0x%x csmask:0x%x, clkmask:0x%x digmask:0x%x\n",
            *valw, addr, csmask, clkmask, digoutmask));

    // start point
    (*valw) = ((bus_r(addr) | csmask | clkmask) &(~digoutmask));
    bus_w (addr, (*valw));
    FILE_LOG(logDEBUG2, ("startpoint. valw:0x%08x\n", *valw));

    // chip sel bar down
    (*valw) &= ~csmask;
    bus_w (addr, (*valw));
    FILE_LOG(logDEBUG2, ("chip sel bar down. valw:0x%08x\n", *valw));
}


void SPIChipDeselect (uint32_t* valw, uint32_t addr,  uint32_t csmask, uint32_t clkmask) {
    FILE_LOG(logDEBUG2, ("SPI chip deselect. valw:0x%08x addr:0x%x csmask:0x%x, clkmask:0x%x\n",
            *valw, addr, csmask, clkmask));

    // chip sel bar up
    (*valw) |= csmask;
    bus_w (addr, (*valw));
    FILE_LOG(logDEBUG2, ("chip sel bar up. valw:0x%08x\n", *valw));

    //clk down
    (*valw) &= ~clkmask;
    bus_w (addr, (*valw));
    FILE_LOG(logDEBUG2, ("clk down. valw:0x%08x\n", *valw));

    // stop point = start point of course
    (*valw) |= csmask;
    bus_w (addr, (*valw)); //FIXME: for ctb slow adcs, might need to set it to low again
    FILE_LOG(logDEBUG2, ("stop point. valw:0x%08x\n", *valw));
}

void sendDataToSPI (uint32_t* valw, uint32_t addr, uint32_t val, int numbitstosend, uint32_t clkmask, uint32_t digoutmask, int digofset) {
    FILE_LOG(logDEBUG2, ("SPI send data. valw:0x%08x addr:0x%x val:0x%x, numbitstosend:%d, clkmask:0x%x digmask:0x%x digofst:%d\n",
            *valw, addr, val, numbitstosend, clkmask, digoutmask, digofset));

    int i = 0;
    for (i = 0; i < numbitstosend; ++i) {

        // clk down
        (*valw) &= ~clkmask;
        bus_w (addr, (*valw));
        FILE_LOG(logDEBUG2, ("clk down. valw:0x%08x\n", *valw));

        // write data (i)
        (*valw) = (((*valw) & ~digoutmask) +                                      // unset bit
                (((val >> (numbitstosend - 1 - i)) & 0x1) << digofset));    // each bit from val starting from msb
        bus_w (addr, (*valw));
        FILE_LOG(logDEBUG2, ("write data %d. valw:0x%08x\n", i, *valw));

        // clk up
        (*valw) |= clkmask ;
        bus_w (addr, (*valw));
        FILE_LOG(logDEBUG2, ("clk up. valw:0x%08x\n", *valw));
    }
}

uint32_t receiveDataFromSPI (uint32_t* valw, uint32_t addr, int numbitstoreceive, uint32_t clkmask, uint32_t readaddr) {
    FILE_LOG(logDEBUG2, ("SPI send data. valw:0x%08x addr:0x%x numbitstoreceive:%d, clkmask:0x%x readaddr:0x%x \n",
            *valw, addr, numbitstoreceive, clkmask, readaddr));

    uint32_t retval = 0;
    int i = 0;
    for (i = 0; i < numbitstoreceive; ++i) {

        // clk down
        (*valw) &= ~clkmask;
        bus_w (addr, (*valw));
        FILE_LOG(logDEBUG2, ("clk down. valw:0x%08x\n", *valw));

        // read data (i)
        retval |= ((bus_r(readaddr) & 0x1) << (numbitstoreceive - 1 - i));
        FILE_LOG(logDEBUG2, ("read data %d. retval:0x%08x\n", i, retval));

        // clk up
        (*valw) |= clkmask ;
        bus_w (addr, (*valw));
        FILE_LOG(logDEBUG2, ("clk up. valw:0x%08x\n", *valw));
    }
    return retval;
}

void serializeToSPI(uint32_t addr, uint32_t val, uint32_t csmask, int numbitstosend, uint32_t clkmask, uint32_t digoutmask, int digofset) {
    if (numbitstosend == 16) {
        FILE_LOG(logDEBUG2, ("Writing to SPI Register: 0x%04x\n", val));
    } else {
        FILE_LOG(logDEBUG2, ("Writing to SPI Register: 0x%08x\n", val));
    }
    uint32_t valw;

    SPIChipSelect (&valw, addr, csmask, clkmask, digoutmask);

    sendDataToSPI(&valw, addr, val, numbitstosend, clkmask, digoutmask, digofset);

    SPIChipDeselect(&valw, addr, csmask, clkmask);
}

uint32_t serializeFromSPI(uint32_t addr, uint32_t csmask, int numbitstoreceive, uint32_t clkmask, uint32_t digoutmask, uint32_t readaddr) {

    uint32_t valw;

    SPIChipSelect (&valw, addr, csmask, clkmask, digoutmask);

    uint32_t retval = receiveDataFromSPI(&valw, addr, numbitstoreceive, clkmask, readaddr);

    SPIChipDeselect(&valw, addr, csmask, clkmask);

    if (numbitstoreceive == 16) {
        FILE_LOG(logDEBUG2, ("Read From SPI Register: 0x%04x\n", retval));
    } else {
        FILE_LOG(logDEBUG2, ("Read From SPI Register: 0x%08x\n", retval));
    }
    return retval;
}
