
#include "catch.hpp"
#include "network_utils.h"
#include <iostream>
#include <vector>

#include "string_utils.h"
#include "sls_detector_exceptions.h"

using namespace sls;

TEST_CASE("Convert mac address using classes", "[support]") {

    std::vector<uint64_t> vec_addr{346856806822, 346856806852, 262027939863028};
    std::vector<std::string> vec_ans{"00:50:c2:46:d9:a6", "00:50:c2:46:d9:c4",
                                     "ee:50:22:46:d9:f4"};
    for (size_t i = 0; i != vec_addr.size(); ++i) {
        auto mac0 = MacAddr(vec_addr[i]);
        auto mac1 = MacAddr(vec_ans[i]);

        CHECK(mac0 == vec_addr[i]);
        CHECK(mac1 == vec_addr[i]);
        CHECK(mac0 == vec_ans[i]);
        CHECK(mac1 == vec_ans[i]);
        CHECK(mac0.str() == vec_ans[i]);
        CHECK(mac1.str() == vec_ans[i]);
    }
}

TEST_CASE("Hex representation of MAC", "[support]") {
    MacAddr m{346856806822};
    CHECK(m.hex() == "0050c246d9a6");
    CHECK(m.str() == "00:50:c2:46:d9:a6");
    CHECK_FALSE(m == 7);

    MacAddr m2{"00:50:c2:46:d9:c4"};
    CHECK(m2 == 346856806852);
    CHECK(m2.hex() == "0050c246d9c4");
    CHECK(m2.str() == "00:50:c2:46:d9:c4");

    CHECK_FALSE(m2 == 3);
}

TEST_CASE("Convert IP using classes ", "[support]") {
    std::vector<uint32_t> vec_addr{4073554305, 2747957633, 2697625985};
    std::vector<std::string> vec_ans{"129.129.205.242", "129.129.202.163",
                                     "129.129.202.160"};

    for (size_t i = 0; i != vec_addr.size(); ++i) {
        auto ip0 = IpAddr(vec_addr[i]);
        auto ip1 = IpAddr(vec_ans[i]);

        CHECK(ip0 == ip1);
        CHECK(ip0 == vec_addr[i]);
        CHECK(ip1 == vec_addr[i]);
        CHECK(ip0 == vec_ans[i]);
        CHECK(ip1 == vec_ans[i]);
        CHECK(ip0.str() == vec_ans[i]);
        CHECK(ip1.str() == vec_ans[i]);
    }
}

TEST_CASE("Strange input gives 0", "[support]") {
    CHECK(IpAddr("hej") == 0);
    CHECK(MacAddr("hej") == 0);
}

TEST_CASE("Convert to uint for sending over network", "[support]") {
    MacAddr addr{346856806822};
    uint64_t a = addr.uint64();
    CHECK(a == 346856806822);

    IpAddr addr2{"129.129.205.242"};
    uint32_t b = addr2.uint32();
    CHECK(b == 4073554305);
}

TEST_CASE("Hostname lookup failed throws", "[support]"){
    CHECK_THROWS_AS(HostnameToIp("pippifax"), RuntimeError);
}