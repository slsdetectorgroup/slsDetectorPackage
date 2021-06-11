#include "loadPattern.h"
#include "clogger.h"
#include "common.h"
#include "readDefaultPattern.h"
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

int loadPattern(enum TLogLevel printLevel, patternParameters *pat) {
    LOG(logINFOBLUE, ("Loading Pattern\n"));
    int ret = OK;
    trimmingPrint = printLevel;

    for (int i = 0; i < MAX_PATTERN_LENGTH; ++i) {
        if ((i % 10 == 0) && pat->word[i] != 0) {
            LOG(logDEBUG5, ("Setting Pattern Word (addr:0x%x, word:0x%llx)\n",
                            i, (long long int)pat->word[i]));
        }
        writePatternWord(i, pat->word[i]);
    }
#ifndef MYTHEN3D
    if (ret == OK) {
        uint64_t retval64 = writePatternIOControl(pat->ioctrl);
        validate64(pat->ioctrl, retval64, "set pattern IO Control", HEX);
    }
#endif
    if (ret == OK) {
        int numLoops = -1;
        int retval0 = pat->limits[0];
        int retval1 = pat->limits[1];
        setPatternLoop(-1, &retval0, &retval1, &numLoops);
        validate(pat->limits[0], retval0, "set pattern Limits start address",
                 HEX);
        validate(pat->limits[1], retval1, "set pattern Limits start address",
                 HEX);
    }
    uint64_t retval64;
    if (ret == OK) {
        for (int i = 0; i <= 2; ++i) {
            char msg[128];
            int retval0 = -1, retval1 = -1, numLoops = -1;

            // patloop
            retval0 = pat->loop[i * 2 + 0];
            retval1 = pat->loop[i * 2 + 1];
            numLoops = pat->nloop[i];
            setPatternLoop(i, &retval0, &retval1, &numLoops);
            memset(msg, 0, sizeof(msg));
            sprintf(msg, "set pattern Loop %d start address", i);
            validate(pat->loop[i * 2 + 0], retval0, msg, HEX);
            if (ret == FAIL) {
                break;
            }
            memset(msg, 0, sizeof(msg));
            sprintf(msg, "set pattern Loop %d stop address", i);
            validate(pat->loop[i * 2 + 1], retval1, msg, HEX);
            if (ret == FAIL) {
                break;
            }
            memset(msg, 0, sizeof(msg));
            sprintf(msg, "set pattern Loop %d num loops", i);
            validate(pat->nloop[i], numLoops, msg, HEX);
            if (ret == FAIL) {
                break;
            }
            // patwait
            memset(msg, 0, sizeof(msg));
            sprintf(msg, "set pattern Loop %d wait address", i);
            retval0 = setPatternWaitAddress(i, pat->wait[i]);
            validate(pat->wait[i], retval0, msg, HEX);
            if (ret == FAIL) {
                break;
            }

            // patwaittime
            memset(msg, 0, sizeof(msg));
            sprintf(msg, "set pattern Loop %d wait time", i);
            retval64 = setPatternWaitTime(i, pat->waittime[i]);
            validate64(pat->waittime[i], retval64, msg, HEX);
            if (retval64 == FAIL) {
                break;
            }
        }
    }
    trimmingPrint = logINFO;

    return ret;
}
