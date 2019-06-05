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
    // TODO! read padding from somewhere
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

TEST_CASE("index", "[.cmd]") {
    auto oss = std::ostringstream{};
    multiSlsDetectorClient("index 57", PUT, nullptr, oss);
    REQUIRE(oss.str() == "index 57\n");

    oss = std::ostringstream{};
    multiSlsDetectorClient("index", GET, nullptr, oss);
    REQUIRE(oss.str() == "index 57\n");

    oss = std::ostringstream{};
    multiSlsDetectorClient("index 0", PUT, nullptr, oss);
    REQUIRE(oss.str() == "index 0\n");
}

TEST_CASE("rx_tcpport", "[.cmd]") {
    multiSlsDetector d;
    int port = 1500;
    int base = 1954;
    for (size_t i = 0; i != d.size(); ++i) {
        std::ostringstream oss;
        std::string cmd =
            std::to_string(i) + ":rx_tcpport " + std::to_string(port + i);
        std::cout << cmd << "\n";
        multiSlsDetectorClient(cmd, PUT, nullptr, oss);
        REQUIRE(oss.str() == cmd + "\n");
    }
    {
        std::ostringstream oss;
        REQUIRE_THROWS(
            multiSlsDetectorClient("rx_tcpport 15", PUT, nullptr, oss));
    }

    for (size_t i = 0; i != d.size(); ++i) {
        std::ostringstream oss;
        std::string cmd = std::to_string(i) + ":rx_tcpport";
        multiSlsDetectorClient(cmd, GET, nullptr, oss);
        REQUIRE(oss.str() == cmd + " " + std::to_string(port + i) + "\n");
    }

    for (size_t i = 0; i != d.size(); ++i) {
        std::ostringstream oss;
        std::string cmd =
            std::to_string(i) + ":rx_tcpport " + std::to_string(base + i);
        multiSlsDetectorClient(cmd, PUT, nullptr, oss);
        REQUIRE(oss.str() == cmd + "\n");
    }
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

TEST_CASE("r_silent", "[.cmd]") {
    auto oss = std::ostringstream{};
    multiSlsDetectorClient("r_silent 1", PUT, nullptr, oss);
    REQUIRE(oss.str() == "r_silent 1\n");

    oss = std::ostringstream{};
    multiSlsDetectorClient("r_silent", GET, nullptr, oss);
    REQUIRE(oss.str() == "r_silent 1\n");

    oss = std::ostringstream{};
    multiSlsDetectorClient("r_silent 0", PUT, nullptr, oss);
    REQUIRE(oss.str() == "r_silent 0\n");
}

// TEST_CASE("rx_jsonaddheader", "[.cmd]") {
//     auto oss = std::ostringstream{};
//     multiSlsDetectorClient("rx_jsonaddheader \"hej\":\"5\"", PUT, nullptr,
//     oss); REQUIRE(oss.str() == "rx_jsonaddheader \"hej\":\"5\"\n");

//     oss = std::ostringstream{};
//     multiSlsDetectorClient("rx_jsonaddheader", GET, nullptr, oss);
//     REQUIRE(oss.str() == "rx_jsonaddheader \"hej\":\"5\"\n");

//     oss = std::ostringstream{};
//     multiSlsDetectorClient("rx_jsonaddheader \"\"", PUT, nullptr, oss);
//     REQUIRE(oss.str() == "rx_jsonaddheader\n");
// }

// TEST_CASE("rx_udpsocksize", "[.cmd]") {
//     auto oss = std::ostringstream{};
//     multiSlsDetectorClient("rx_udpsocksize 4857600", PUT, nullptr, oss);
//     REQUIRE(oss.str() == "rx_udpsocksize 4857600\n");

//     oss = std::ostringstream{};
//     multiSlsDetectorClient("rx_udpsocksize", GET, nullptr, oss);
//     REQUIRE(oss.str() == "rx_udpsocksize 4857600\n");

//     oss = std::ostringstream{};
//     multiSlsDetectorClient("rx_udpsocksize 104857600", PUT, nullptr, oss);
//     REQUIRE(oss.str() == "rx_udpsocksize 104857600\n");
// }

TEST_CASE("r_framesperfile", "[.cmd]") {
    auto oss = std::ostringstream{};
    multiSlsDetectorClient("r_framesperfile 50", PUT, nullptr, oss);
    REQUIRE(oss.str() == "r_framesperfile 50\n");

    oss = std::ostringstream{};
    multiSlsDetectorClient("r_framesperfile", GET, nullptr, oss);
    REQUIRE(oss.str() == "r_framesperfile 50\n");

    oss = std::ostringstream{};
    multiSlsDetectorClient("r_framesperfile 10000", PUT, nullptr, oss);
    REQUIRE(oss.str() == "r_framesperfile 10000\n");
}

TEST_CASE("r_discardpolicy", "[.cmd]") {
    auto oss = std::ostringstream{};
    multiSlsDetectorClient("r_discardpolicy discardempty", PUT, nullptr, oss);
    REQUIRE(oss.str() == "r_discardpolicy discardempty\n");

    oss = std::ostringstream{};
    multiSlsDetectorClient("r_discardpolicy", GET, nullptr, oss);
    REQUIRE(oss.str() == "r_discardpolicy discardempty\n");

    oss = std::ostringstream{};
    multiSlsDetectorClient("r_discardpolicy discardpartial", PUT, nullptr, oss);
    REQUIRE(oss.str() == "r_discardpolicy discardpartial\n");

    oss = std::ostringstream{};
    multiSlsDetectorClient("r_discardpolicy nodiscard", PUT, nullptr, oss);
    REQUIRE(oss.str() == "r_discardpolicy nodiscard\n");
}

TEST_CASE("r_padding", "[.cmd]") {
    auto oss = std::ostringstream{};
    multiSlsDetectorClient("r_padding 0", PUT, nullptr, oss);
    REQUIRE(oss.str() == "r_padding 0\n");

    oss = std::ostringstream{};
    multiSlsDetectorClient("r_padding", GET, nullptr, oss);
    REQUIRE(oss.str() == "r_padding 0\n");

    oss = std::ostringstream{};
    multiSlsDetectorClient("r_padding 1", PUT, nullptr, oss);
    REQUIRE(oss.str() == "r_padding 1\n");
}
