#include "catch.hpp"
#include "multiSlsDetectorClient.h"
#include "sls_detector_defs.h"
#include <sstream>

auto GET = slsDetectorDefs::GET_ACTION;
auto PUT = slsDetectorDefs::PUT_ACTION;

TEST_CASE("fail", "[.cmd]"){
    std::ostringstream oss1;
    multiSlsDetectorClient("rx_fifodepth 10", PUT, nullptr, oss1);
    REQUIRE(oss1.str() == "rx_fifodepth 10\n");

    std::ostringstream oss2;
    multiSlsDetectorClient("rx_fifodepth 100", PUT, nullptr, oss2);
    REQUIRE(oss2.str() == "rx_fifodepth 100\n");

    std::ostringstream oss3;
    multiSlsDetectorClient("rx_fifodepth", GET, nullptr, oss3);
    REQUIRE(oss3.str() == "rx_fifodepth 100\n");
}