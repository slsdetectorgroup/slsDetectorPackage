// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "catch.hpp"
#include "sls/logger.h"
#include <chrono>
#include <fstream>
#include <iostream>

namespace sls {

TEST_CASE("LogLevel to string") {
    CHECK(Logger::ToString(logERROR) == "ERROR");
    CHECK(Logger::ToString(logWARNING) == "WARNING");
    CHECK(Logger::ToString(logINFOBLUE) == "INFO");
    CHECK(Logger::ToString(logINFOGREEN) == "INFO");
    CHECK(Logger::ToString(logINFORED) == "INFO");
    CHECK(Logger::ToString(logINFO) == "INFO");
    CHECK(Logger::ToString(logDEBUG) == "DEBUG");
    CHECK(Logger::ToString(logDEBUG1) == "DEBUG1");
    CHECK(Logger::ToString(logDEBUG2) == "DEBUG2");
    CHECK(Logger::ToString(logDEBUG3) == "DEBUG3");
    CHECK(Logger::ToString(logDEBUG4) == "DEBUG4");
    CHECK(Logger::ToString(logDEBUG5) == "DEBUG5");
}

TEST_CASE("Test output") {

    auto old_value = Logger::ReportingLevel();

    Logger::ReportingLevel() = logERROR;

    // Redirect std::clog to local buffer
    std::ostringstream local;
    auto clog_buff = std::clog.rdbuf();
    std::clog.rdbuf(local.rdbuf());

    // Try printing something with too low level
    LOG(logDEBUG) << "This should not be printed";
    CHECK(local.str().empty());

    // Try printing something with a higher level
    LOG(logERROR) << "This should be printed";
    CHECK(!local.str().empty());
    std::clog.rdbuf(clog_buff);           // restore
    Logger::ReportingLevel() = old_value; // reset

    // Check that the message is in the printed string
    auto r = local.str();
    auto pos = r.find("This should be printed");
    CHECK(pos != std::string::npos);
}

} // namespace sls
