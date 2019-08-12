#include "catch.hpp"
#include "TypeTraits.h"
#include <vector>
#include <array>

TEST_CASE("something", "[n3]"){

    CHECK(sls::is_container<std::vector<int>>::value == true);
    CHECK(sls::is_container<std::array<double, 3>>::value == true);
}