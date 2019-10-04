#include "catch.hpp"
#include "multiSlsDetector.h"
#include "multiSlsDetectorClient.h"
#include "sls_detector_defs.h"
#include <sstream>

#include "tests/globals.h"

auto GET = slsDetectorDefs::GET_ACTION;
auto PUT = slsDetectorDefs::PUT_ACTION;

TEST_CASE("settings", "[.cmd]") {
    switch(test::type) {
        case slsDetectorDefs::EIGER:
            REQUIRE_THROWS(multiSlsDetectorClient("settings mediumgain", PUT));
            REQUIRE_NOTHROW(multiSlsDetectorClient("settings standard", PUT));
            break;
        case slsDetectorDefs::JUNGFRAU:
            REQUIRE_THROWS(multiSlsDetectorClient("settings standard", PUT));
            REQUIRE_NOTHROW(multiSlsDetectorClient("settings dynamicgain", PUT));
            REQUIRE_NOTHROW(multiSlsDetectorClient("settings dynamichg0", PUT));
            REQUIRE_NOTHROW(multiSlsDetectorClient("settings fixgain1", PUT));
            REQUIRE_NOTHROW(multiSlsDetectorClient("settings fixgain2", PUT));
            REQUIRE_NOTHROW(multiSlsDetectorClient("settings forceswitchg1", PUT));
            REQUIRE_NOTHROW(multiSlsDetectorClient("settings forceswitchg2", PUT));
        break;
        default:
            break;
    }
    REQUIRE_NOTHROW(multiSlsDetectorClient("settings", GET));
}

TEST_CASE("detsize", "[.cmd]") {
    CHECK_NOTHROW(multiSlsDetectorClient("detize", GET));
}

TEST_CASE("type", "[.cmd]") {
    CHECK_NOTHROW(multiSlsDetectorClient("type", GET));
}

TEST_CASE("firmwareversion", "[.cmd]") {
    {
        std::ostringstream oss;
        CHECK_NOTHROW(multiSlsDetectorClient("firmwareversion", GET, nullptr, oss));
    }
}
TEST_CASE("status", "[.cmd]") {

    multiSlsDetectorClient("timing auto", PUT);
    multiSlsDetectorClient("frames 10", PUT);
    multiSlsDetectorClient("period 1", PUT);

    {
        std::ostringstream oss;
        multiSlsDetectorClient("start", PUT, nullptr, oss);
        REQUIRE(oss.str() == "start successful\n");
    }
    {
        std::ostringstream oss;
        multiSlsDetectorClient("status", GET, nullptr, oss);
        REQUIRE(oss.str() == "status running\n");
    }
    {
        std::ostringstream oss;
        multiSlsDetectorClient("stop", PUT, nullptr, oss);
        REQUIRE(oss.str() == "stop successful\n");
    }
    {
        std::ostringstream oss;
        multiSlsDetectorClient("status", GET, nullptr, oss);
        REQUIRE(oss.str() == "status idle\n");
    }
}

TEST_CASE("trigger", "[.cmd][.eiger]") {
    // trigger
    {
        std::ostringstream oss;
        multiSlsDetectorClient("timing trigger", PUT, nullptr, oss);
        REQUIRE(oss.str() == "timing trigger\n");
    }
    int startingfnum = 0;
    {
        std::ostringstream oss;
        multiSlsDetectorClient("startingfnum", GET, nullptr, oss);
        std::string s = (oss.str()).erase (0, strlen("startingfnum "));
        startingfnum = std::stoi(s);
    }
    {
        std::ostringstream oss;
        multiSlsDetectorClient("start", PUT, nullptr, oss);
        REQUIRE(oss.str() == "start successful\n");
    } 
    {
        std::ostringstream oss;
        multiSlsDetectorClient("status", GET, nullptr, oss);
        REQUIRE(oss.str() == "status running\n");
    }   
    {
        std::ostringstream oss;
        multiSlsDetectorClient("trigger", PUT, nullptr, oss);
        REQUIRE(oss.str() == "trigger successful\n");
    }
    multiSlsDetectorClient("stop", PUT);
    int currentfnum = 0;
    {
        std::ostringstream oss;
        multiSlsDetectorClient("startingfnum", GET, nullptr, oss);
        std::string s = (oss.str()).erase (0, strlen("startingfnum "));
        currentfnum = std::stoi(s);
    } 
    REQUIRE((startingfnum + 1) == currentfnum);


     multiSlsDetectorClient("timing auto", PUT);
}



