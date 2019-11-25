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
This file should contain receiver specific tests
25/11/2019 we have:
rx_datastream
rx_dbitlist
rx_dbitoffset
rx_discardpolicy
rx_fifodepth
rx_frameindex
rx_framescaught
rx_framesperfile
rx_hostname
rx_jsonaddheader
rx_jsonpara
rx_lastclient
rx_lock
rx_missingpackets
rx_padding
rx_printconfig
rx_readfreq
rx_realudpsocksize
rx_silent
rx_start
rx_status
rx_stop
rx_tcpport
rx_udpsocksize
rx_version
rx_zmqip
rx_zmqport

*/

TEST_CASE("rx_version", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    std::ostringstream oss;
    proxy.Call("rx_version", {}, -1, GET, oss);
    std::ostringstream vs;
    vs << "rx_version 0x" << std::hex << APIRECEIVER << '\n';
    REQUIRE(oss.str() == vs.str());
}

TEST_CASE("rx_start", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    //PUT only command
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

TEST_CASE("rx_stop", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    //PUT only command
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

TEST_CASE("rx_missingpackets", "[.cmd]") {
    // TODO! This only tests for no crash how can we test
    // for correct values?
    Detector det;
    CmdProxy proxy(&det);
    proxy.Call("rx_missingpackets", {}, -1, GET);
}

TEST_CASE("rx_frameindex", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    proxy.Call("rx_frameindex", {}, -1, GET);

    // This is a get only command
    REQUIRE_THROWS(proxy.Call("rx_frameindex", {"2"}, -1, PUT));
}

TEST_CASE("rx_lastclient", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    std::ostringstream oss;
    proxy.Call("rx_lastclient", {}, -1, GET, oss);
    REQUIRE(oss.str() == "rx_lastclient " + test::my_ip + "\n");
}

TEST_CASE("rx_printconfig", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    proxy.Call("rx_printconfig", {}, -1, GET);
}

TEST_CASE("rx_fifodepth", "[.cmd]") {
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

TEST_CASE("rx_status", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
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

TEST_CASE("rx_silent", "[.cmd]") {
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

TEST_CASE("rx_jsonaddheader", "[.cmd]") {
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

TEST_CASE("rx_udpsocksize", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    {
        std::ostringstream oss;
        proxy.Call("rx_udpsocksize", {"4857600"}, -1, PUT, oss);
        REQUIRE(oss.str() == "rx_udpsocksize 4857600\n");
    }
    {
        std::ostringstream oss;
        proxy.Call("rx_udpsocksize", {}, -1, GET, oss);
        REQUIRE(oss.str() == "rx_udpsocksize 4857600\n");
    }
}

TEST_CASE("rx_realudpsocksize", "[.cmd]") {
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
        REQUIRE(rval == val * 2);
    }
}

TEST_CASE("rx_framesperfile", "[.cmd]") {
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

TEST_CASE("rx_discardpolicy", "[.cmd]") {
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

TEST_CASE("rx_padding", "[.cmd]") {
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

TEST_CASE("rx_readfreq", "[.cmd]") {
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

TEST_CASE("rx_lock", "[.cmd]") {
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

// TEST_CASE("rx_zmqport", "[.cmd]") {
//     multiSlsDetector d;
//     int socketsperdetector = 1;
//     if (test::type == slsDetectorDefs::EIGER) {
//         socketsperdetector *= 2;
//     } else if (test::type == slsDetectorDefs::JUNGFRAU) {
//         REQUIRE_NOTHROW(multiSlsDetectorClient("numinterfaces 2", PUT));
//         socketsperdetector *= 2;
//     }
//     int port = 3500;
//     REQUIRE_NOTHROW(multiSlsDetectorClient("rx_zmqport " +
//     std::to_string(port), PUT)); for (int i = 0; i != d.size(); ++i) {
//         std::ostringstream oss;
//         REQUIRE_NOTHROW(multiSlsDetectorClient(std::to_string(i) +
//         ":rx_zmqport", GET, nullptr, oss)); REQUIRE(oss.str() == "rx_zmqport
//         " + std::to_string(port + i * socketsperdetector) + '\n');
//     }
//     port = 30001;
//     REQUIRE_NOTHROW(multiSlsDetectorClient("rx_zmqport " +
//     std::to_string(port), PUT)); for (int i = 0; i != d.size(); ++i) {
//         std::ostringstream oss;
//         REQUIRE_NOTHROW(multiSlsDetectorClient(std::to_string(i) +
//         ":rx_zmqport", GET, nullptr, oss)); REQUIRE(oss.str() == "rx_zmqport
//         " + std::to_string(port + i * socketsperdetector) + '\n');
//     }
//     if (test::type == slsDetectorDefs::JUNGFRAU) {
//         REQUIRE_NOTHROW(multiSlsDetectorClient("numinterfaces 1", PUT));
//     }
// }

TEST_CASE("rx_datastream", "[.cmd]") {
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

TEST_CASE("rx_tcpport", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);

    int port = 3500;

    proxy.Call("rx_tcpport", {std::to_string(port)}, -1, PUT);
    for (int i = 0; i != det.size(); ++i) {
        std::ostringstream oss;
        proxy.Call("rx_tcpport", {}, -1, GET, oss);
        REQUIRE(oss.str() == "rx_tcpport " + std::to_string(port + i) + '\n');
    }
    REQUIRE_THROWS(proxy.Call("rx_tcpport", {"15"}, -1, PUT));
    port = 1954;
    proxy.Call("rx_tcpport", {std::to_string(port)}, -1, PUT);
    for (int i = 0; i != det.size(); ++i) {
        std::ostringstream oss;
        proxy.Call("rx_tcpport", {}, -1, GET, oss);
        REQUIRE(oss.str() == "rx_tcpport " + std::to_string(port + i) + '\n');
    }
}

// TEST_CASE("rx_zmqip", "[.cmd]") {
//     std::string s;
//     {
//         std::ostringstream oss;
//         REQUIRE_NOTHROW(multiSlsDetectorClient("0:rx_zmqip", GET, nullptr,
//         oss)); s = oss.str();
//     }
//     {
//         REQUIRE_NOTHROW(multiSlsDetectorClient(s, PUT));
//         std::ostringstream oss;
//         REQUIRE_NOTHROW(multiSlsDetectorClient("0:rx_zmqip", GET, nullptr,
//         oss)); REQUIRE(oss.str() == s);
//     }
// }

// TEST_CASE("burstmode", "[.cmd][.gotthard2]") {
//     if (test::type == slsDetectorDefs::GOTTHARD2) {
//         REQUIRE_NOTHROW(multiSlsDetectorClient("burstmode 0", PUT));
//         REQUIRE_NOTHROW(multiSlsDetectorClient("burstmode 1", PUT));
//         REQUIRE_NOTHROW(multiSlsDetectorClient("burstmode", GET));
//     } else {
//         REQUIRE_THROWS(multiSlsDetectorClient("burstmod", GET));
//     }
// }

// TEST_CASE("vetoref", "[.cmd][.gotthard2]") {
//     if (test::type == slsDetectorDefs::GOTTHARD2) {
//         REQUIRE_THROWS(multiSlsDetectorClient("vetoref 3 0x3ff", PUT)); //
//         invalid chip index REQUIRE_THROWS(multiSlsDetectorClient("vetoref 0
//         0xFFFF", PUT)); // invalid value
//         REQUIRE_NOTHROW(multiSlsDetectorClient("vetoref 1 0x010", PUT));
//         REQUIRE_THROWS(multiSlsDetectorClient("vetoref", GET));
//     } else {
//         REQUIRE_THROWS(multiSlsDetectorClient("vetoref 3 0x0", PUT));
//     }
// }

// TEST_CASE("vetophoton", "[.cmd][.gotthard2]") {
//     if (test::type == slsDetectorDefs::GOTTHARD2) {
//         REQUIRE_THROWS(multiSlsDetectorClient("vetophoton 12 1 39950
//         examples/gotthard2_veto_photon.txt", PUT)); // invalid chip index
//         REQUIRE_THROWS(multiSlsDetectorClient("vetophoton -1 0 39950
//         examples/gotthard2_veto_photon.txt", PUT)); // invalid photon number
//         REQUIRE_NOTHROW(multiSlsDetectorClient("vetophoton -1 1 39950
//         examples/gotthard2_veto_photon.txt", PUT));
//         REQUIRE_THROWS(multiSlsDetectorClient("vetophoton", GET));
//         REQUIRE_NOTHROW(multiSlsDetectorClient("vetophoton -1", GET));
//     } else {
//         REQUIRE_THROWS(multiSlsDetectorClient("vetophoton -1", GET));
//     }
// }

// TEST_CASE("inj_ch", "[.cmd][.gotthard2]") {
//     if (test::type == slsDetectorDefs::GOTTHARD2) {
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("inj_ch 0 1", PUT,
//             nullptr, oss)); REQUIRE(oss.str() == "inj_ch [0, 1]\n");
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("inj_ch", GET, nullptr,
//             oss)); REQUIRE(oss.str() == "inj_ch [0, 1]\n");
//         }
//         REQUIRE_THROWS(multiSlsDetectorClient("inj_ch -1 1", PUT));
//         REQUIRE_THROWS(multiSlsDetectorClient("inj_ch 0 0", PUT));
//     } else {
//         REQUIRE_THROWS(multiSlsDetectorClient("inj_ch", GET));
//     }
// }
