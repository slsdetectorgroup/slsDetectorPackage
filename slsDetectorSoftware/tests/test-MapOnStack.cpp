#include "MapOnStack.h"
#include "catch.hpp"
#include "sls/bit_utils.h"

#include <string>

namespace sls {

using Key = FixedString<32>;

TEST_CASE("MapOnStack with int values") {
    MapOnStack<Key, int, 64, true> map;

    map.addKey("bit0", 55);
    map.addKeyOrSetValue("bit1", 66);
    REQUIRE(map.size() == 2);
    REQUIRE(map.capacity() == 64);
    REQUIRE(map.empty() == false);

    REQUIRE(map.getValue("bit0") == 55);
    REQUIRE(map.getValue("bit1") == 66);

    // Duplicate key throws
    REQUIRE_THROWS(map.addKey("bit0", 77));
    // Duplicate value throws
    REQUIRE_THROWS(map.addKey("bit2", 66));

    // modifies
    map.addKeyOrSetValue("bit0", 77);
    REQUIRE(map.getValue("bit0") == 77);

    // containsKey / hasValue
    REQUIRE(map.containsKey("bit1"));
    REQUIRE(map.hasValue(77));

    // Clear map
    map.clear();
    REQUIRE(map.empty());
    REQUIRE(map.size() == 0);
    REQUIRE(map.capacity() == 64);
}

TEST_CASE("MapOnStack with BitAddress values") {
    MapOnStack<Key, BitAddress, 64, true> map;

    RegisterAddress addr0(0x100);
    RegisterAddress addr1(0x200);
    BitAddress b0(addr0, 0);
    BitAddress b1(addr0, 1);
    BitAddress b2(addr1, 0);

    map.addKey("bit0", b0);
    map.addKeyOrSetValue("bit1", b1);
    REQUIRE(map.size() == 2);
    REQUIRE(map.capacity() == 64);
    REQUIRE(map.empty() == false);

    REQUIRE(map.getValue("bit0") == b0);
    REQUIRE(map.getValue("bit1") == b1);

    // Duplicate key throws
    REQUIRE_THROWS(map.addKey("bit0", b2));
    // Duplicate value throws
    REQUIRE_THROWS(map.addKey("bit2", b1));

    // modifies
    map.addKeyOrSetValue("bit0", b2);
    REQUIRE(map.getValue("bit0") == b2);

    // containsKey / hasValue
    REQUIRE(map.containsKey("bit1"));
    REQUIRE(map.hasValue(b2));

    // Clear map
    map.clear();
    REQUIRE(map.empty());
    REQUIRE(map.size() == 0);
    REQUIRE(map.capacity() == 64);
}

TEST_CASE("Set and get a single entry") {
    MapOnStack<Key, RegisterAddress, 64, true> map;
    RegisterAddress addr0(0x100);

    map.addKey("bit0", addr0);
    auto entries = map.getMap();
    REQUIRE(entries.size() == 1);

    REQUIRE(map.getValue("bit0") == addr0);
    REQUIRE(entries["bit0"] == addr0);
}

TEST_CASE("Set a name that is too large throws") {
    MapOnStack<Key, int, 64, true> map;
    REQUIRE_THROWS(map.addKey("somestringthatisreallytolongforadatac", 12));
}

TEST_CASE("Length of dac name cannot be 0") {
    MapOnStack<Key, int, 64, true> map;
    REQUIRE_THROWS(map.addKey("", 12));
}

TEST_CASE("Copy a MapOnStack") {
    MapOnStack<Key, int, 64, true> m1;
    m1.addKey("key1", 12);

    auto m2 = m1;
    // change the first object
    // to detecto shallow copy
    m1.setValue("key1", 34);
    REQUIRE(m2.getValue("key1") == 12);
}

TEST_CASE("Move MapOnStack ") {
    MapOnStack<Key, int, 64, true> m1;
    m1.addKey("key1", 12);
    MapOnStack<Key, int, 64, true> m2(std::move(m1));
    REQUIRE(m1.size() == 0);
    REQUIRE(m2.size() == 1);
}

TEST_CASE("Set map from std::map") {
    MapOnStack<Key, int, 64, true> map;
    std::map<Key, int> m1 = {{"key1", 11}, {"key2", 22}, {"key3", 33}};
    map.setMap(m1);
    REQUIRE(map.size() == 3);
    REQUIRE(map.getValue("key2") == 22);

    auto retrieved_map = map.getMap();
    REQUIRE(retrieved_map.size() == 3);
    REQUIRE(retrieved_map["key3"] == 33);

} // namespace sls
