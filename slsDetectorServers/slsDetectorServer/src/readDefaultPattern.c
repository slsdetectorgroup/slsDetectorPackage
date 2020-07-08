#include "readDefaultPattern.h"
#include "ansi.h"
#include "clogger.h"
#include "slsDetectorServer_defs.h"
#include "sls_detector_defs.h"

#include <string.h>

extern char initErrorMessage[MAX_STR_LENGTH];
extern int initError;

#ifndef MYTHEN3D
extern uint64_t writePatternIOControl(uint64_t word);
#endif
extern uint64_t writePatternWord(int addr, uint64_t word);
extern int setPatternWaitAddress(int level, int addr);
extern uint64_t setPatternWaitTime(int level, uint64_t t);
extern void setPatternLoop(int level, int *startAddr, int *stopAddr,
                           int *nLoop);

int loadDefaultPattern(char *fname) {
    if (initError == FAIL) {
        return initError;
    }

    FILE *fd = fopen(fname, "r");
    if (fd == NULL) {
        sprintf(initErrorMessage, "Could not open pattern file [%s].\n", fname);
        initError = FAIL;
        LOG(logERROR, ("%s\n\n", initErrorMessage));
        return FAIL;
    }
    LOG(logINFOBLUE, ("Reading default pattern file %s\n", fname));

    // Initialization
    const size_t LZ = 256;
    char line[LZ];
    memset(line, 0, LZ);
    char command[LZ];

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
                sprintf(initErrorMessage,
                        "Could not scan patword arguments from default "
                        "pattern file. Line:[%s].\n",
                        line);
                break;
            }

            if (default_writePatternWord(line, addr, word) == FAIL) {
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
                sprintf(initErrorMessage,
                        "Could not scan patioctrl arguments from default "
                        "pattern file. Line:[%s].\n",
                        line);
                break;
            }

            if (default_writePatternIOControl(line, arg) == FAIL) {
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
                sprintf(initErrorMessage,
                        "Could not scan patlimits arguments from default "
                        "pattern file. Line:[%s].\n",
                        line);
                break;
            }

            if (default_setPatternLoopLimits(line, startAddr, stopAddr) ==
                FAIL) {
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
                sprintf(initErrorMessage,
                        "Could not scan patloop%d arguments from default "
                        "pattern file. Line:[%s].\n",
                        level, line);
                break;
            }

            if (default_setPatternLoopAddresses(line, level, startAddr,
                                                stopAddr) == FAIL) {
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
                sprintf(initErrorMessage,
                        "Could not scan patnloop%d arguments from default "
                        "pattern file. Line:[%s].\n",
                        level, line);
                break;
            }

            if (default_setPatternLoopCycles(line, level, numLoops) == FAIL) {
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
                sprintf(initErrorMessage,
                        "Could not scan patwait%d arguments from default "
                        "pattern file. Line:[%s].\n",
                        level, line);
                break;
            }

            if (default_setPatternWaitAddresses(line, level, addr) == FAIL) {
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
                sprintf(initErrorMessage,
                        "Could not scan patwaittime%d arguments from default "
                        "pattern file. Line:[%s].\n",
                        level, line);
                break;
            }

            if (default_setPatternWaitTime(line, level, waittime) == FAIL) {
                break;
            }
        }

        memset(line, 0, LZ);
    }
    fclose(fd);

    if (strlen(initErrorMessage)) {
        initError = FAIL;
        LOG(logERROR, ("%s\n\n", initErrorMessage));
    } else {
        LOG(logINFOBLUE, ("Successfully read default pattern file\n"));
    }
    return initError;
}

int default_writePatternWord(char *line, uint32_t addr, uint64_t word) {
    // validations
    if ((int32_t)addr < 0 || addr >= MAX_PATTERN_LENGTH) {
        sprintf(initErrorMessage,
                "Cannot set pattern word from default "
                "pattern file. Addr must be between 0 and 0x%x. Line:[%s]\n",
                MAX_PATTERN_LENGTH, line);
        return FAIL;
    }
    writePatternWord(addr, word);
    // cannot validate for moench, ctb ( same as executing pattern word)
    return OK;
}

#ifndef MYTHEN3D
int default_writePatternIOControl(char *line, uint64_t arg) {
    uint64_t retval = writePatternIOControl(arg);
    if (retval != arg) {
#ifdef VIRTUAL
        sprintf(initErrorMessage,
                "Could not set patioctrl from default pattern "
                "file. Set 0x%lx, read 0x%lx. Line:[%s]\n",
                arg, retval, line);
#else
        sprintf(initErrorMessage,
                "Could not set patioctrl from default pattern "
                "file. Set 0x%llx, read 0x%llx. Line:[%s]\n",
                arg, retval, line);
#endif
        return FAIL;
    }
    return OK;
}
#endif

