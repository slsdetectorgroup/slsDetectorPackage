// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once

#include "sls/ansi.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef FIFODEBUG
#define FILELOG_MAX_LEVEL logDEBUG5
#elif VERYVERBOSE
#define FILELOG_MAX_LEVEL logDEBUG4
#elif VERBOSE
#define FILELOG_MAX_LEVEL logDEBUG
#elif DEBUG1
#define FILELOG_MAX_LEVEL logDEBUG1
#endif

#ifndef FILELOG_MAX_LEVEL
#define FILELOG_MAX_LEVEL logINFO
#endif

enum TLogLevel {
    logERROR,
    logWARNING,
    logINFOBLUE,
    logINFOGREEN,
    logINFORED,
    logINFO,
    logDEBUG,
    logDEBUG1,
    logDEBUG2,
    logDEBUG3,
    logDEBUG4,
    logDEBUG5
};

#define ERROR_MSG_LENGTH 1000

#define LOG(lvl, fmt, ...)                                                     \
    if (lvl > FILELOG_MAX_LEVEL)                                               \
        ;                                                                      \
    else {                                                                     \
        char *temp = FILELOG_BuildLog fmt;                                     \
        FILELOG_PrintLog(lvl, temp);                                           \
        free(temp);                                                            \
    }

static inline void FILELOG_PrintLog(enum TLogLevel level, char *m) {
    switch (level) {
    case logERROR:
        cprintf(RED BOLD, "ERROR: %s", m);
        break;
    case logWARNING:
        cprintf(YELLOW BOLD, "WARNING: %s", m);
        break;
    case logINFOBLUE:
        cprintf(BLUE, "INFO: %s", m);
        break;
    case logINFOGREEN:
        cprintf(GREEN, "INFO: %s", m);
        break;
    case logINFORED:
        cprintf(RED, "INFO: %s", m);
        break;
    case logINFO:
        cprintf(RESET, "INFO: %s", m);
        break;
    case logDEBUG:
        cprintf(MAGENTA, "DEBUG: %s", m);
        break;
    case logDEBUG1:
        cprintf(MAGENTA, "DEBUG1: %s", m);
        break;
    case logDEBUG2:
        cprintf(MAGENTA, "DEBUG2: %s", m);
        break;
    case logDEBUG3:
        cprintf(MAGENTA, "DEBUG3: %s", m);
        break;
    case logDEBUG4:
        cprintf(MAGENTA, "DEBUG4: %s", m);
        break;
    case logDEBUG5:
        cprintf(MAGENTA, "DEBUG5: %s", m);
        break;
    }
    fflush(stdout);
}

static inline char *FILELOG_BuildLog(const char *fmt, ...) {
    char *p = NULL;
    va_list ap;
    p = malloc(ERROR_MSG_LENGTH);
    va_start(ap, fmt);
    int ret = vsnprintf(p, ERROR_MSG_LENGTH, fmt, ap);
    va_end(ap);
    if (ret < 0 || ret >= ERROR_MSG_LENGTH) {
        FILELOG_PrintLog(logERROR,
                         ("Could not print the "
                          "complete error message in the next print.\n"));
    }
    return p;
};
