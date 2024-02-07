// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "loadPattern.h"
#include "RegisterDefs.h"
#include "common.h"
#include "sls/ansi.h"
#include "slsDetectorServer_defs.h"

#include <string.h>
#include <unistd.h>

#ifdef MYTHEN3D
extern enum TLogLevel trimmingPrint;
#endif

#if defined(CHIPTESTBOARDD) || defined(XILINX_CHIPTESTBOARDD)
#ifdef VIRTUAL
uint64_t virtual_pattern[MAX_PATTERN_LENGTH];
#endif
#endif

extern void bus_w(u_int32_t offset, u_int32_t data);
extern u_int32_t bus_r(u_int32_t offset);
// extern int64_t get64BitReg(int aLSB, int aMSB); TODO for all servers (only
// uint64_t) extern int64_t set64BitReg(int64_t value, int aLSB, int aMSB);
extern uint64_t getU64BitReg(int aLSB, int aMSB);
extern void setU64BitReg(uint64_t value, int aLSB, int aMSB);

#ifdef MYTHEN3D
#define MAX_LEVELS M3_MAX_PATTERN_LEVELS
#else
#define MAX_LEVELS MAX_PATTERN_LEVELS
#endif

char clientPatternfile[MAX_STR_LENGTH];

void initializePatternAddresses() {
    LOG(logDEBUG1, ("Setting default Loop and Wait Addresses(0x%x)\n",
                    MAX_PATTERN_LENGTH - 1));
    for (int i = 0; i != MAX_LEVELS; ++i) {
        setPatternLoopAddresses(i, MAX_PATTERN_LENGTH - 1,
                                MAX_PATTERN_LENGTH - 1);
        setPatternWaitAddress(i, MAX_PATTERN_LENGTH - 1);
    }
}

#if defined(CHIPTESTBOARDD) || defined(XILINX_CHIPTESTBOARDD)
#ifdef VIRTUAL
void initializePatternWord() {
    memset(virtual_pattern, 0, sizeof(virtual_pattern));
}
#endif
#endif

#if defined(CHIPTESTBOARDD) || defined(XILINX_CHIPTESTBOARDD)
uint64_t validate_readPatternIOControl() {
#if defined(CHIPTESTBOARDD)
    return getU64BitReg(PATTERN_IO_CNTRL_LSB_REG, PATTERN_IO_CNTRL_MSB_REG);
#elif defined(XILINX_CHIPTESTBOARDD)
    return (uint64_t)(bus_r(PINIOCTRLREG));
#endif
}

int validate_writePatternIOControl(char *message, uint64_t arg) {
    // validate input
#ifdef XILINX_CHIPTESTBOARDD
    if (arg > BIT32_MSK) {
        strcpy(message, "Could not set pattern IO Control. Must be 32 bit for "
                        "this detector\n");
        LOG(logERROR, (message));
        return FAIL;
    }
#endif

    writePatternIOControl(arg);

    // validate result
    uint64_t retval = validate_readPatternIOControl();
    LOG(logDEBUG1,
        ("Pattern IO Control retval: 0x%llx\n", (long long int)retval));
    int ret = OK;
    if (retval != arg) {
        ret = FAIL;
        sprintf(
            message,
            "Could not set pattern IO Control. Set 0x%llx, but read 0x%llx\n",
            (long long unsigned int)arg, (long long unsigned int)retval);
        LOG(logERROR, (message));
    }
    return ret;
}

void writePatternIOControl(uint64_t word) {
#ifdef CHIPTESTBOARDD
    LOG(logINFO,
        ("Setting Pattern I/O Control: 0x%llx\n", (long long int)word));
    setU64BitReg(word, PATTERN_IO_CNTRL_LSB_REG, PATTERN_IO_CNTRL_MSB_REG);
#elif defined(XILINX_CHIPTESTBOARDD)
    uint32_t val = (uint32_t)word;
    LOG(logINFO, ("Setting Pattern I/O Control: 0x%x\n", val));
    bus_w(PINIOCTRLREG, val);
#endif
}
#endif

int validate_readPatternWord(char *message, int addr, uint64_t *word) {
    // validate input
    if (addr < 0 || addr >= MAX_PATTERN_LENGTH) {
        sprintf(message,
                "Cannot read pattern word. Addr must be between 0 and 0x%x.\n",
                MAX_PATTERN_LENGTH);
        LOG(logERROR, (message));
        return FAIL;
    }

    *word = readPatternWord(addr);
    return OK;
}

uint64_t readPatternWord(int addr) {
#ifdef MYTHEN3D
    LOG(logDEBUG1, ("  Reading Pattern Word (addr:0x%x)\n", addr));
    // the first word in RAM as base plus the offset of the word to write (addr)
    uint32_t reg_lsb = PATTERN_STEP0_LSB_REG + addr * REG_OFFSET * 2;
    uint32_t reg_msb = PATTERN_STEP0_MSB_REG + addr * REG_OFFSET * 2;
    return getU64BitReg(reg_lsb, reg_msb);
#else
    LOG(logDEBUG1, ("  Reading (Executing) Pattern Word (addr:0x%x)\n", addr));
    uint32_t reg = PATTERN_CNTRL_REG;

    // overwrite with  only addr
    bus_w(reg, ((addr << PATTERN_CNTRL_ADDR_OFST) & PATTERN_CNTRL_ADDR_MSK));

    // set read strobe
    bus_w(reg, bus_r(reg) | PATTERN_CNTRL_RD_MSK);

    // unset read strobe
    bus_w(reg, bus_r(reg) & (~PATTERN_CNTRL_RD_MSK));
    usleep(WAIT_TIME_PATTERN_READ);

    // read value
#ifndef VIRTUAL
    return getU64BitReg(PATTERN_OUT_LSB_REG, PATTERN_OUT_MSB_REG);
#else
    return virtual_pattern[addr];
#endif
#endif
}

