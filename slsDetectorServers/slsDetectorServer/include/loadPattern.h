// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#ifndef LOADPATTERN_H
#define LOADPATTERN_H
#include "Pattern.h"
#include "clogger.h"

void initializePatternAddresses();
#if defined(CHIPTESTBOARDD) || defined(XILINX_CHIPTESTBOARDD)
#ifdef VIRTUAL
void initializePatternWord();
#endif
#endif
#if defined(CHIPTESTBOARDD) || defined(XILINX_CHIPTESTBOARDD)
uint64_t validate_readPatternIOControl();
int validate_writePatternIOControl(char *message, uint64_t arg);
void writePatternIOControl(uint64_t word);
#endif

int validate_readPatternWord(char *message, int addr, uint64_t *word);
uint64_t readPatternWord(int addr);
int validate_writePatternWord(char *message, int addr, uint64_t word);
void writePatternWord(int addr, uint64_t word);

int validate_getPatternWaitAddresses(char *message, int level, int *addr);
int getPatternWaitAddress(int level);
int validate_setPatternWaitAddresses(char *message, int level, int addr);
void setPatternWaitAddress(int level, int addr);

int validate_getPatternWaitTime(char *message, int level, uint64_t *waittime);
uint64_t getPatternWaitTime(int level);
int validate_setPatternWaitTime(char *message, int level, uint64_t waittime);
void setPatternWaitTime(int level, uint64_t t);

int validate_getPatternLoopCycles(char *message, int level, int *numLoops);
int getPatternLoopCycles(int level);
int validate_setPatternLoopCycles(char *message, int level, int numLoops);
void setPatternLoopCycles(int level, int nLoop);

void validate_getPatternLoopLimits(int *startAddr, int *stopAddr);
int validate_setPatternLoopLimits(char *message, int startAddr, int stopAddr);
void setPatternLoopLimits(int startAddr, int stopAddr);

int validate_getPatternLoopAddresses(char *message, int level, int *startAddr,
                                     int *stopAddr);
void getPatternLoopAddresses(int level, int *startAddr, int *stopAddr);
int validate_setPatternLoopAddresses(char *message, int level, int startAddr,
                                     int stopAddr);
void setPatternLoopAddresses(int level, int startAddr, int stopAddr);

void setPatternMask(uint64_t mask);
uint64_t getPatternMask();
void setPatternBitMask(uint64_t mask);
uint64_t getPatternBitMask();

#ifdef MYTHEN3D
void startPattern();
#endif
char *getPatternFileName();
int loadPattern(char *mess, enum TLogLevel printLevel, patternParameters *pat,
                char *patfname);
int getPattern(char *mess, patternParameters *pat);
int loadPatternFile(char *patFname, char *errMessage);

#endif
