#ifndef LOADPATTERN_H
#define LOADPATTERN_H
#include "Pattern.h"
#include "clogger.h"

int pattern_writeWord(char *message, uint32_t addr, uint64_t word);
#ifndef MYTHEN3D
int pattern_writeIOControl(char *message, uint64_t arg);
#endif
int pattern_setLoopLimits(char *message, uint32_t startAddr, uint32_t stopAddr);
int pattern_setLoopAddresses(char *message, int level, uint32_t startAddr,
                             uint32_t stopAddr);
int pattern_setLoopCycles(char *message, int level, int numLoops);
int pattern_setWaitAddresses(char *message, int level, uint32_t addr);
int pattern_setWaitTime(char *message, int level, uint64_t waittime);
int loadPattern(char *mess, enum TLogLevel printLevel, patternParameters *pat);
int loadPatternFile(char *patFname, char *errMessage);
#endif
