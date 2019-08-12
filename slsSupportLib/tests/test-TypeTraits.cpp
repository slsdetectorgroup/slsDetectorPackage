#include "TypeTraits.h"
#include "catch.hpp"
#include <array>
#include <vector>
#include <sstream>
#include <chrono>
#include <initializer_list>

//Dummy classes only used here for testing
class DummyWithStr {
  public:
    std::string str();
};

class DummyNoStr {
  public:
    std::string somethingelse();
};

TEST_CASE("sls::is_container") {

    CHECK(sls::is_container<std::vector<int>>::value == true);
    CHECK(sls::is_container<std::array<double, 3>>::value == true);
}

TEST_CASE("Check for str() method") {
    REQUIRE(sls::has_str<DummyWithStr>::value == true);
    REQUIRE(sls::has_str<DummyNoStr>::value == false);
}

TEST_CASE("Check for str() on ostream") {
    REQUIRE(sls::has_str<std::ostringstream>::value == true);
}

TEST_CASE("sls::is_duration"){
    REQUIRE(sls::is_duration<std::chrono::nanoseconds>::value == true);
    REQUIRE(sls::is_duration<std::chrono::seconds>::value == true);
    REQUIRE(sls::is_duration<std::chrono::hours>::value == true);

    REQUIRE(sls::is_duration<int>::value == false);
    REQUIRE(sls::is_duration<std::vector<int>>::value == false);
}

TEST_CASE("initializer list"){
    REQUIRE(sls::is_light_container<std::initializer_list<int>>::value == true);
}