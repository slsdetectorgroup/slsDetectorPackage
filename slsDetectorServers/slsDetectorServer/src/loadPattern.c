#include "loadPattern.h"
#include "RegisterDefs.h"
#include "common.h"
#include "sls/ansi.h"
#include "sls/sls_detector_defs.h"
#include "slsDetectorServer_defs.h"

#include <string.h>
#include <unistd.h>

#ifdef MYTHEN3D
extern enum TLogLevel trimmingPrint;
#endif

#if defined(CHIPTESTBOARDD) || defined(MOENCHD)
#ifdef VIRTUAL
uint64_t virtual_pattern[MAX_PATTERN_LENGTH];
#endif
#endif

extern void bus_w(u_int32_t offset, u_int32_t data);
extern u_int32_t bus_r(u_int32_t offset);
extern int64_t get64BitReg(int aLSB, int aMSB);
extern int64_t set64BitReg(int64_t value, int aLSB, int aMSB);

int loadPattern(char *message, enum TLogLevel printLevel,
                patternParameters *pat) {
    LOG(logINFOBLUE, ("Loading Pattern from structure\n"));
    int ret = OK;
#ifdef MYTHEN3D
    trimmingPrint = printLevel;
#endif
    // words
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
    // iocontrol
#ifndef MYTHEN3D
    if (ret == OK) {
        ret = pattern_writeIOControl(message, pat->ioctrl);
    }
#endif
    // limits
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

int getPattern(char *message, patternParameters *pat) {
    LOG(logINFO, ("Getting Pattern into structure\n"));

    int ret = OK;
    uint64_t retval64 = 0;
    int retval1 = -1, retval2 = -1;
    // words
    for (int i = 0; i < MAX_PATTERN_LENGTH; ++i) {
        ret = pattern_readWord(message, i, &retval64);
        if (ret == FAIL) {
            break;
        }
        pat->word[i] = retval64;
    }
    // iocontrol
#ifndef MYTHEN3D
    if (ret == OK) {
        pattern_readIOControl();
    }
#endif
    // limits
    if (ret == OK) {
        pattern_getLoopLimits(&retval1, &retval2);
        pat->limits[0] = retval1;
        pat->limits[1] = retval2;
    }
    if (ret == OK) {
        for (int i = 0; i <= 2; ++i) {
            // loop addr
            ret = pattern_getLoopAddresses(message, i, &retval1, &retval2);
            if (ret == FAIL) {
                break;
            }
            pat->loop[i * 2 + 0] = retval1;
            pat->loop[i * 2 + 1] = retval2;

            // num loops
            ret = pattern_getLoopCycles(message, i, &retval1);
            if (ret == FAIL) {
                break;
            }
            pat->nloop[i] = retval1;

            // wait addr
            ret = pattern_getWaitAddresses(message, i, &retval1);
            if (ret == FAIL) {
                break;
            }
            pat->wait[i] = retval1;

            // wait time
            ret = pattern_getWaitTime(message, i, &retval64);
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
            int startAddr = 0;
            int stopAddr = 0;

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

            int startAddr = 0;
            int stopAddr = 0;
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

            int addr = 0;
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

#if defined(CHIPTESTBOARDD) || defined(MOENCHD)
#ifdef VIRTUAL
void initializePatternWord() {
    memset(virtual_pattern, 0, sizeof(virtual_pattern));
}
#endif

uint64_t pattern_readIOControl() {
    return get64BitReg(PATTERN_IO_CNTRL_LSB_REG, PATTERN_IO_CNTRL_MSB_REG);
}

int pattern_writeIOControl(char *message, uint64_t arg) {
    writePatternIOControl(arg);

    // validate result
    uint64_t retval = pattern_readIOControl();
    LOG(logDEBUG1,
        ("Pattern IO Control retval: 0x%llx\n", (long long int)retval));
    int ret = OK;
    validate64(&ret, message, arg, retval, "set pattern IO Control", HEX);
    return ret;
}

void writePatternIOControl(uint64_t word) {
    LOG(logINFO,
        ("Setting Pattern I/O Control: 0x%llx\n", (long long int)word));
    set64BitReg(word, PATTERN_IO_CNTRL_LSB_REG, PATTERN_IO_CNTRL_MSB_REG);
}
#endif

int pattern_readWord(char *message, int addr, uint64_t *word) {
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
    return get64BitReg(reg_lsb, reg_msb);
#else
    LOG(logINFORED, ("  Reading (Executing) Pattern Word (addr:0x%x)\n", addr));
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
    return get64BitReg(PATTERN_OUT_LSB_REG, PATTERN_OUT_MSB_REG);
#else
    return virtual_pattern[addr];
#endif
#endif
}

int pattern_writeWord(char *message, int addr, uint64_t word) {
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
    // cannot validate for moench, ctb ( same as executing pattern word)
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
    set64BitReg(word, PATTERN_IN_LSB_REG, PATTERN_IN_MSB_REG);

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
    set64BitReg(word, reg_lsb, reg_msb);
#endif
}

int pattern_getWaitAddresses(char *message, int level, int *addr) {
    // validate input
    if (level < 0 || level > 2) {
        sprintf(message,
                "Cannot get patwait address. Level must be between 0 and 2.\n");
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
    default:
        return -1;
    }
}

int pattern_setWaitAddresses(char *message, int level, int addr) {
    // validate input
    if (level < 0 || level > 2) {
        sprintf(message,
                "Cannot set patwait address. Level must be between 0 and 2.\n");
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
    default:
        return;
    }
}

int pattern_getWaitTime(char *message, int level, uint64_t *waittime) {
    // validate input
    if (level < 0 || level > 2) {
        sprintf(message,
                "Cannot get patwaittime. Level must be between 0 and 2.\n");
        LOG(logERROR, (message));
        return FAIL;
    }
    *waittime = getPatternWaitTime(level);
    return OK;
}

uint64_t getPatternWaitTime(int level) {
    switch (level) {
    case 0:
        return get64BitReg(PATTERN_WAIT_TIMER_0_LSB_REG,
                           PATTERN_WAIT_TIMER_0_MSB_REG);
    case 1:
        return get64BitReg(PATTERN_WAIT_TIMER_1_LSB_REG,
                           PATTERN_WAIT_TIMER_1_MSB_REG);
    case 2:
        return get64BitReg(PATTERN_WAIT_TIMER_2_LSB_REG,
                           PATTERN_WAIT_TIMER_2_MSB_REG);
    default:
        return -1;
    }
}

int pattern_setWaitTime(char *message, int level, uint64_t waittime) {
    // validate input
    if (level < 0 || level > 2) {
        sprintf(message,
                "Cannot set patwaittime. Level must be between 0 and 2.\n");
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
        set64BitReg(t, PATTERN_WAIT_TIMER_0_LSB_REG,
                    PATTERN_WAIT_TIMER_0_MSB_REG);
        break;
    case 1:
        set64BitReg(t, PATTERN_WAIT_TIMER_1_LSB_REG,
                    PATTERN_WAIT_TIMER_1_MSB_REG);
        break;
    case 2:
        set64BitReg(t, PATTERN_WAIT_TIMER_2_LSB_REG,
                    PATTERN_WAIT_TIMER_2_MSB_REG);
        break;
    default:
        return;
    }
}

int pattern_getLoopCycles(char *message, int level, int *numLoops) {
    // validate input
    if (level < 0 || level > 2) {
        sprintf(message,
                "Cannot get patnloop. Level must be between 0 and 2.\n");
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
    default:
        return -1;
    }
}

int pattern_setLoopCycles(char *message, int level, int numLoops) {
    // validate input
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
    default:
        return;
    }
}

void pattern_getLoopLimits(int *startAddr, int *stopAddr) {
    *startAddr = ((bus_r(PATTERN_LIMIT_REG) & PATTERN_LIMIT_STRT_MSK) >>
                  PATTERN_LIMIT_STRT_OFST);
    *stopAddr = ((bus_r(PATTERN_LIMIT_REG) & PATTERN_LIMIT_STP_MSK) >>
                 PATTERN_LIMIT_STP_OFST);
}

int pattern_setLoopLimits(char *message, int startAddr, int stopAddr) {
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
    pattern_getLoopLimits(&r_startAddr, &r_stopAddr);
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

int pattern_getLoopAddresses(char *message, int level, int *startAddr,
                             int *stopAddr) {
    // validate input
    if (level < 0 || level > 2) {
        sprintf(
            message,
            "Cannot get patloop addresses. Level must be between 0 and 2.\n");
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
    default:
        return;
    }
}

int pattern_setLoopAddresses(char *message, int level, int startAddr,
                             int stopAddr) {
    // validate input
    if (level < 0 || level > 2) {
        sprintf(
            message,
            "Cannot set patloop addresses. Level must be between 0 and 2.\n");
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
    default:
        return;
    }
}

void setPatternMask(uint64_t mask) {
    LOG(logINFO, ("Setting pattern mask to 0x%llx\n", mask));
    set64BitReg(mask, PATTERN_MASK_LSB_REG, PATTERN_MASK_MSB_REG);
}

uint64_t getPatternMask() {
    return get64BitReg(PATTERN_MASK_LSB_REG, PATTERN_MASK_MSB_REG);
}

void setPatternBitMask(uint64_t mask) {
    LOG(logINFO, ("Setting pattern bit mask to 0x%llx\n", mask));
    set64BitReg(mask, PATTERN_SET_LSB_REG, PATTERN_SET_MSB_REG);
}

uint64_t getPatternBitMask() {
    return get64BitReg(PATTERN_SET_LSB_REG, PATTERN_SET_MSB_REG);
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