int validate_writePatternWord(char *message, int addr, uint64_t word) {
    // validate input
    if (addr < 0 || addr >= MAX_PATTERN_LENGTH) {
        sprintf(message,
                "Cannot set pattern word. Addr must be between 0 and 0x%x.\n",
                MAX_PATTERN_LENGTH);
        LOG(logERROR, (message));
        return FAIL;
    }

    writePatternWord(addr, word);

    // validate result
    int ret = OK;
    // cannot validate for ctb ( same as executing pattern word)
#ifdef MYTHEN3D
    uint64_t retval = readPatternWord(addr);
    LOG(logDEBUG1, ("Pattern word (addr:0x%x) retval: 0x%llx\n", addr,
                    (long long int)retval));
    char mode[128];
    memset(mode, 0, sizeof(mode));
    sprintf(mode, "set pattern word for addr 0x%x", addr);
    validate64(&ret, message, word, retval, "set pattern word", HEX);
#endif
    return ret;
}

void writePatternWord(int addr, uint64_t word) {
    LOG(logDEBUG1, ("Setting Pattern Word (addr:0x%x, word:0x%llx)\n", addr,
                    (long long int)word));

#ifndef MYTHEN3D
    uint32_t reg = PATTERN_CNTRL_REG;

    // write word
    setU64BitReg(word, PATTERN_IN_LSB_REG, PATTERN_IN_MSB_REG);

    // overwrite with  only addr
    bus_w(reg, ((addr << PATTERN_CNTRL_ADDR_OFST) & PATTERN_CNTRL_ADDR_MSK));

    // set write strobe
    bus_w(reg, bus_r(reg) | PATTERN_CNTRL_WR_MSK);

    // unset write strobe
    bus_w(reg, bus_r(reg) & (~PATTERN_CNTRL_WR_MSK));
#ifdef VIRTUAL
    virtual_pattern[addr] = word;
#endif
// mythen
#else
    // the first word in RAM as base plus the offset of the word to write (addr)
    uint32_t reg_lsb = PATTERN_STEP0_LSB_REG + addr * REG_OFFSET * 2;
    uint32_t reg_msb = PATTERN_STEP0_MSB_REG + addr * REG_OFFSET * 2;
    setU64BitReg(word, reg_lsb, reg_msb);
#endif
}

int validate_getPatternWaitAddresses(char *message, int level, int *addr) {
    // validate input
    if (level < 0 || level >= MAX_LEVELS) {
        sprintf(
            message,
            "Cannot get patwait address. Level %d must be between 0 and %d.\n",
            level, MAX_LEVELS - 1);
        LOG(logERROR, (message));
        return FAIL;
    }
    *addr = getPatternWaitAddress(level);
    return OK;
}

int getPatternWaitAddress(int level) {
    switch (level) {
    case 0:
        return ((bus_r(PATTERN_WAIT_0_ADDR_REG) & PATTERN_WAIT_0_ADDR_MSK) >>
                PATTERN_WAIT_0_ADDR_OFST);
    case 1:
        return ((bus_r(PATTERN_WAIT_1_ADDR_REG) & PATTERN_WAIT_1_ADDR_MSK) >>
                PATTERN_WAIT_1_ADDR_OFST);
    case 2:
        return ((bus_r(PATTERN_WAIT_2_ADDR_REG) & PATTERN_WAIT_2_ADDR_MSK) >>
                PATTERN_WAIT_2_ADDR_OFST);
#ifndef MYTHEN3D
    case 3:
        return ((bus_r(PATTERN_WAIT_3_ADDR_REG) & PATTERN_WAIT_3_ADDR_MSK) >>
                PATTERN_WAIT_3_ADDR_OFST);
    case 4:
        return ((bus_r(PATTERN_WAIT_4_ADDR_REG) & PATTERN_WAIT_4_ADDR_MSK) >>
                PATTERN_WAIT_4_ADDR_OFST);
    case 5:
        return ((bus_r(PATTERN_WAIT_5_ADDR_REG) & PATTERN_WAIT_5_ADDR_MSK) >>
                PATTERN_WAIT_5_ADDR_OFST);
#endif
    default:
        return -1;
    }
}