int default_setPatternLoopLimits(char *line, uint32_t startAddr,
                                 uint32_t stopAddr) {
    // validations
    if ((int32_t)startAddr < 0 || startAddr >= MAX_PATTERN_LENGTH ||
        (int32_t)stopAddr < 0 || stopAddr >= MAX_PATTERN_LENGTH) {
        sprintf(initErrorMessage,
                "Cannot set patlimits from default "
                "pattern file. Addr must be between 0 and 0x%x. Line:[%s]\n",
                MAX_PATTERN_LENGTH, line);
        return FAIL;
    }
    int numLoops = -1;
    int r_startAddr = startAddr, r_stopAddr = stopAddr;
    setPatternLoop(-1, &r_startAddr, &r_stopAddr, &numLoops);

    // validate
    if (r_startAddr != (int)startAddr || r_stopAddr != (int)stopAddr) {
        sprintf(initErrorMessage,
                "Could not set patlimits from default pattern "
                "file. Read start addr:0x%x, stop addr: 0x%x. Line:[%s]\n",
                r_startAddr, r_stopAddr, line);
        return FAIL;
    }
    return OK;
}

int default_setPatternLoopAddresses(char *line, int level, uint32_t startAddr,
                                    uint32_t stopAddr) {
    // validations
    if (level < 0 || level > 2) {
        sprintf(initErrorMessage,
                "Cannot set patloop from default "
                "pattern file. Level must be between 0 and 2. Line:[%s]\n",
                line);
        return FAIL;
    }
    if ((int32_t)startAddr < 0 || startAddr >= MAX_PATTERN_LENGTH ||
        (int32_t)stopAddr < 0 || stopAddr >= MAX_PATTERN_LENGTH) {
        sprintf(initErrorMessage,
                "Cannot set patloop (level: %d) from default "
                "pattern file. Addr must be between 0 and 0x%x. Line:[%s]\n",
                level, MAX_PATTERN_LENGTH, line);
        return FAIL;
    }
    int numLoops = -1;
    int r_startAddr = startAddr, r_stopAddr = stopAddr;
    setPatternLoop(level, &r_startAddr, &r_stopAddr, &numLoops);

    // validate
    if (r_startAddr != (int)startAddr || r_stopAddr != (int)stopAddr) {
        sprintf(
            initErrorMessage,
            "Could not set patloop (level: %d) from default "
            "pattern file. Read start addr:0x%x, stop addr: 0x%x. Line:[%s]\n",
            level, r_startAddr, r_stopAddr, line);
        return FAIL;
    }
    return OK;
}

int default_setPatternLoopCycles(char *line, int level, int numLoops) {
    // validations
    if (level < 0 || level > 2) {
        sprintf(initErrorMessage,
                "Cannot set patnloop from default "
                "pattern file. Level must be between 0 and 2. Line:[%s]\n",
                line);
        return FAIL;
    }
    if (numLoops < 0) {
        sprintf(initErrorMessage,
                "Cannot set patnloop from default "
                "pattern file. Iterations must be between > 0. Line:[%s]\n",
                line);
        return FAIL;
    }
    int startAddr = -1;
    int stopAddr = -1;
    int r_numLoops = numLoops;
    setPatternLoop(level, &startAddr, &stopAddr, &r_numLoops);

    // validate
    if (r_numLoops != numLoops) {
        sprintf(initErrorMessage,
                "Could not set patnloop (level: %d) from default "
                "pattern file. Read %d loops. Line:[%s]\n",
                level, r_numLoops, line);
        return FAIL;
    }
    return OK;
}

int default_setPatternWaitAddresses(char *line, int level, uint32_t addr) {
    // validations
    if (level < 0 || level > 2) {
        sprintf(initErrorMessage,
                "Cannot set patwait address from default "
                "pattern file. Level must be between 0 and 2. Line:[%s]\n",
                line);
        return FAIL;
    }
    if ((int32_t)addr < 0 || addr >= MAX_PATTERN_LENGTH) {
        sprintf(initErrorMessage,
                "Cannot set patwait address (level: %d) from default "
                "pattern file. Addr must be between 0 and 0x%x. Line:[%s]\n",
                level, MAX_PATTERN_LENGTH, line);
        return FAIL;
    }

    uint32_t retval = setPatternWaitAddress(level, addr);

    // validate
    if (retval != addr) {
        sprintf(initErrorMessage,
                "Could not set patwait address (level: %d) from default "
                "pattern file. Read addr: 0x%x. Line:[%s]\n",
                level, retval, line);
        return FAIL;
    }
    return OK;
}

int default_setPatternWaitTime(char *line, int level, uint64_t waittime) {
    // validations
    if (level < 0 || level > 2) {
        sprintf(initErrorMessage,
                "Cannot set patwaittime from default "
                "pattern file. Level must be between 0 and 2. Line:[%s]\n",
                line);
        return FAIL;
    }
    uint64_t retval = setPatternWaitTime(level, waittime);

    // validate
    if (retval != waittime) {
#ifdef VIRTUAL
        sprintf(initErrorMessage,
                "Could not set patwaittime (level: %d) from default "
                "pattern file. Read %ld wait time. Line:[%s]\n",
                level, retval, line);
#else
        sprintf(initErrorMessage,
                "Could not set patwaittime (level: %d) from default "
                "pattern file. Read %lld wait time. Line:[%s]\n",
                level, retval, line);
#endif
        return FAIL;
    }
    return OK;
}
