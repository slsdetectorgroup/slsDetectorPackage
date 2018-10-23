#pragma once

#include "ansi.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>


#ifdef FIFODEBUG
#define FILELOG_MAX_LEVEL logDEBUG5
#elif VERYVERBOSE
#define FILELOG_MAX_LEVEL logDEBUG4
#elif VERBOSE
#define FILELOG_MAX_LEVEL logDEBUG
#endif

#ifndef FILELOG_MAX_LEVEL
#define FILELOG_MAX_LEVEL logINFOBLUE
#endif

typedef enum {
logERROR, logWARNING, logINFO, logINFOBLUE, logGREEN,
logDEBUG, logDEBUG1, logDEBUG2, logDEBUG3, logDEBUG4, logDEBUG5
}TLogLevel;

#define ERROR_MSG_LENGTH 1000

#define FILE_LOG(lvl, fmt, ...) 				\
	if (lvl > FILELOG_MAX_LEVEL);				\
	else {char* temp = FILELOG_BuildLog fmt; FILELOG_PrintLog(lvl, temp);free(temp);}

static inline void FILELOG_PrintLog(TLogLevel level, char* m) {
	switch(level) {
	case logERROR: 		cprintf(RED BOLD, "ERROR: %s", m);		break;
	case logWARNING:	cprintf(YELLOW BOLD, "WARNING: %s", m);	break;
	case logINFOBLUE: 	cprintf(BLUE, "INFO: %s", m);			break;
	case logGREEN: 		cprintf(GREEN, "INFO: %s", m);			break;
	case logINFO: 		cprintf(RESET, "INFO: %s", m);			break;
	case logDEBUG: 		cprintf(MAGENTA, "DEBUG: %s", m);		break;
	case logDEBUG1: 	cprintf(MAGENTA, "DEBUG1: %s", m);		break;
	case logDEBUG2: 	cprintf(MAGENTA, "DEBUG2: %s", m);		break;
	case logDEBUG3: 	cprintf(MAGENTA, "DEBUG3: %s", m);		break;
	case logDEBUG4: 	cprintf(MAGENTA, "DEBUG4: %s", m);		break;
	case logDEBUG5: 	cprintf(MAGENTA, "DEBUG5: %s", m);		break;
	}
}

static inline char* FILELOG_BuildLog(const char* fmt, ...) {
	char* p;
	va_list ap;
	p = malloc(ERROR_MSG_LENGTH);
	va_start(ap, fmt);
	int ret = vsnprintf(p, ERROR_MSG_LENGTH, fmt, ap);
	va_end(ap);
	if (ret < 0 || ret >= ERROR_MSG_LENGTH) {
		FILELOG_PrintLog(logERROR, ("Could not print the "
				"complete error message in the next print.\n"));
	}
	return p;
};



