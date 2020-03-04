#pragma once
/*Utility to log to console*/

#include "ansi.h" //Colors
#include "logger.h"
#include <iostream>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <map>

/*
Define the max level that is visible
The compiler should optimize away any calls below
this level
*/
#ifndef LOG_MAX_LEVEL
#define LOG_MAX_LEVEL logINFO
#endif

namespace sls {
class Logger {
    std::ostringstream os;
    TLogLevel level = LOG_MAX_LEVEL;

  public:
    Logger() = default;
    Logger(TLogLevel level) : level(level){};
    ~Logger() {
        // output happens in the destructor to allow for <<
        os << Reset() << '\n';
        std::clog << os.str(); // Single write
    }

    static TLogLevel &ReportingLevel() { // singelton eeh
        static TLogLevel reportingLevel = logINFO;
        return reportingLevel;
    }

    // Danger this buffer need as many elements as TLogLevel
    static const char *Color(TLogLevel level) {
        static const char *const colors[] = {
            RED BOLD, YELLOW BOLD, RESET, BLUE,  RED,   RESET,
            RESET,    RESET,       RESET, RESET, RESET, RESET};
        return colors[level];
    }
    static const char *Reset() {
        static const char *reset = RESET;
        return reset;
    }

    // Danger this buffer need as many elements as TLogLevel
    static std::string ToString(TLogLevel level) {
        static const char *const buffer[] = {
            "ERROR", "WARNING", "INFO",   "INFO",   "INFO",   "INFO",
            "DEBUG", "DEBUG1",  "DEBUG2", "DEBUG3", "DEBUG4", "DEBUG5"};
        return buffer[level];
    }

    std::ostringstream &Get() {
        os << Color(level);
        os << "- " << Time();
        os << " " << ToString(level) << ": ";
        return os;
    }
    std::string Time(decltype(std::chrono::system_clock::now()) now =
                         std::chrono::system_clock::now()) {
        std::ostringstream oss;
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                      now.time_since_epoch()) %
                  1000;
        std::time_t now_time = std::chrono::system_clock::to_time_t(now);
        oss << std::put_time(std::localtime(&now_time), "%H:%M:%S") << "."
            << std::setw(3) << std::setfill('0') << ms.count();
        return oss.str();
    }
};

#define LOG(level)                                                             \
    if (level > LOG_MAX_LEVEL)                                                 \
        ;                                                                      \
    else if (level > sls::Logger::ReportingLevel())                            \
        ;                                                                      \
    else                                                                       \
        sls::Logger(level).Get()

} // namespace sls