TEST_CASE("rx_fifodepth", "[.cmd]") {

    {
        std::ostringstream oss;
        multiSlsDetectorClient("rx_fifodepth 10", PUT, nullptr, oss);
        REQUIRE(oss.str() == "rx_fifodepth 10\n");
    }

    {
        std::ostringstream oss;
        multiSlsDetectorClient("rx_fifodepth 100", PUT, nullptr, oss);
        REQUIRE(oss.str() == "rx_fifodepth 100\n");
    }

    {
        std::ostringstream oss;
        multiSlsDetectorClient("rx_fifodepth", GET, nullptr, oss);
        REQUIRE(oss.str() == "rx_fifodepth 100\n");
    }
}

TEST_CASE("frames", "[.cmd]") {

    {
        std::ostringstream oss;
        multiSlsDetectorClient("frames 1000", PUT, nullptr, oss);
        REQUIRE(oss.str() == "frames 1000\n");
    }

    {
        std::ostringstream oss;
        multiSlsDetectorClient("frames", GET, nullptr, oss);
        REQUIRE(oss.str() == "frames 1000\n");
    }
    {
        std::ostringstream oss;
        multiSlsDetectorClient("frames 1", PUT, nullptr, oss);
        REQUIRE(oss.str() == "frames 1\n");
    }
}

TEST_CASE("rx_status", "[.cmd]") {
    {
        std::ostringstream oss;
        multiSlsDetectorClient("rx_start", PUT, nullptr, oss);
        REQUIRE(oss.str() == "rx_start successful\n");
    }
    {
        std::ostringstream oss;
        multiSlsDetectorClient("rx_status", GET, nullptr, oss);
        REQUIRE(oss.str() == "rx_status running\n");
    }
    {
        std::ostringstream oss;
        multiSlsDetectorClient("rx_stop", PUT, nullptr, oss);
        REQUIRE(oss.str() == "rx_stop successful\n");
    }
    {
        std::ostringstream oss;
        multiSlsDetectorClient("rx_status", GET, nullptr, oss);
        REQUIRE(oss.str() == "rx_status idle\n");
    }
}

TEST_CASE("fwrite", "[.cmd]") {
    {
        std::ostringstream oss;
        multiSlsDetectorClient("fwrite 1", PUT, nullptr, oss);
        REQUIRE(oss.str() == "fwrite 1\n");
    }
    {
        std::ostringstream oss;
        multiSlsDetectorClient("fwrite", GET, nullptr, oss);
        REQUIRE(oss.str() == "fwrite 1\n");
    }
    {
        std::ostringstream oss;
        multiSlsDetectorClient("fwrite 0", PUT, nullptr, oss);
        REQUIRE(oss.str() == "fwrite 0\n");
    }
}

TEST_CASE("enablefoverwrite", "[.cmd]") {
    {
        std::ostringstream oss;
        multiSlsDetectorClient("foverwrite 1", PUT, nullptr, oss);
        REQUIRE(oss.str() == "foverwrite 1\n");
    }
    {
        std::ostringstream oss;
        multiSlsDetectorClient("foverwrite", GET, nullptr, oss);
        REQUIRE(oss.str() == "foverwrite 1\n");
    }
    {
        std::ostringstream oss;
        multiSlsDetectorClient("foverwrite 0", PUT, nullptr, oss);
        REQUIRE(oss.str() == "foverwrite 0\n");
    }
}

