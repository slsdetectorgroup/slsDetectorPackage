#include "catch.hpp"
#include <string>

#include <stdlib.h>

#include "CtbConfig.h"
#include "SharedMemory.h"
#include <fstream>

namespace sls {

TEST_CASE("Default construction") {
    static_assert(sizeof(CtbConfig) == ((18 + 32 + 64 + 5 + 8) * 20),
                  "Size of CtbConfig does not match");

    CtbConfig c;
    auto dacnames = c.getDacNames();
    REQUIRE(dacnames.size() == 18);
    REQUIRE(dacnames[0] == "DAC0");
    REQUIRE(dacnames[1] == "DAC1");
    REQUIRE(dacnames[2] == "DAC2");
    REQUIRE(dacnames[3] == "DAC3");
    auto adcnames = c.getAdcNames();
    REQUIRE(adcnames.size() == 32);
    REQUIRE(adcnames[0] == "ADC0");
    REQUIRE(adcnames[1] == "ADC1");
    REQUIRE(adcnames[2] == "ADC2");
    REQUIRE(adcnames[3] == "ADC3");
    auto powernames = c.getPowerNames();
    REQUIRE(powernames.size() == 5);
    REQUIRE(powernames[0] == "VA");
    REQUIRE(powernames[1] == "VB");
    REQUIRE(powernames[2] == "VC");
    REQUIRE(powernames[3] == "VD");
    REQUIRE(powernames[4] == "VIO");
    auto signalnames = c.getSignalNames();
    REQUIRE(signalnames.size() == 64);
    REQUIRE(signalnames[0] == "BIT0");
    REQUIRE(signalnames[1] == "BIT1");
    REQUIRE(signalnames[2] == "BIT2");
    REQUIRE(signalnames[3] == "BIT3");
    auto sensenames = c.getSlowADCNames();
    REQUIRE(sensenames.size() == 8);
    REQUIRE(sensenames[0] == "SLOWADC0");
    REQUIRE(sensenames[1] == "SLOWADC1");
    REQUIRE(sensenames[2] == "SLOWADC2");
    REQUIRE(sensenames[3] == "SLOWADC3");
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
