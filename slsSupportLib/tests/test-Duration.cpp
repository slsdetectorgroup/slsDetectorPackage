

#include "catch.hpp"
#include "sls/Duration.h"
using sls::Duration;

TEST_CASE("Default initialization gives 0"){
    Duration t;
    REQUIRE(t.count()==0);
}

TEST_CASE("Construction from double"){
    REQUIRE(Duration(1e-9).count()==1);
    REQUIRE(Duration(10e-9).count()==10);
    REQUIRE(Duration(10.3e-9).count()==10);
    REQUIRE(Duration(100e-9).count()==100);
    REQUIRE(Duration(1.0).count()==1000000000);
}

TEST_CASE("Construct from chrono ns"){
    std::chrono::nanoseconds t(573);
    REQUIRE(Duration(t).count()==573);
}

bool takes_ns(std::chrono::nanoseconds t){
    return t.count()==10;
}

bool takes_duration(Duration t){
    return t.count()==10;
}


TEST_CASE("Calling a function that takes std::chrono::nanoseconds"){
    Duration t(10e-9);
    REQUIRE(takes_ns(t));
}

TEST_CASE("Calling a function that takes Duration with std::chrono::nanoseconds"){
    std::chrono::nanoseconds t(10);
    REQUIRE(takes_duration(t));
}

TEST_CASE("Getting total seconds"){
    REQUIRE(Duration(10e-9).total_seconds()==10e-9);
}