// EIGER ONLY
// TEST_CASE("activatecmd", "[.cmd]") {

//     {
//         // TODO! read padding from somewhere
//         std::ostringstream oss;
//         multiSlsDetectorClient("activate 0", PUT, nullptr, oss);
//         REQUIRE(oss.str() == "activate 0 padding\n");
//     }
//     {
//         std::ostringstream oss;
//         multiSlsDetectorClient("activate", GET, nullptr, oss);
//         REQUIRE(oss.str() == "activate 0 padding\n");
//     }
//     {
//         std::ostringstream oss;
//         multiSlsDetectorClient("activate 1", PUT, nullptr, oss);
//         REQUIRE(oss.str() == "activate 1 padding\n");
//     }
// }

TEST_CASE("fmaster", "[.cmd]") {
    {
        std::ostringstream oss;
        multiSlsDetectorClient("fmaster 0", PUT, nullptr, oss);
        REQUIRE(oss.str() == "fmaster 0\n");
    }
    {
        std::ostringstream oss;
        multiSlsDetectorClient("fmaster", GET, nullptr, oss);
        REQUIRE(oss.str() == "fmaster 0\n");
    }
    {
        std::ostringstream oss;
        multiSlsDetectorClient("fmaster 1", PUT, nullptr, oss);
        REQUIRE(oss.str() == "fmaster 1\n");
    }
}

TEST_CASE("findex", "[.cmd]") {
    {
        std::ostringstream oss;
        multiSlsDetectorClient("findex 57", PUT, nullptr, oss);
        REQUIRE(oss.str() == "findex 57\n");
    }
    {
        std::ostringstream oss;
        multiSlsDetectorClient("findex", GET, nullptr, oss);
        REQUIRE(oss.str() == "findex 57\n");
    }
    {

        std::ostringstream oss;
        multiSlsDetectorClient("findex 0", PUT, nullptr, oss);
        REQUIRE(oss.str() == "findex 0\n");
    }
}

