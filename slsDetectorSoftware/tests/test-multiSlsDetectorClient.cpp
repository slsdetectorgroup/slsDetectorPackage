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

TEST_CASE("enablefwrite", "[.cmd]") {
    auto oss = std::ostringstream{};
    multiSlsDetectorClient("enablefwrite 1", PUT, nullptr, oss);
    REQUIRE(oss.str() == "enablefwrite 1\n");


    oss = std::ostringstream{};
    multiSlsDetectorClient("enablefwrite", GET, nullptr, oss);
    REQUIRE(oss.str() == "enablefwrite 1\n");

    oss = std::ostringstream{};
    multiSlsDetectorClient("enablefwrite 0", PUT, nullptr, oss);
    REQUIRE(oss.str() == "enablefwrite 0\n");
}

TEST_CASE("enableoverwrite", "[.cmd]") {
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

TEST_CASE("activatecmd", "[.cmd]") {
    //TODO! read padding from somewhere
    auto oss = std::ostringstream{};
    multiSlsDetectorClient("activate 0", PUT, nullptr, oss);
    REQUIRE(oss.str() == "activate 0 padding\n");

    oss = std::ostringstream{};
    multiSlsDetectorClient("activate", GET, nullptr, oss);
    REQUIRE(oss.str() == "activate 0 padding\n");

    oss = std::ostringstream{};
    multiSlsDetectorClient("activate 1", PUT, nullptr, oss);
    REQUIRE(oss.str() == "activate 1 padding\n");
}

TEST_CASE("masterfile", "[.cmd]") {
    auto oss = std::ostringstream{};
    multiSlsDetectorClient("masterfile 0", PUT, nullptr, oss);
    REQUIRE(oss.str() == "masterfile 0\n");


    oss = std::ostringstream{};
    multiSlsDetectorClient("masterfile", GET, nullptr, oss);
    REQUIRE(oss.str() == "masterfile 0\n");

    oss = std::ostringstream{};
    multiSlsDetectorClient("masterfile 1", PUT, nullptr, oss);
    REQUIRE(oss.str() == "masterfile 1\n");
}

TEST_CASE("rx_tcpport", "[.cmd]") {
    auto oss = std::ostringstream{};
    multiSlsDetectorClient("rx_tcpport 1500", PUT, nullptr, oss);
    REQUIRE(oss.str() == "rx_tcpport 1500\n");

    oss = std::ostringstream{};
    REQUIRE_THROWS( multiSlsDetectorClient("rx_tcpport 15", PUT, nullptr, oss));
    

    oss = std::ostringstream{};
    multiSlsDetectorClient("rx_tcpport", GET, nullptr, oss);
    REQUIRE(oss.str() == "rx_tcpport 1500\n");

    oss = std::ostringstream{};
    multiSlsDetectorClient("rx_tcpport 1954", PUT, nullptr, oss);
    REQUIRE(oss.str() == "rx_tcpport 1954\n");
}

TEST_CASE("fname", "[.cmd]") {
    auto oss = std::ostringstream{};
    multiSlsDetectorClient("fname somename", PUT, nullptr, oss);
    REQUIRE(oss.str() == "fname somename\n");


    oss = std::ostringstream{};
    multiSlsDetectorClient("fname", GET, nullptr, oss);
    REQUIRE(oss.str() == "fname somename\n");

    oss = std::ostringstream{};
    multiSlsDetectorClient("fname run", PUT, nullptr, oss);
    REQUIRE(oss.str() == "fname run\n");
}

TEST_CASE("resetframescaught get framescaught", "[.cmd]") {
    auto oss = std::ostringstream{};
    multiSlsDetectorClient("resetframescaught 0", PUT, nullptr, oss);
    REQUIRE(oss.str() == "resetframescaught successful\n");


    oss = std::ostringstream{};
    multiSlsDetectorClient("framescaught", GET, nullptr, oss);
    REQUIRE(oss.str() == "framescaught 0\n");
}

