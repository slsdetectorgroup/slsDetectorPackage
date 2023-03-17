// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "catch.hpp"
#include "sls/container_utils.h"
#include <exception>
#include <string>
#include <vector>

namespace sls {

TEST_CASE("Equality of an empty vector", "[support]") {
    std::vector<int> v;
    REQUIRE(v.empty());
    REQUIRE_FALSE(allEqual(v));
    REQUIRE_FALSE(allEqualWithTol(v, 2));
    REQUIRE_FALSE(allEqualTo(v, 5));
    REQUIRE_FALSE(anyEqualTo(v, 5));
    REQUIRE_FALSE(anyEqualToWithTol(v, 5, 1));
}

TEST_CASE("Equality of a vector with one element", "[support]") {
    std::vector<int> v{5};
    REQUIRE(v.size() == 1);
    REQUIRE(allEqual(v));
    REQUIRE(allEqualWithTol(v, 1));
    REQUIRE(allEqualTo(v, 5));
    REQUIRE(allEqualToWithTol(v, 5, 2));
    REQUIRE(anyEqualTo(v, 5));
    REQUIRE(anyEqualToWithTol(v, 5, 1));
}

TEST_CASE("A larger vector of the same elements", "[support]") {
    std::vector<int> v(101, 5);
    REQUIRE(v.size() == 101);
    REQUIRE(allEqual(v));
    REQUIRE(allEqualWithTol(v, 1));
    REQUIRE(allEqualTo(v, 5));
    REQUIRE(anyEqualTo(v, 5));

    SECTION("Push back another element to create a vector where not all are "
            "equal") {
        v.push_back(7);
        REQUIRE(v.size() == 102);
        REQUIRE_FALSE(allEqual(v));

        REQUIRE_FALSE(allEqualWithTol(v, 1));
        REQUIRE(allEqualWithTol(v, 3));

        REQUIRE_FALSE(allEqualTo(v, 5));

        REQUIRE_FALSE(allEqualToWithTol(v, 5, 1));
        REQUIRE(allEqualToWithTol(v, 5, 3));
        REQUIRE(anyEqualTo(v, 5));
    }
}

TEST_CASE("A vector of double with different values", "[support]") {
    std::vector<double> v{1.2, 2., 4.2, 4, 1.1};

    REQUIRE(allEqual(v) == false);
    REQUIRE(allEqualWithTol(v, 0.3) == false);
    REQUIRE(allEqualWithTol(v, 3.2));
}

TEST_CASE("Sum of empty vector", "[support]") {
    std::vector<float> v;
    REQUIRE(sum(v) == Approx(0));
}

TEST_CASE("Sum of vector", "[support]") {
    std::vector<double> v{1.2, 2., 4.2, 4, 1.13};
    REQUIRE(sum(v) == Approx(12.53));
}

TEST_CASE("Minus one if different", "[support]") {
    std::vector<double> v;
    REQUIRE(v.empty());
    double d = -1;
    REQUIRE(minusOneIfDifferent(v) == d);

    SECTION("single element") {
        v.push_back(7.3);
        REQUIRE(v.size() == 1);
        REQUIRE(minusOneIfDifferent(v) == Approx(7.3));
    }
    SECTION("different elements") {
        v.push_back(7.3);
        v.push_back(1.0);
        v.push_back(62.1);
        REQUIRE(minusOneIfDifferent(v) == Approx(-1.0));
    }
}

TEST_CASE("minus one does not have side effects", "[support]") {
    std::vector<int> v{1, 1, 1};
    int i = minusOneIfDifferent(v);
    REQUIRE(i == 1);
    i = 5;
    REQUIRE(v[0] == 1);
}

TEST_CASE("Compare a vector containing two vectors", "[support]") {

    std::vector<std::vector<int>> a{{0, 1, 2, 3, 4, 5}, {0, 1, 2, 3, 4, 5}};
    std::vector<std::vector<int>> b{{0, 1, 2, 3, 4, 5}, {0, 1, 2, 3, 3, 5}};
    std::vector<std::vector<int>> c{{0, 1, 2, 3, 4}, {0, 1, 2, 3, 3, 5}};
    std::vector<std::vector<int>> d{
        {0, 1, 2, 3, 4}, {0, 1, 2, 3, 4}, {0, 1, 2, 3, 4}};
    std::vector<int> e{0, 1, 2, 3, 4, 5};

    CHECK(minusOneIfDifferent(a) == a[0]);
    CHECK(minusOneIfDifferent(a) == e);
    CHECK(minusOneIfDifferent(b) == std::vector<int>{-1});
    CHECK(minusOneIfDifferent(c) == std::vector<int>{-1});
    CHECK(minusOneIfDifferent(d) == d[2]);
}

TEST_CASE("vector of bool", "[support]") {
    std::vector<bool> a{true, true, true};
    std::vector<bool> b{false, false, false};
    std::vector<bool> c{true, false, true};

    CHECK(minusOneIfDifferent(a) == 1);
    CHECK(minusOneIfDifferent(b) == 0);
    CHECK(minusOneIfDifferent(c) == -1);
}

TEST_CASE("compare a vector of arrays", "[support]") {

    std::vector<std::array<uint64_t, 3>> vec0{{5, 6, 8}, {5, 6, 8}, {5, 6, 8}};
    CHECK(minusOneIfDifferent(vec0) == std::array<uint64_t, 3>{5, 6, 8});

    std::array<uint64_t, 3> arr;
    arr.fill(-1);
    std::vector<std::array<uint64_t, 3>> vec1{{5, 90, 8}, {5, 6, 8}, {5, 6, 8}};
    CHECK(minusOneIfDifferent(vec1) == arr);
}

TEST_CASE("check if vector has duplicates") {
    std::vector<int> vec{1, 0, 2, 5, 3, 1, 8, 6};
    REQUIRE(hasDuplicates(vec) == true);
}

TEST_CASE("check for duplicates in vector of pairs") {
    std::vector<std::pair<std::string, int>> vec;
    vec.emplace_back("localhost", 1954);
    REQUIRE(hasDuplicates(vec) == false);

    vec.emplace_back("localhost", 1800);
    REQUIRE(hasDuplicates(vec) == false);

    vec.emplace_back("localhost", 1954);
    REQUIRE(hasDuplicates(vec) == true);
}

TEST_CASE("remove duplicates from vector") {
    std::vector<int> v{5, 6, 5, 3};
    auto r = removeDuplicates(v);
    CHECK(r == true); // did indeed remove elements
    CHECK(v == std::vector<int>{3, 5, 6});
}

TEST_CASE("remove duplicated empty vector") {
    std::vector<int> v;
    auto r = removeDuplicates(v);
    CHECK(r == false); // no elements to remove
    CHECK(v == std::vector<int>{});
}

} // namespace sls
