#include "TimeHelper.h"
#include "ToString.h"
#include "catch.hpp"
#include "network_utils.h"
#include "sls_detector_defs.h"
#include <array>
#include <map>
#include <sstream>
#include <vector>

// using namespace sls;
using sls::defs;
using sls::StringTo;
using sls::ToString;
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
    sls::IpAddr addr;
    REQUIRE(ToString(addr) == "0.0.0.0");
    REQUIRE(ToString(sls::IpAddr{}) == "0.0.0.0");
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
    slsDetectorDefs::ROI roi{5, 159};
    REQUIRE(ToString(roi) == "[5, 159]");
}

TEST_CASE("Streaming of slsDetectorDefs::ROI") {
    using namespace sls;
    slsDetectorDefs::ROI roi{-10, 1};
    std::ostringstream oss;
    oss << roi;
    REQUIRE(oss.str() == "[-10, 1]");
}

