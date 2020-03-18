#include "CmdProxy.h"
#include "Detector.h"
#include "catch.hpp"
#include "sls_detector_defs.h"
#include <sstream>

#include "tests/globals.h"
#include "versionAPI.h"

using sls::CmdProxy;
using sls::Detector;
using test::GET;
using test::PUT;

/*
This file should contain receiver specific tests use
python/scripts/list_tested_cmd.py to check if all commands are covered

*/

// TEST_CASE("A test", "[.hey]"){
//     std::cout << "start\n";
//     SECTION("a section"){
//         std::cout << "s1\n";
//     }
//     SECTION("another section"){
//         std::cout << "s2\n";
//     }
//     std::cout << "end\n";
// }

TEST_CASE("rx_hostname", "[.cmd][.rx]") {
    // TODO! find a proper way to test, now we read out the rx_hostname
    // and then put it to see that we don't crash
    Detector det;
    CmdProxy proxy(&det);
    std::string hostname =
        det.getRxHostname().tsquash("hostname must be same for test");

    auto det_type = det.getDetectorType().squash();
    sls::Result<sls::ns> time;
    if (det_type == defs::EIGER) {
        time = det.getSubDeadTime();
    }

    {
        // disable receiver
        std::ostringstream oss1, oss2;
        proxy.Call("rx_hostname", {"none"}, -1, PUT, oss1);
        REQUIRE(oss1.str() == "rx_hostname none\n");
        proxy.Call("rx_hostname", {}, -1, GET, oss2);
        REQUIRE(oss2.str() == "rx_hostname none\n");
        // receiver should be disabled
        REQUIRE(det.getUseReceiverFlag().tsquash(
                    "different values of flag in test") == false);
    }
    {
        // put back the old hostname
        std::ostringstream oss1, oss2;
        proxy.Call("rx_hostname", {hostname}, -1, PUT, oss1);
        REQUIRE(oss1.str() == "rx_hostname " + hostname + "\n");
        proxy.Call("rx_hostname", {}, -1, GET, oss2);
        REQUIRE(oss2.str() == "rx_hostname " + hostname + "\n");
    }

    //Bug rx_hostname could change subdeadtime
    if (det_type == defs::EIGER) {
        auto time2 = det.getSubDeadTime();
        for (int i = 0; i != det.size(); ++i) {
            REQUIRE(time[i].count() == time2[i].count());
        }
    }
}

TEST_CASE("rx_framescaught", "[.cmd][.rx]") {
    Detector det;
    CmdProxy proxy(&det);

    // This ensures 0 caught frames
    det.startReceiver();
    det.stopReceiver();

    {
        std::ostringstream oss;
        proxy.Call("rx_framescaught", {}, -1, GET, oss);
        REQUIRE(oss.str() == "rx_framescaught 0\n");
    }

    // Currently disabled may activate if we have a stable env
    // Now take one frame and see that we caught it
    // det.setNumberOfFrames(1);
    // det.acquire();
    // {
    //     std::ostringstream oss;
    //     proxy.Call("rx_framescaught", {}, -1, GET, oss);
    //     REQUIRE(oss.str() == "rx_framescaught 1\n");
    // }
}

TEST_CASE("rx_status", "[.cmd][.rx]") {
    Detector det;
    CmdProxy proxy(&det);
    std::ostringstream oss;
    proxy.Call("rx_status", {}, -1, GET, oss);
    REQUIRE(oss.str() == "rx_status idle\n");
}

TEST_CASE("rx_version", "[.cmd][.rx]") {
    Detector det;
    CmdProxy proxy(&det);
    std::ostringstream oss;
    proxy.Call("rx_version", {}, -1, GET, oss);
    std::ostringstream vs;
    vs << "rx_version 0x" << std::hex << APIRECEIVER << '\n';
    REQUIRE(oss.str() == vs.str());
}