int validate_setPatternWaitAddresses(char *message, int level, int addr) {
    // validate input
    if (level < 0 || level >= MAX_LEVELS) {
        sprintf(
            message,
            "Cannot set patwait address. Level %d must be between 0 and %d.\n",
            level, MAX_LEVELS - 1);
        LOG(logERROR, (message));
        return FAIL;
    }
    if (addr < 0 || addr >= MAX_PATTERN_LENGTH) {
        sprintf(message,
                "Cannot set patwait address (level: %d). Addr must be between "
                "0 and 0x%x.\n",
                level, MAX_PATTERN_LENGTH);
        LOG(logERROR, (message));
        return FAIL;
    }

    setPatternWaitAddress(level, addr);

    // validate result
    int retval = getPatternWaitAddress(level);
    LOG(logDEBUG1,
        ("Pattern wait address (level:%d) retval: 0x%x\n", level, retval));
    int ret = OK;
    char mode[128];
    memset(mode, 0, sizeof(mode));
    sprintf(mode, "set pattern Loop %d wait address", level);
    validate(&ret, message, addr, retval, mode, HEX);
    return ret;
}

void setPatternWaitAddress(int level, int addr) {
#ifdef MYTHEN3D
    LOG(trimmingPrint,
#else
    LOG(logINFO,
#endif
        ("Setting Pattern Wait Address (level:%d, addr:0x%x)\n", level, addr));
    switch (level) {
    case 0:
        bus_w(PATTERN_WAIT_0_ADDR_REG,
              ((addr << PATTERN_WAIT_0_ADDR_OFST) & PATTERN_WAIT_0_ADDR_MSK));
        break;
    case 1:
        bus_w(PATTERN_WAIT_1_ADDR_REG,
              ((addr << PATTERN_WAIT_1_ADDR_OFST) & PATTERN_WAIT_1_ADDR_MSK));
        break;
    case 2:
        bus_w(PATTERN_WAIT_2_ADDR_REG,
              ((addr << PATTERN_WAIT_2_ADDR_OFST) & PATTERN_WAIT_2_ADDR_MSK));
        break;
#ifndef MYTHEN3D
    case 3:
        bus_w(PATTERN_WAIT_3_ADDR_REG,
              ((addr << PATTERN_WAIT_3_ADDR_OFST) & PATTERN_WAIT_3_ADDR_MSK));
        break;
    case 4:
        bus_w(PATTERN_WAIT_4_ADDR_REG,
              ((addr << PATTERN_WAIT_4_ADDR_OFST) & PATTERN_WAIT_4_ADDR_MSK));
        break;
    case 5:
        bus_w(PATTERN_WAIT_5_ADDR_REG,
              ((addr << PATTERN_WAIT_5_ADDR_OFST) & PATTERN_WAIT_5_ADDR_MSK));
        break;
#endif
    default:
        return;
    }
}

int validate_getPatternWaitTime(char *message, int level, uint64_t *waittime) {
    // validate input
    if (level < 0 || level >= MAX_LEVELS) {
        sprintf(message,
                "Cannot get patwaittime. Level %d must be between 0 and %d.\n",
                level, MAX_LEVELS - 1);
        LOG(logERROR, (message));
        return FAIL;
    }
    *waittime = getPatternWaitTime(level);
    return OK;
}

uint64_t getPatternWaitTime(int level) {
    switch (level) {
    case 0:
        return getU64BitReg(PATTERN_WAIT_TIMER_0_LSB_REG,
                            PATTERN_WAIT_TIMER_0_MSB_REG);
    case 1:
        return getU64BitReg(PATTERN_WAIT_TIMER_1_LSB_REG,
                            PATTERN_WAIT_TIMER_1_MSB_REG);
    case 2:
        return getU64BitReg(PATTERN_WAIT_TIMER_2_LSB_REG,
                            PATTERN_WAIT_TIMER_2_MSB_REG);
#ifndef MYTHEN3D
    case 3:
        return getU64BitReg(PATTERN_WAIT_TIMER_3_LSB_REG,
                            PATTERN_WAIT_TIMER_3_MSB_REG);
    case 4:
        return getU64BitReg(PATTERN_WAIT_TIMER_4_LSB_REG,
                            PATTERN_WAIT_TIMER_4_MSB_REG);
    case 5:
        return getU64BitReg(PATTERN_WAIT_TIMER_5_LSB_REG,
                            PATTERN_WAIT_TIMER_5_MSB_REG);
#endif
    default:
        return -1;
    }
}

int validate_setPatternWaitTime(char *message, int level, uint64_t waittime) {
    // validate input
    if (level < 0 || level >= MAX_LEVELS) {
        sprintf(message,
                "Cannot set patwaittime. Level %d must be between 0 and %d.\n",
                level, MAX_LEVELS - 1);
        LOG(logERROR, (message));
        return FAIL;
    }

    setPatternWaitTime(level, waittime);

    // validate result
    uint64_t retval = getPatternWaitTime(level);
    LOG(logDEBUG1, ("Pattern wait time (level:%d) retval: %d\n", level,
                    (long long int)retval));
    int ret = OK;
    char mode[128];
    memset(mode, 0, sizeof(mode));
    sprintf(mode, "set pattern Loop %d wait time", level);
    validate64(&ret, message, waittime, retval, mode, DEC);
    return ret;
}

void setPatternWaitTime(int level, uint64_t t) {
#ifdef MYTHEN3D
    LOG(trimmingPrint,
#else
    LOG(logINFO,
#endif
        ("Setting Pattern Wait Time (level:%d) :%lld\n", level,
         (long long int)t));
    switch (level) {
    case 0:
        setU64BitReg(t, PATTERN_WAIT_TIMER_0_LSB_REG,
                     PATTERN_WAIT_TIMER_0_MSB_REG);
        break;
    case 1:
        setU64BitReg(t, PATTERN_WAIT_TIMER_1_LSB_REG,
                     PATTERN_WAIT_TIMER_1_MSB_REG);
        break;
    case 2:
        setU64BitReg(t, PATTERN_WAIT_TIMER_2_LSB_REG,
                     PATTERN_WAIT_TIMER_2_MSB_REG);
        break;
#ifndef MYTHEN3D
    case 3:
        setU64BitReg(t, PATTERN_WAIT_TIMER_3_LSB_REG,
                     PATTERN_WAIT_TIMER_3_MSB_REG);
        break;
    case 4:
        setU64BitReg(t, PATTERN_WAIT_TIMER_4_LSB_REG,
                     PATTERN_WAIT_TIMER_4_MSB_REG);
        break;
    case 5:
        setU64BitReg(t, PATTERN_WAIT_TIMER_5_LSB_REG,
                     PATTERN_WAIT_TIMER_5_MSB_REG);
        break;
#endif
    default:
        return;
    }
}

int validate_getPatternLoopCycles(char *message, int level, int *numLoops) {
    // validate input
    if (level < 0 || level >= MAX_LEVELS) {
        sprintf(message,
                "Cannot get patnloop. Level %d must be between 0 and %d.\n",
                level, MAX_LEVELS - 1);
        LOG(logERROR, (message));
        return FAIL;
    }
    *numLoops = getPatternLoopCycles(level);
    return OK;
}

int getPatternLoopCycles(int level) {
    switch (level) {
    case 0:
        return bus_r(PATTERN_LOOP_0_ITERATION_REG);
    case 1:
        return bus_r(PATTERN_LOOP_1_ITERATION_REG);
    case 2:
        return bus_r(PATTERN_LOOP_2_ITERATION_REG);
#ifndef MYTHEN3D
    case 3:
        return bus_r(PATTERN_LOOP_3_ITERATION_REG);
    case 4:
        return bus_r(PATTERN_LOOP_4_ITERATION_REG);
    case 5:
        return bus_r(PATTERN_LOOP_5_ITERATION_REG);
#endif
    default:
        return -1;
    }
}

int validate_setPatternLoopCycles(char *message, int level, int numLoops) {
    // validate input
    if (level < 0 || level >= MAX_LEVELS) {
        sprintf(message,
                "Cannot set patnloop. Level %d must be between 0 and %d.\n",
                level, MAX_LEVELS);
        LOG(logERROR, (message));
        return FAIL;
    }
    if (numLoops < 0) {
        sprintf(message,
                "Cannot set patnloop. Iterations must be between > 0.\n");
        LOG(logERROR, (message));
        return FAIL;
    }

    setPatternLoopCycles(level, numLoops);

    // validate result
    int retval = getPatternLoopCycles(level);
    int ret = OK;
    char mode[128];
    memset(mode, 0, sizeof(mode));
    sprintf(mode, "set pattern Loop %d num loops", level);
    validate(&ret, message, numLoops, retval, mode, DEC);
    return ret;
}

void setPatternLoopCycles(int level, int nLoop) {
#ifdef MYTHEN3D
    LOG(trimmingPrint,
#else
    LOG(logINFO,
#endif
        ("Setting Pattern Loop Cycles(level:%d, nLoop:%d)\n", level, nLoop));
    switch (level) {
    case 0:
        bus_w(PATTERN_LOOP_0_ITERATION_REG, nLoop);
        break;
    case 1:
        bus_w(PATTERN_LOOP_1_ITERATION_REG, nLoop);
        break;
    case 2:
        bus_w(PATTERN_LOOP_2_ITERATION_REG, nLoop);
        break;
#ifndef MYTHEN3D
    case 3:
        bus_w(PATTERN_LOOP_3_ITERATION_REG, nLoop);
        break;
    case 4:
        bus_w(PATTERN_LOOP_4_ITERATION_REG, nLoop);
        break;
    case 5:
        bus_w(PATTERN_LOOP_5_ITERATION_REG, nLoop);
        break;
#endif
    default:
        return;
    }
}

void validate_getPatternLoopLimits(int *startAddr, int *stopAddr) {
    *startAddr = ((bus_r(PATTERN_LIMIT_REG) & PATTERN_LIMIT_STRT_MSK) >>
                  PATTERN_LIMIT_STRT_OFST);
    *stopAddr = ((bus_r(PATTERN_LIMIT_REG) & PATTERN_LIMIT_STP_MSK) >>
                 PATTERN_LIMIT_STP_OFST);
}

int validate_setPatternLoopLimits(char *message, int startAddr, int stopAddr) {
    // validate input
    if (startAddr < 0 || startAddr >= MAX_PATTERN_LENGTH || stopAddr < 0 ||
        stopAddr >= MAX_PATTERN_LENGTH) {
        sprintf(message,
                "Cannot set patlimits from default "
                "pattern file. Addr must be between 0 and 0x%x.\n",
                MAX_PATTERN_LENGTH);
        LOG(logERROR, (message));
        return FAIL;
    }

    setPatternLoopLimits(startAddr, stopAddr);

    // validate result
    int r_startAddr = -1, r_stopAddr = -1;
    validate_getPatternLoopLimits(&r_startAddr, &r_stopAddr);
    int ret = OK;
    // start addr
    validate(&ret, message, startAddr, r_startAddr,
             "set pattern Limits start addr", HEX);
    if (ret == FAIL) {
        return FAIL;
    }
    // stop addr
    validate(&ret, message, stopAddr, r_stopAddr,
             "set pattern Limits stop addr", HEX);
    return ret;
}

void setPatternLoopLimits(int startAddr, int stopAddr) {
#ifdef MYTHEN3D
    LOG(trimmingPrint,
#else
    LOG(logINFO,
#endif
        ("Setting Pattern Loop Limits(startaddr:0x%x, stopaddr:0x%x)\n",
         startAddr, stopAddr));
    bus_w(PATTERN_LIMIT_REG,
          ((startAddr << PATTERN_LIMIT_STRT_OFST) & PATTERN_LIMIT_STRT_MSK) |
              ((stopAddr << PATTERN_LIMIT_STP_OFST) & PATTERN_LIMIT_STP_MSK));
}

int validate_getPatternLoopAddresses(char *message, int level, int *startAddr,
                                     int *stopAddr) {
    // validate input
    if (level < 0 || level >= MAX_LEVELS) {
        sprintf(message,
                "Cannot get patloop addresses. Level %d must be between 0 and "
                "%d.\n",
                level, MAX_LEVELS - 1);
        LOG(logERROR, (message));
        return FAIL;
    }

    getPatternLoopAddresses(level, startAddr, stopAddr);
    return OK;
}

void getPatternLoopAddresses(int level, int *startAddr, int *stopAddr) {
    switch (level) {
    case 0:
        *startAddr =
            ((bus_r(PATTERN_LOOP_0_ADDR_REG) & PATTERN_LOOP_0_ADDR_STRT_MSK) >>
             PATTERN_LOOP_0_ADDR_STRT_OFST);
        *stopAddr =
            ((bus_r(PATTERN_LOOP_0_ADDR_REG) & PATTERN_LOOP_0_ADDR_STP_MSK) >>
             PATTERN_LOOP_0_ADDR_STP_OFST);
        break;
    case 1:
        *startAddr =
            ((bus_r(PATTERN_LOOP_1_ADDR_REG) & PATTERN_LOOP_1_ADDR_STRT_MSK) >>
             PATTERN_LOOP_1_ADDR_STRT_OFST);
        *stopAddr =
            ((bus_r(PATTERN_LOOP_1_ADDR_REG) & PATTERN_LOOP_1_ADDR_STP_MSK) >>
             PATTERN_LOOP_1_ADDR_STP_OFST);
        break;
    case 2:
        *startAddr =
            ((bus_r(PATTERN_LOOP_2_ADDR_REG) & PATTERN_LOOP_2_ADDR_STRT_MSK) >>
             PATTERN_LOOP_2_ADDR_STRT_OFST);
        *stopAddr =
            ((bus_r(PATTERN_LOOP_2_ADDR_REG) & PATTERN_LOOP_2_ADDR_STP_MSK) >>
             PATTERN_LOOP_2_ADDR_STP_OFST);
        break;
#ifndef MYTHEN3D
    case 3:
        *startAddr =
            ((bus_r(PATTERN_LOOP_3_ADDR_REG) & PATTERN_LOOP_3_ADDR_STRT_MSK) >>
             PATTERN_LOOP_3_ADDR_STRT_OFST);
        *stopAddr =
            ((bus_r(PATTERN_LOOP_3_ADDR_REG) & PATTERN_LOOP_3_ADDR_STP_MSK) >>
             PATTERN_LOOP_3_ADDR_STP_OFST);
        break;
    case 4:
        *startAddr =
            ((bus_r(PATTERN_LOOP_4_ADDR_REG) & PATTERN_LOOP_4_ADDR_STRT_MSK) >>
             PATTERN_LOOP_4_ADDR_STRT_OFST);
        *stopAddr =
            ((bus_r(PATTERN_LOOP_4_ADDR_REG) & PATTERN_LOOP_4_ADDR_STP_MSK) >>
             PATTERN_LOOP_4_ADDR_STP_OFST);
        break;
    case 5:
        *startAddr =
            ((bus_r(PATTERN_LOOP_5_ADDR_REG) & PATTERN_LOOP_5_ADDR_STRT_MSK) >>
             PATTERN_LOOP_5_ADDR_STRT_OFST);
        *stopAddr =
            ((bus_r(PATTERN_LOOP_5_ADDR_REG) & PATTERN_LOOP_5_ADDR_STP_MSK) >>
             PATTERN_LOOP_5_ADDR_STP_OFST);
        break;
#endif
    default:
        return;
    }
}

int validate_setPatternLoopAddresses(char *message, int level, int startAddr,
                                     int stopAddr) {
    // validate input
    if (level < 0 || level >= MAX_LEVELS) {
        sprintf(message,
                "Cannot set patloop addresses. Level %d must be between 0 and "
                "%d.\n",
                level, MAX_LEVELS - 1);
        LOG(logERROR, (message));
        return FAIL;
    }
    if ((int32_t)startAddr < 0 || startAddr >= MAX_PATTERN_LENGTH ||
        (int32_t)stopAddr < 0 || stopAddr >= MAX_PATTERN_LENGTH) {
        sprintf(message,
                "Cannot set patloop addresses (level: %d). Addr must be "
                "between 0 and "
                "0x%x.\n",
                level, MAX_PATTERN_LENGTH);
        LOG(logERROR, (message));
        return FAIL;
    }

    setPatternLoopAddresses(level, startAddr, stopAddr);

    // validate result
    int r_startAddr = -1, r_stopAddr = -1;
    getPatternLoopAddresses(level, &r_startAddr, &r_stopAddr);
    int ret = OK;
    char mode[128];
    // start addr
    memset(mode, 0, sizeof(mode));
    sprintf(mode, "set pattern Loop %d start addr", level);
    validate(&ret, message, startAddr, r_startAddr, mode, HEX);
    if (ret == FAIL) {
        return FAIL;
    }
    // stop addr
    memset(mode, 0, sizeof(mode));
    sprintf(mode, "set pattern Loop %d stop addr", level);
    validate(&ret, message, stopAddr, r_stopAddr, mode, HEX);
    return ret;
}

void setPatternLoopAddresses(int level, int startAddr, int stopAddr) {
#ifdef MYTHEN3D
    LOG(trimmingPrint,
#else
    LOG(logINFO,
#endif
        ("Setting Pattern Loop Address (level:%d, startaddr:0x%x, "
         "stopaddr:0x%x)\n",
         level, startAddr, stopAddr));
    switch (level) {
    case 0:
        bus_w(PATTERN_LOOP_0_ADDR_REG,
              ((startAddr << PATTERN_LOOP_0_ADDR_STRT_OFST) &
               PATTERN_LOOP_0_ADDR_STRT_MSK) |
                  ((stopAddr << PATTERN_LOOP_0_ADDR_STP_OFST) &
                   PATTERN_LOOP_0_ADDR_STP_MSK));
        break;
    case 1:
        bus_w(PATTERN_LOOP_1_ADDR_REG,
              ((startAddr << PATTERN_LOOP_1_ADDR_STRT_OFST) &
               PATTERN_LOOP_1_ADDR_STRT_MSK) |
                  ((stopAddr << PATTERN_LOOP_1_ADDR_STP_OFST) &
                   PATTERN_LOOP_1_ADDR_STP_MSK));
        break;
    case 2:
        bus_w(PATTERN_LOOP_2_ADDR_REG,
              ((startAddr << PATTERN_LOOP_2_ADDR_STRT_OFST) &
               PATTERN_LOOP_2_ADDR_STRT_MSK) |
                  ((stopAddr << PATTERN_LOOP_2_ADDR_STP_OFST) &
                   PATTERN_LOOP_2_ADDR_STP_MSK));
        break;
#ifndef MYTHEN3D
    case 3:
        bus_w(PATTERN_LOOP_3_ADDR_REG,
              ((startAddr << PATTERN_LOOP_3_ADDR_STRT_OFST) &
               PATTERN_LOOP_3_ADDR_STRT_MSK) |
                  ((stopAddr << PATTERN_LOOP_3_ADDR_STP_OFST) &
                   PATTERN_LOOP_3_ADDR_STP_MSK));
        break;
    case 4:
        bus_w(PATTERN_LOOP_4_ADDR_REG,
              ((startAddr << PATTERN_LOOP_4_ADDR_STRT_OFST) &
               PATTERN_LOOP_4_ADDR_STRT_MSK) |
                  ((stopAddr << PATTERN_LOOP_4_ADDR_STP_OFST) &
                   PATTERN_LOOP_4_ADDR_STP_MSK));
        break;
    case 5:
        bus_w(PATTERN_LOOP_5_ADDR_REG,
              ((startAddr << PATTERN_LOOP_5_ADDR_STRT_OFST) &
               PATTERN_LOOP_5_ADDR_STRT_MSK) |
                  ((stopAddr << PATTERN_LOOP_5_ADDR_STP_OFST) &
                   PATTERN_LOOP_5_ADDR_STP_MSK));
        break;
#endif
    default:
        return;
    }
}

void setPatternMask(uint64_t mask) {
    LOG(logINFO, ("Setting pattern mask to 0x%llx\n", mask));
    setU64BitReg(mask, PATTERN_MASK_LSB_REG, PATTERN_MASK_MSB_REG);
}

uint64_t getPatternMask() {
    return getU64BitReg(PATTERN_MASK_LSB_REG, PATTERN_MASK_MSB_REG);
}

void setPatternBitMask(uint64_t mask) {
    LOG(logINFO, ("Setting pattern bit mask to 0x%llx\n", mask));
    setU64BitReg(mask, PATTERN_SET_LSB_REG, PATTERN_SET_MSB_REG);
}

uint64_t getPatternBitMask() {
    return getU64BitReg(PATTERN_SET_LSB_REG, PATTERN_SET_MSB_REG);
}

#ifdef MYTHEN3D
void startPattern() {
    LOG(logINFOBLUE, ("Starting Pattern\n"));
    bus_w(CONTROL_REG, bus_r(CONTROL_REG) | CONTROL_STRT_PATTERN_MSK);
    usleep(1);
    while (bus_r(PAT_STATUS_REG) & PAT_STATUS_RUN_BUSY_MSK) {
        usleep(1);
    }
    LOG(logINFOBLUE, ("Pattern done\n"));
}
#endif

char *getPatternFileName() { return clientPatternfile; }

int loadPattern(char *message, enum TLogLevel printLevel,
                patternParameters *pat, char *patfname) {
    LOG(logINFOBLUE, ("Loading Pattern from structure\n"));
    int ret = OK;
    memset(clientPatternfile, 0, MAX_STR_LENGTH);
    memcpy(clientPatternfile, patfname, MAX_STR_LENGTH);
    printf("Client Pattern File:%s\n", clientPatternfile);
#ifdef MYTHEN3D
    trimmingPrint = printLevel;
#endif
    initializePatternAddresses();

    // words
    for (int i = 0; i < MAX_PATTERN_LENGTH; ++i) {
        if ((i % 10 == 0) && pat->word[i] != 0) {
            LOG(logDEBUG5, ("Setting Pattern Word (addr:0x%x, word:0x%llx)\n",
                            i, (long long int)pat->word[i]));
        }
        ret = validate_writePatternWord(message, i, pat->word[i]);
        if (ret == FAIL) {
            break;
        }
    }
    // iocontrol
#if !defined(MYTHEN3D) && !defined(XILINX_CHIPTESTBOARDD) // TODO
    if (ret == OK) {
        ret = validate_writePatternIOControl(message, pat->ioctrl);
    }
#endif
    // limits
    if (ret == OK) {
        ret = validate_setPatternLoopLimits(message, pat->limits[0],
                                            pat->limits[1]);
    }

    if (ret == OK) {
        for (int i = 0; i < MAX_LEVELS; ++i) {
            // loop addr
            ret = validate_setPatternLoopAddresses(
                message, i, pat->startloop[i], pat->stoploop[i]);
            if (ret == FAIL) {
                break;
            }

            // num loops
            ret = validate_setPatternLoopCycles(message, i, pat->nloop[i]);
            if (ret == FAIL) {
                break;
            }

            // wait addr
            ret = validate_setPatternWaitAddresses(message, i, pat->wait[i]);
            if (ret == FAIL) {
                break;
            }

            // wait time
            ret = validate_setPatternWaitTime(message, i, pat->waittime[i]);
            if (ret == FAIL) {
                break;
            }
        }
    }
#ifdef MYTHEN3D
    trimmingPrint = logINFO;
#endif
    return ret;
}

int getPattern(char *message, patternParameters *pat) {
    LOG(logINFO, ("Getting Pattern into structure\n"));

    int ret = OK;
    uint64_t retval64 = 0;
    int retval1 = -1, retval2 = -1;
    // words
    for (int i = 0; i < MAX_PATTERN_LENGTH; ++i) {
        ret = validate_readPatternWord(message, i, &retval64);
        if (ret == FAIL) {
            break;
        }
        pat->word[i] = retval64;
    }
    // iocontrol
#if !defined(MYTHEN3D) && !defined(XILINX_CHIPTESTBOARDD) // TODO
    if (ret == OK) {
        validate_readPatternIOControl();
    }
#endif
    // limits
    if (ret == OK) {
        validate_getPatternLoopLimits(&retval1, &retval2);
        pat->limits[0] = retval1;
        pat->limits[1] = retval2;
    }
    if (ret == OK) {
        for (int i = 0; i < MAX_LEVELS; ++i) {
            // loop addr
            ret = validate_getPatternLoopAddresses(message, i, &retval1,
                                                   &retval2);
            if (ret == FAIL) {
                break;
            }
            pat->startloop[i] = retval1;
            pat->stoploop[i] = retval2;

            // num loops
            ret = validate_getPatternLoopCycles(message, i, &retval1);
            if (ret == FAIL) {
                break;
            }
            pat->nloop[i] = retval1;

            // wait addr
            ret = validate_getPatternWaitAddresses(message, i, &retval1);
            if (ret == FAIL) {
                break;
            }
            pat->wait[i] = retval1;

            // wait time
            ret = validate_getPatternWaitTime(message, i, &retval64);
            if (ret == FAIL) {
                break;
            }
            pat->waittime[i] = retval64;
        }
    }
    return ret;
}

int loadPatternFile(char *patFname, char *errMessage) {
    char fname[128];
    if (getAbsPath(fname, 128, patFname) == FAIL) {
        return FAIL;
    }

    // open config file
    FILE *fd = fopen(fname, "r");
    if (fd == NULL) {
        sprintf(errMessage, "Could not open pattern file [%s].\n", patFname);
        LOG(logERROR, ("%s\n\n", errMessage));
        return FAIL;
    }
    LOG(logINFOBLUE, ("Reading default pattern file %s\n", patFname));

    // Initialization
    const size_t LZ = 256;
    char line[LZ];
    memset(line, 0, LZ);
    char command[LZ];
    char temp[MAX_STR_LENGTH];
    memset(temp, 0, MAX_STR_LENGTH);

    initializePatternAddresses();

    // keep reading a line
    while (fgets(line, LZ, fd)) {

        // ignore comments
        if (line[0] == '#') {
            LOG(logDEBUG1, ("Ignoring Comment\n"));
            continue;
        }

        // ignore empty lines
        if (strlen(line) <= 1) {
            LOG(logDEBUG1, ("Ignoring Empty line\n"));
            continue;
        }

        // removing leading spaces
        if (line[0] == ' ' || line[0] == '\t') {
            int len = strlen(line);
            // find first valid character
            int i = 0;
            for (i = 0; i < len; ++i) {
                if (line[i] != ' ' && line[i] != '\t') {
                    break;
                }
            }
            // ignore the line full of spaces (last char \n)
            if (i >= len - 1) {
                LOG(logDEBUG1, ("Ignoring line full of spaces\n"));
                continue;
            }
            // copying only valid char
            char temp[LZ];
            memset(temp, 0, LZ);
            memcpy(temp, line + i, strlen(line) - i);
            memset(line, 0, LZ);
            memcpy(line, temp, strlen(temp));
            LOG(logDEBUG1, ("Removing leading spaces.\n"));
        }

        LOG(logDEBUG1, ("Command to process: (size:%d) %.*s\n", strlen(line),
                        strlen(line) - 1, line));
        memset(command, 0, LZ);

        // patword
        if (!strncmp(line, "patword", strlen("patword"))) {
            int addr = 0;
            uint64_t word = 0;

            // cannot scan values
#if defined(VIRTUAL) || defined(XILINX_CHIPTESTBOARDD)
            if (sscanf(line, "%s 0x%x 0x%lx", command, &addr, &word) != 3) {
#else
            if (sscanf(line, "%s 0x%x 0x%llx", command, &addr, &word) != 3) {
#endif
                strcpy(temp, "Could not scan patword arguments.\n");
                break;
            }

            if (validate_writePatternWord(temp, addr, word) == FAIL) {
                break;
            }
        }

        // patioctrl
#if !defined(MYTHEN3D) && !defined(XILINX_CHIPTESTBOARDD) // TODO
        if (!strncmp(line, "patioctrl", strlen("patioctrl"))) {
            uint64_t arg = 0;

            // cannot scan values
#ifdef VIRTUAL
            if (sscanf(line, "%s 0x%lx", command, &arg) != 2) {
#else
            if (sscanf(line, "%s 0x%llx", command, &arg) != 2) {
#endif
                strcpy(temp, "Could not scan patioctrl arguments.\n");
                break;
            }

            if (validate_writePatternIOControl(temp, arg) == FAIL) {
                break;
            }
        }
#endif

        // patlimits
        if (!strncmp(line, "patlimits", strlen("patlimits"))) {
            int startAddr = 0;
            int stopAddr = 0;

            // cannot scan values
            if (sscanf(line, "%s 0x%x 0x%x", command, &startAddr, &stopAddr) !=
                3) {
                strcpy(temp, "Could not scan patlimits arguments.\n");
                break;
            }

            if (validate_setPatternLoopLimits(temp, startAddr, stopAddr) ==
                FAIL) {
                break;
            }
        }

        // patloop
        if (!strncmp(line, "patloop", strlen("patloop"))) {
            int level = -1;
            int startAddr = 0;
            int stopAddr = 0;
            // cannot scan values
            if (sscanf(line, "%s %d 0x%x 0x%x", command, &level, &startAddr,
                       &stopAddr) != 4) {
                strcpy(temp, "Could not scan patloop arguments.\n");
                break;
            }

            if (validate_setPatternLoopAddresses(temp, level, startAddr,
                                                 stopAddr) == FAIL) {
                break;
            }
        }

        // patnloop
        if (!strncmp(line, "patnloop", strlen("patnloop"))) {
            int level = -1;
            int numLoops = -1;
            // cannot scan values
            if (sscanf(line, "%s %d  %d", command, &level, &numLoops) != 3) {
                strcpy(temp, "Could not scan patnloop arguments.\n");
                break;
            }

            if (validate_setPatternLoopCycles(temp, level, numLoops) == FAIL) {
                break;
            }
        }

        // patwait
        if (!strncmp(line, "patwait ", strlen("patwait "))) {
            int level = -1;
            int addr = 0;
            // cannot scan values
            if (sscanf(line, "%s %d 0x%x", command, &level, &addr) != 3) {
                strcpy(temp, "Could not scan patwait arguments.\n");
                break;
            }

            if (validate_setPatternWaitAddresses(temp, level, addr) == FAIL) {
                break;
            }
        }

        // patwaittime
        if (!strncmp(line, "patwaittime", strlen("patwaittime"))) {
            int level = -1;
            uint64_t waittime = 0;

            // cannot scan values
#if defined(VIRTUAL) || defined(XILINX_CHIPTESTBOARDD)
            if (sscanf(line, "%s %d %ld", command, &level, &waittime) != 3) {
#else
            if (sscanf(line, "%s %d %lld", command, &level, &waittime) != 3) {
#endif
                sprintf(temp, "Could not scan patwaittime%d arguments.\n",
                        level);
                break;
            }

            if (validate_setPatternWaitTime(temp, level, waittime) == FAIL) {
                break;
            }
        }

        memset(line, 0, LZ);
    }

    fclose(fd);

    if (strlen(temp)) {
        sprintf(errMessage, "%s(Default pattern file. Line: %s)\n", temp, line);
        return FAIL;
    }

    LOG(logINFOBLUE, ("Successfully read default pattern file\n"));
    return OK;
}
