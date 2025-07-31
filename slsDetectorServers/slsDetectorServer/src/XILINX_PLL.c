// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "XILINX_PLL.h"
#include "arm64.h"
#include <unistd.h>
#include <stdbool.h>
#include "clogger.h"
#include <math.h>

// https://docs.amd.com/r/en-US/pg065-clk-wiz/Register-Space  (simplified, we leave some things away)

#define XILINX_PLL_INPUT_FREQ (100000)          // 100 MHz input clock frequency
#define XILINX_PLL_BASE_ADDR (0x0) 
#define XILINX_PLL_RESET_REG (0x000)            // write 0xA to reset. Nothing else !
#define XILINX_PLL_STATUS_REG (0x004)           // Bit[0] = LOCKED
#define XILINX_PLL_LOCKED_BIT (0)
#define XILINX_PLL_GLOBAL_CONFIG_REG (0x200)    // Bit[7:0] = DIVCLK_DIVIDE, Bit[15:8] = CLKFBOUT_MULT , Bit[25:16] = CLKFBOUT_FRAC Multiply
                                                // The value of CLKFBOUT fractional divide can be from 0 to 875 representing the fractional multiplied by 1000. 
#define XILINX_PLL_NUM_CLKS (7)                 // 7 clocks supported
#define XILINX_PLL_CLK_CONFIG_BASE_ADDR (0x208) // 7 clock config blocks starting here
#define XILINX_PLL_CLK_CONFIG_WIDTH (3)         // 3 words for each clock config block, containing:
#define XILINX_PLL_CLK_DIV_OFFSET (0x0)         // Bit[7:0] = CLKOUT_DIVIDE, Bit[17:8] = CLKOUT0_FRAC, NOTE: fractional part works on IDX 0 only !!! 
#define XILINX_PLL_CLK_PHASE_OFFSET (0x4)       // Phase values entered are Signed Number for +/- phase. 
#define XILINX_PLL_CLK_DUTY_OFFSET (0x8)        // (Duty Cycle in %) * 1000 
#define XILINX_PLL_LOAD_REG (0x25C)             // write 0x3 here to apply changes, 0x1 to return to default values as compiled into firmware

#define XILINX_PLL_MEASURE_BASE_ADDR0 (0x1000)  // this is added externally and not part of the CLKWIZ core
#define XILINX_PLL_MEASURE_BASE_ADDR1 (0x2000)

// freq in kHz !!
void XILINX_PLL_setFrequency(uint32_t clkIDX, uint32_t freq) {
    if (clkIDX >= XILINX_PLL_NUM_CLKS) {
        LOG(logERROR, ("XILINX_PLL: Invalid clock index %d\n", clkIDX));
        return;
    }

    if(freq < 10000 || freq > 250000) {
        LOG(logERROR, ("XILINX_PLL: Frequency out of range\n"));
        return;
    }
    uint32_t global_reg = bus_r_csp2(XILINX_PLL_BASE_ADDR + XILINX_PLL_GLOBAL_CONFIG_REG);
    uint32_t base_clk_freq = ((global_reg >> 8) & 0xFF)* XILINX_PLL_INPUT_FREQ;
    base_clk_freq = base_clk_freq + ((global_reg >> 16) & 0x2FF)* XILINX_PLL_INPUT_FREQ/875;
    base_clk_freq = base_clk_freq / (global_reg & 0xFF);
    
    uint32_t clk_div = base_clk_freq / freq;
    float clk_div_frac_f = 0;
    uint32_t clk_div_frac = 0;
    if (clk_div < 1 || clk_div > 256) {
        LOG(logERROR, ("XILINX_PLL: Invalid clock divider, need to change base clock\n"));
        return;
    }

    // the first clock supports fractional division, increase the precision for that one
    // fractional divide is not allowed in fixed or dynamic phase shift mode !!!!
    if(clkIDX == 0){
        clk_div_frac_f = (float)base_clk_freq / freq - clk_div;
        clk_div_frac = (uint32_t)round(clk_div_frac_f * 1000);
        clk_div_frac = ((clk_div_frac + 62) / 125) * 125; // Xilinx documentation suggests way too many divider settings here, go to the closest one that actually does something
        if(clk_div_frac == 1000){
            clk_div_frac = 0;
            clk_div++;
        }
    }

    LOG(logINFOBLUE, ("XILINX_PLL: Setting clock divider to %u.%u\n", clk_div, clk_div_frac));
    bus_w_csp2(XILINX_PLL_BASE_ADDR + XILINX_PLL_CLK_CONFIG_BASE_ADDR + clkIDX * XILINX_PLL_CLK_CONFIG_WIDTH * 4 + XILINX_PLL_CLK_DIV_OFFSET, (clk_div & 0xFF) | ((clk_div_frac & 0x3FF) << 8));
    XILINX_PLL_load();
    XILINX_PLL_waitForLock();
}

uint32_t XILINX_PLL_getFrequency(uint32_t clkIDX){
    if (clkIDX >= XILINX_PLL_NUM_CLKS) {
        LOG(logERROR, ("XILINX_PLL: Invalid clock index %d\n", clkIDX));
        return 0;
    }
    if (clkIDX >= 4) {
        LOG(logERROR, ("XILINX_PLL: get frequency not implemented for this clock %d\n", clkIDX));
        return 0;
    }

    uint32_t counter_val = 0;
    if(clkIDX < 2)
        counter_val = bus_r_csp2(XILINX_PLL_MEASURE_BASE_ADDR0 + clkIDX * 8);
    else    
        counter_val = bus_r_csp2(XILINX_PLL_MEASURE_BASE_ADDR1 + (clkIDX - 2) * 8);
    
    uint32_t freq_kHz = (counter_val+500)/1000; // round to nearest kHz
    return freq_kHz;
}

bool XILINX_PLL_isLocked() {
    uint32_t status = bus_r_csp2(XILINX_PLL_BASE_ADDR + XILINX_PLL_STATUS_REG);
    return (status & (1 << XILINX_PLL_LOCKED_BIT)) != 0;
}

void XILINX_PLL_reset() {
    bus_w_csp2(XILINX_PLL_BASE_ADDR + XILINX_PLL_RESET_REG, 0xA);
}

void XILINX_PLL_load() {
    bus_w_csp2(XILINX_PLL_BASE_ADDR + XILINX_PLL_LOAD_REG, 0x3);
}

void XILINX_PLL_waitForLock() {
    int timeout = 500;
    while (timeout > 0) {
        usleep(10000);
        if (XILINX_PLL_isLocked())
            return;
        timeout--;
    }
    LOG(logERROR, ("XILINX_PLL: Timeout waiting for PLL to lock\n"));
}