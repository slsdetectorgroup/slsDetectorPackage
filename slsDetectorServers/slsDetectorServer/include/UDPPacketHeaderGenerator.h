// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once

#include <inttypes.h>

uint32_t getUDPPacketNumber();
uint64_t getUDPFrameNumber();
void setUDPFrameNumber(uint64_t fnum);
/**
 * @param id module id
 */
void createUDPPacketHeader(char *buffer, uint16_t id);
int fillUDPPacket(char *buffer);
