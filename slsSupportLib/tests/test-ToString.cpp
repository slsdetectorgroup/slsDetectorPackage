#include "TimeHelper.h"
#include "ToString.h"
#include "catch.hpp"
#include <vector>
using namespace sls;


TEST_CASE("Integer conversions", "[support][now]"){
    REQUIRE(ToString(0) == "0");
    REQUIRE(ToString(1) == "1");
    REQUIRE(ToString(-1) == "-1");
    REQUIRE(ToString(100) == "100");
    REQUIRE(ToString(589633100) == "589633100");

}

TEST_CASE("floating point conversions", "[support][now]"){
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

TEST_CASE("conversion from duration to string", "[support][now]") {
    REQUIRE(ToString(time::ns(150)) == "150ns");
    REQUIRE(ToString(time::ms(783)) == "0.783s");
    REQUIRE(ToString(time::ms(783), "ms") == "783ms");
    REQUIRE(ToString(time::us(0)) == "0ns"); // Defaults to the lowest unit
    REQUIRE(ToString(time::us(0), "s") == "0s");
    REQUIRE(ToString(time::s(-1)) == "-1s");
    REQUIRE(ToString(time::us(-100)) == "-100us");
}

TEST_CASE("string to std::chrono::duration", "[support][now]") {
    REQUIRE(StringTo<time::ns>("150", "ns") == time::ns(150));
    REQUIRE(StringTo<time::ns>("150ns") == time::ns(150));
    REQUIRE(StringTo<time::ns>("150s") == time::s(150));
    REQUIRE(StringTo<time::s>("3 s") == time::s(3));

    REQUIRE_THROWS(StringTo<time::ns>("5xs"));
    REQUIRE_THROWS(StringTo<time::ns>("asvn"));
}

TEST_CASE("Convert vector of time", "[support][now]"){
    std::vector<time::ns> vec{time::ns(150), time::us(10), time::ns(600)};
    REQUIRE(ToString(vec) == "[150ns, 10us, 600ns]");
    REQUIRE(ToString(vec, "ns") == "[150ns, 10000ns, 600ns]");
}

TEST_CASE("Vector of int", "[support][now]"){
    std::vector<int> vec;
    REQUIRE(ToString(vec) == "[]");

    vec.push_back(1);
    REQUIRE(ToString(vec) == "[1]");

    vec.push_back(172);
    REQUIRE(ToString(vec) == "[1, 172]");

    vec.push_back(5000);
    REQUIRE(ToString(vec) == "[1, 172, 5000]");

}

TEST_CASE("Vector of double", "[support][now]"){
    std::vector<double> vec;
    REQUIRE(ToString(vec) == "[]");

    vec.push_back(1.3);
    REQUIRE(ToString(vec) == "[1.3]");

    vec.push_back(5669.325);
    REQUIRE(ToString(vec) == "[1.3, 5669.325]");

    vec.push_back(-5669.325005);
    REQUIRE(ToString(vec) == "[1.3, 5669.325, -5669.325005]");

}