TEST_CASE("rx_start", "[.cmd][.rx]") {
    Detector det;
    CmdProxy proxy(&det);
    // PUT only command
    REQUIRE_THROWS(proxy.Call("rx_start", {}, -1, GET));
    {
        std::ostringstream oss;
        proxy.Call("rx_start", {}, -1, PUT, oss);
        REQUIRE(oss.str() == "rx_start successful\n");
    }
    {
        std::ostringstream oss;
        proxy.Call("rx_status", {}, -1, GET, oss);
        REQUIRE(oss.str() == "rx_status running\n");
    }
}

TEST_CASE("rx_stop", "[.cmd][.rx]") {
    Detector det;
    CmdProxy proxy(&det);
    // PUT only command
    REQUIRE_THROWS(proxy.Call("rx_stop", {}, -1, GET));
    {
        std::ostringstream oss;
        proxy.Call("rx_stop", {}, -1, PUT, oss);
        REQUIRE(oss.str() == "rx_stop successful\n");
    }
    {
        std::ostringstream oss;
        proxy.Call("rx_status", {}, -1, GET, oss);
        REQUIRE(oss.str() == "rx_status idle\n");
    }
}

TEST_CASE("rx_missingpackets", "[.cmd][.rx]") {
    // TODO! This only tests for no crash how can we test
    // for correct values?
    Detector det;
    CmdProxy proxy(&det);
    proxy.Call("rx_missingpackets", {}, -1, GET);
}

TEST_CASE("rx_frameindex", "[.cmd][.rx]") {
    Detector det;
    CmdProxy proxy(&det);
    proxy.Call("rx_frameindex", {}, -1, GET);

    // This is a get only command
    REQUIRE_THROWS(proxy.Call("rx_frameindex", {"2"}, -1, PUT));
}

TEST_CASE("rx_lastclient", "[.cmd][.rx]") {
    Detector det;
    CmdProxy proxy(&det);
    std::ostringstream oss;
    proxy.Call("rx_lastclient", {}, -1, GET, oss);
    REQUIRE(oss.str() == "rx_lastclient " + test::my_ip + "\n");
}

TEST_CASE("rx_printconfig", "[.cmd][.rx]") {
    Detector det;
    CmdProxy proxy(&det);
    proxy.Call("rx_printconfig", {}, -1, GET);
}

TEST_CASE("rx_fifodepth", "[.cmd][.rx]") {
    Detector det;
    CmdProxy proxy(&det);
    int prev_val = det.getRxFifoDepth().squash();
    {
        std::ostringstream oss;
        proxy.Call("rx_fifodepth", {"10"}, -1, PUT, oss);
        REQUIRE(oss.str() == "rx_fifodepth 10\n");
    }
    {
        std::ostringstream oss;
        proxy.Call("rx_fifodepth", {"100"}, -1, PUT, oss);
        REQUIRE(oss.str() == "rx_fifodepth 100\n");
    }

    {
        std::ostringstream oss;
        proxy.Call("rx_fifodepth", {}, -1, GET, oss);
        REQUIRE(oss.str() == "rx_fifodepth 100\n");
    }
    det.setRxFifoDepth(prev_val);
}

TEST_CASE("rx_silent", "[.cmd][.rx]") {
    Detector det;
    CmdProxy proxy(&det);
    {
        std::ostringstream oss;
        proxy.Call("rx_silent", {"1"}, -1, PUT, oss);
        REQUIRE(oss.str() == "rx_silent 1\n");
    }
    {
        std::ostringstream oss;
        proxy.Call("rx_silent", {}, -1, GET, oss);
        REQUIRE(oss.str() == "rx_silent 1\n");
    }
    {
        std::ostringstream oss;
        proxy.Call("rx_silent", {"0"}, -1, PUT, oss);
        REQUIRE(oss.str() == "rx_silent 0\n");
    }
}

