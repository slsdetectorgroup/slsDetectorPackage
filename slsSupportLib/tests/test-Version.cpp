// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "catch.hpp"
#include "sls/Version.h"

namespace sls {

TEST_CASE("check if version is semantic", "[version]") {

    auto [version_string, has_semantic_version] =
        GENERATE(std::make_tuple("developer 0x250512", false),
                 std::make_tuple("0.0.0 0x250512", false),
                 std::make_tuple("10.1.1 0x250512", true),
                 std::make_tuple("0x250512", false));

    Version version(version_string);

    CHECK(version.hasSemanticVersioning() == has_semantic_version);
}

TEST_CASE("semantic version ordering compares numerically", "[version]") {
    Version v9("9.1.1 0x250512");
    Version v10("10.0.1 0x250512");

    CHECK(v9 <= v10);
    CHECK_FALSE(v10 <= v9);
}

TEST_CASE("semantic version compares prerelease lexicographically",
          "[version]") {
    Version rc("10.1.1.rc 0x250512");
    Version zz("10.1.1.zz 0x250512");

    CHECK(rc <= zz);
    CHECK_FALSE(zz <= rc);
}

TEST_CASE("semantic version compares prerelease with no prerelease",
          "[version]") {
    Version prerelease("10.1.1.rc 0x250512");
    Version release("10.1.1 0x250512");

    CHECK_FALSE(prerelease <= release);
    CHECK(release <= prerelease);
}

TEST_CASE(
    "semantic comparison falls back to date on malformed semantic version",
    "[version]") {
    Version malformed("10.1 0x250512");
    Version valid("10.1.1 0x250513");

    CHECK(malformed <= valid);
    CHECK_FALSE(valid <= malformed);
}

TEST_CASE("semantic version compares prerelease and without", "[version]") {
    Version v1011rc("10.1.1.rc 0x250512");
    Version v1010("10.1.0 0x250512");

    CHECK_FALSE(v1011rc <= v1010);
    CHECK(v1010 <= v1011rc);
}

TEST_CASE("semantic version", "[version]") {
    auto version = Version("10.1.1.rc 0x250512");
    CHECK(version.hasSemanticVersioning());
    CHECK(version.getDate() == "0x250512");
    CHECK(version.toString() == "10.1.1.rc 0x250512");
    auto semver = version.getSemanticVersion();
    CHECK(semver.toString() == "10.1.1.rc");
    CHECK(semver.major == 10);
    CHECK(semver.minor == 1);
    CHECK(semver.patch == 1);
    CHECK(semver.prerelease == ".rc");

    auto version2 = Version("9.2.1 0x250512");
    CHECK(version2.hasSemanticVersioning());
    CHECK(version2.getDate() == "0x250512");
    CHECK(version2.toString() == "9.2.1 0x250512");
    auto semver2 = version2.getSemanticVersion();
    CHECK(semver2.toString() == "9.2.1");
    CHECK(semver2.major == 9);
    CHECK(semver2.minor == 2);
    CHECK(semver2.patch == 1);
    CHECK(semver2.prerelease == "");
    CHECK(version != version2);

    auto dev_version = Version("developer 0x250512");
    CHECK_FALSE(dev_version.hasSemanticVersioning());
    CHECK(dev_version.getDate() == "0x250512");
    CHECK(dev_version.toString() == "0.0.0 0x250512");
    REQUIRE_THROWS(dev_version.getSemanticVersion());
    REQUIRE_THROWS(dev_version.getMajorVersion());
    REQUIRE_THROWS(dev_version.getMinorVersion());
    REQUIRE_THROWS(dev_version.getPatchVersion());
    REQUIRE_THROWS(dev_version.getPrereleaseVersion());
    REQUIRE_THROWS(dev_version.isBackwardCompatible(version));
    CHECK(dev_version == version);

    auto zero_version = Version("0.0.0 0x250512");
    CHECK_FALSE(zero_version.hasSemanticVersioning());
    CHECK(zero_version.getDate() == "0x250512");
    CHECK(zero_version.toString() == "0.0.0 0x250512");
    REQUIRE_THROWS(zero_version.getSemanticVersion());
    REQUIRE_THROWS(zero_version.getMajorVersion());
    REQUIRE_THROWS(zero_version.getMinorVersion());
    REQUIRE_THROWS(zero_version.getPatchVersion());
    REQUIRE_THROWS(zero_version.getPrereleaseVersion());
    REQUIRE_THROWS(zero_version.isBackwardCompatible(version));
    CHECK(zero_version == dev_version);

    auto date_only_version = Version("0x250512");
    CHECK_FALSE(date_only_version.hasSemanticVersioning());
    CHECK(date_only_version.getDate() == "0x250512");
    CHECK(date_only_version.toString() == "0.0.0 0x250512");
    REQUIRE_THROWS(date_only_version.getSemanticVersion());
    REQUIRE_THROWS(date_only_version.getMajorVersion());
    REQUIRE_THROWS(date_only_version.getMinorVersion());
    REQUIRE_THROWS(date_only_version.getPatchVersion());
    REQUIRE_THROWS(date_only_version.getPrereleaseVersion());
    REQUIRE_THROWS(date_only_version.isBackwardCompatible(version));
    CHECK(date_only_version == zero_version);
}

} // namespace sls
