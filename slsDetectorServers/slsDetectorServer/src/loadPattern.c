#include "loadPattern.h"
#include "common.h"
#include "sls/ansi.h"
#include "sls/sls_detector_defs.h"
#include "slsDetectorServer_defs.h"

#include <string.h>
#include <unistd.h>

#ifndef MYTHEN3D
extern uint64_t writePatternIOControl(uint64_t word);
#else
extern enum TLogLevel trimmingPrint;
extern uint64_t readPatternWord(int addr);
#endif
extern uint64_t writePatternWord(int addr, uint64_t word);
extern int setPatternWaitAddress(int level, int addr);
extern uint64_t setPatternWaitTime(int level, uint64_t t);
extern void setPatternLoop(int level, int *startAddr, int *stopAddr,
                           int *nLoop);

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

int pattern_setLoopAddresses(char *message, int level, uint32_t startAddr,
                             uint32_t stopAddr) {
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
                "Cannot set patloop (level: %d). Addr must be between 0 and "
                "0x%x.\n",
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
                "Cannot set patwait address (level: %d). Addr must be between "
                "0 and 0x%x.\n",
                level, MAX_PATTERN_LENGTH);
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

int loadPattern(char *message, enum TLogLevel printLevel,
                patternParameters *pat) {
    LOG(logINFOBLUE, ("Loading Pattern\n"));
    int ret = OK;
#ifdef MYTHEN3D
    trimmingPrint = printLevel;
#endif

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
            ret = pattern_setLoopAddresses(message, i, pat->loop[i * 2 + 0],
                                           pat->loop[i * 2 + 1]);
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
#ifdef MYTHEN3D
    trimmingPrint = logINFO;
#endif
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
            uint32_t addr = 0;
            uint64_t word = 0;

            // cannot scan values
#ifdef VIRTUAL
            if (sscanf(line, "%s 0x%x 0x%lx", command, &addr, &word) != 3) {
#else
            if (sscanf(line, "%s 0x%x 0x%llx", command, &addr, &word) != 3) {
#endif
                strcpy(temp, "Could not scan patword arguments.\n");
                break;
            }

            if (pattern_writeWord(temp, addr, word) == FAIL) {
                break;
            }
        }

        // patioctrl
#ifndef MYTHEN3D
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

            if (pattern_writeIOControl(temp, arg) == FAIL) {
                break;
            }
        }
#endif

        // patlimits
        if (!strncmp(line, "patlimits", strlen("patlimits"))) {
            uint32_t startAddr = 0;
            uint32_t stopAddr = 0;

            // cannot scan values
            if (sscanf(line, "%s 0x%x 0x%x", command, &startAddr, &stopAddr) !=
                3) {
                strcpy(temp, "Could not scan patlimits arguments.\n");
                break;
            }

            if (pattern_setLoopLimits(temp, startAddr, stopAddr) == FAIL) {
                break;
            }
        }

        // patloop
        if ((!strncmp(line, "patloop0", strlen("patloop0"))) ||
            (!strncmp(line, "patloop1", strlen("patloop1"))) ||
            (!strncmp(line, "patloop2", strlen("patloop2")))) {

            // level
            int level = -1;
            if (!strncmp(line, "patloop0", strlen("patloop0"))) {
                level = 0;
            } else if (!strncmp(line, "patloop1", strlen("patloop1"))) {
                level = 1;
            } else {
                level = 2;
            }

            uint32_t startAddr = 0;
            uint32_t stopAddr = 0;
            // cannot scan values
            if (sscanf(line, "%s 0x%x 0x%x", command, &startAddr, &stopAddr) !=
                3) {
                sprintf(temp, "Could not scan patloop%d arguments.\n", level);
                break;
            }

            if (pattern_setLoopAddresses(temp, level, startAddr, stopAddr) ==
                FAIL) {
                break;
            }
        }

        // patnloop
        if ((!strncmp(line, "patnloop0", strlen("patnloop0"))) ||
            (!strncmp(line, "patnloop1", strlen("patnloop1"))) ||
            (!strncmp(line, "patnloop2", strlen("patnloop2")))) {

            // level
            int level = -1;
            if (!strncmp(line, "patnloop0", strlen("patnloop0"))) {
                level = 0;
            } else if (!strncmp(line, "patnloop1", strlen("patnloop1"))) {
                level = 1;
            } else {
                level = 2;
            }

            int numLoops = -1;
            // cannot scan values
            if (sscanf(line, "%s %d", command, &numLoops) != 2) {
                sprintf(temp, "Could not scan patnloop %d arguments.\n", level);
                break;
            }

            if (pattern_setLoopCycles(temp, level, numLoops) == FAIL) {
                break;
            }
        }

        // patwait
        if ((!strncmp(line, "patwait0", strlen("patwait0"))) ||
            (!strncmp(line, "patwait1", strlen("patwait1"))) ||
            (!strncmp(line, "patwait2", strlen("patwait2")))) {

            // level
            int level = -1;
            if (!strncmp(line, "patwait0", strlen("patwait0"))) {
                level = 0;
            } else if (!strncmp(line, "patwait1", strlen("patwait1"))) {
                level = 1;
            } else {
                level = 2;
            }

            uint32_t addr = 0;
            // cannot scan values
            if (sscanf(line, "%s 0x%x", command, &addr) != 2) {
                sprintf(temp, "Could not scan patwait%d arguments.\n", level);
                break;
            }

            if (pattern_setWaitAddresses(temp, level, addr) == FAIL) {
                break;
            }
        }

        // patwaittime
        if ((!strncmp(line, "patwaittime0", strlen("patwaittime0"))) ||
            (!strncmp(line, "patwaittime1", strlen("patwaittime1"))) ||
            (!strncmp(line, "patwaittime2", strlen("patwaittime2")))) {

            // level
            int level = -1;
            if (!strncmp(line, "patwaittime0", strlen("patwaittime0"))) {
                level = 0;
            } else if (!strncmp(line, "patwaittime1", strlen("patwaittime1"))) {
                level = 1;
            } else {
                level = 2;
            }

            uint64_t waittime = 0;

            // cannot scan values
#ifdef VIRTUAL
            if (sscanf(line, "%s %ld", command, &waittime) != 2) {
#else
            if (sscanf(line, "%s %lld", command, &waittime) != 2) {
#endif
                sprintf(temp, "Could not scan patwaittime%d arguments.\n",
                        level);
                break;
            }

            if (pattern_setWaitTime(temp, level, waittime) == FAIL) {
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