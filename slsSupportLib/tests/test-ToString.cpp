#include "TimeHelper.h"
#include "ToString.h"
#include "catch.hpp"
using namespace sls;

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