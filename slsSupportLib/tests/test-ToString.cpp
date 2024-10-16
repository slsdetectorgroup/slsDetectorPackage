// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "catch.hpp"
#include "sls/Pattern.h"
#include "sls/TimeHelper.h"
#include "sls/ToString.h"
#include "sls/container_utils.h"
#include "sls/network_utils.h"
#include "sls/sls_detector_defs.h"
#include <array>
#include <map>
#include <sstream>
#include <vector>

namespace sls {

using namespace sls::time;

TEST_CASE("Integer conversions", "[support]") {
    REQUIRE(ToString(0) == "0");
    REQUIRE(ToString(1) == "1");
    REQUIRE(ToString(-1) == "-1");
    REQUIRE(ToString(100) == "100");
    REQUIRE(ToString(589633100) == "589633100");
}

TEST_CASE("floating point conversions", "[support]") {
    // Should strip trailing zeros
    REQUIRE(ToString(0.) == "0");
    REQUIRE(ToString(1.) == "1");
    REQUIRE(ToString(-1.) == "-1");
    REQUIRE(ToString(100.) == "100");
    REQUIRE(ToString(589633100.) == "589633100");
    REQUIRE(ToString(2.35) == "2.35");
    REQUIRE(ToString(2.3500) == "2.35");
    REQUIRE(ToString(2.35010) == "2.3501");
    REQUIRE(ToString(5000) == "5000");
    REQUIRE(ToString(5E15) == "5000000000000000");
}

TEST_CASE("conversion from duration to string", "[support]") {
    REQUIRE(ToString(ns(150)) == "150ns");
    REQUIRE(ToString(ms(783)) == "0.783s");
    REQUIRE(ToString(ms(783), "ms") == "783ms");
    REQUIRE(ToString(us(0)) == "0ns"); // Defaults to the lowest unit
    REQUIRE(ToString(us(0), "s") == "0s");
    REQUIRE(ToString(s(-1)) == "-1s");
    REQUIRE(ToString(us(-100)) == "-100us");
}

TEST_CASE("Convert vector of time", "[support]") {
    std::vector<ns> vec{ns(150), us(10), ns(600)};
    REQUIRE(ToString(vec) == "[150ns, 10us, 600ns]");
    REQUIRE(ToString(vec, "ns") == "[150ns, 10000ns, 600ns]");
}

TEST_CASE("Vector of int", "[support]") {
    std::vector<int> vec;
    REQUIRE(ToString(vec) == "[]");

    vec.push_back(1);
    REQUIRE(ToString(vec) == "[1]");

    vec.push_back(172);
    REQUIRE(ToString(vec) == "[1, 172]");

    vec.push_back(5000);
    REQUIRE(ToString(vec) == "[1, 172, 5000]");
}

TEST_CASE("Vector of double", "[support]") {
    std::vector<double> vec;
    REQUIRE(ToString(vec) == "[]");

    vec.push_back(1.3);
    REQUIRE(ToString(vec) == "[1.3]");

    vec.push_back(5669.325);
    REQUIRE(ToString(vec) == "[1.3, 5669.325]");

    vec.push_back(-5669.325005);
    REQUIRE(ToString(vec) == "[1.3, 5669.325, -5669.325005]");
}

TEST_CASE("Array") {
    std::array<int, 3> arr{1, 2, 3};
    REQUIRE(ToString(arr) == "[1, 2, 3]");
}

TEST_CASE("Convert types with str method") {
    IpAddr addr;
    REQUIRE(ToString(addr) == "0.0.0.0");
    REQUIRE(ToString(IpAddr{}) == "0.0.0.0");
}

TEST_CASE("String to string", "[support]") {
    std::string s = "hej";
    REQUIRE(ToString(s) == "hej");
}

TEST_CASE("vector of strings") {
    std::vector<std::string> vec{"5", "s"};
    REQUIRE(ToString(vec) == "[5, s]");

    std::vector<std::string> vec2{"some", "strange", "words", "75"};
    REQUIRE(ToString(vec2) == "[some, strange, words, 75]");
}

TEST_CASE("run status") {
    using defs = slsDetectorDefs;
    REQUIRE(ToString(defs::runStatus::ERROR) == "error");
    REQUIRE(ToString(defs::runStatus::WAITING) == "waiting");
    REQUIRE(ToString(defs::runStatus::TRANSMITTING) == "transmitting");
    REQUIRE(ToString(defs::runStatus::RUN_FINISHED) == "finished");
    REQUIRE(ToString(defs::runStatus::STOPPED) == "stopped");
    REQUIRE(ToString(defs::runStatus::IDLE) == "idle");
}

/** Conversion from string (break out in it's own file?) */

TEST_CASE("string to std::chrono::duration", "[support]") {
    REQUIRE(StringTo<ns>("150", "ns") == ns(150));
    REQUIRE(StringTo<ns>("150ns") == ns(150));
    REQUIRE(StringTo<ns>("150s") == s(150));
    REQUIRE(StringTo<s>("3 s") == s(3));
    REQUIRE_THROWS(StringTo<ns>("5xs"));
    REQUIRE_THROWS(StringTo<ns>("asvn"));
}

TEST_CASE("string to detectorType") {
    using dt = slsDetectorDefs::detectorType;
    REQUIRE(StringTo<dt>("Eiger") == dt::EIGER);
    REQUIRE(StringTo<dt>("Gotthard") == dt::GOTTHARD);
    REQUIRE(StringTo<dt>("Jungfrau") == dt::JUNGFRAU);
    REQUIRE(StringTo<dt>("ChipTestBoard") == dt::CHIPTESTBOARD);
    REQUIRE(StringTo<dt>("Moench") == dt::MOENCH);
    REQUIRE(StringTo<dt>("Mythen3") == dt::MYTHEN3);
    REQUIRE(StringTo<dt>("Gotthard2") == dt::GOTTHARD2);
    REQUIRE(StringTo<dt>("Xilinx_ChipTestBoard") == dt::XILINX_CHIPTESTBOARD);
}

TEST_CASE("vec") {
    using rs = slsDetectorDefs::runStatus;
    std::vector<rs> vec{rs::ERROR, rs::IDLE};
    REQUIRE(ToString(vec) == "[error, idle]");
}

TEST_CASE("uint32 from string") {
    REQUIRE(StringTo<uint32_t>("0") == 0);
    REQUIRE(StringTo<uint32_t>("5") == 5u);
    REQUIRE(StringTo<uint32_t>("16") == 16u);
    REQUIRE(StringTo<uint32_t>("20") == 20u);
    REQUIRE(StringTo<uint32_t>("0x14") == 20u);
    REQUIRE(StringTo<uint32_t>("0x15") == 21u);
    REQUIRE(StringTo<uint32_t>("0x15") == 0x15);
    REQUIRE(StringTo<uint32_t>("0xffffff") == 0xffffff);
}

TEST_CASE("uint64 from string") {
    REQUIRE(StringTo<uint64_t>("0") == 0);
    REQUIRE(StringTo<uint64_t>("5") == 5u);
    REQUIRE(StringTo<uint64_t>("16") == 16u);
    REQUIRE(StringTo<uint64_t>("20") == 20u);
    REQUIRE(StringTo<uint64_t>("0x14") == 20u);
    REQUIRE(StringTo<uint64_t>("0x15") == 21u);
    REQUIRE(StringTo<uint64_t>("0xffffff") == 0xffffff);
}

TEST_CASE("int from string") {
    REQUIRE(StringTo<int>("-1") == -1);
    REQUIRE(StringTo<int>("-0x1") == -0x1);
    REQUIRE(StringTo<int>("-0x1") == -1);
    REQUIRE(StringTo<int>("0") == 0);
    REQUIRE(StringTo<int>("5") == 5);
    REQUIRE(StringTo<int>("16") == 16);
    REQUIRE(StringTo<int>("20") == 20);
    REQUIRE(StringTo<int>("0x14") == 20);
    REQUIRE(StringTo<int>("0x15") == 21);
    REQUIRE(StringTo<int>("0xffffff") == 0xffffff);
}

TEST_CASE("int64_t from string") {
    REQUIRE(StringTo<int64_t>("-1") == -1);
    REQUIRE(StringTo<int64_t>("-0x1") == -0x1);
    REQUIRE(StringTo<int64_t>("-0x1") == -1);
    REQUIRE(StringTo<int64_t>("0") == 0);
    REQUIRE(StringTo<int64_t>("5") == 5);
    REQUIRE(StringTo<int64_t>("16") == 16);
    REQUIRE(StringTo<int64_t>("20") == 20);
    REQUIRE(StringTo<int64_t>("0x14") == 20);
    REQUIRE(StringTo<int64_t>("0x15") == 21);
    REQUIRE(StringTo<int64_t>("0xffffff") == 0xffffff);
}

TEST_CASE("std::map of strings") {
    std::map<std::string, std::string> m;
    m["key"] = "value";
    auto s = ToString(m);
    REQUIRE(s == "{key: value}");

    m["chrusi"] = "musi";
    REQUIRE(ToString(m) == "{chrusi: musi, key: value}");

    m["test"] = "tree";
    REQUIRE(ToString(m) == "{chrusi: musi, key: value, test: tree}");
}

TEST_CASE("std::map of ints") {

    std::map<int, int> m;
    m[5] = 10;
    REQUIRE(ToString(m) == "{5: 10}");
    m[500] = 50;
    REQUIRE(ToString(m) == "{5: 10, 500: 50}");
    m[372] = 999;
    REQUIRE(ToString(m) == "{5: 10, 372: 999, 500: 50}");
}

TEST_CASE("Detector type") {
    auto dt = defs::detectorType::EIGER;
    REQUIRE(ToString(dt) == "Eiger");
    REQUIRE(StringTo<defs::detectorType>("Eiger") == dt);
}

TEST_CASE("Formatting slsDetectorDefs::ROI") {
    slsDetectorDefs::ROI roi(5, 159);
    REQUIRE(ToString(roi) == "[5, 159]");
    slsDetectorDefs::ROI roi1(5, 159, 6, 170);
    REQUIRE(ToString(roi1) == "[5, 159, 6, 170]");
}

TEST_CASE("Streaming of slsDetectorDefs::ROI") {
    using namespace sls;
    slsDetectorDefs::ROI roi(-10, 1);
    std::ostringstream oss, oss1;
    oss << roi;
    REQUIRE(oss.str() == "[-10, 1]");
    slsDetectorDefs::ROI roi1(-10, 1, 5, 11);
    oss1 << roi1;
    REQUIRE(oss1.str() == "[-10, 1, 5, 11]");
}

TEST_CASE("std::array") {
    std::array<int, 3> arr{4, 6, 7};
    REQUIRE(ToString(arr) == "[4, 6, 7]");
}

TEST_CASE("string to dac index") {
    // test a few dacs
    REQUIRE(defs::VCAL == StringTo<defs::dacIndex>("vcal"));
    REQUIRE(defs::VIN_CM == StringTo<defs::dacIndex>("vin_cm"));
    REQUIRE(defs::VB_DS == StringTo<defs::dacIndex>("vb_ds"));
}

TEST_CASE("dac index to string") {
    REQUIRE(ToString(defs::VCAL) == "vcal");
    REQUIRE(ToString(defs::VB_DS) == "vb_ds");
}

TEST_CASE("convert vector of strings to dac index") {
    std::vector<std::string> dacs{"vcassh", "vth2", "vrshaper"};
    std::vector<defs::dacIndex> daci{defs::VCASSH, defs::VTH2, defs::VRSHAPER};
    auto r = StringTo<defs::dacIndex>(dacs);
    REQUIRE(r == daci);
}

TEST_CASE("vector of dac index to string") {
    std::vector<defs::dacIndex> daci{defs::VCASSH, defs::VTH2, defs::VRSHAPER};
    auto r = ToString(daci);
    REQUIRE(r == "[vcassh, vth2, vrshaper]");
}

TEST_CASE("int or uin64_t to a string in hex") {
    REQUIRE(ToStringHex(65535) == "0xffff");
    REQUIRE(ToStringHex(65535, 8) == "0x0000ffff");
    REQUIRE(ToStringHex(8927748192) == "0x21422a060");
    REQUIRE(ToStringHex(8927748192, 16) == "0x000000021422a060");
    std::vector<int> temp{244, 65535, 1638582};
    auto r = ToStringHex(temp);
    REQUIRE(r == "[0xf4, 0xffff, 0x1900b6]");
    r = ToStringHex(temp, 8);
    REQUIRE(r == "[0x000000f4, 0x0000ffff, 0x001900b6]");
}

TEST_CASE("Streaming of slsDetectorDefs::scanParameters") {
    using namespace sls;
    {
        defs::scanParameters t{};
        std::ostringstream oss;
        oss << t;
        REQUIRE(oss.str() == "[disabled]");
    }
    {
        defs::scanParameters t{defs::VTH2, 500, 1500, 500};
        std::ostringstream oss;
        oss << t;
        REQUIRE(oss.str() == "[enabled\ndac vth2\nstart 500\nstop 1500\nstep "
                             "500\nsettleTime 1ms\n]");
    }
    {
        defs::scanParameters t{defs::VTH2, 500, 1500, 500,
                               std::chrono::milliseconds{500}};
        std::ostringstream oss;
        oss << t;
        REQUIRE(oss.str() == "[enabled\ndac vth2\nstart 500\nstop 1500\nstep "
                             "500\nsettleTime 0.5s\n]");
    }
}

TEST_CASE("Printing c style arrays of int") {
    int arr[]{3, 5};
    REQUIRE(ToString(arr) == "[3, 5]");
}

TEST_CASE("Printing c style arrays of uint8") {
    uint8_t arr[]{1, 2, 3, 4, 5};
    REQUIRE(ToString(arr) == "[1, 2, 3, 4, 5]");
}

TEST_CASE("Printing c style arrays of double") {
    double arr[]{3.4, 5.3, 6.2};
    REQUIRE(ToString(arr) == "[3.4, 5.3, 6.2]");
}

TEST_CASE("Print a member of patternParameters") {
    auto pat = make_unique<patternParameters>();
    pat->limits[0] = 4;
    pat->limits[1] = 100;
    REQUIRE(ToString(pat->limits) == "[4, 100]");
}

TEST_CASE("streamingInterface") {
    REQUIRE(ToString(defs::streamingInterface::NONE) == "none");
    REQUIRE(ToString(defs::streamingInterface::ETHERNET_10GB) == "10gbe");
    REQUIRE(ToString(defs::streamingInterface::LOW_LATENCY_LINK) == "lll");
    REQUIRE(ToString(defs::streamingInterface::LOW_LATENCY_LINK |
                     defs::streamingInterface::ETHERNET_10GB) == "lll, 10gbe");
}

// Speed level
TEST_CASE("speedLevel to string") {
    REQUIRE(ToString(defs::speedLevel::FULL_SPEED) == "full_speed");
    REQUIRE(ToString(defs::speedLevel::HALF_SPEED) == "half_speed");
    REQUIRE(ToString(defs::speedLevel::QUARTER_SPEED) == "quarter_speed");
    REQUIRE(ToString(defs::speedLevel::G2_108MHZ) == "108");
    REQUIRE(ToString(defs::speedLevel::G2_144MHZ) == "144");
}

TEST_CASE("string to speedLevel") {
    REQUIRE(StringTo<defs::speedLevel>("full_speed") ==
            defs::speedLevel::FULL_SPEED);
    REQUIRE(StringTo<defs::speedLevel>("half_speed") ==
            defs::speedLevel::HALF_SPEED);
    REQUIRE(StringTo<defs::speedLevel>("quarter_speed") ==
            defs::speedLevel::QUARTER_SPEED);
    REQUIRE(StringTo<defs::speedLevel>("108") == defs::speedLevel::G2_108MHZ);
    REQUIRE(StringTo<defs::speedLevel>("144") == defs::speedLevel::G2_144MHZ);
}

// Timing Info Decoder
TEST_CASE("timingInfoDecoder to string") {
    REQUIRE(ToString(defs::timingInfoDecoder::SWISSFEL) == "swissfel");
    REQUIRE(ToString(defs::timingInfoDecoder::SHINE) == "shine");
}

TEST_CASE("string to timingInfoDecoder") {
    REQUIRE(StringTo<defs::timingInfoDecoder>("swissfel") ==
            defs::timingInfoDecoder::SWISSFEL);
    REQUIRE(StringTo<defs::timingInfoDecoder>("shine") ==
            defs::timingInfoDecoder::SHINE);
}

} // namespace sls
