// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "catch.hpp"
#include "sls/bit_utils.h"
#include <vector>
#include <sstream>

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

TEST_CASE("Convert RegisterAddress using classes ", "[support][bit_utils]") {
    std::vector<uint32_t> vec_addr{0x305, 0xffffffff, 0x0, 0x34550987, 0x1fff1fff};
    std::vector<std::string> vec_ans{"0x305", "0xffffffff", "0x0", "0x34550987", "0x1fff1fff"};

    for (size_t i = 0; i != vec_addr.size(); ++i) {
        auto reg0 = RegisterAddress(vec_addr[i]);
        auto reg1 = RegisterAddress(vec_ans[i]);
        auto reg2 = RegisterAddress(vec_addr[0]);

        CHECK(reg0 == reg1);
        if (i != 0)
            CHECK(reg2 != reg1);
        CHECK(reg0 == vec_addr[i]);
        CHECK(reg1 == vec_addr[i]);
        CHECK(reg0.str() == vec_ans[i]);
        CHECK(reg1.str() == vec_ans[i]);
        CHECK(static_cast<uint32_t>(reg0) == vec_addr[i]);
        CHECK(static_cast<uint32_t>(reg1) == vec_addr[i]);
    }
}

TEST_CASE("Convert RegisterValue using classes ", "[support][bit_utils]") {
    std::vector<uint32_t> vec_addr{0x305, 0xffffffff, 0x0, 500254562, 0x1fff1fff};
    std::vector<std::string> vec_ans{"0x305", "0xffffffff", "0x0", "0x1dd14762", "0x1fff1fff"};

    for (size_t i = 0; i != vec_addr.size(); ++i) {
        auto reg0 = RegisterValue(vec_addr[i]);
        auto reg1 = RegisterValue(vec_ans[i]);
        auto reg2 = RegisterValue(vec_addr[0]);

        CHECK(reg0 == reg1);
        if (i != 0)
            CHECK(reg2 != reg1);
        CHECK(reg0 == vec_addr[i]);
        CHECK(reg1 == vec_addr[i]);
        CHECK(reg0.str() == vec_ans[i]);
        CHECK(reg1.str() == vec_ans[i]);
        CHECK(static_cast<uint32_t>(reg0) == vec_addr[i]);
        CHECK(static_cast<uint32_t>(reg1) == vec_addr[i]);
        CHECK((reg0 | 0xffffffffu) == 0xffffffffu);
        CHECK((reg0 | 0x0) == reg0);
    }
}

TEST_CASE("Convert BitPosition using classes ", "[support][bit_utils]") {
    std::vector<RegisterAddress> vec_addr{
        RegisterAddress(0x305), 
        RegisterAddress(0xffffffffu), 
        RegisterAddress(0x0), 
        RegisterAddress(0x34550987), 
        RegisterAddress(0x1fff1fff)
    };
    std::vector<int> vec_bitpos{0, 15, 31, 7, 23};
    std::vector<std::string> vec_ans{
        "[0x305, 0]", 
        "[0xffffffff, 15]", 
        "[0x0, 31]", 
        "[0x34550987, 7]", 
        "[0x1fff1fff, 23]",
    };

    for (size_t i = 0; i != vec_addr.size(); ++i) {
        auto reg0 = BitPosition(vec_addr[i], vec_bitpos[i]);

        BitPosition reg1;
        reg1.setAddress(vec_addr[i]);
        reg1.setBitPosition(vec_bitpos[i]);

        auto reg2 = BitPosition(vec_addr[0], vec_bitpos[0]);

        CHECK(reg0 == reg1);
        if (i != 0)
            CHECK(reg2 != reg1);
        CHECK(reg0.address() == vec_addr[i]);
        CHECK(reg1.address() == vec_addr[i]);
        CHECK(reg0.bitPosition() == vec_bitpos[i]);
        CHECK(reg1.bitPosition() == vec_bitpos[i]);

        CHECK(reg0.str() == vec_ans[i]);
        CHECK(reg1.str() == vec_ans[i]);
    }
}

TEST_CASE("Output operator gives same result as string", "[support][bit_utils]") {
    {
        RegisterAddress addr{"0x3456af"};
        std::ostringstream os;
        os << addr;
        CHECK(os.str() == "0x3456af");
        CHECK(os.str() == addr.str());
    }
    {   
        RegisterValue addr{"0x3456af"};
        std::ostringstream os;
        os << addr;
        CHECK(os.str() == "0x3456af");
        CHECK(os.str() == addr.str());
    }
}

TEST_CASE("Strange input throws", "[support][bit_utils]") {
    REQUIRE_THROWS(RegisterAddress("hej"));
    // ensure correct exception message
    try {
        RegisterAddress("hej");
    } catch (const std::exception &e) {
        REQUIRE(std::string(e.what()) == "Address must be an integer value.");
    }

    REQUIRE_THROWS(RegisterValue("hej"));
    // ensure correct exception message
    try {
        RegisterValue("hej");
    } catch (const std::exception &e) {
        REQUIRE(std::string(e.what()) == "Value must be an integer value.");
    }

    REQUIRE_THROWS(BitPosition(RegisterAddress(0x305), 32));
    // ensure correct exception message
    try {
        BitPosition(RegisterAddress(0x305), 32);
    } catch (const std::exception &e) {
        REQUIRE(std::string(e.what()) == "Bit position must be between 0 and 31.");
    }

}

TEST_CASE("Implicitly convert to uint for sending over network", "[support][bit_utils]") {
    RegisterAddress addr{0x305};
    uint64_t a = addr;
    CHECK(a == 0x305);

    RegisterValue addr1{0x305};
    uint64_t a1 = addr1;
    CHECK(a1 == 0x305);
}

} // namespace sls
