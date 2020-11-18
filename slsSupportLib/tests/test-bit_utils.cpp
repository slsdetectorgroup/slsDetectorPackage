#include "catch.hpp"
#include "sls/bit_utils.h"
#include <vector>

TEST_CASE("Get set bits from 0") {
    auto vec = sls::getSetBits(0);
    REQUIRE(vec.empty());
}

TEST_CASE("Get set bits from 1") {
    auto vec = sls::getSetBits(1);
    REQUIRE(vec.size() == 1);
    REQUIRE(vec[0] == 0);
}

TEST_CASE("Get set bits from 2") {
    auto vec = sls::getSetBits(2ul);
    REQUIRE(vec.size() == 1);
    REQUIRE(vec[0] == 1);
}

TEST_CASE("Get set bits from 3") {
    auto vec = sls::getSetBits(3u);
    REQUIRE(vec.size() == 2);
    REQUIRE(vec[0] == 0);
    REQUIRE(vec[1] == 1);
}

TEST_CASE("All bits set") {
    uint8_t val = -1;
    auto vec = sls::getSetBits(val);
    REQUIRE(vec == std::vector<int>{0, 1, 2, 3, 4, 5, 6, 7});
}

TEST_CASE("Get set bits from 523") {
    // 0b1000001011 == 523
    auto vec = sls::getSetBits(523);
    REQUIRE(vec == std::vector<int>{0, 1, 3, 9});
}
