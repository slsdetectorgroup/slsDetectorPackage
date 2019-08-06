#include "Result.h"
#include "catch.hpp"

using sls::Result;

TEST_CASE("Default construction", "[detector][n2]") {
    Result<int> res;
    REQUIRE(res.size() == 0);
    REQUIRE(res.empty() == true);
}

TEST_CASE("Initializer list construction", "[n2]") {
    Result<int> res{1, 2, 5};
    REQUIRE(res.empty() == false);
    REQUIRE(res.size() == 3);
    REQUIRE(res[0] == 1);
    REQUIRE(res[1] == 2);
    REQUIRE(res[2] == 5);
}

TEST_CASE("Construct with value and number", "[n2]") {
    Result<int> res(5, 7);
    REQUIRE(res.size() == 5);
    REQUIRE(res[0] == 7);
    REQUIRE(res[1] == 7);
    REQUIRE(res[2] == 7);
    REQUIRE(res[3] == 7);
    REQUIRE(res[4] == 7);
}

TEST_CASE("Squash empty", "[n2]"){
    Result<double> res;
    REQUIRE(res.squash() == 0.);
}

TEST_CASE("Squash", "[n2]"){
    Result<int> res{3,3,3};
    REQUIRE(res.squash() == 3);

    res.push_back(5);
    REQUIRE(res.squash() == 0);
}