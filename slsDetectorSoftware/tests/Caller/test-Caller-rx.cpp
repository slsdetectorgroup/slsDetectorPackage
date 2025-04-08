// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "Caller.h"
#include "catch.hpp"
#include "sls/Detector.h"
#include "sls/Version.h"
#include "sls/sls_detector_defs.h"
#include "test-Caller-global.h"

#include <sstream>

#include "sls/versionAPI.h"
#include "tests/globals.h"

namespace sls {

using test::GET;
using test::PUT;

/*
This file should contain receiver specific tests use
python/scripts/list_tested_cmd.py to check if all commands are covered
*/

/* configuration */

TEST_CASE("rx_version", "[.cmdcall][.rx]") {
    Detector det;
    Caller caller(&det);
    std::ostringstream oss;
    caller.call("rx_version", {}, -1, GET, oss);
    sls::Version v(APIRECEIVER);
    std::ostringstream vs;
    vs << "rx_version " << v.concise() << '\n';
    REQUIRE(oss.str() == vs.str());

    REQUIRE_THROWS(caller.call("rx_version", {"0"}, -1, PUT));
}

/* acquisition */
TEST_CASE("rx_start", "[.cmdcall][.rx]") {
    Detector det;
    Caller caller(&det);
    det.setFileWrite(false); // avoid writing or error on file creation
    // PUT only command
    REQUIRE_THROWS(caller.call("rx_start", {}, -1, GET));
    {
        std::ostringstream oss;
        caller.call("rx_start", {}, -1, PUT, oss);
        REQUIRE(oss.str() == "rx_start successful\n");
    }
    {
        std::ostringstream oss;
        caller.call("rx_status", {}, -1, GET, oss);
        REQUIRE(oss.str() == "rx_status running\n");
    }
}

TEST_CASE("rx_stop", "[.cmdcall][.rx]") {
    Detector det;
    Caller caller(&det);
    // PUT only command
    REQUIRE_THROWS(caller.call("rx_stop", {}, -1, GET));
    {
        std::ostringstream oss;
        caller.call("rx_stop", {}, -1, PUT, oss);
        REQUIRE(oss.str() == "rx_stop successful\n");
    }
    {
        std::ostringstream oss;
        caller.call("rx_status", {}, -1, GET, oss);
        REQUIRE(oss.str() == "rx_status idle\n");
    }
}

TEST_CASE("rx_status", "[.cmdcall][.rx]") {
    Detector det;
    Caller caller(&det);
    det.setFileWrite(false); // avoid writing or error on file creation
    det.startReceiver();
    {
        std::ostringstream oss;
        caller.call("rx_status", {}, -1, GET, oss);
        REQUIRE(oss.str() == "rx_status running\n");
    }
    det.stopReceiver();
    {
        std::ostringstream oss;
        caller.call("rx_status", {}, -1, GET, oss);
        REQUIRE(oss.str() == "rx_status idle\n");
    }
}

TEST_CASE("rx_framescaught", "[.cmdcall][.rx]") {
    Detector det;
    Caller caller(&det);
    // This ensures 0 caught frames
    auto prev_val = det.getFileWrite();
    det.setFileWrite(false); // avoid writing or error on file creation
    det.startReceiver();
    det.stopReceiver();
    {
        std::ostringstream oss;
        caller.call("rx_framescaught", {}, -1, GET, oss);
        if (det.getNumberofUDPInterfaces().tsquash(
                "inconsistent number of interfaces") == 1) {
            REQUIRE(oss.str() == "rx_framescaught [0]\n");
        } else {
            REQUIRE(oss.str() == "rx_framescaught [0, 0]\n");
        }
    }

    // Currently disabled may activate if we have a stable env
    // Now take one frame and see that we caught it
    // det.setNumberOfFrames(1);
    // det.acquire();
    // {
    //     std::ostringstream oss;
    //     caller.call("rx_framescaught", {}, -1, GET, oss);
    //     REQUIRE(oss.str() == "rx_framescaught 1\n");
    // }

    for (int i = 0; i != det.size(); ++i) {
        det.setFileWrite(prev_val[i], {i});
    }
}

TEST_CASE("rx_missingpackets", "[.cmdcall][.rx]") {
    Detector det;
    Caller caller(&det);
    auto prev_val = det.getFileWrite();
    det.setFileWrite(false); // avoid writing or error on file creation
    auto prev_frames =
        det.getNumberOfFrames().tsquash("inconsistent #frames in test");
    det.setNumberOfFrames(100);
    {
        // some missing packets
        det.startReceiver();
        det.stopReceiver();
        std::ostringstream oss;
        caller.call("rx_missingpackets", {}, -1, GET, oss);
        if (det.getNumberofUDPInterfaces().tsquash(
                "inconsistent number of interfaces") == 1) {
            REQUIRE(oss.str() != "rx_missingpackets [0]\n");
        } else {
            REQUIRE(oss.str() != "rx_missingpackets [0, 0]\n");
        }
    }
    auto det_type = det.getDetectorType().squash();
    if (det_type != defs::CHIPTESTBOARD &&
        det_type != defs::XILINX_CHIPTESTBOARD) {
        // 0 missing packets (takes into account that acquisition is
        // stopped)
        det.startReceiver();
        det.startDetector();
        det.stopDetector();
        det.stopReceiver();
        std::ostringstream oss;
        caller.call("rx_missingpackets", {}, -1, GET, oss);
        if (det.getNumberofUDPInterfaces().tsquash(
                "inconsistent number of interfaces") == 1) {
            REQUIRE(oss.str() == "rx_missingpackets [0]\n");
        } else {
            REQUIRE(oss.str() == "rx_missingpackets [0, 0]\n");
        }
    }
    for (int i = 0; i != det.size(); ++i) {
        det.setFileWrite(prev_val[i], {i});
    }
    det.setNumberOfFrames(prev_frames);
}

TEST_CASE("rx_frameindex", "[.cmdcall][.rx]") {
    Detector det;
    Caller caller(&det);
    caller.call("rx_frameindex", {}, -1, GET);

    // This is a get only command
    REQUIRE_THROWS(caller.call("rx_frameindex", {"2"}, -1, PUT));
}

/* Network Configuration (Detector<->Receiver) */

TEST_CASE("rx_printconfig", "[.cmdcall][.rx]") {
    Detector det;
    Caller caller(&det);
    REQUIRE_NOTHROW(caller.call("rx_printconfig", {}, -1, GET));
}

/* Receiver Config */

TEST_CASE("rx_hostname", "[.cmdcall][.rx]") {
    Detector det;
    Caller caller(&det);
    auto prev_val = det.getRxHostname();

    // Cannot set rx_hostname (will reset parameters in rxr and no shm
    // variables to update)
    // {
    //     // disable receiver
    //     std::ostringstream oss;
    //     caller.call("rx_hostname", {"none"}, -1, PUT, oss);
    //     REQUIRE(oss.str() == "rx_hostname [none]\n");
    // }
    // {
    //     std::ostringstream oss;
    //     caller.call("rx_hostname", {}, -1, GET, oss);
    //     REQUIRE(oss.str() == "rx_hostname none\n");
    //     // receiver should be disabled
    //     REQUIRE(det.getUseReceiverFlag().tsquash(
    //                 "different values of flag in test") == false);
    // }
    // put back old values (not necessary when we dont set it to none)
    // for (int i = 0; i != det.size(); ++i) {
    //     det.setRxHostname(prev_val[i], {i});
    // }
    {
        std::ostringstream oss;
        caller.call("rx_hostname", {}, 0, GET, oss);
        REQUIRE(oss.str() == "rx_hostname " + prev_val[0] + "\n");
    }
}

TEST_CASE("rx_tcpport", "[.cmdcall][.rx]") {
    Detector det;
    Caller caller(&det);
    auto prev_val = det.getRxPort();

    uint16_t port = 3500;
    caller.call("rx_tcpport", {std::to_string(port)}, -1, PUT);
    for (int i = 0; i != det.size(); ++i) {
        std::ostringstream oss;
        caller.call("rx_tcpport", {}, i, GET, oss);
        REQUIRE(oss.str() == "rx_tcpport " + std::to_string(port + i) + '\n');
    }
    port = 5754;
    caller.call("rx_tcpport", {std::to_string(port)}, -1, PUT);
    for (int i = 0; i != det.size(); ++i) {
        std::ostringstream oss;
        caller.call("rx_tcpport", {}, i, GET, oss);
        REQUIRE(oss.str() == "rx_tcpport " + std::to_string(port + i) + '\n');
    }
    test_valid_port_caller("rx_tcpport", {}, -1, PUT);
    test_valid_port_caller("rx_tcpport", {}, 0, PUT);
    // should fail for the second module
    if (det.size() > 1) {
        REQUIRE_THROWS(caller.call("rx_tcpport", {"65535"}, -1, PUT));
        auto rxHostname = det.getRxHostname().squash("none");
        if (rxHostname != "none") {
            std::ostringstream oss;
            for (int i = 0; i != det.size(); ++i) {
                oss << rxHostname << ":" << 65536 + i << "+";
            }
            REQUIRE_THROWS(caller.call("rx_hostname", {oss.str()}, -1, PUT));
        }
    }

    for (int i = 0; i != det.size(); ++i) {
        det.setRxPort(prev_val[i], i);
    }
}

TEST_CASE("rx_fifodepth", "[.cmdcall][.rx]") {
    Detector det;
    Caller caller(&det);
    auto prev_val = det.getRxFifoDepth();
    {
        std::ostringstream oss;
        caller.call("rx_fifodepth", {"10"}, -1, PUT, oss);
        REQUIRE(oss.str() == "rx_fifodepth 10\n");
    }
    {
        std::ostringstream oss;
        caller.call("rx_fifodepth", {"100"}, -1, PUT, oss);
        REQUIRE(oss.str() == "rx_fifodepth 100\n");
    }
    {
        std::ostringstream oss;
        caller.call("rx_fifodepth", {}, -1, GET, oss);
        REQUIRE(oss.str() == "rx_fifodepth 100\n");
    }
    for (int i = 0; i != det.size(); ++i) {
        det.setRxFifoDepth(prev_val[i], {i});
    }
}

TEST_CASE("rx_silent", "[.cmdcall][.rx]") {
    Detector det;
    Caller caller(&det);
    auto prev_val = det.getRxSilentMode();
    {
        std::ostringstream oss;
        caller.call("rx_silent", {"1"}, -1, PUT, oss);
        REQUIRE(oss.str() == "rx_silent 1\n");
    }
    {
        std::ostringstream oss;
        caller.call("rx_silent", {}, -1, GET, oss);
        REQUIRE(oss.str() == "rx_silent 1\n");
    }
    {
        std::ostringstream oss;
        caller.call("rx_silent", {"0"}, -1, PUT, oss);
        REQUIRE(oss.str() == "rx_silent 0\n");
    }
    for (int i = 0; i != det.size(); ++i) {
        det.setRxSilentMode(prev_val[i], {i});
    }
}

TEST_CASE("rx_discardpolicy", "[.cmdcall][.rx]") {
    Detector det;
    Caller caller(&det);
    auto prev_val = det.getRxFrameDiscardPolicy();
    {
        std::ostringstream oss;
        caller.call("rx_discardpolicy", {"discardempty"}, -1, PUT, oss);
        REQUIRE(oss.str() == "rx_discardpolicy discardempty\n");
    }
    {
        std::ostringstream oss;
        caller.call("rx_discardpolicy", {}, -1, GET, oss);
        REQUIRE(oss.str() == "rx_discardpolicy discardempty\n");
    }
    {
        std::ostringstream oss;
        caller.call("rx_discardpolicy", {"discardpartial"}, -1, PUT, oss);
        REQUIRE(oss.str() == "rx_discardpolicy discardpartial\n");
    }
    {
        std::ostringstream oss;
        caller.call("rx_discardpolicy", {"nodiscard"}, -1, PUT, oss);
        REQUIRE(oss.str() == "rx_discardpolicy nodiscard\n");
    }
    for (int i = 0; i != det.size(); ++i) {
        det.setRxFrameDiscardPolicy(prev_val[i], {i});
    }
}

TEST_CASE("rx_padding", "[.cmdcall][.rx]") {
    Detector det;
    Caller caller(&det);
    auto prev_val = det.getPartialFramesPadding();
    {
        std::ostringstream oss;
        caller.call("rx_padding", {"0"}, -1, PUT, oss);
        REQUIRE(oss.str() == "rx_padding 0\n");
    }
    {
        std::ostringstream oss;
        caller.call("rx_padding", {}, -1, GET, oss);
        REQUIRE(oss.str() == "rx_padding 0\n");
    }
    {
        std::ostringstream oss;
        caller.call("rx_padding", {"1"}, -1, PUT, oss);
        REQUIRE(oss.str() == "rx_padding 1\n");
    }
    for (int i = 0; i != det.size(); ++i) {
        det.setPartialFramesPadding(prev_val[i], {i});
    }
}

TEST_CASE("rx_udpsocksize", "[.cmdcall][.rx]") {
    Detector det;
    Caller caller(&det);
    int64_t prev_val = det.getRxUDPSocketBufferSize().tsquash(
        "Need same udp socket buffer size to test");
    std::string s_new_val = std::to_string(prev_val);
    /*std::string s_new_val = std::to_string(prev_val - 1000);
    { Need permissions
        std::ostringstream oss;
        caller.call("rx_udpsocksize", {s_new_val}, -1, PUT, oss);
        REQUIRE(oss.str() >= "rx_udpsocksize " + s_new_val + "\n");
    }*/
    {
        std::ostringstream oss;
        caller.call("rx_udpsocksize", {}, -1, GET, oss);
        REQUIRE(oss.str() >= "rx_udpsocksize " + s_new_val + "\n");
    }
    det.setRxUDPSocketBufferSize(prev_val);
}

TEST_CASE("rx_realudpsocksize", "[.cmdcall][.rx]") {
    Detector det;
    Caller caller(&det);
    uint64_t val = 0;
    {
        std::ostringstream oss;
        caller.call("rx_udpsocksize", {}, -1, GET, oss);
        std::string s = (oss.str()).erase(0, strlen("rx_udpsocksize "));
        val = std::stol(s);
    }
    {
        std::ostringstream oss;
        caller.call("rx_realudpsocksize", {}, -1, GET, oss);
        std::string s = (oss.str()).erase(0, strlen("rx_realudpsocksize "));
        uint64_t rval = std::stol(s);
        REQUIRE(rval >= val * 2);
    }
}

TEST_CASE("rx_lock", "[.cmdcall][.rx]") {
    Detector det;
    Caller caller(&det);
    auto prev_val = det.getRxLock();
    {
        std::ostringstream oss;
        caller.call("rx_lock", {"1"}, -1, PUT, oss);
        REQUIRE(oss.str() == "rx_lock 1\n");
    }
    {
        std::ostringstream oss;
        caller.call("rx_lock", {}, -1, GET, oss);
        REQUIRE(oss.str() == "rx_lock 1\n");
    }
    {
        std::ostringstream oss;
        caller.call("rx_lock", {"0"}, -1, PUT, oss);
        REQUIRE(oss.str() == "rx_lock 0\n");
    }
    for (int i = 0; i != det.size(); ++i) {
        det.setRxLock(prev_val[i], {i});
    }
}

TEST_CASE("rx_lastclient", "[.cmdcall][.rx]") {
    Detector det;
    Caller caller(&det);
    std::ostringstream oss;
    REQUIRE_NOTHROW(caller.call("rx_lastclient", {}, -1, GET, oss));
    if (test::my_ip != "undefined") {
        REQUIRE(oss.str() == "rx_lastclient " + test::my_ip + "\n");
    }
}

TEST_CASE("rx_threads", "[.cmdcall][.rx]") {
    Detector det;
    Caller caller(&det);
    std::ostringstream oss;
    REQUIRE_NOTHROW(caller.call("rx_threads", {}, -1, GET, oss));
}

TEST_CASE("rx_arping", "[.cmdcall][.rx]") {
    Detector det;
    Caller caller(&det);
    auto prev_val = det.getRxArping();
    if (det.getDestinationUDPIP()[0].str() != "127.0.0.1") {
        {
            std::ostringstream oss;
            caller.call("rx_arping", {"1"}, -1, PUT, oss);
            REQUIRE(oss.str() == "rx_arping 1\n");
        }
        {
            std::ostringstream oss;
            caller.call("rx_arping", {}, -1, GET, oss);
            REQUIRE(oss.str() == "rx_arping 1\n");
        }
        {
            std::ostringstream oss;
            caller.call("rx_arping", {"0"}, -1, PUT, oss);
            REQUIRE(oss.str() == "rx_arping 0\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setRxArping(prev_val[i], {i});
        }
    }
}

TEST_CASE("rx_roi", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::CHIPTESTBOARD ||
        det_type == defs::XILINX_CHIPTESTBOARD) {
        REQUIRE_THROWS(caller.call("rx_roi", {"5", "10"}, -1, PUT));
    } else {
        auto prev_val = det.getRxROI();
        defs::xy detsize = det.getDetectorSize();

        // 1d
        if (det_type == defs::GOTTHARD || det_type == defs::GOTTHARD2 ||
            det_type == defs::MYTHEN3) {
            {
                std::ostringstream oss;
                caller.call("rx_roi", {"5", "10"}, -1, PUT, oss);
                REQUIRE(oss.str() == "rx_roi [5, 10]\n");
            }
            {
                std::ostringstream oss;
                caller.call("rx_roi", {"10", "15"}, -1, PUT, oss);
                REQUIRE(oss.str() == "rx_roi [10, 15]\n");
            }
            REQUIRE_THROWS(caller.call("rx_roi", {"-1", "-1"}, -1, PUT));
            REQUIRE_THROWS(
                caller.call("rx_roi", {"10", "15", "25", "30"}, -1, PUT));
        }
        // 2d
        else {
            {
                std::ostringstream oss;
                caller.call("rx_roi", {"10", "15", "1", "5"}, -1, PUT, oss);
                REQUIRE(oss.str() == "rx_roi [10, 15, 1, 5]\n");
            }
            {
                std::ostringstream oss;
                caller.call("rx_roi", {"10", "22", "18", "19"}, -1, PUT, oss);
                REQUIRE(oss.str() == "rx_roi [10, 22, 18, 19]\n");
            }
            {
                std::ostringstream oss;
                caller.call("rx_roi",
                            {"1", std::to_string(detsize.x - 5), "1",
                             std::to_string(detsize.y - 5)},
                            -1, PUT, oss);
                REQUIRE(oss.str() == std::string("rx_roi [1, ") +
                                         std::to_string(detsize.x - 5) +
                                         std::string(", 1, ") +
                                         std::to_string(detsize.y - 5) +
                                         std::string("]\n"));
            }
            REQUIRE_THROWS(
                caller.call("rx_roi", {"-1", "-1", "-1", "-1"}, -1, PUT));
        }

        for (int i = 0; i != det.size(); ++i) {
            det.setRxROI(prev_val);
        }
    }
}

TEST_CASE("rx_clearroi", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::CHIPTESTBOARD ||
        det_type == defs::XILINX_CHIPTESTBOARD) {
        REQUIRE_THROWS(caller.call("rx_clearroi", {}, -1, PUT));
    } else {
        auto prev_val = det.getRxROI();
        {
            std::ostringstream oss;
            caller.call("rx_clearroi", {}, -1, PUT, oss);
            REQUIRE(oss.str() == "rx_clearroi successful\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setRxROI(prev_val);
        }
    }
}

/* File */

TEST_CASE("fformat", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto prev_val = det.getFileFormat();
    {
        std::ostringstream oss;
        caller.call("fformat", {"binary"}, -1, PUT, oss);
        REQUIRE(oss.str() == "fformat binary\n");
    }
    {
        std::ostringstream oss;
        caller.call("fformat", {}, -1, GET, oss);
        REQUIRE(oss.str() == "fformat binary\n");
    }
    for (int i = 0; i != det.size(); ++i) {
        det.setFileFormat(prev_val[i], {i});
    }
}

TEST_CASE("fpath", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto prev_val = det.getFilePath();
    {
        std::ostringstream oss;
        caller.call("fpath", {"/tmp"}, -1, PUT, oss);
        REQUIRE(oss.str() == "fpath /tmp\n");
    }
    {
        std::ostringstream oss;
        caller.call("fpath", {}, -1, GET, oss);
        REQUIRE(oss.str() == "fpath /tmp\n");
    }
    for (int i = 0; i != det.size(); ++i) {
        det.setFilePath(prev_val[i], {i});
    }
}

TEST_CASE("fname", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto prev_val = det.getFileNamePrefix();
    {
        std::ostringstream oss;
        caller.call("fname", {"somename"}, -1, PUT, oss);
        REQUIRE(oss.str() == "fname somename\n");
    }
    {
        std::ostringstream oss;
        caller.call("fname", {}, -1, GET, oss);
        REQUIRE(oss.str() == "fname somename\n");
    }
    {
        std::ostringstream oss;
        caller.call("fname", {"run"}, -1, PUT, oss);
        REQUIRE(oss.str() == "fname run\n");
    }
    REQUIRE_THROWS(caller.call("fname", {"fdf/dfd"}, -1, PUT));
    REQUIRE_THROWS(caller.call("fname", {"fdf dfd"}, -1, PUT));

    for (int i = 0; i != det.size(); ++i) {
        det.setFileNamePrefix(prev_val[i], {i});
    }
}

TEST_CASE("findex", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto prev_val = det.getAcquisitionIndex();
    {
        std::ostringstream oss;
        caller.call("findex", {"57"}, -1, PUT, oss);
        REQUIRE(oss.str() == "findex 57\n");
    }
    {
        std::ostringstream oss;
        caller.call("findex", {}, -1, GET, oss);
        REQUIRE(oss.str() == "findex 57\n");
    }
    {
        std::ostringstream oss;
        caller.call("findex", {"0"}, -1, PUT, oss);
        REQUIRE(oss.str() == "findex 0\n");
    }
    for (int i = 0; i != det.size(); ++i) {
        det.setAcquisitionIndex(prev_val[i], {i});
    }
}

TEST_CASE("fwrite", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto prev_val = det.getFileWrite();
    {
        std::ostringstream oss;
        caller.call("fwrite", {"1"}, -1, PUT, oss);
        REQUIRE(oss.str() == "fwrite 1\n");
    }
    {
        std::ostringstream oss;
        caller.call("fwrite", {}, -1, GET, oss);
        REQUIRE(oss.str() == "fwrite 1\n");
    }
    {
        std::ostringstream oss;
        caller.call("fwrite", {"0"}, -1, PUT, oss);
        REQUIRE(oss.str() == "fwrite 0\n");
    }
    for (int i = 0; i != det.size(); ++i) {
        det.setFileWrite(prev_val[i], {i});
    }
}

TEST_CASE("fmaster", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto prev_val = det.getMasterFileWrite();
    {
        std::ostringstream oss;
        caller.call("fmaster", {"0"}, -1, PUT, oss);
        REQUIRE(oss.str() == "fmaster 0\n");
    }
    {
        std::ostringstream oss;
        caller.call("fmaster", {}, -1, GET, oss);
        REQUIRE(oss.str() == "fmaster 0\n");
    }
    {
        std::ostringstream oss;
        caller.call("fmaster", {"1"}, -1, PUT, oss);
        REQUIRE(oss.str() == "fmaster 1\n");
    }
    det.setMasterFileWrite(prev_val);
}

TEST_CASE("foverwrite", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto prev_val = det.getFileOverWrite();
    {
        std::ostringstream oss;
        caller.call("foverwrite", {"1"}, -1, PUT, oss);
        REQUIRE(oss.str() == "foverwrite 1\n");
    }
    {
        std::ostringstream oss;
        caller.call("foverwrite", {}, -1, GET, oss);
        REQUIRE(oss.str() == "foverwrite 1\n");
    }
    {
        std::ostringstream oss;
        caller.call("foverwrite", {"0"}, -1, PUT, oss);
        REQUIRE(oss.str() == "foverwrite 0\n");
    }
    for (int i = 0; i != det.size(); ++i) {
        det.setFileOverWrite(prev_val[i], {i});
    }
}

TEST_CASE("rx_framesperfile", "[.cmdcall][.rx]") {
    Detector det;
    Caller caller(&det);
    auto prev_val = det.getFramesPerFile();
    {
        std::ostringstream oss;
        caller.call("rx_framesperfile", {"50"}, -1, PUT, oss);
        REQUIRE(oss.str() == "rx_framesperfile 50\n");
    }
    {
        std::ostringstream oss;
        caller.call("rx_framesperfile", {}, -1, GET, oss);
        REQUIRE(oss.str() == "rx_framesperfile 50\n");
    }
    {
        std::ostringstream oss;
        caller.call("rx_framesperfile", {"10000"}, -1, PUT, oss);
        REQUIRE(oss.str() == "rx_framesperfile 10000\n");
    }
    {
        std::ostringstream oss;
        caller.call("rx_framesperfile", {"0"}, -1, PUT, oss);
        REQUIRE(oss.str() == "rx_framesperfile 0\n");
    }
    for (int i = 0; i != det.size(); ++i) {
        det.setFramesPerFile(prev_val[i], {i});
    }
}

/* ZMQ Streaming Parameters (Receiver<->Client) */

TEST_CASE("rx_zmqstream", "[.cmdcall][.rx]") {
    Detector det;
    Caller caller(&det);
    auto prev_val = det.getRxZmqDataStream();
    {
        std::ostringstream oss;
        caller.call("rx_zmqstream", {"1"}, -1, PUT, oss);
        REQUIRE(oss.str() == "rx_zmqstream 1\n");
        REQUIRE(det.getRxZmqDataStream().squash() == true);
    }
    {
        std::ostringstream oss;
        caller.call("rx_zmqstream", {}, -1, GET, oss);
        REQUIRE(oss.str() == "rx_zmqstream 1\n");
    }
    {
        std::ostringstream oss;
        caller.call("rx_zmqstream", {"0"}, -1, PUT, oss);
        REQUIRE(oss.str() == "rx_zmqstream 0\n");
        REQUIRE(det.getRxZmqDataStream().squash() == false);
    }
    for (int i = 0; i != det.size(); ++i) {
        det.setRxZmqDataStream(prev_val[i], {i});
    }
}

TEST_CASE("rx_zmqfreq", "[.cmdcall][.rx]") {
    Detector det;
    Caller caller(&det);
    auto prev_val = det.getRxZmqFrequency();
    {
        std::ostringstream oss;
        caller.call("rx_zmqfreq", {"1"}, -1, PUT, oss);
        REQUIRE(oss.str() == "rx_zmqfreq 1\n");
    }
    {
        std::ostringstream oss;
        caller.call("rx_zmqfreq", {}, -1, GET, oss);
        REQUIRE(oss.str() == "rx_zmqfreq 1\n");
    }
    {
        std::ostringstream oss;
        caller.call("rx_zmqfreq", {"0"}, -1, PUT, oss);
        REQUIRE(oss.str() == "rx_zmqfreq 0\n");
    }
    for (int i = 0; i != det.size(); ++i) {
        det.setRxZmqFrequency(prev_val[i], {i});
    }
}

TEST_CASE("rx_zmqstartfnum", "[.cmdcall][.rx]") {
    Detector det;
    Caller caller(&det);
    auto prev_val = det.getRxZmqStartingFrame();
    {
        std::ostringstream oss;
        caller.call("rx_zmqstartfnum", {"5"}, -1, PUT, oss);
        REQUIRE(oss.str() == "rx_zmqstartfnum 5\n");
    }
    {
        std::ostringstream oss;
        caller.call("rx_zmqstartfnum", {}, -1, GET, oss);
        REQUIRE(oss.str() == "rx_zmqstartfnum 5\n");
    }
    {
        std::ostringstream oss;
        caller.call("rx_zmqstartfnum", {"0"}, -1, PUT, oss);
        REQUIRE(oss.str() == "rx_zmqstartfnum 0\n");
    }
    for (int i = 0; i != det.size(); ++i) {
        det.setRxZmqStartingFrame(prev_val[i], {i});
    }
}

TEST_CASE("rx_zmqport", "[.cmdcall][.rx]") {
    Detector det;
    Caller caller(&det);
    auto prev_val_zmqport = det.getRxZmqPort();
    auto prev_val_numinterfaces = det.getNumberofUDPInterfaces().tsquash(
        "inconsistent number of udp interfaces to test");

    int socketsperdetector = 1;
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::EIGER) {
        socketsperdetector *= 2;
    } else if (det_type == defs::JUNGFRAU || det_type == defs::MOENCH) {
        caller.call("numinterfaces", {"2"}, -1, PUT);
        socketsperdetector *= 2;
    }
    uint16_t port = 3500;
    caller.call("rx_zmqport", {std::to_string(port)}, -1, PUT);
    for (int i = 0; i != det.size(); ++i) {
        std::ostringstream oss;
        caller.call("rx_zmqport", {}, i, GET, oss);
        REQUIRE(oss.str() == "rx_zmqport " +
                                 std::to_string(port + i * socketsperdetector) +
                                 '\n');
    }
    port = 30001;
    caller.call("rx_zmqport", {std::to_string(port)}, -1, PUT);
    for (int i = 0; i != det.size(); ++i) {
        std::ostringstream oss;
        caller.call("rx_zmqport", {}, i, GET, oss);
        REQUIRE(oss.str() == "rx_zmqport " +
                                 std::to_string(port + i * socketsperdetector) +
                                 '\n');
    }
    test_valid_port_caller("rx_zmqport", {}, -1, PUT);
    test_valid_port_caller("rx_zmqport", {}, 0, PUT);
    // should fail for the second module
    if (det.size() > 1) {
        REQUIRE_THROWS(caller.call("rx_zmqport", {"65535"}, -1, PUT));
    }

