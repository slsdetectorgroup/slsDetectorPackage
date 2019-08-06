#include "Result.h"
#include "catch.hpp"


using sls::Result;


TEST_CASE("Default construction", "[detector][n2]"){
    Result<int> res;
    REQUIRE(res.size() == 0);
}

TEST_CASE("Initializer list construction", "[n2]"){
    Result<int> res{1, 2, 5};
    std::vector<int> vec{1,2,3};
    REQUIRE(res.size() == 3);
    REQUIRE(res[0] == 1);
    REQUIRE(res[1] == 2);
    REQUIRE(res[2] == 5);
}

TEST_CASE("Construct with value and number"){
    Result<int> res(5,7);
    REQUIRE(res.size() == 5);
}