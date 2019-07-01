#include "catch.hpp"
#include "ToString.h"
// #include <chrono>
#include "TimeAlias.h"
using namespace sls;
TEST_CASE("something", "[support][now]"){
    ns t(150);
    REQUIRE(ToString(t) == "150 ns");
}