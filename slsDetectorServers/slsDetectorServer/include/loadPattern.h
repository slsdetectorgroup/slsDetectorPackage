#ifndef LOADPATTERN_H
#define LOADPATTERN_H
#include "Pattern.h"
#include "clogger.h"

int loadPattern(char *mess, enum TLogLevel printLevel, patternParameters *pat);
int getPattern(char *mess, patternParameters *pat);
int loadPatternFile(char *patFname, char *errMessage);

#if defined(CHIPTESTBOARDD) || defined(MOENCHD)
#ifdef VIRTUAL
void initializePatternWord();

#endif
uint64_t pattern_readIOControl();
int pattern_writeIOControl(char *message, uint64_t arg);
void writePatternIOControl(uint64_t word);
#endif

int pattern_readWord(char *message, int addr, uint64_t *word);
uint64_t readPatternWord(int addr);
int pattern_writeWord(char *message, int addr, uint64_t word);
void writePatternWord(int addr, uint64_t word);

int pattern_getWaitAddresses(char *message, int level, int *addr);
int getPatternWaitAddress(int level);
int pattern_setWaitAddresses(char *message, int level, int addr);
void setPatternWaitAddress(int level, int addr);

int pattern_getWaitTime(char *message, int level, uint64_t *waittime);
uint64_t getPatternWaitTime(int level);
int pattern_setWaitTime(char *message, int level, uint64_t waittime);
void setPatternWaitTime(int level, uint64_t t);

int pattern_getLoopCycles(char *message, int level, int *numLoops);
int getPatternLoopCycles(int level);
int pattern_setLoopCycles(char *message, int level, int numLoops);
void setPatternLoopCycles(int level, int nLoop);

void pattern_getLoopLimits(int *startAddr, int *stopAddr);
int pattern_setLoopLimits(char *message, int startAddr, int stopAddr);
void setPatternLoopLimits(int startAddr, int stopAddr);

int pattern_getLoopAddresses(char *message, int level, int *startAddr,
                             int *stopAddr);
void getPatternLoopAddresses(int level, int *startAddr, int *stopAddr);
int pattern_setLoopAddresses(char *message, int level, int startAddr,
                             int stopAddr);
void setPatternLoopAddresses(int level, int startAddr, int stopAddr);

void setPatternMask(uint64_t mask);
uint64_t getPatternMask();
void setPatternBitMask(uint64_t mask);
uint64_t getPatternBitMask();

#ifdef MYTHEN3D
void startPattern();
#endif
#endif
