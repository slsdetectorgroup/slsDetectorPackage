// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once
/*Utility to log to console*/

#include "sls/ansi.h" //Colors
#include <iostream>
#include <sstream>
#include <sys/time.h>

namespace sls {

enum TLogLevel {
    logERROR,
    logWARNING,
    logINFOBLUE,
    logINFOGREEN,
    logINFORED,
    logINFOCYAN,
    logINFOMAGENTA,
    logINFO,
    logDEBUG,
    logDEBUG1,
    logDEBUG2,
    logDEBUG3,
    logDEBUG4,
    logDEBUG5
};

// Compiler should optimize away anything below this value
#ifndef LOG_MAX_REPORTING_LEVEL
#define LOG_MAX_REPORTING_LEVEL sls::logINFO
#endif

#define __AT__                                                                 \
    std::string(__FILE__) + std::string("::") + std::string(__func__) +        \
        std::string("(): ")
#define __SHORT_FORM_OF_FILE__                                                 \
    (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define __SHORT_AT__                                                           \
    std::string(__SHORT_FORM_OF_FILE__) + std::string("::") +                  \
        std::string(__func__) + std::string("(): ")

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
            RED BOLD, YELLOW BOLD, BLUE,  GREEN, RED,   CYAN,  MAGENTA,
            RESET,    RESET,       RESET, RESET, RESET, RESET, RESET};
        // out of bounds
        if (level < 0 || level >= sizeof(colors) / sizeof(colors[0])) {
            return RESET;
        }
        return colors[level];
    }

    // Danger this buffer need as many elements as TLogLevel
    static std::string ToString(TLogLevel level) {
        static const char *const buffer[] = {
            "ERROR",  "WARNING", "INFO",   "INFO",  "INFO",
            "INFO",   "INFO",    "INFO",   "DEBUG", "DEBUG1",
            "DEBUG2", "DEBUG3",  "DEBUG4", "DEBUG5"};
        // out of bounds
        if (level < 0 || level >= sizeof(buffer) / sizeof(buffer[0])) {
            return "UNKNOWN";
        }
        return buffer[level];
    }

    std::ostringstream &Get() {
        os << Color(level) << "- " << Timestamp() << " " << ToString(level)
           << ": ";
        return os;
    }

    static std::string Timestamp() {
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
