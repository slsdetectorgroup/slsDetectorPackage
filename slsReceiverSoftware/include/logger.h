#ifndef __LOG_H__
#define __LOG_H__


#include <sstream>
#include <string>
#include <stdio.h>
#include <unistd.h>
#include <ansi.h>


#ifdef FIFODEBUG
#define FILELOG_MAX_LEVEL logDEBUG5
#elif VERYVERBOSE
#define FILELOG_MAX_LEVEL logDEBUG4
#elif VERBOSE
#define FILELOG_MAX_LEVEL logDEBUG
#endif

#ifndef FILELOG_MAX_LEVEL
#define FILELOG_MAX_LEVEL logINFO
#endif

#define REPORT_LEVEL logDEBUG5


#include <sys/time.h>


class Logger {
public:
	Logger(){};

	enum TLogLevel {logERROR, logWARNING, logINFO, logDEBUG, logDEBUG1, logDEBUG2, logDEBUG3, logDEBUG4, logDEBUG5};


	static void FILE_LOG(TLogLevel level, char* msg)
	{
	    char buffer[11];
	    const int buffer_len = sizeof(buffer);
	    time_t t;
	    time(&t);
	    tm r = {0};
	    strftime(buffer, buffer_len, "%X", localtime_r(&t, &r));
	    buffer[buffer_len - 1] = 0;
	    struct timeval tv;
	    gettimeofday(&tv, 0);
	    char result[100];
	    const int result_len = sizeof(result);
	    snprintf(result, result_len, "%s.%03ld", buffer, (long)tv.tv_usec / 1000);
	    result[result_len - 1] = 0;


	    /*
	    const char* const slevel[] = {"ERROR", "WARNING", "INFO", "DEBUG", "DEBUG1", "DEBUG2", "DEBUG3", "DEBUG4","DEBUG5"};
	    ostringstream os;
	    os << "- " << string(result);
	    os << " " << string(slevel[level]) << ": ";
	  // if (level > logDEBUG)
	//  os << std::string(level - logDEBUG, '\t');
	    os << msg;
	string smessage = os.str();

	    switch(level){
	    case logERROR:	cprintf(RED BOLD,	"%s\n",		smessage.c_str()); 	break;
	    case logWARNING:	cprintf(YELLOW BOLD,   	"%s\n",		smessage.c_str()); 	break;
	    case logINFO:	cprintf(GRAY,		"%s\n",		smessage.c_str()); 	break;
	    default: break;
	    }
	   */

	    switch(level){
	    case logERROR:		cprintf(RED BOLD,	"- %s ERROR: %s",		result, msg); 	break;
	    case logWARNING:	cprintf(YELLOW BOLD,"- %s WARNING: %s",		result, msg); 	break;
	    case logINFO:		cprintf(GRAY,		"- %s INFO: %s",		result, msg);	break;
	    default: break;
	    }


	    fflush(stdout);
	}

};


#endif //__LOG_H__
