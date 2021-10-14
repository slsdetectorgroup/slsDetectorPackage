// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once

#include <inttypes.h>

void SPIChipSelect(uint32_t *valw, uint32_t addr, uint32_t csmask,
                   uint32_t clkmask, uint32_t digoutmask, int convBit);

void SPIChipDeselect(uint32_t *valw, uint32_t addr, uint32_t csmask,
                     uint32_t clkmask, uint32_t digoutmask, int convBit);

void sendDataToSPI(uint32_t *valw, uint32_t addr, uint32_t val,
                   int numbitstosend, uint32_t clkmask, uint32_t digoutmask,
                   int digofset);

uint32_t receiveDataFromSPI(uint32_t *valw, uint32_t addr, int numbitstoreceive,
                            uint32_t clkmask, uint32_t readaddr);

void serializeToSPI(uint32_t addr, uint32_t val, uint32_t csmask,
                    int numbitstosend, uint32_t clkmask, uint32_t digoutmask,
                    int digofset, int convBit);

uint32_t serializeFromSPI(uint32_t addr, uint32_t csmask, int numbitstoreceive,
                          uint32_t clkmask, uint32_t digoutmask,
                          uint32_t readaddr, int convBit);
