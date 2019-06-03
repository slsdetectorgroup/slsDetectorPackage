#pragma once

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
// #define FILELOG_MAX_LEVEL logDEBUG5
#endif


#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define MYCONCAT(x,y)  
#define __AT__  std::string(__FILE__) + std::string("::") + std::string(__func__) + std::string("(): ")
#define __SHORT_FORM_OF_FILE__ \
(strrchr(__FILE__,'/') \
? strrchr(__FILE__,'/')+1 \
: __FILE__ \
)
#define __SHORT_AT__  std::string(__SHORT_FORM_OF_FILE__) + std::string("::") + std::string(__func__) + std::string("(): ")




inline std::string NowTime();
// 1 normal debug, 3 function names, 5 fifodebug
enum TLogLevel {logERROR, logWARNING, logINFOBLUE, logINFOGREEN, logINFORED, logINFO,
	logDEBUG, logDEBUG1, logDEBUG2, logDEBUG3, logDEBUG4, logDEBUG5};

template <typename T> class Log{
 public:
	Log();
	virtual ~Log();
	std::ostringstream& Get(TLogLevel level = logINFO);
	static TLogLevel& ReportingLevel();
	static std::string ToString(TLogLevel level);
	static TLogLevel FromString(const std::string& level);
 protected:
	std::ostringstream os;
	TLogLevel lev;
 private:
	Log(const Log&);
	Log& operator =(const Log&);
};


class Output2FILE {
public:
    static FILE*& Stream();
    static void Output(const std::string& msg);
    static void Output(const std::string& msg, TLogLevel level);
};


#define FILELOG_DECLSPEC

class FILELOG_DECLSPEC FILELog : public Log<Output2FILE> {};


#define FILE_LOG(level) \
	if (level > FILELOG_MAX_LEVEL) ;				\
	else if (level > FILELog::ReportingLevel() || !Output2FILE::Stream()) ; \
	else FILELog().Get(level)


#include <sys/time.h>

inline std::string NowTime()
{
    char buffer[12];
    const int buffer_len = sizeof(buffer);
    time_t t;
    time(&t);
    tm r;
    strftime(buffer, buffer_len, "%X", localtime_r(&t, &r));
    buffer[buffer_len - 1] = 0;
    struct timeval tv;
    gettimeofday(&tv, 0);
    char result[100];
    const int result_len = sizeof(result);
    snprintf(result, result_len, "%s.%03ld", buffer, (long)tv.tv_usec / 1000);
    result[result_len - 1] = 0;
    return result;
}


template <typename T> Log<T>::Log():lev(logDEBUG){}

template <typename T> std::ostringstream& Log<T>::Get(TLogLevel level)
{
	lev = level;
    os << "- " << NowTime();
    os << " " << ToString(level) << ": ";
    if (level > logDEBUG)
    	os << std::string(level - logDEBUG, ' ');
    return os;
}

template <typename T> Log<T>::~Log()
{
    os << std::endl;
    T::Output( os.str(),lev); // T::Output( os.str());
}

template <typename T> TLogLevel& Log<T>::ReportingLevel()
{
    static TLogLevel reportingLevel = logDEBUG5;
    return reportingLevel;
}

template <typename T> std::string Log<T>::ToString(TLogLevel level)
{
	static const char* const buffer[] = {
			"ERROR", "WARNING", "INFO", "INFO", "INFO", "INFO",
			"DEBUG", "DEBUG1", "DEBUG2", "DEBUG3", "DEBUG4","DEBUG5"};
    return buffer[level];
}

template <typename T>
TLogLevel Log<T>::FromString(const std::string& level)
{
    if (level == "DEBUG5")
        return logDEBUG5;
    if (level == "DEBUG4")
        return logDEBUG4;
    if (level == "DEBUG3")
        return logDEBUG3;
    if (level == "DEBUG2")
        return logDEBUG2;
    if (level == "DEBUG1")
        return logDEBUG1;
    if (level == "DEBUG")
        return logDEBUG;
    if (level == "INFO")
        return logINFO;
    if (level == "WARNING")
        return logWARNING;
    if (level == "ERROR")
        return logERROR;
    Log<T>().Get(logWARNING) << "Unknown logging level '" << level << "'. Using INFO level as default.";
    return logINFO;
}


inline FILE*& Output2FILE::Stream()
{
    static FILE* pStream = stderr;
    return pStream;
}

inline void Output2FILE::Output(const std::string& msg)
{   
    FILE* pStream = Stream();
    if (!pStream)
        return;
    fprintf(pStream, "%s", msg.c_str());
    fflush(pStream);
}

inline void Output2FILE::Output(const std::string& msg, TLogLevel level)
{
    FILE* pStream = Stream();
    if (!pStream)
        return;
    bool out = true;
    switch(level){
    case logERROR:		cprintf(RED BOLD,"%s",msg.c_str()); 	break;
    case logWARNING:	cprintf(YELLOW BOLD,"%s",msg.c_str()); 	break;
    case logINFO:		cprintf(RESET,"%s",msg.c_str());		break;
    case logINFOBLUE:	cprintf(BLUE,"%s",msg.c_str());			break;
    case logINFORED:	cprintf(RED,"%s",msg.c_str());			break;
    case logINFOGREEN:	cprintf(GREEN,"%s",msg.c_str());		break;
    default: 			fprintf(pStream,"%s",msg.c_str()); 	out = false; 	break;
    }
    fflush(out ? stdout : pStream);
}
