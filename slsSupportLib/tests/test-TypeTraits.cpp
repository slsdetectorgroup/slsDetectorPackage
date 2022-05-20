// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "catch.hpp"
#include "sls/TypeTraits.h"
#include <array>
#include <chrono>
#include <initializer_list>
#include <sstream>
#include <vector>

// Dummy classes only used here for testing
class DummyWithStr {
  public:
    std::string str();
};

class DummyNoStr {
  public:
    std::string somethingelse();
};

TEST_CASE("sls::is_container") {

    CHECK(sls::is_container<std::vector<int>>::value == true);
    CHECK(sls::is_container<std::array<double, 3>>::value == true);
}

TEST_CASE("Check for str() method") {
    REQUIRE(sls::has_str<DummyWithStr>::value == true);
    REQUIRE(sls::has_str<DummyNoStr>::value == false);
}

TEST_CASE("Check for str() on ostream") {
    REQUIRE(sls::has_str<std::ostringstream>::value == true);
}

TEST_CASE("sls::is_duration") {
    REQUIRE(sls::is_duration<std::chrono::nanoseconds>::value == true);
    REQUIRE(sls::is_duration<std::chrono::seconds>::value == true);
    REQUIRE(sls::is_duration<std::chrono::hours>::value == true);

    REQUIRE(sls::is_duration<int>::value == false);
    REQUIRE(sls::is_duration<std::vector<int>>::value == false);
}

TEST_CASE("initializer list") {
    REQUIRE(sls::is_light_container<std::initializer_list<int>>::value == true);
}

TEST_CASE("Check for emplace back") {
    // we know vector should have this its the type trait that is tested
    REQUIRE(sls::has_emplace_back<std::vector<int>>::value == true);
}