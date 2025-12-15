#include "catch.hpp"
#include <string>

#include <stdlib.h>

#include "CtbConfig.h"
#include "SharedMemory.h"
#include <fstream>
#include <set>

namespace sls {

TEST_CASE("Default construction") {
    /*TODO static_assert(sizeof(CtbConfig) ==
                      (2 * sizeof(int) + (18 + 32 + 64 + 5 + 8) * 32) +
                        (sizeof(size_t) * 2) + ((sizeof(uint32_t) + 32) * 64) +
       ((32 + sizeof(uint32_t) + sizeof(int)) * 64), "Size of CtbConfig does not
       match "); //8952 (8696)*/

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
    auto regisernames = c.getRegisterNames();
    REQUIRE(regisernames.size() == 0);
    auto bitnames = c.getBitNames();
    REQUIRE(bitnames.size() == 0);
    REQUIRE(c.getRegisterNamesCount() == 0);
    auto registerNames = c.getRegisterNames();
    REQUIRE(registerNames.size() == 0);
    REQUIRE(c.getBitNamesCount() == 0);
    auto bitNames = c.getBitNames();
    REQUIRE(bitNames.size() == 0);
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

TEST_CASE("Add or modify a register name", "[.reg]") {
    CtbConfig c;
    auto addr = RegisterAddress(100);
    auto addr1 = RegisterAddress(300);

    REQUIRE(c.getRegisterNamesCount() == 0);
    REQUIRE_THROWS(
        c.setRegisterName("reg1_with_a_really_long_name_to_crash", addr));
    // add an entry
    REQUIRE_NOTHROW(c.setRegisterName("reg1", addr));
    REQUIRE(c.getRegisterName(addr) == "reg1");
    REQUIRE(c.getRegisterAddress("reg1") == addr);
    REQUIRE(c.getRegisterNamesCount() == 1);
    // modify an entry
    REQUIRE_NOTHROW(c.setRegisterName("reg1", addr1));
    REQUIRE(c.getRegisterAddress("reg1") == addr1);
    REQUIRE(c.getRegisterName(addr1) == "reg1");
    // clear all entries
    REQUIRE_NOTHROW(c.clearRegisterNames());
    REQUIRE(c.getRegisterNamesCount() == 0);
    REQUIRE_THROWS(c.getRegisterName(addr));
    REQUIRE_THROWS(c.getRegisterAddress("reg1"));
    REQUIRE_THROWS(c.getRegisterName(addr1));
}

TEST_CASE("Add a register list", "[.reg]") {
    CtbConfig c;
    REQUIRE(c.getRegisterNamesCount() == 0);

    // add a list
    std::map<std::string, RegisterAddress> list = {
        {"reg1", RegisterAddress(0x100)},
        {"reg2", RegisterAddress(0x200)},
        {"reg3", RegisterAddress(0x300)}};
    REQUIRE_NOTHROW(c.setRegisterNames(list));
    REQUIRE(c.getRegisterNamesCount() == static_cast<int>(list.size()));
    auto names = c.getRegisterNames();
    REQUIRE(names.size() == 3);

    // TODO std::set<RegisterAddress> seen_values;
    for (const auto &[key, val] : list) {
        // check for duplicate keys, and key-value match
        REQUIRE(names.count(key) == 1);
        REQUIRE(names.at(key) == val);

        // check for duplicate values
        // TODO REQUIRE(seen_values.count(val) == 0);
        // TODO seen_values.insert(val);
    }

    // clear all entries
    REQUIRE_NOTHROW(c.clearRegisterNames());
    REQUIRE(c.getRegisterNames().size() == 0);
}

TEST_CASE("Finding a regiser name or address", "[.reg]") {
    CtbConfig c;
    RegisterAddress addr1(0x100);
    RegisterAddress addr2(0x200);
    RegisterAddress addr3(0x300);

    // find nothing
    REQUIRE(c.getRegisterNamesCount() == 0);
    REQUIRE_THROWS(c.getRegisterName(addr1));
    REQUIRE_THROWS(c.getRegisterAddress("reg1"));

    std::map<std::string, RegisterAddress> list = {
        {"reg1", addr1}, {"reg2", addr2}, {"reg3", addr3}};
    REQUIRE_NOTHROW(c.setRegisterNames(list));

    // find
    REQUIRE(c.getRegisterName(addr1) == "reg1");
    REQUIRE(c.getRegisterName(addr2) == "reg2");
    REQUIRE(c.getRegisterAddress("reg3") == addr3);
}

TEST_CASE("Add or modify a bit name", "[.reg]") {
    CtbConfig c;
    RegisterAddress addr(0x100);
    BitAddress pos(addr, 2);
    BitAddress pos1(addr, 5);

    REQUIRE(c.getBitNamesCount() == 0);

    REQUIRE_THROWS(c.setBitName("bit1_with_a_really_long_name_to_crash",
                                BitAddress(addr, 100)));
    REQUIRE_THROWS(c.setBitName("bit1", BitAddress(addr, 32)));
    REQUIRE_THROWS(c.setBitName("bit1", BitAddress(addr, -1)));

    // add an entry
    REQUIRE_NOTHROW(c.setBitName("bit1", pos));
    REQUIRE(c.getBitName(pos) == "bit1");
    REQUIRE(c.getBitAddress("bit1") == pos);
    REQUIRE(c.getBitNamesCount() == 1);
    // modify an entry
    REQUIRE_NOTHROW(c.setBitName("bit1", pos1));
    REQUIRE(c.getBitAddress("bit1") == pos1);
    REQUIRE(c.getBitName(pos1) == "bit1");
    // clear all entries
    REQUIRE_NOTHROW(c.clearBitNames());
    REQUIRE(c.getBitNamesCount() == 0);
    REQUIRE_THROWS(c.getBitName(pos));
    REQUIRE_THROWS(c.getBitAddress("bit1"));
    REQUIRE_THROWS(c.getBitName(pos1));
}

TEST_CASE("Add a bit list", "[.reg]") {
    CtbConfig c;
    REQUIRE(c.getBitNamesCount() == 0);

    RegisterAddress addr(0x100);
    BitAddress pos1(addr, 2);
    BitAddress pos2(addr, 21);

    BitAddress pos3(addr, 31);

    // add a list
    std::map<std::string, BitAddress> list = {
        {"bit1", pos1}, {"bit2", pos2}, {"bit3", pos3}};
    REQUIRE_NOTHROW(c.setBitNames(list));
    REQUIRE(c.getBitNamesCount() == 3);
    auto names = c.getBitNames();
    REQUIRE(names.size() == 3);

    // TODO std::set<BitAddress> seen_values;
    for (const auto &[key, val] : list) {
        // check for duplicate keys, and key-value match
        REQUIRE(names.count(key) == 1);
        REQUIRE(names.at(key) == val);

        // check for duplicate values
        // TODO REQUIRE(seen_values.count(val) == 0);
        // TODO seen_values.insert(val);
    }

    // clear all entries
    REQUIRE_NOTHROW(c.clearBitNames());
    REQUIRE(c.getBitNames().size() == 0);
}

TEST_CASE("Finding a bit value", "[.reg]") {
    CtbConfig c;
    RegisterAddress addr(0x100);
    BitAddress pos(addr, 2);
    BitAddress pos1(addr, 21);
    BitAddress pos2(addr, 31);

    // find nothing
    REQUIRE(c.getBitNamesCount() == 0);
    REQUIRE_THROWS(c.getBitAddress("bit"));

    std::map<std::string, BitAddress> list = {
        {"bit0", pos}, {"bit1", pos1}, {"bit2", pos2}};
    REQUIRE_NOTHROW(c.setBitNames(list));

    // find
    REQUIRE(c.getBitAddress("bit2") == pos2);
    REQUIRE(c.getBitName(pos1) == "bit1");
}

} // namespace sls
