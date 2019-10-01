#include "TimeHelper.h"
#include "ToString.h"
#include "network_utils.h"
#include "catch.hpp"
#include <array>
#include <vector>

// using namespace sls;
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

TEST_CASE("string to std::chrono::duration", "[support]") {
    REQUIRE(StringTo<ns>("150", "ns") == ns(150));
    REQUIRE(StringTo<ns>("150ns") == ns(150));
    REQUIRE(StringTo<ns>("150s") == s(150));
    REQUIRE(StringTo<s>("3 s") == s(3));

    REQUIRE_THROWS(StringTo<ns>("5xs"));
    REQUIRE_THROWS(StringTo<ns>("asvn"));
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

TEST_CASE("Convert types with str method"){
    sls::IpAddr addr;
    REQUIRE(ToString(addr) == "0.0.0.0");
    REQUIRE(ToString(sls::IpAddr{}) == "0.0.0.0");
}

TEST_CASE("String to string", "[support]"){
    std::string s = "hej";
    REQUIRE(ToString(s) == "hej");
}

TEST_CASE("vector of strings"){
    std::vector<std::string> vec{"5", "s"};
    REQUIRE(ToString(vec) == "[5, s]");
    
    std::vector<std::string> vec2{"some", "strange", "words", "75"};
    REQUIRE(ToString(vec2) == "[some, strange, words, 75]");

}