TEST_CASE("rx_jsonaddheader", "[.cmd][.rx]") {
    Detector det;
    CmdProxy proxy(&det);

    {
        std::ostringstream oss;
        proxy.Call("rx_jsonaddheader", {"\"hej\":\"5\""}, -1, PUT, oss);
        REQUIRE(oss.str() == "rx_jsonaddheader \"hej\":\"5\"\n");
    }
    {
        std::ostringstream oss;
        proxy.Call("rx_jsonaddheader", {}, -1, GET, oss);
        REQUIRE(oss.str() == "rx_jsonaddheader \"hej\":\"5\"\n");
    }
    {
        std::ostringstream oss;
        proxy.Call("rx_jsonaddheader", {"\"\""}, -1, PUT, oss);
        REQUIRE(oss.str() == "rx_jsonaddheader \"\"\n");
    }
}

TEST_CASE("rx_udpsocksize", "[.cmd][.rx]") {
    Detector det;
    CmdProxy proxy(&det);
    {
        std::ostringstream oss;
        proxy.Call("rx_udpsocksize", {"4857600"}, -1, PUT, oss);
        REQUIRE(oss.str() >= "rx_udpsocksize 4857600\n");
    }
    {
        std::ostringstream oss;
        proxy.Call("rx_udpsocksize", {}, -1, GET, oss);
        REQUIRE(oss.str() >= "rx_udpsocksize 4857600\n");
    }
}

TEST_CASE("rx_realudpsocksize", "[.cmd][.rx]") {
    // TODO! Is the real socket size always twice?
    Detector det;
    CmdProxy proxy(&det);
    uint64_t val = 0;
    {
        std::ostringstream oss;
        proxy.Call("rx_udpsocksize", {}, -1, GET, oss);
        std::string s = (oss.str()).erase(0, strlen("rx_udpsocksize "));
        val = std::stol(s);
    }
    {
        std::ostringstream oss;
        proxy.Call("rx_realudpsocksize", {}, -1, GET, oss);
        std::string s = (oss.str()).erase(0, strlen("rx_realudpsocksize "));
        uint64_t rval = std::stol(s);
        REQUIRE(rval >= val * 2);
    }
}

TEST_CASE("rx_framesperfile", "[.cmd][.rx]") {
    Detector det;
    CmdProxy proxy(&det);
    {
        std::ostringstream oss;
        proxy.Call("rx_framesperfile", {"50"}, -1, PUT, oss);
        REQUIRE(oss.str() == "rx_framesperfile 50\n");
    }
    {
        std::ostringstream oss;
        proxy.Call("rx_framesperfile", {}, -1, GET, oss);
        REQUIRE(oss.str() == "rx_framesperfile 50\n");
    }
    {
        std::ostringstream oss;
        proxy.Call("rx_framesperfile", {"10000"}, -1, PUT, oss);
        REQUIRE(oss.str() == "rx_framesperfile 10000\n");
    }
}

TEST_CASE("rx_discardpolicy", "[.cmd][.rx]") {
    Detector det;
    CmdProxy proxy(&det);
    {
        std::ostringstream oss;
        proxy.Call("rx_discardpolicy", {"discardempty"}, -1, PUT, oss);
        REQUIRE(oss.str() == "rx_discardpolicy discardempty\n");
    }
    {
        std::ostringstream oss;
        proxy.Call("rx_discardpolicy", {}, -1, GET, oss);
        REQUIRE(oss.str() == "rx_discardpolicy discardempty\n");
    }
    {
        std::ostringstream oss;
        proxy.Call("rx_discardpolicy", {"discardpartial"}, -1, PUT, oss);
        REQUIRE(oss.str() == "rx_discardpolicy discardpartial\n");
    }
    {
        std::ostringstream oss;
        proxy.Call("rx_discardpolicy", {"nodiscard"}, -1, PUT, oss);
        REQUIRE(oss.str() == "rx_discardpolicy nodiscard\n");
    }
}

