// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package

#include <stdbool.h>
#include <stdint.h> 

void XILINX_PLL_setFrequency(uint32_t clkIDX, uint32_t freq);
uint32_t XILINX_PLL_getFrequency(uint32_t clkIDX);
bool XILINX_PLL_isLocked();
void XILINX_PLL_reset();
void XILINX_PLL_waitForLock();
void XILINX_PLL_load();