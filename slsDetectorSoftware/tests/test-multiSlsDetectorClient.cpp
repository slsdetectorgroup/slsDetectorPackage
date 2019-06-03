#include "catch.hpp"
#include "multiSlsDetectorClient.h"
#include "sls_detector_defs.h"
#include <sstream>

auto GET = slsDetectorDefs::GET_ACTION;
auto PUT = slsDetectorDefs::PUT_ACTION;

TEST_CASE("rx_fifodepth", "[.cmd]") {
    auto oss = std::ostringstream{};
    multiSlsDetectorClient("rx_fifodepth 10", PUT, nullptr, oss);
    REQUIRE(oss.str() == "rx_fifodepth 10\n");

    oss = std::ostringstream{};
    multiSlsDetectorClient("rx_fifodepth 100", PUT, nullptr, oss);
    REQUIRE(oss.str() == "rx_fifodepth 100\n");

    oss = std::ostringstream{};
    multiSlsDetectorClient("rx_fifodepth", GET, nullptr, oss);
    REQUIRE(oss.str() == "rx_fifodepth 100\n");

    oss = std::ostringstream{};
    multiSlsDetectorClient("0:rx_fifodepth", GET, nullptr, oss);
    REQUIRE(oss.str() == "0:rx_fifodepth 100\n");
}

TEST_CASE("frames", "[.cmd]") {
    auto oss = std::ostringstream{};
    multiSlsDetectorClient("frames 1000", PUT, nullptr, oss);
    REQUIRE(oss.str() == "frames 1000\n");

    oss = std::ostringstream{};
    multiSlsDetectorClient("frames", GET, nullptr, oss);
    REQUIRE(oss.str() == "frames 1000\n");

    oss = std::ostringstream{};
    multiSlsDetectorClient("frames 1", PUT, nullptr, oss);
    REQUIRE(oss.str() == "frames 1\n");
}

TEST_CASE("receiver", "[.cmd]") {
    auto oss = std::ostringstream{};
    multiSlsDetectorClient("receiver start", PUT, nullptr, oss);
    REQUIRE(oss.str() == "receiver running\n");

    oss = std::ostringstream{};
    multiSlsDetectorClient("receiver", GET, nullptr, oss);
    REQUIRE(oss.str() == "receiver running\n");

    oss = std::ostringstream{};
    multiSlsDetectorClient("receiver stop", PUT, nullptr, oss);
    REQUIRE(oss.str() == "receiver idle\n");

    oss = std::ostringstream{};
    multiSlsDetectorClient("receiver", GET, nullptr, oss);
    REQUIRE(oss.str() == "receiver idle\n");
}

TEST_CASE("enableoverwrite, [.cmd]") {
    auto oss = std::ostringstream{};
    multiSlsDetectorClient("overwrite 1", PUT, nullptr, oss);
    REQUIRE(oss.str() == "overwrite 1\n");

    oss = std::ostringstream{};
    multiSlsDetectorClient("overwrite", GET, nullptr, oss);
    REQUIRE(oss.str() == "overwrite 1\n");

    oss = std::ostringstream{};
    multiSlsDetectorClient("overwrite 0", PUT, nullptr, oss);
    REQUIRE(oss.str() == "overwrite 0\n");
}