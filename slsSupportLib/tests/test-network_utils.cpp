
#include "catch.hpp"
#include "network_utils.h"
#include <iostream>
#include <vector>

#include "string_utils.h"

using namespace sls;


TEST_CASE("Convert mac address using classes") {

    std::vector<uint64_t> vec_addr{346856806822, 346856806852, 262027939863028};
    std::vector<std::string> vec_ans{"00:50:c2:46:d9:a6", "00:50:c2:46:d9:c4", "ee:50:22:46:d9:f4"};
    for (int i = 0; i != vec_addr.size(); ++i) {
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


TEST_CASE("Convert IP using classes ") {
    std::vector<uint32_t> vec_addr{4073554305, 2747957633, 2697625985};
    std::vector<std::string> vec_ans{"129.129.205.242", "129.129.202.163", "129.129.202.160"};

    for (int i = 0; i != vec_addr.size(); ++i) {
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

TEST_CASE("Strange input gives 0"){
    CHECK(IpAddr("hej")== 0);
    CHECK(MacAddr("hej")== 0);
}