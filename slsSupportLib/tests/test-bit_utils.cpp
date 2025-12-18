// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "catch.hpp"
#include "sls/bit_utils.h"
#include <sstream>
#include <vector>

namespace sls {

TEST_CASE("Get set bits from 0") {
    auto vec = getSetBits(0);
    REQUIRE(vec.empty());
}

TEST_CASE("Get set bits from 1") {
    auto vec = getSetBits(1);
    REQUIRE(vec.size() == 1);
    REQUIRE(vec[0] == 0);
}

TEST_CASE("Get set bits from 2") {
    auto vec = getSetBits(2ul);
    REQUIRE(vec.size() == 1);
    REQUIRE(vec[0] == 1);
}

TEST_CASE("Get set bits from 3") {
    auto vec = getSetBits(3u);
    REQUIRE(vec.size() == 2);
    REQUIRE(vec[0] == 0);
    REQUIRE(vec[1] == 1);
}

TEST_CASE("All bits set") {
    uint8_t val = -1;
    auto vec = getSetBits(val);
    REQUIRE(vec == std::vector<int>{0, 1, 2, 3, 4, 5, 6, 7});
}

TEST_CASE("Get set bits from 523") {
    // 0b1000001011 == 523
    auto vec = getSetBits(523);
    REQUIRE(vec == std::vector<int>{0, 1, 3, 9});
}

TEST_CASE("Convert RegisterAddress using classes ", "[support][.bit_utils]") {
    std::vector<uint32_t> vec_addr{0x305, 0xffffffff, 0x0, 0x34550987,
                                   0x1fff1fff};
    std::vector<std::string> vec_ans{"0x305", "0xffffffff", "0x0", "0x34550987",
                                     "0x1fff1fff"};

    for (size_t i = 0; i != vec_addr.size(); ++i) {
        auto reg0 = RegisterAddress(vec_addr[i]);
        auto reg1 = RegisterAddress(vec_ans[i]);
        auto reg2 = RegisterAddress(vec_addr[0]);

        CHECK(reg0 == reg1);
        if (i != 0)
            CHECK(reg2 != reg1);
        CHECK(reg0.value() == vec_addr[i]);
        CHECK(reg1.value() == vec_addr[i]);
        CHECK(reg0.str() == vec_ans[i]);
        CHECK(reg1.str() == vec_ans[i]);
    }
}

TEST_CASE("Convert RegisterValue using classes ", "[support][.bit_utils]") {
    std::vector<uint32_t> vec_addr{0x305, 0xffffffff, 0x0, 500254562,
                                   0x1fff1fff};
    std::vector<std::string> vec_ans{"0x305", "0xffffffff", "0x0", "0x1dd14762",
                                     "0x1fff1fff"};

    for (size_t i = 0; i != vec_addr.size(); ++i) {
        auto reg0 = RegisterValue(vec_addr[i]);
        auto reg2 = RegisterValue(vec_addr[0]);

        if (i != 0)
            CHECK(reg2 != reg0);
        CHECK(reg0.value() == vec_addr[i]);
        CHECK(reg0.str() == vec_ans[i]);
        CHECK((reg0 | 0xffffffffu) == RegisterValue(0xffffffffu));
        CHECK((reg0 | 0x0) == reg0);
        CHECK((reg0 | 0x1) == reg0);
    }
}

TEST_CASE("Convert BitAddress using classes", "[support][.bit_utils]") {
    std::vector<RegisterAddress> vec_addr{
        RegisterAddress(0x305), RegisterAddress(0xffffffffu),
        RegisterAddress(0x0), RegisterAddress(0x34550987),
        RegisterAddress(0x1fff1fff)};
    std::vector<std::string> vec_addr_str{"0x305", "0xffffffff", "0x0",
                                          "0x34550987", "0x1fff1fff"};
    std::vector<uint32_t> vec_bitpos{0, 15, 31, 7, 23};
    std::vector<std::string> vec_bitpos_str{"0", "15", "31", "7", "23"};
    std::vector<std::string> vec_ans{
        "[0x305, 0]",      "[0xffffffff, 15]", "[0x0, 31]",
        "[0x34550987, 7]", "[0x1fff1fff, 23]",
    };

    for (size_t i = 0; i != vec_addr.size(); ++i) {
        auto reg0 = BitAddress(vec_addr[i], vec_bitpos[i]);

        BitAddress reg1(vec_addr[i], vec_bitpos[i]);
        CHECK(reg1.address() == vec_addr[i]);
        CHECK(reg1.bitPosition() == vec_bitpos[i]);

        auto reg2 = BitAddress(vec_addr[0], vec_bitpos[0]);

        CHECK(reg0 == reg1);
        if (i != 0)
            CHECK(reg2 != reg1);
        CHECK(reg0.address() == vec_addr[i]);
        CHECK(reg1.address() == vec_addr[i]);
        CHECK(reg0.bitPosition() == vec_bitpos[i]);
        CHECK(reg1.bitPosition() == vec_bitpos[i]);
        CHECK(std::to_string(reg0.bitPosition()) == vec_bitpos_str[i]);
        CHECK(std::to_string(reg1.bitPosition()) == vec_bitpos_str[i]);

        CHECK(reg0.str() == vec_ans[i]);
        CHECK(reg1.str() == vec_ans[i]);
    }
}

TEST_CASE("Output operator gives same result as string",
          "[support][.bit_utils]") {
    {
        RegisterAddress addr{0x3456af};
        std::ostringstream os;
        os << addr;
        CHECK(os.str() == "0x3456af");
        CHECK(os.str() == addr.str());
    }
    {
        BitAddress addr{RegisterAddress(0x3456af), 15};
        std::ostringstream os;
        os << addr;
        CHECK(os.str() == "[0x3456af, 15]");
        CHECK(os.str() == addr.str());
    }
    {
        RegisterValue addr{0x3456af};
        std::ostringstream os;
        os << addr;
        CHECK(os.str() == "0x3456af");
        CHECK(os.str() == addr.str());
    }
}

} // namespace sls
