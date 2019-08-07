#include "Result.h"
#include "catch.hpp"

#include <type_traits>

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

TEST_CASE("Squash empty", "[n2]") {
    Result<double> res;
    REQUIRE(res.squash() == 0.);
}

TEST_CASE("Squash gives either value or default constructed value", "[n2]") {
    Result<int> res{3, 3, 3};
    REQUIRE(res.squash() == 3);

    res.push_back(5);
    REQUIRE(res.squash() == 0);
    REQUIRE(res.squash(-1) == -1);
}

TEST_CASE("Updating an element", "[n2]") {

    Result<int> res{1, 2, 3};
    REQUIRE(res[0] == 1);
    REQUIRE(res[1] == 2);
    REQUIRE(res[2] == 3);

    res[0] = 5;
    REQUIRE(res[0] == 5);
    REQUIRE(res[1] == 2);
    REQUIRE(res[2] == 3);
}

TEST_CASE("equal", "[n2]"){
    Result<float> res;

    // There are no elements to compare
    REQUIRE(res.equal() == false);

    //all (the one) elements are equal
    res.push_back(1.2);
    REQUIRE(res.equal() == true);

    res.push_back(1.2);
    REQUIRE(res.equal() == true);

    res.push_back(1.3);
    REQUIRE(res.equal() == false);
}
