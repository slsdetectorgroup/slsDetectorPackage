// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "commonServerFunctions.h"
#include "blackfin.h"
#include "clogger.h"

#include <unistd.h> // usleep

void SPIChipSelect(uint32_t *valw, uint32_t addr, uint32_t csmask,
                   uint32_t clkmask, uint32_t digoutmask, int convBit) {
    LOG(logDEBUG2, ("SPI chip select. valw:0x%08x addr:0x%x csmask:0x%x, "
                    "clkmask:0x%x digmask:0x%x convbit:%d\n",
                    *valw, addr, csmask, clkmask, digoutmask, convBit));

    // start point
    if (convBit) {
        // needed for the slow adcs for apprx 20 ns before and after rising of
        // convbit (usleep val is vague assumption)
        usleep(20);
        // clkmask has to be down for conversion to have correct value (for conv
        // bit = 1)
        (*valw) = (((bus_r(addr) | csmask) & (~clkmask)) & (~digoutmask));
    } else {
        (*valw) = ((bus_r(addr) | csmask | clkmask) & (~digoutmask));
    }
    bus_w(addr, (*valw));
    LOG(logDEBUG2, ("startpoint. valw:0x%08x\n", *valw));

    // needed for the slow adcs for apprx 10 ns  before and after rising of
    // convbit (usleep val is vague assumption)
    if (convBit)
        usleep(10);

    // chip sel bar down
    (*valw) &= ~csmask;
    bus_w(addr, (*valw));
    LOG(logDEBUG2, ("chip sel bar down. valw:0x%08x\n", *valw));
}

void SPIChipDeselect(uint32_t *valw, uint32_t addr, uint32_t csmask,
                     uint32_t clkmask, uint32_t digoutmask, int convBit) {
    LOG(logDEBUG2, ("SPI chip deselect. valw:0x%08x addr:0x%x csmask:0x%x, "
                    "clkmask:0x%x digmask:0x%x convbit:%d\n",
                    *valw, addr, csmask, clkmask, digoutmask, convBit));

    // needed for the slow adcs for apprx 20 ns before and after rising of
    // convbit (usleep val is vague assumption)
    if (convBit)
        usleep(20);

    // chip sel bar up
    (*valw) |= csmask;
    bus_w(addr, (*valw));
    LOG(logDEBUG2, ("chip sel bar up. valw:0x%08x\n", *valw));

    // needed for the slow adcs for apprx 10 ns  before and after rising of
    // convbit (usleep val is vague assumption)
    if (convBit)
        usleep(10);

    // clk down
    (*valw) &= ~clkmask;
    bus_w(addr, (*valw));
    LOG(logDEBUG2, ("clk down. valw:0x%08x\n", *valw));

    // stop point = start point of course
    (*valw) &= ~digoutmask;
    // slow adcs use convBit (has to go high and then low) instead of csmask
    if (convBit) {
        (*valw) &= ~csmask;
    } else {
        (*valw) |= csmask;
    }
    bus_w(
        addr,
        (*valw)); // FIXME: for ctb slow adcs, might need to set it to low again
    LOG(logDEBUG2, ("stop point. valw:0x%08x\n", *valw));
}

void sendDataToSPI(uint32_t *valw, uint32_t addr, uint32_t val,
                   int numbitstosend, uint32_t clkmask, uint32_t digoutmask,
                   int digofset) {
    LOG(logDEBUG2,
        ("SPI send data. valw:0x%08x addr:0x%x val:0x%x, numbitstosend:%d, "
         "clkmask:0x%x digmask:0x%x digofst:%d\n",
         *valw, addr, val, numbitstosend, clkmask, digoutmask, digofset));

    for (int i = 0; i < numbitstosend; ++i) {

        // clk down
        (*valw) &= ~clkmask;
        bus_w(addr, (*valw));
        LOG(logDEBUG2, ("clk down. valw:0x%08x\n", *valw));

        // write data (i)
        (*valw) = (((*valw) & ~digoutmask) + // unset bit
                   (((val >> (numbitstosend - 1 - i)) & 0x1)
                    << digofset)); // each bit from val starting from msb
        bus_w(addr, (*valw));
        LOG(logDEBUG2, ("write data %d. valw:0x%08x\n", i, *valw));

        // clk up
        (*valw) |= clkmask;
        bus_w(addr, (*valw));
        LOG(logDEBUG2, ("clk up. valw:0x%08x\n", *valw));
    }
}

uint32_t receiveDataFromSPI(uint32_t *valw, uint32_t addr, int numbitstoreceive,
                            uint32_t clkmask, uint32_t readaddr) {
    LOG(logDEBUG2, ("SPI send data. valw:0x%08x addr:0x%x numbitstoreceive:%d, "
                    "clkmask:0x%x readaddr:0x%x \n",
                    *valw, addr, numbitstoreceive, clkmask, readaddr));

    uint32_t retval = 0;
    for (int i = 0; i < numbitstoreceive; ++i) {

        // clk down
        (*valw) &= ~clkmask;
        bus_w(addr, (*valw));
        LOG(logDEBUG2, ("clk down. valw:0x%08x\n", *valw));

        // read data (i)
        retval |= ((bus_r(readaddr) & 0x1) << (numbitstoreceive - 1 - i));
        LOG(logDEBUG2, ("read data %d. retval:0x%08x\n", i, retval));

        usleep(20);

        // clk up
        (*valw) |= clkmask;
        bus_w(addr, (*valw));
        LOG(logDEBUG2, ("clk up. valw:0x%08x\n", *valw));

        usleep(20);
    }

    return retval;
}

void serializeToSPI(uint32_t addr, uint32_t val, uint32_t csmask,
                    int numbitstosend, uint32_t clkmask, uint32_t digoutmask,
                    int digofset, int convBit) {
    if (numbitstosend == 16) {
        LOG(logDEBUG2, ("Writing to SPI Register: 0x%04x\n", val));
    } else {
        LOG(logDEBUG2, ("Writing to SPI Register: 0x%08x\n", val));
    }
    uint32_t valw;

    SPIChipSelect(&valw, addr, csmask, clkmask, digoutmask, convBit);

    sendDataToSPI(&valw, addr, val, numbitstosend, clkmask, digoutmask,
                  digofset);

    SPIChipDeselect(&valw, addr, csmask, clkmask, digoutmask, convBit);
}

uint32_t serializeFromSPI(uint32_t addr, uint32_t csmask, int numbitstoreceive,
                          uint32_t clkmask, uint32_t digoutmask,
                          uint32_t readaddr, int convBit) {

    uint32_t valw;

    SPIChipSelect(&valw, addr, csmask, clkmask, digoutmask, convBit);

    uint32_t retval =
        receiveDataFromSPI(&valw, addr, numbitstoreceive, clkmask, readaddr);

    // not needed for conv bit (not a chip select)
    // SPIChipDeselect(&valw, addr, csmask, clkmask, digoutmask, convBit); //
    // moving this before bringin up earlier changes temp of slow adc

    if (numbitstoreceive == 16) {
        LOG(logDEBUG2, ("Read From SPI Register: 0x%04x\n", retval));
    } else {
        LOG(logDEBUG2, ("Read From SPI Register: 0x%08x\n", retval));
    }
    return retval;
}