    for (int i = 0; i != det.size(); ++i) {
        det.setRxZmqPort(prev_val_zmqport[i], i);
    }
    if (det_type == defs::JUNGFRAU || det_type == defs::MOENCH) {
        det.setNumberofUDPInterfaces(prev_val_numinterfaces);
    }
}

TEST_CASE("rx_zmqhwm", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto prev_val =
        det.getRxZmqHwm().tsquash("Inconsistent values for rx_zmqhwm to test");
    {
        std::ostringstream oss;
        caller.call("rx_zmqhwm", {"50"}, -1, PUT, oss);
        REQUIRE(oss.str() == "rx_zmqhwm 50\n");
    }
    {
        std::ostringstream oss;
        caller.call("rx_zmqhwm", {}, -1, GET, oss);
        REQUIRE(oss.str() == "rx_zmqhwm 50\n");
    }
    {
        std::ostringstream oss;
        caller.call("rx_zmqhwm", {"0"}, -1, PUT, oss);
        REQUIRE(oss.str() == "rx_zmqhwm 0\n");
    }
    {
        std::ostringstream oss;
        caller.call("rx_zmqhwm", {"-1"}, -1, PUT, oss);
        REQUIRE(oss.str() == "rx_zmqhwm -1\n");
    }
    det.setRxZmqHwm(prev_val);
}

