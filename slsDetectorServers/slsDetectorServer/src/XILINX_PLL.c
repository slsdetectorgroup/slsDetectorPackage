// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "XILINX_PLL.h"
#include "arm64.h"
#include "clogger.h"
#include <math.h>
#include <stdbool.h>
#include <unistd.h>

// https://docs.amd.com/r/en-US/pg065-clk-wiz/Register-Space  (simplified, we
// leave some things away)

// clang-format off
#define XILINX_PLL_INPUT_FREQ           (100000000) // 100 MHz
#define XILINX_PLL_MIN_FREQ             (10000000)
#define XILINX_PLL_MAX_FREQ             (250000000)
#define XILINX_PLL_MAX_CLK_DIV          (256)
#define XILINX_PLL_NUM_CLKS             (7)
#define XILINX_PLL_MAX_NUM_CLKS_FOR_GET (3)
#define XILINX_PLL_STEP_SIZE            (125)
#define XILINX_PLL_HALF_STEP_SIZE       (62)

#define XILINX_PLL_BASE_ADDR            (0x0)
#define XILINX_PLL_MEASURE_BASE_ADDR0   (0x1000) // added externally,  not part of CLKWIZ core for clks 0 and 1
#define XILINX_PLL_MEASURE_BASE_ADDR0_MAX_CLKS (2)
#define XILINX_PLL_MEASURE_BASE_ADDR1   (0x2000) // for clks 2 to 6
#define XILINX_PLL_MEASURE_WIDTH        (8)      // per clock

#define XILINX_PLL_RESET_REG            (0x000)
#define XILINX_PLL_RESET_VAL            (0xA)

#define XILINX_PLL_STATUS_REG           (0x004)
#define XILINX_PLL_STATUS_LOCKED_OFST   (0)
#define XILINX_PLL_STATUS_LOCKED_MSK    (0x00000001 << XILINX_PLL_STATUS_LOCKED_OFST)

#define XILINX_PLL_CLKCONFIG_REG        (XILINX_PLL_BASE_ADDR + 0x200)
#define XILINX_PLL_DIVCLK_DIVIDE_OFST   (0)
#define XILINX_PLL_DIVCLK_DIVIDE_MSK    (0x000000FF << XILINX_PLL_DIVCLK_DIVIDE_OFST)
#define XILINX_PLL_CLKFBOUT_MULT_OFST   (8)
#define XILINX_PLL_CLKFBOUT_MULT_MSK    (0x000000FF << XILINX_PLL_CLKFBOUT_MULT_OFST)
#define XILINX_PLL_CLKFBOUT_FRAC_OFST   (16)
#define XILINX_PLL_CLKFBOUT_FRAC_MSK    (0x000003FF << XILINX_PLL_CLKFBOUT_FRAC_OFST)
// The value from 0 to 875 representing the fractional multiplied by 1000
#define XILINX_PLL_CLKFBOUT_FRAC_MAX_VAL (875)


#define XILINX_PLL_CLKCONFIG_BASE_ADDR  (XILINX_PLL_BASE_ADDR + 0x208)
#define XILINX_PLL_CLKCONFIG_WIDTH      (3 * 4) // per clock (7 clocks)

#define XILINX_PLL_CLK_DIV_REG_OFST     (0)
#define XILINX_PLL_CLK_DIV_DIVIDE_OFST  (0)
#define XILINX_PLL_CLK_DIV_DIVIDE_MSK   (0x000000FF << XILINX_PLL_CLK_DIV_DIVIDE_OFST)
#define XILINX_PLL_CLK_DIV_FRAC_OFST    (8) // works on IDX 0 only
#define XILINX_PLL_CLK_DIV_FRAC_MSK     (0x000003FF << XILINX_PLL_CLK_DIV_FRAC_OFST)

#define XILINX_PLL_CLK_PHASE_REG_OFST   (4) // signed num for +/- phase
#define XILINX_PLL_CLK_PHASE_OFST       (0)
#define XILINX_PLL_CLK_PHASE_MSK        (0x0000FFFF << XILINX_PLL_CLK_PHASE_OFST)

#define XILINX_PLL_CLK_DUTY_REG_OFST    (8) // (in %) * 1000
#define XILINX_PLL_CLK_DUTY_OFST        (0) 
#define XILINX_PLL_CLK_DUTY_MSK         (0x0000FFFF << XILINX_PLL_CLK_DUTY_OFST)



#define XILINX_PLL_LOAD_REG             (0x25C)
#define XILINX_PLL_LOAD_RECONFIGURE_OFST (0) // load and reconfigure state machine
#define XILINX_PLL_LOAD_RECONFIGURE_MSK (0x00000001 << XILINX_PLL_LOAD_RECONFIGURE_OFST)
#define XILINX_PLL_LOAD_FROM_REGS_OFST  (1) // 0 for default values as compiled into firmware
#define XILINX_PLL_LOAD_FROM_REGS_MSK   (0x00000001 << XILINX_PLL_LOAD_FROM_REGS_OFST)

// clang-format on

