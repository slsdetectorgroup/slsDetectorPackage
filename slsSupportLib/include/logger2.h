#pragma once
/*Utility to log to console*/

#include "ansi.h"   //Colors
// #include "logger.h" //for enum, to be removed
#include <iostream>
#include <sstream>

// Compiler should optimize away anything below this value
#ifndef LOG_MAX_REPORTING_LEVEL
#define LOG_MAX_REPORTING_LEVEL logINFO
#endif

namespace sls {
class Logger {
    std::ostringstream os;
    TLogLevel level = LOG_MAX_REPORTING_LEVEL;

  public:
    Logger() = default;
    explicit Logger(TLogLevel level) : level(level){};
    ~Logger() {
        // output in the destructor to allow for << syntax
        os << RESET << '\n';
        std::clog << os.str() << std::flush; // Single write
    }

    static TLogLevel &ReportingLevel() { // singelton eeh
        static TLogLevel reportingLevel = logINFO;
        return reportingLevel;
    }

    // Danger this buffer need as many elements as TLogLevel
    static const char *Color(TLogLevel level) noexcept {
        static const char *const colors[] = {
            RED BOLD, YELLOW BOLD, BLUE,  GREEN, RED,   RESET,
            RESET,    RESET,       RESET, RESET, RESET, RESET};
        return colors[level];
    }

    // Danger this buffer need as many elements as TLogLevel
    static std::string ToString(TLogLevel level) {
        static const char *const buffer[] = {
            "ERROR", "WARNING", "INFO",   "INFO",   "INFO",   "INFO",
            "DEBUG", "DEBUG1",  "DEBUG2", "DEBUG3", "DEBUG4", "DEBUG5"};
        return buffer[level];
    }

    std::ostringstream &Get() {
        os << Color(level) << "- " << Timestamp() << " " << ToString(level)
           << ": ";
        return os;
    }

    std::string Timestamp() {
        constexpr size_t buffer_len = 12;
        char buffer[buffer_len];
        time_t t;
        ::time(&t);
        tm r;
        strftime(buffer, buffer_len, "%X", localtime_r(&t, &r));
        buffer[buffer_len - 1] = '\0';
        struct timeval tv;
        gettimeofday(&tv, nullptr);
        constexpr size_t result_len = 100;
        char result[result_len];
        snprintf(result, result_len, "%s.%03ld", buffer,
                 (long)tv.tv_usec / 1000);
        result[result_len - 1] = '\0';
        return result;
    }
};

#define LOG(level)                                                             \
    if (level > LOG_MAX_REPORTING_LEVEL)                                       \
        ;                                                                      \
    else if (level > sls::Logger::ReportingLevel())                            \
        ;                                                                      \
    else                                                                       \
        sls::Logger(level).Get()

} // namespace sls