TEST_CASE("rx_padding", "[.cmd][.rx]") {
    Detector det;
    CmdProxy proxy(&det);
    {
        std::ostringstream oss;
        proxy.Call("rx_padding", {"0"}, -1, PUT, oss);
        REQUIRE(oss.str() == "rx_padding 0\n");
    }
    {
        std::ostringstream oss;
        proxy.Call("rx_padding", {}, -1, GET, oss);
        REQUIRE(oss.str() == "rx_padding 0\n");
    }
    {
        std::ostringstream oss;
        proxy.Call("rx_padding", {"1"}, -1, PUT, oss);
        REQUIRE(oss.str() == "rx_padding 1\n");
    }
}

TEST_CASE("rx_readfreq", "[.cmd][.rx]") {
    Detector det;
    CmdProxy proxy(&det);
    {
        std::ostringstream oss;
        proxy.Call("rx_readfreq", {"1"}, -1, PUT, oss);
        REQUIRE(oss.str() == "rx_readfreq 1\n");
    }
    {
        std::ostringstream oss;
        proxy.Call("rx_readfreq", {}, -1, GET, oss);
        REQUIRE(oss.str() == "rx_readfreq 1\n");
    }
    {
        std::ostringstream oss;
        proxy.Call("rx_readfreq", {"0"}, -1, PUT, oss);
        REQUIRE(oss.str() == "rx_readfreq 0\n");
    }
}

TEST_CASE("rx_lock", "[.cmd][.rx]") {
    Detector det;
    CmdProxy proxy(&det);
    {
        std::ostringstream oss;
        proxy.Call("rx_lock", {"1"}, -1, PUT, oss);
        REQUIRE(oss.str() == "rx_lock 1\n");
    }
    {
        std::ostringstream oss;
        proxy.Call("rx_lock", {}, -1, GET, oss);
        REQUIRE(oss.str() == "rx_lock 1\n");
    }
    {
        std::ostringstream oss;
        proxy.Call("rx_lock", {"0"}, -1, PUT, oss);
        REQUIRE(oss.str() == "rx_lock 0\n");
    }
}

TEST_CASE("rx_zmqport", "[.cmd][.rx]") {
    Detector det;
    CmdProxy proxy(&det);
    int socketsperdetector = 1;
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::EIGER) {
        socketsperdetector *= 2;
    } else if (det_type == defs::JUNGFRAU) {
        proxy.Call("numinterfaces", {"2"}, -1, PUT);
        socketsperdetector *= 2;
    }
    int port = 3500;
    proxy.Call("rx_zmqport", {std::to_string(port)}, -1, PUT);
    for (int i = 0; i != det.size(); ++i) {
        std::ostringstream oss;
        proxy.Call("rx_zmqport", {}, i, GET, oss);
        std::cout << "oss: " << oss.str() << "\n";
        REQUIRE(oss.str() == "rx_zmqport " +
                                 std::to_string(port + i * socketsperdetector) +
                                 '\n');
    }
    port = 30001;
    proxy.Call("rx_zmqport", {std::to_string(port)}, -1, PUT);
    for (int i = 0; i != det.size(); ++i) {
        std::ostringstream oss;
        proxy.Call("rx_zmqport", {}, i, GET, oss);
        REQUIRE(oss.str() == "rx_zmqport " +
                                 std::to_string(port + i * socketsperdetector) +
                                 '\n');
    }
    if (det_type == slsDetectorDefs::JUNGFRAU) {
        proxy.Call("numinterfaces", {"1"}, -1, PUT);
    }
}

TEST_CASE("rx_datastream", "[.cmd][.rx]") {
    Detector det;
    CmdProxy proxy(&det);
    {
        std::ostringstream oss;
        proxy.Call("rx_datastream", {"1"}, -1, PUT, oss);
        REQUIRE(oss.str() == "rx_datastream 1\n");
        REQUIRE(det.getRxZmqDataStream().squash() == true);
    }
    {
        std::ostringstream oss;
        proxy.Call("rx_datastream", {}, -1, GET, oss);
        REQUIRE(oss.str() == "rx_datastream 1\n");
    }
    {
        std::ostringstream oss;
        proxy.Call("rx_datastream", {"0"}, -1, PUT, oss);
        REQUIRE(oss.str() == "rx_datastream 0\n");
        REQUIRE(det.getRxZmqDataStream().squash() == false);
    }
}