// freq in Hz !!
int XILINX_PLL_setFrequency(uint32_t clk_index, uint32_t freq) {
    if (clk_index >= XILINX_PLL_NUM_CLKS) {
        LOG(logERROR, ("XILINX_PLL: Invalid clock index %d\n", clk_index));
        return 1;
    }
    if (freq < XILINX_PLL_MIN_FREQ || freq > XILINX_PLL_MAX_FREQ) {
        LOG(logERROR, ("XILINX_PLL: Frequency %d Hz is out of range\n", freq));
        return 1;
    }

    // calculate base clock frequency
    uint32_t global_reg = bus_r_csp2(XILINX_PLL_CLKCONFIG_REG);
#ifdef VIRTUAL
    global_reg = 3073;
#endif
    uint32_t clkfbout_mult = ((global_reg & XILINX_PLL_CLKFBOUT_MULT_MSK) >>
                              XILINX_PLL_CLKFBOUT_MULT_OFST);
    uint32_t clkfbout_frac = ((global_reg & XILINX_PLL_CLKFBOUT_FRAC_MSK) >>
                              XILINX_PLL_CLKFBOUT_FRAC_OFST);
    uint32_t divclk_divide = ((global_reg & XILINX_PLL_DIVCLK_DIVIDE_MSK) >>
                              XILINX_PLL_DIVCLK_DIVIDE_OFST);
    uint32_t base_clk_freq = clkfbout_mult * XILINX_PLL_INPUT_FREQ;
    base_clk_freq += (clkfbout_frac * XILINX_PLL_INPUT_FREQ /
                      XILINX_PLL_CLKFBOUT_FRAC_MAX_VAL);
    base_clk_freq /= divclk_divide;

    // calcualte clock divider
    uint32_t clk_div = base_clk_freq / freq;
    if (clk_div < 1 || clk_div > XILINX_PLL_MAX_CLK_DIV) {
        LOG(logERROR,
            ("XILINX_PLL: Invalid clock divider, need to change base clock\n"));
        return 1;
    }

    uint32_t clk_div_frac = 0;
    // the first clock supports fractional division, increase the precision for
    // that one fractional divide is not allowed in fixed or dynamic phase shift
    // mode !!!!
    if (clk_index == 0) {
        float clk_div_frac_f =
            (float)base_clk_freq / freq - clk_div; // eg. 2.333 => 0.333
        clk_div_frac = (uint32_t)round(clk_div_frac_f * 1000); // 0.333 => 333
        clk_div_frac = ((clk_div_frac + XILINX_PLL_HALF_STEP_SIZE) /
                        XILINX_PLL_STEP_SIZE) *
                       XILINX_PLL_STEP_SIZE; // round to multiples of step size,
                                             // 333 = > 375
        if (clk_div_frac == 1000) {
            clk_div_frac = 0;
            clk_div++;
        }
    }

    LOG(logINFOBLUE, ("XILINX_PLL: Setting clock divider to %u.%u\n", clk_div,
                      clk_div_frac));
    uint32_t clk_addr = XILINX_PLL_CLKCONFIG_BASE_ADDR +
                        clk_index * XILINX_PLL_CLKCONFIG_WIDTH +
                        XILINX_PLL_CLK_DIV_REG_OFST;
    uint32_t clk_config_val = ((clk_div << XILINX_PLL_CLK_DIV_DIVIDE_OFST) &
                               XILINX_PLL_CLK_DIV_DIVIDE_MSK) |
                              ((clk_div_frac << XILINX_PLL_CLK_DIV_FRAC_OFST) &
                               XILINX_PLL_CLK_DIV_FRAC_MSK);

    bus_w_csp2(clk_addr, clk_config_val);
    XILINX_PLL_load();
    XILINX_PLL_waitForLock();

    // wait for firmware to measure the actual frequency
    usleep(2 * 1000 * 1000);
    return 0;
}

uint32_t XILINX_PLL_getFrequency(uint32_t clk_index) {
    if (clk_index >= XILINX_PLL_NUM_CLKS) {
        LOG(logERROR, ("XILINX_PLL: Invalid clock index %d\n", clk_index));
        return -1;
    }
    if (clk_index > XILINX_PLL_MAX_NUM_CLKS_FOR_GET) {
        LOG(logERROR,
            ("XILINX_PLL: get frequency not implemented for this clock %d\n",
             clk_index));
        return -1;
    }

    uint32_t base_addr = XILINX_PLL_MEASURE_BASE_ADDR0;
    if (clk_index >= XILINX_PLL_MEASURE_BASE_ADDR0_MAX_CLKS) {
        clk_index -= XILINX_PLL_MEASURE_BASE_ADDR0_MAX_CLKS;
        base_addr = XILINX_PLL_MEASURE_BASE_ADDR1;
    }
    return bus_r_csp2(base_addr + clk_index * XILINX_PLL_MEASURE_WIDTH);
}

bool XILINX_PLL_isLocked() {
    uint32_t status = bus_r_csp2(XILINX_PLL_BASE_ADDR + XILINX_PLL_STATUS_REG);
    return ((status & XILINX_PLL_STATUS_LOCKED_MSK) >>
            XILINX_PLL_STATUS_LOCKED_OFST);
}

void XILINX_PLL_reset() {
    bus_w_csp2(XILINX_PLL_BASE_ADDR + XILINX_PLL_RESET_REG,
               XILINX_PLL_RESET_VAL);
}

void XILINX_PLL_load() {
    bus_w_csp2(
        XILINX_PLL_BASE_ADDR + XILINX_PLL_LOAD_REG,
        (XILINX_PLL_LOAD_RECONFIGURE_MSK | XILINX_PLL_LOAD_FROM_REGS_MSK));
}

void XILINX_PLL_waitForLock() {
#ifdef VIRTUAL
    return;
#endif
    int timeout_us = 10 * 1000;
    int count = 500;
    while (count > 0) {
        usleep(timeout_us);
        if (XILINX_PLL_isLocked())
            return;
        count--;
    }
    LOG(logERROR, ("XILINX_PLL: Timeout waiting for PLL to lock (%d ms)\n",
                   (count * timeout_us) / 1000));
}