TEST_CASE("rx_tcpport", "[.cmd]") {
    multiSlsDetector d;
    int port = 3500;
    int base = 1954;
    for (size_t i = 0; i != d.size(); ++i) {
        std::ostringstream oss;
        std::string cmd =
            std::to_string(i) + ":rx_tcpport " + std::to_string(port + i);
        std::cout << cmd << "\n";
        std::string cmd2 = cmd.erase(0,2);
        multiSlsDetectorClient(cmd, PUT, nullptr, oss);
        REQUIRE(oss.str() == cmd2 + "\n");
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

    {
        std::ostringstream oss;
        multiSlsDetectorClient("fname somename", PUT, nullptr, oss);
        REQUIRE(oss.str() == "fname somename\n");
    }
    {
        std::ostringstream oss;
        multiSlsDetectorClient("fname", GET, nullptr, oss);
        REQUIRE(oss.str() == "fname somename\n");
    }
    {
        std::ostringstream oss;
        multiSlsDetectorClient("fname run", PUT, nullptr, oss);
        REQUIRE(oss.str() == "fname run\n");
    }
}

TEST_CASE("resetframescaught get framescaught", "[.cmd]") {
    {
        std::ostringstream oss;
        multiSlsDetectorClient("resetframescaught 0", PUT, nullptr, oss);
        REQUIRE(oss.str() == "resetframescaught successful\n");
    }
    {
        std::ostringstream oss;
        multiSlsDetectorClient("framescaught", GET, nullptr, oss);
        REQUIRE(oss.str() == "framescaught 0\n");
    }
}

TEST_CASE("rx_silent", "[.cmd]") {
    {
        std::ostringstream oss;
        multiSlsDetectorClient("rx_silent 1", PUT, nullptr, oss);
        REQUIRE(oss.str() == "rx_silent 1\n");
    }
    {
        std::ostringstream oss;
        multiSlsDetectorClient("rx_silent", GET, nullptr, oss);
        REQUIRE(oss.str() == "rx_silent 1\n");
    }
    {
        std::ostringstream oss;
        multiSlsDetectorClient("rx_silent 0", PUT, nullptr, oss);
        REQUIRE(oss.str() == "rx_silent 0\n");
    }
}

// TEST_CASE("rx_jsonaddheader", "[.cmd]") {
//     std::ostringstream oss;
//     multiSlsDetectorClient("rx_jsonaddheader \"hej\":\"5\"", PUT, nullptr,
//     oss); REQUIRE(oss.str() == "rx_jsonaddheader \"hej\":\"5\"\n");

//     std::ostringstream oss;
//     multiSlsDetectorClient("rx_jsonaddheader", GET, nullptr, oss);
//     REQUIRE(oss.str() == "rx_jsonaddheader \"hej\":\"5\"\n");

//     std::ostringstream oss;
//     multiSlsDetectorClient("rx_jsonaddheader \"\"", PUT, nullptr, oss);
//     REQUIRE(oss.str() == "rx_jsonaddheader\n");
// }

// TEST_CASE("rx_udpsocksize", "[.cmd]") {
//     std::ostringstream oss;
//     multiSlsDetectorClient("rx_udpsocksize 4857600", PUT, nullptr, oss);
//     REQUIRE(oss.str() == "rx_udpsocksize 4857600\n");

//     std::ostringstream oss;
//     multiSlsDetectorClient("rx_udpsocksize", GET, nullptr, oss);
//     REQUIRE(oss.str() == "rx_udpsocksize 4857600\n");

//     std::ostringstream oss;
//     multiSlsDetectorClient("rx_udpsocksize 104857600", PUT, nullptr, oss);
//     REQUIRE(oss.str() == "rx_udpsocksize 104857600\n");
// }

TEST_CASE("rx_framesperfile", "[.cmd]") {
    {
        std::ostringstream oss;
        multiSlsDetectorClient("rx_framesperfile 50", PUT, nullptr, oss);
        REQUIRE(oss.str() == "rx_framesperfile 50\n");
    }
    {
        std::ostringstream oss;
        multiSlsDetectorClient("rx_framesperfile", GET, nullptr, oss);
        REQUIRE(oss.str() == "rx_framesperfile 50\n");
    }
    {
        std::ostringstream oss;
        multiSlsDetectorClient("rx_framesperfile 10000", PUT, nullptr, oss);
        REQUIRE(oss.str() == "rx_framesperfile 10000\n");
    }
}

TEST_CASE("rx_discardpolicy", "[.cmd]") {
    {
        std::ostringstream oss;
        multiSlsDetectorClient("rx_discardpolicy discardempty", PUT, nullptr,
                               oss);
        REQUIRE(oss.str() == "rx_discardpolicy discardempty\n");
    }
    {
        std::ostringstream oss;
        multiSlsDetectorClient("rx_discardpolicy", GET, nullptr, oss);
        REQUIRE(oss.str() == "rx_discardpolicy discardempty\n");
    }
    {
        std::ostringstream oss;
        multiSlsDetectorClient("rx_discardpolicy discardpartial", PUT, nullptr,
                               oss);
        REQUIRE(oss.str() == "rx_discardpolicy discardpartial\n");
    }
    {
        std::ostringstream oss;
        multiSlsDetectorClient("rx_discardpolicy nodiscard", PUT, nullptr, oss);
        REQUIRE(oss.str() == "rx_discardpolicy nodiscard\n");
    }
}

TEST_CASE("rx_padding", "[.cmd]") {
    {
        std::ostringstream oss;
        multiSlsDetectorClient("rx_padding 0", PUT, nullptr, oss);
        REQUIRE(oss.str() == "rx_padding 0\n");
    }
    {
        std::ostringstream oss;
        multiSlsDetectorClient("rx_padding", GET, nullptr, oss);
        REQUIRE(oss.str() == "rx_padding 0\n");
    }
    {
        std::ostringstream oss;
        multiSlsDetectorClient("rx_padding 1", PUT, nullptr, oss);
        REQUIRE(oss.str() == "rx_padding 1\n");
    }
}

TEST_CASE("rx_readfreq", "[.cmd]") {
    {
        std::ostringstream oss;
        multiSlsDetectorClient("rx_readfreq 1", PUT, nullptr, oss);
        REQUIRE(oss.str() == "rx_readfreq 1\n");
    }
    {
        std::ostringstream oss;
        multiSlsDetectorClient("rx_readfreq", GET, nullptr, oss);
        REQUIRE(oss.str() == "rx_readfreq 1\n");
    }
    {
        std::ostringstream oss;
        multiSlsDetectorClient("rx_readfreq 0", PUT, nullptr, oss);
        REQUIRE(oss.str() == "rx_readfreq 0\n");
    }
}

TEST_CASE("rx_lock", "[.cmd]") {
    {
        std::ostringstream oss;
        multiSlsDetectorClient("rx_lock 1", PUT, nullptr, oss);
        REQUIRE(oss.str() == "rx_lock 1\n");
    }
    {
        std::ostringstream oss;
        multiSlsDetectorClient("rx_lock", GET, nullptr, oss);
        REQUIRE(oss.str() == "rx_lock 1\n");
    }
    {
        std::ostringstream oss;
        multiSlsDetectorClient("rx_lock 0", PUT, nullptr, oss);
        REQUIRE(oss.str() == "rx_lock 0\n");
    }
}

TEST_CASE("lock", "[.cmd]") {
    {
        std::ostringstream oss;
        multiSlsDetectorClient("lock 1", PUT, nullptr, oss);
        REQUIRE(oss.str() == "lock 1\n");
    }
    {
        std::ostringstream oss;
        multiSlsDetectorClient("lock", GET, nullptr, oss);
        REQUIRE(oss.str() == "lock 1\n");
    }
    {
        std::ostringstream oss;
        multiSlsDetectorClient("lock 0", PUT, nullptr, oss);
        REQUIRE(oss.str() == "lock 0\n");
    }
}


TEST_CASE("rx_lastclient", "[.cmd]") {

    std::ostringstream oss;
    multiSlsDetectorClient("rx_lastclient", GET, nullptr, oss);
    REQUIRE(oss.str() == "rx_lastclient " + test::my_ip + "\n");
}



TEST_CASE("rx_checkversion", "[.cmd]") {

    std::ostringstream oss;
    multiSlsDetectorClient("rx_checkversion", GET, nullptr, oss);
    REQUIRE(oss.str() == "rx_checkversion compatible\n");
}

TEST_CASE("exptime", "[.cmd]") {
    {
        std::ostringstream oss;
        multiSlsDetectorClient("exptime 0.05", PUT, nullptr, oss);
        REQUIRE(oss.str() == "exptime 0.05\n");
    }
    {
        std::ostringstream oss;
        multiSlsDetectorClient("exptime", GET, nullptr, oss);
        REQUIRE(oss.str() == "exptime 50ms\n");
    }
    {
        std::ostringstream oss;
        multiSlsDetectorClient("exptime 1", PUT, nullptr, oss);
        REQUIRE(oss.str() == "exptime 1\n");
    }
}


TEST_CASE("period", "[.cmd]") {
    {
        std::ostringstream oss;
        multiSlsDetectorClient("period 1.25s", PUT, nullptr, oss);
        REQUIRE(oss.str() == "period 1.25s\n");
    }
    {
        std::ostringstream oss;
        multiSlsDetectorClient("period", GET, nullptr, oss);
        REQUIRE(oss.str() == "period 1.25s\n");
    }
    {
        std::ostringstream oss;
        multiSlsDetectorClient("period 0", PUT, nullptr, oss);
        REQUIRE(oss.str() == "period 0\n");
    }
}