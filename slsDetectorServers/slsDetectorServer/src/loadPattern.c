#include "loadPattern.h"
#include "clogger.h"
#include "common.h"
#include "sls/ansi.h"
#include "sls/sls_detector_defs.h"
#include "slsDetectorServer_defs.h"

#if defined(CHIPTESTBOARDD) || defined(MOENCHD) || defined(MYTHEN3D)
#include "Pattern.h"
#endif

#include <string.h>
#include <unistd.h>

extern char initErrorMessage[MAX_STR_LENGTH];
extern enum TLogLevel trimmingPrint;

#ifndef MYTHEN3D
extern uint64_t writePatternIOControl(uint64_t word);
#endif
extern uint64_t writePatternWord(int addr, uint64_t word);
extern int setPatternWaitAddress(int level, int addr);
extern uint64_t setPatternWaitTime(int level, uint64_t t);
extern void setPatternLoop(int level, int *startAddr, int *stopAddr,
                           int *nLoop);

int loadPattern(char *message, enum TLogLevel printLevel, patternParameters *pat) {
    LOG(logINFOBLUE, ("Loading Pattern\n"));
    int ret = OK;
    trimmingPrint = printLevel;

    for (int i = 0; i < MAX_PATTERN_LENGTH; ++i) {
        if ((i % 10 == 0) && pat->word[i] != 0) {
            LOG(logDEBUG5, ("Setting Pattern Word (addr:0x%x, word:0x%llx)\n",
                            i, (long long int)pat->word[i]));
        }
        ret = pattern_writeWord(message, i, pat->word[i]);
        if (ret == FAIL) {
            break;
        }
    }
#ifndef MYTHEN3D
    if (ret == OK) {
        ret = pattern_writeIOControl(message, pat->ioctrl);
    }
#endif
    if (ret == OK) {
        ret = pattern_setLoopLimits(message, pat->limits[0], pat->limits[1]);
    }

    if (ret == OK) {
        for (int i = 0; i <= 2; ++i) {
            // loop addr
            ret = pattern_setLoopAddresses(message, i, pat->loop[i * 2 + 0], pat->loop[i * 2 + 1]);
            if (ret == FAIL) {
                break;
            }

            // num loops
            ret = pattern_setLoopCycles(message, i, pat->nloop[i]);
            if (ret == FAIL) {
                break;
            }
           
            // wait addr
            ret = pattern_setWaitAddresses(message, i, pat->wait[i]);
            if (ret == FAIL) {
                break;
            }

            // wait time
            ret = pattern_setWaitTime(message, i, pat->waittime[i]);
            if (ret == FAIL) {
                break;
            }
        }
    }
    trimmingPrint = logINFO;

    return ret;
}

int pattern_writeWord(char *message, uint32_t addr, uint64_t word) {
    // vaiidate input
    if ((int32_t)addr < 0 || addr >= MAX_PATTERN_LENGTH) {
        sprintf(message,
                "Cannot set pattern word. Addr must be between 0 and 0x%x.\n",
                MAX_PATTERN_LENGTH);
        LOG(logERROR, (message));
        return FAIL;
    }

    writePatternWord(addr, word);
    
    // validate result
    int ret = OK;
    // cannot validate for moench, ctb ( same as executing pattern word)
#ifdef MYTHEN3D
    uint64_t retval = readPatternWord(addr);
    char mode[128];
    memset(mode, 0, sizeof(mode));
    sprintf(mode, "set pattern word for addr 0x%x", addr);
    validate64(&ret, message, word, retval, "set pattern word", HEX);
#endif
    return ret;
}

#ifndef MYTHEN3D
int pattern_writeIOControl(char *message, uint64_t arg) {
    uint64_t retval = writePatternIOControl(arg);

    // validate result
    int ret = OK;
    validate64(&ret, message, arg, retval, "set pattern IO Control", HEX);
    return ret;
}
#endif

