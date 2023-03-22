// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "catch.hpp"
#include "sls/logger.h"
#include <iostream>
#include <vector>

#include "sls/string_utils.h"

namespace sls {

TEST_CASE("copy a string") {

    char src[10] = "hej";
    REQUIRE(src[3] == '\0');

    char dst[20];

    strcpy_safe(dst, src);
    REQUIRE(dst[0] == 'h');
    REQUIRE(dst[1] == 'e');
    REQUIRE(dst[2] == 'j');
    REQUIRE(dst[3] == '\0');
}

#ifdef NDEBUG
// This test can only run in release since we assert on the length of the string
TEST_CASE("copy a long string") {
    auto src = "some very very long sting that does not fit";
    char dst[3];
    strcpy_safe(dst, src);
    REQUIRE(dst[0] == 's');
    REQUIRE(dst[1] == 'o');
    REQUIRE(dst[2] == '\0');
}
#endif

TEST_CASE("split a string with end delimiter") {
    std::string s("abra+kadabra+");
    auto r = split(s, '+');
    REQUIRE(r.size() == 2);
    REQUIRE(r[0] == "abra");
    REQUIRE(r[1] == "kadabra");
}

TEST_CASE("split a string without end delimiter") {
    std::string s("abra+kadabra+filibom");
    auto r = split(s, '+');
    REQUIRE(r.size() == 3);
    REQUIRE(r[0] == "abra");
    REQUIRE(r[1] == "kadabra");
    REQUIRE(r[2] == "filibom");
}

TEST_CASE("Remove char from string") {
    char str[] = "sometest";
    removeChar(str, 'e');
    REQUIRE(std::string(str) == "somtst");
}

TEST_CASE("Remove char from empty string") {
    char str[50] = {};
    removeChar(str, 'e');
    REQUIRE(std::string(str).empty());
}

TEST_CASE("Many characters in a row") {
    char str[] = "someeequitellll::ongstring";
    removeChar(str, 'l');
    REQUIRE(std::string(str) == "someeequite::ongstring");
}

TEST_CASE("Check is string is integer") {

    REQUIRE(is_int("75"));
    REQUIRE(is_int("11675"));
    REQUIRE_FALSE(is_int("7.5"));
    REQUIRE_FALSE(is_int("hej"));
    REQUIRE_FALSE(is_int("7a"));
    REQUIRE_FALSE(is_int(""));
}

TEST_CASE("Replace substring in string") {
    std::string s = "this string should be replaced";
    auto r = replace_first(&s, "string ", "");
    REQUIRE(r == true);
    REQUIRE(s == "this should be replaced");
}

TEST_CASE("Replace --help in command") {
    std::string s = "sls_detector_get --help exptime";
    auto r = replace_first(&s, " --help", "");
    REQUIRE(r == true);
    REQUIRE(s == "sls_detector_get exptime");
}

TEST_CASE("Replace -h in command") {
    std::string s = "sls_detector_get -h exptime";
    auto r = replace_first(&s, " -h", "");
    REQUIRE(r == true);
    REQUIRE(s == "sls_detector_get exptime");
}

TEST_CASE("replace --help") {
    std::string s = "list --help";
    auto r = replace_first(&s, " --help", "");
    REQUIRE(r == true);
    REQUIRE(s == "list");
}

TEST_CASE("port host") {
    std::string hostport = "localhost:1954";
    auto res = ParseHostPort(hostport);
    REQUIRE(res.first == "localhost");
    REQUIRE(res.second == 1954);
}

TEST_CASE("port missing") {
    // TODO! is this the intended result?
    std::string host = "localhost";
    auto res = ParseHostPort(host);
    REQUIRE(res.first == "localhost");
    REQUIRE(res.second == 0);
}

// TEST_CASE("concat things not being strings")

} // namespace sls
