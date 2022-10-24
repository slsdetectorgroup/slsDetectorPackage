// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "catch.hpp"
#include "sls/Pattern.h"

namespace sls {

TEST_CASE("Pattern is default constructable and has zeroed fields") {
    Pattern p;
    for (int i = 0; i != MAX_PATTERN_LENGTH; ++i)
        REQUIRE(p.data()->word[i] == 0);
    REQUIRE(p.data()->ioctrl == 0);
}

TEST_CASE("Copy construct pattern") {
    Pattern p;
    p.data()->startloop[0] = 7;
    Pattern p1(p);
    REQUIRE(p1.data()->startloop[0] == 7);
}

TEST_CASE("Compare patterns") {
    Pattern p;
    Pattern p1;
    REQUIRE(p == p1);

    p1.data()->word[500] = 1;
    REQUIRE_FALSE(p == p1);
}

} // namespace sls