int pattern_setLoopLimits(char *message, uint32_t startAddr,
                                 uint32_t stopAddr) {
    // vaiidate input
    if ((int32_t)startAddr < 0 || startAddr >= MAX_PATTERN_LENGTH ||
        (int32_t)stopAddr < 0 || stopAddr >= MAX_PATTERN_LENGTH) {
        sprintf(message,
                "Cannot set patlimits from default "
                "pattern file. Addr must be between 0 and 0x%x.\n",
                MAX_PATTERN_LENGTH);
        LOG(logERROR, (message));
        return FAIL;
    }

    int numLoops = -1;
    int r_startAddr = startAddr, r_stopAddr = stopAddr;
    setPatternLoop(-1, &r_startAddr, &r_stopAddr, &numLoops);

     // validate result
    int ret = OK;
    // start addr
    validate(&ret, message, startAddr, r_startAddr, "set pattern Limits start addr", HEX);
    if (ret == FAIL) {
        return FAIL;
    }
    // stop addr
    validate(&ret, message, stopAddr, r_stopAddr, "set pattern Limits stop addr", HEX);
    return ret;

}

int pattern_setLoopAddresses(char *message, int level, uint32_t startAddr, uint32_t stopAddr) {
    // vaiidate input
    if (level < 0 || level > 2) {
        sprintf(message,
                "Cannot set patloop. Level must be between 0 and 2.\n");
        LOG(logERROR, (message));
        return FAIL;
    }
    if ((int32_t)startAddr < 0 || startAddr >= MAX_PATTERN_LENGTH ||
        (int32_t)stopAddr < 0 || stopAddr >= MAX_PATTERN_LENGTH) {
        sprintf(message,
                "Cannot set patloop (level: %d). Addr must be between 0 and 0x%x.\n",
                level, MAX_PATTERN_LENGTH);
        LOG(logERROR, (message));
        return FAIL;
    }

    int numLoops = -1;
    int r_startAddr = startAddr, r_stopAddr = stopAddr;
    setPatternLoop(level, &r_startAddr, &r_stopAddr, &numLoops);

     // validate result
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

int pattern_setLoopCycles(char *message, int level, int numLoops) {
    // vaiidate input
    if (level < 0 || level > 2) {
        sprintf(message,
                "Cannot set patnloop. Level must be between 0 and 2.\n");
        LOG(logERROR, (message));
        return FAIL;
    }
    if (numLoops < 0) {
        sprintf(message,
                "Cannot set patnloop. Iterations must be between > 0.\n");
        LOG(logERROR, (message));
        return FAIL;
    }

    int startAddr = -1;
    int stopAddr = -1;
    int r_numLoops = numLoops;
    setPatternLoop(level, &startAddr, &stopAddr, &r_numLoops);

    // validate result
    int ret = OK;
    char mode[128];
    memset(mode, 0, sizeof(mode));
    sprintf(mode, "set pattern Loop %d num loops", level);
    validate(&ret, message, numLoops, r_numLoops, mode, DEC);
    return ret;
}

int pattern_setWaitAddresses(char *message, int level, uint32_t addr) {
    // validate input
    if (level < 0 || level > 2) {
        sprintf(message,
                "Cannot set patwait address. Level must be between 0 and 2.\n");
        LOG(logERROR, (message));
        return FAIL;
    }
    if ((int32_t)addr < 0 || addr >= MAX_PATTERN_LENGTH) {
        sprintf(message,
                "Cannot set patwait address (level: %d). Addr must be between 0 and 0x%x.\n", level, MAX_PATTERN_LENGTH);
        LOG(logERROR, (message));
        return FAIL;
    }

    uint32_t retval = setPatternWaitAddress(level, addr);

    // validate result
    int ret = OK;
    char mode[128];
    memset(mode, 0, sizeof(mode));
    sprintf(mode, "set pattern Loop %d wait address", level);
    validate(&ret, message, addr, retval, mode, HEX);
    return ret;
}

int pattern_setWaitTime(char *message, int level, uint64_t waittime) {
    memset(message, 0, sizeof(message));

    // validate input
    if (level < 0 || level > 2) {
        sprintf(message,
                "Cannot set patwaittime. Level must be between 0 and 2.\n");
        LOG(logERROR, (message));
        return FAIL;
    }

    uint64_t retval = setPatternWaitTime(level, waittime);

    // validate result
    int ret = OK;
    char mode[128];
    memset(mode, 0, sizeof(mode));
    sprintf(mode, "set pattern Loop %d wait time", level);
    validate64(&ret, message, waittime, retval, mode, DEC);
    return ret;
}
