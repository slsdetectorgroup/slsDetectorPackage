// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "catch.hpp"
#include "sls/Version.h"

namespace sls {

TEST_CASE("check if version is semantic", "[version]") {

    auto [version_string, has_semantic_version] =
        GENERATE(std::make_tuple("developer 0x250512", false),
                 std::make_tuple("0.0.0 0x250512", false));

    Version version(version_string);

    CHECK(version.hasSemanticVersioning() == has_semantic_version);
}

TEST_CASE("semantic version ordering compares numerically", "[version]") {
    Version v9("9.1.1 0x250512");
    Version v10("10.0.1 0x250512");

    CHECK(v9 <= v10);
    CHECK_FALSE(v10 <= v9);
}

TEST_CASE("semantic version compares extra suffix lexicographically",
          "[version]") {
    Version rc("10.1.1.rc 0x250512");
    Version zz("10.1.1.zz 0x250512");

    CHECK(rc <= zz);
    CHECK_FALSE(zz <= rc);
}

TEST_CASE(
    "semantic comparison falls back to date on malformed semantic version",
    "[version]") {
    Version malformed("10.1 0x250512");
    Version valid("10.1.1 0x250513");

    CHECK(malformed <= valid);
    CHECK_FALSE(valid <= malformed);
}

TEST_CASE("semantic version compares extra suffix and without", "[version]") {
    Version v1011rc("10.1.1.rc 0x250512");
    Version v1010("10.1.0 0x250512");

    CHECK_FALSE(v1011rc <= v1010);
    CHECK(v1010 <= v1011rc);
}

} // namespace sls
