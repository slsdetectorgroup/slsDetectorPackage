#pragma once

#include <inttypes.h>
#include <sys/types.h>

int loadDefaultPattern(char *patFname, char *errMessage);

int default_writePatternWord(char *line, uint32_t addr, uint64_t word);

#ifndef MYTHEN3D
int default_writePatternIOControl(char *line, uint64_t arg);
#endif

int default_setPatternLoopLimits(char *line, uint32_t startAddr,
                                 uint32_t stopAddr);

int default_setPatternLoopAddresses(char *line, int level, uint32_t startAddr,
                                    uint32_t stopAddr);

int default_setPatternLoopCycles(char *line, int level, int numLoops);

int default_setPatternWaitAddresses(char *line, int level, uint32_t addr);

int default_setPatternWaitTime(char *line, int level, uint64_t waittime);