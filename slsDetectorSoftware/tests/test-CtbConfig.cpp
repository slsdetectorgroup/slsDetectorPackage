#include "catch.hpp"
#include <string>

#include <stdlib.h>

#include "CtbConfig.h"
#include "SharedMemory.h"
#include <fstream>

namespace sls {

TEST_CASE("Default construction") {
    static_assert(sizeof(CtbConfig) == ((18 + 32) * 20),
                  "Size of CtbConfig does not match");

    CtbConfig c;
    auto names = c.getDacNames();
    REQUIRE(names.size() == 18);
    REQUIRE(names[0] == "dac0");
    REQUIRE(names[1] == "dac1");
    REQUIRE(names[2] == "dac2");
    REQUIRE(names[3] == "dac3");
}

TEST_CASE("Set and get a single dac name") {
    CtbConfig c;
    c.setDacName(3, "vrf");
    auto names = c.getDacNames();

    REQUIRE(c.getDacName(3) == "vrf");
    REQUIRE(names[3] == "vrf");
}

TEST_CASE("Set a name that is too large throws") {
    CtbConfig c;
    REQUIRE_THROWS(c.setDacName(3, "somestringthatisreallytolongforadatac"));
}

TEST_CASE("Length of dac name cannot be 0") {
    CtbConfig c;
    REQUIRE_THROWS(c.setDacName(1, ""));
}

TEST_CASE("Copy a CTB config") {
    CtbConfig c1;
    c1.setDacName(5, "somename");

    auto c2 = c1;
    // change the name on the first object
    // to detecto shallow copy
    c1.setDacName(5, "someothername");
    REQUIRE(c2.getDacName(5) == "somename");
}

TEST_CASE("Move CtbConfig ") {
    CtbConfig c1;
    c1.setDacName(3, "yetanothername");
    CtbConfig c2(std::move(c1));
    REQUIRE(c2.getDacName(3) == "yetanothername");
}

} // namespace sls
