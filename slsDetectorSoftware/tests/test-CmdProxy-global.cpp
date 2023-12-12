// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "test-CmdProxy-global.h"
#include "CmdProxy.h"
#include "catch.hpp"
#include "sls/Detector.h"
#include "tests/globals.h"

namespace sls {

using test::GET;
using test::PUT;

void test_valid_port(const std::string &command,
                     const std::vector<std::string> &arguments, int detector_id,
                     int action) {
    Detector det;
    CmdProxy proxy(&det);

    std::vector<std::string> arg(arguments);
    if (arg.empty())
        arg.push_back("0");

    int test_values[3] = {77797, -1, 0};
    for (int i = 0; i != 3; ++i) {
        int port_number = test_values[i];
        arg[arg.size() - 1] = std::to_string(port_number);
        REQUIRE_THROWS(proxy.Call(command, arg, detector_id, action));
        /*REQUIRE_THROWS_WITH(proxy.Call(command, arguments, detector_id,
           action), "Invalid port range. Must be between 1 - 65535.");*/
    }
}

void test_dac(defs::dacIndex index, const std::string &dacname, int dacvalue) {
    Detector det;
    CmdProxy proxy(&det);
    std::ostringstream oss_set, oss_get;
    auto dacstr = std::to_string(dacvalue);
    auto previous = det.getDAC(index, false);
    // chip test board
    if (dacname == "dac") {
        auto dacIndexstr = std::to_string(static_cast<int>(index));
        proxy.Call(dacname, {dacIndexstr, dacstr}, -1, PUT, oss_set);
        REQUIRE(oss_set.str() ==
                dacname + " " + dacIndexstr + " " + dacstr + "\n");
        proxy.Call(dacname, {dacIndexstr}, -1, GET, oss_get);
        REQUIRE(oss_get.str() ==
                dacname + " " + dacIndexstr + " " + dacstr + "\n");
    }
    // other detectors
    else {
        proxy.Call("dac", {dacname, dacstr}, -1, PUT, oss_set);
        REQUIRE(oss_set.str() == "dac " + dacname + " " + dacstr + "\n");
        proxy.Call("dac", {dacname}, -1, GET, oss_get);
        REQUIRE(oss_get.str() == "dac " + dacname + " " + dacstr + "\n");
    }
    // Reset all dacs to previous value
    for (int i = 0; i != det.size(); ++i) {
        det.setDAC(index, previous[i], false, {i});
    }
}

void test_onchip_dac(defs::dacIndex index, const std::string &dacname,
                     int dacvalue) {
    Detector det;
    CmdProxy proxy(&det);
    REQUIRE_THROWS(proxy.Call(dacname, {}, -1, GET));
    REQUIRE_THROWS(
        proxy.Call(dacname, {"10", "0x0"}, -1, PUT)); // chip index (-1 to 9)
    REQUIRE_THROWS(
        proxy.Call(dacname, {"-1", "0x400"}, -1, PUT)); // max val is 0x3ff

    int chipIndex = -1; // for now, it is -1 only
    auto prev_val = det.getOnChipDAC(index, chipIndex);
    auto dacValueStr = ToStringHex(dacvalue);
    auto chipIndexStr = std::to_string(chipIndex);
    std::ostringstream oss_set, oss_get;
    proxy.Call(dacname, {chipIndexStr, dacValueStr}, -1, PUT, oss_set);
    REQUIRE(oss_set.str() ==
            dacname + " " + chipIndexStr + " " + dacValueStr + "\n");
    proxy.Call(dacname, {chipIndexStr}, -1, GET, oss_get);
    REQUIRE(oss_get.str() ==
            dacname + " " + chipIndexStr + " " + dacValueStr + "\n");

    // Reset all dacs to previous value
    for (int i = 0; i != det.size(); ++i) {
        det.setOnChipDAC(index, chipIndex, prev_val[i], {i});
    }
}

} // namespace sls