/* CTB Specific */

TEST_CASE("rx_dbitlist", "[.cmdcall][.rx]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::CHIPTESTBOARD ||
        det_type == defs::XILINX_CHIPTESTBOARD) {
        auto prev_val = det.getRxDbitList();
        {
            std::ostringstream oss;
            caller.call("rx_dbitlist",
                        {"0", "4", "5", "8", "9", "10", "52", "63"}, -1, PUT,
                        oss);
            REQUIRE(oss.str() == "rx_dbitlist [0, 4, 5, 8, 9, 10, 52, 63]\n");
        }
        {
            std::ostringstream oss;
            caller.call("rx_dbitlist", {}, -1, GET, oss);
            REQUIRE(oss.str() == "rx_dbitlist [0, 4, 5, 8, 9, 10, 52, 63]\n");
        }
        REQUIRE_THROWS(caller.call("rx_dbitlist", {"67"}, -1, PUT));
        REQUIRE_THROWS(caller.call("rx_dbitlist", {"-1"}, -1, PUT));
        REQUIRE_NOTHROW(caller.call("rx_dbitlist", {"all"}, -1, PUT));
        for (int i = 0; i != det.size(); ++i) {
            det.setRxDbitList(prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(caller.call("rx_dbitlist", {}, -1, GET));
    }
}

TEST_CASE("rx_dbitoffset", "[.cmdcall][.rx]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::CHIPTESTBOARD ||
        det_type == defs::XILINX_CHIPTESTBOARD) {
        auto prev_val = det.getRxDbitOffset();
        {
            std::ostringstream oss;
            caller.call("rx_dbitoffset", {"1"}, -1, PUT, oss);
            REQUIRE(oss.str() == "rx_dbitoffset 1\n");
        }
        {
            std::ostringstream oss;
            caller.call("rx_dbitoffset", {"0"}, -1, PUT, oss);
            REQUIRE(oss.str() == "rx_dbitoffset 0\n");
        }
        {
            std::ostringstream oss;
            caller.call("rx_dbitoffset", {"15"}, -1, PUT, oss);
            REQUIRE(oss.str() == "rx_dbitoffset 15\n");
        }
        {
            std::ostringstream oss;
            caller.call("rx_dbitoffset", {}, -1, GET, oss);
            REQUIRE(oss.str() == "rx_dbitoffset 15\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setRxDbitOffset(prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(caller.call("rx_dbitoffset", {}, -1, GET));
    }
}

TEST_CASE("rx_jsonaddheader", "[.cmdcall][.rx]") {
    Detector det;
    Caller caller(&det);
    auto prev_val = det.getAdditionalJsonHeader();

    {
        std::ostringstream oss;
        caller.call("rx_jsonaddheader", {"key1", "value1", "key2", "value2"},
                    -1, PUT, oss);
        REQUIRE(oss.str() == "rx_jsonaddheader {key1: value1, key2: value2}\n");
    }
    {
        std::ostringstream oss;
        caller.call("rx_jsonaddheader", {}, -1, GET, oss);
        REQUIRE(oss.str() == "rx_jsonaddheader {key1: value1, key2: value2}\n");
    }
    {
        std::ostringstream oss;
        caller.call("rx_jsonaddheader", {}, -1, PUT, oss);
        REQUIRE(oss.str() == "rx_jsonaddheader {}\n");
    }
    for (int i = 0; i != det.size(); ++i) {
        det.setAdditionalJsonHeader(prev_val[i], {i});
    }
}

TEST_CASE("rx_jsonpara", "[.cmdcall][.rx]") {
    Detector det;
    Caller caller(&det);
    auto prev_val = det.getAdditionalJsonHeader();
    {
        std::ostringstream oss;
        caller.call("rx_jsonpara", {"key1", "value1"}, -1, PUT, oss);
        REQUIRE(oss.str() == "rx_jsonpara {key1: value1}\n");
    }
    {
        std::ostringstream oss;
        caller.call("rx_jsonpara", {"key1", "value2"}, -1, PUT, oss);
        REQUIRE(oss.str() == "rx_jsonpara {key1: value2}\n");
    }
    {
        std::ostringstream oss;
        caller.call("rx_jsonpara", {"key1"}, -1, GET, oss);
        REQUIRE(oss.str() == "rx_jsonpara value2\n");
    }
    {
        std::ostringstream oss;
        caller.call("rx_jsonpara", {"key1"}, -1, PUT, oss);
        REQUIRE(oss.str() == "rx_jsonpara key1 deleted\n");
    }
    REQUIRE_THROWS(caller.call("rx_jsonpara", {"key1"}, -1, GET));
    for (int i = 0; i != det.size(); ++i) {
        det.setAdditionalJsonHeader(prev_val[i], {i});
    }
}

/* Insignificant */

} // namespace sls
