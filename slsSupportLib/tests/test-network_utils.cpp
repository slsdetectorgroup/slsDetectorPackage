
#include "catch.hpp"
#include "network_utils.h"
#include <iostream>
#include <vector>

#include "string_utils.h"

using namespace sls;
TEST_CASE("Convert mac address") {

    std::vector<uint64_t> vec_addr{346856806822, 346856806852, 262027939863028};
    std::vector<std::string> vec_ans{"00:50:c2:46:d9:a6", "00:50:c2:46:d9:c4", "ee:50:22:46:d9:f4"};
    for (int i = 0; i != vec_addr.size(); ++i) {
        auto mac = vec_addr[i];
        auto answer = vec_ans[i];

        std::string string_addr = MacAddrToString(mac);
        CHECK(string_addr == answer);
        CHECK(MacStringToUint(string_addr) == mac);
    }
}

TEST_CASE("Convert IP"){
    std::vector<uint32_t> vec_addr{4073554305, 2747957633, 2697625985};
    std::vector<std::string> vec_ans{"129.129.205.242", "129.129.202.163", "129.129.202.160"};

    for (int i=0; i!= vec_addr.size(); ++i){
        auto ip = vec_addr[i];
        auto answer = vec_ans[i];

        auto string_addr = IpToString(ip);
        CHECK(string_addr == answer);
        CHECK(IpStringToUint(string_addr.c_str()) == ip);
    }
}

TEST_CASE("IP not valid"){

    CHECK(IpStringToUint("hej") == 0);
    CHECK(IpStringToUint("mpc2408") == 0);
}

// TEST_CASE("Lookup ip")