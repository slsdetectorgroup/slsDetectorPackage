// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "test-Caller-global.h"
#include "Caller.h"
#include "GeneralData.h"
#include "sls/Detector.h"
#include "sls/logger.h"
#include "tests/globals.h"

#include "catch.hpp"
#include <regex>

namespace sls {
int chipVersionToX10(const std::string &chipVersion) {
    static const std::regex versionRegex(R"(v?\s*([0-9]+)\.([0-9]+))",
                                         std::regex::icase);
    std::smatch match;
    if (!std::regex_search(chipVersion, match, versionRegex)) {
        throw sls::RuntimeError("Could not parse chip version: " + chipVersion);
    }
    return std::stoi(match[1].str()) * 10 + std::stoi(match[2].str());
}

namespace acq = sls::test::acquire;

using test::GET;
using test::PUT;
void test_valid_port_caller(const std::string &command,
                            const std::vector<std::string> &arguments,
                            int detector_id, int action) {
    Detector det;
    Caller caller(&det);

    std::vector<std::string> arg(arguments);
    if (arg.empty())
        arg.push_back("0");

    int test_values[3] = {77797, -1, 0};
    for (int i = 0; i != 3; ++i) {
        int port_number = test_values[i];
        arg[arg.size() - 1] = std::to_string(port_number);
        REQUIRE_THROWS(caller.call(command, arg, detector_id, action));
        /*REQUIRE_THROWS_WITH(proxy.Call(command, arguments, detector_id,
           action), "Invalid port range. Must be between 1 - 65535.");*/
    }
}

void test_dac_caller(defs::dacIndex index, const std::string &dacname,
                     int dacvalue, bool mV) {
    Detector det;
    Caller caller(&det);
    std::string dac = dacname;
    auto value = std::to_string(dacvalue);
    auto previous = det.getDAC(index, false);
    // chip test board
    if (dacname == "dac") {
        dac = std::to_string(static_cast<int>(index));
    }
    {
        std::ostringstream oss;
        std::vector<std::string> args = {dac, value};
        if (mV)
            args.push_back("mV");
        std::cout << "args:" << ToString(args) << std::endl;
        caller.call("dac", args, -1, PUT, oss);
        REQUIRE(oss.str() == std::string("dac ") + dac + " " + value +
                                 (mV ? " mV\n" : "\n"));
    }
    {
        std::ostringstream oss;
        std::vector<std::string> args = {dac};
        if (mV)
            args.push_back("mV");
        caller.call("dac", args, -1, GET, oss);
        REQUIRE(oss.str() ==
                "dac " + dac + " " + value + (mV ? " mV\n" : "\n"));
    }
    // Reset all dacs to previous value
    for (int i = 0; i != det.size(); ++i) {
        det.setDAC(index, previous[i], false, {i});
    }
}

void test_onchip_dac_caller(defs::dacIndex index, const std::string &dacname,
                            int dacvalue) {
    Detector det;
    Caller caller(&det);
    REQUIRE_THROWS(caller.call(dacname, {}, -1, GET));
    REQUIRE_THROWS(
        caller.call(dacname, {"10", "0x0"}, -1, PUT)); // chip index (-1 to 9)
    REQUIRE_THROWS(
        caller.call(dacname, {"-1", "0x400"}, -1, PUT)); // max val is 0x3ff

    int chipIndex = -1; // for now, it is -1 only
    auto prev_val = det.getOnChipDAC(index, chipIndex);
    auto dacValueStr = ToStringHex(dacvalue);
    auto chipIndexStr = std::to_string(chipIndex);
    std::ostringstream oss_set, oss_get;
    caller.call(dacname, {chipIndexStr, dacValueStr}, -1, PUT, oss_set);
    REQUIRE(oss_set.str() ==
            dacname + " " + chipIndexStr + " " + dacValueStr + "\n");
    caller.call(dacname, {chipIndexStr}, -1, GET, oss_get);
    REQUIRE(oss_get.str() ==
            dacname + " " + chipIndexStr + " " + dacValueStr + "\n");

    // Reset all dacs to previous value
    for (int i = 0; i != det.size(); ++i) {
        det.setOnChipDAC(index, chipIndex, prev_val[i], {i});
    }
}

} // namespace sls