TEST_CASE("rx_tcpport", "[.cmd][.rx]") {
    Detector det;
    CmdProxy proxy(&det);

    int port = 3500;

    proxy.Call("rx_tcpport", {std::to_string(port)}, -1, PUT);
    for (int i = 0; i != det.size(); ++i) {
        std::ostringstream oss;
        proxy.Call("rx_tcpport", {}, i, GET, oss);
        REQUIRE(oss.str() == "rx_tcpport " + std::to_string(port + i) + '\n');
    }
    REQUIRE_THROWS(proxy.Call("rx_tcpport", {"15"}, -1, PUT));
    port = 1954;
    proxy.Call("rx_tcpport", {std::to_string(port)}, -1, PUT);
    for (int i = 0; i != det.size(); ++i) {
        std::ostringstream oss;
        proxy.Call("rx_tcpport", {}, i, GET, oss);
        REQUIRE(oss.str() == "rx_tcpport " + std::to_string(port + i) + '\n');
    }
}

TEST_CASE("rx_zmqip", "[.cmd][.rx]") {
    Detector det;
    CmdProxy proxy(&det);
    {
        std::ostringstream oss;
        proxy.Call("rx_zmqip", {"127.0.0.1"}, 0, PUT, oss);
        REQUIRE(oss.str() == "rx_zmqip 127.0.0.1\n");
        std::cout << "ZMQIP: " << det.getRxZmqIP() << '\n';
    }
    {
        std::ostringstream oss;
        proxy.Call("rx_zmqip", {}, 0, GET, oss);
        REQUIRE(oss.str() == "rx_zmqip 127.0.0.1\n");
    }
}

TEST_CASE("rx_dbitoffset", "[.cmd][.rx]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::CHIPTESTBOARD) {
        auto previous = det.getRxDbitOffset();
        {
            std::ostringstream oss;
            proxy.Call("rx_dbitoffset", {"1"}, -1, PUT, oss);
            REQUIRE(oss.str() == "rx_dbitoffset 1\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("rx_dbitoffset", {"0"}, -1, PUT, oss);
            REQUIRE(oss.str() == "rx_dbitoffset 0\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("rx_dbitoffset", {"15"}, -1, PUT, oss);
            REQUIRE(oss.str() == "rx_dbitoffset 15\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("rx_dbitoffset", {}, -1, GET, oss);
            REQUIRE(oss.str() == "rx_dbitoffset 15\n");
        }
        // Reset to previous value
        for (int i = 0; i != det.size(); ++i) {
            det.setRxDbitOffset(previous[i], {i});
        }
    } else {
        REQUIRE_THROWS(proxy.Call("rx_dbitoffset", {}, -1, GET));
    }
}



TEST_CASE("rx_dbitlist", "[.cmd][.rx]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::CHIPTESTBOARD) {
        auto previous = det.getRxDbitList();
        {
            std::ostringstream oss;
            proxy.Call("rx_dbitlist", {"0", "4", "5", "8", "9", "10", "52", "63"}, -1, PUT, oss);
            REQUIRE(oss.str() == "rx_dbitlist [0, 4, 5, 8, 9, 10, 52, 63]\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("rx_dbitlist", {}, -1, GET, oss);
            REQUIRE(oss.str() == "rx_dbitlist [0, 4, 5, 8, 9, 10, 52, 63]\n");
        }
        REQUIRE_THROWS(proxy.Call("rx_dbitlist", {"67"}, -1, PUT));
        REQUIRE_THROWS(proxy.Call("rx_dbitlist", {"-1"}, -1, PUT));
        REQUIRE_NOTHROW(proxy.Call("rx_dbitlist", {"all"}, -1, PUT));
        // Reset to previous value
        for (int i = 0; i != det.size(); ++i) {
            det.setRxDbitList(previous[i], {i});
        }
    } else {
        REQUIRE_THROWS(proxy.Call("rx_dbitlist", {}, -1, GET));
    }
}