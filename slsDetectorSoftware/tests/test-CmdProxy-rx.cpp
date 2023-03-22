// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "CmdProxy.h"
#include "catch.hpp"
#include "sls/Detector.h"
#include "sls/Version.h"
#include "sls/sls_detector_defs.h"
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

TEST_CASE("rx_version", "[.cmd][.rx]") {
    Detector det;
    CmdProxy proxy(&det);
    std::ostringstream oss;
    proxy.Call("rx_version", {}, -1, GET, oss);
    sls::Version v(APIRECEIVER);
    std::ostringstream vs;
    vs << "rx_version " << v.concise() << '\n';
    REQUIRE(oss.str() == vs.str());

    REQUIRE_THROWS(proxy.Call("rx_version", {"0"}, -1, PUT));
}

/* acquisition */
TEST_CASE("rx_start", "[.cmd][.rx]") {
    Detector det;
    CmdProxy proxy(&det);
    det.setFileWrite(false); // avoid writing or error on file creation
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

TEST_CASE("rx_status", "[.cmd][.rx]") {
    Detector det;
    det.setFileWrite(false); // avoid writing or error on file creation
    CmdProxy proxy(&det);
    det.startReceiver();
    {
        std::ostringstream oss;
        proxy.Call("rx_status", {}, -1, GET, oss);
        REQUIRE(oss.str() == "rx_status running\n");
    }
    det.stopReceiver();
    {
        std::ostringstream oss;
        proxy.Call("rx_status", {}, -1, GET, oss);
        REQUIRE(oss.str() == "rx_status idle\n");
    }
}

TEST_CASE("rx_framescaught", "[.cmd][.rx]") {
    Detector det;
    CmdProxy proxy(&det);

    // This ensures 0 caught frames
    auto prev_val = det.getFileWrite();
    det.setFileWrite(false); // avoid writing or error on file creation
    det.startReceiver();
    det.stopReceiver();
    {
        std::ostringstream oss;
        proxy.Call("rx_framescaught", {}, -1, GET, oss);
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
    //     proxy.Call("rx_framescaught", {}, -1, GET, oss);
    //     REQUIRE(oss.str() == "rx_framescaught 1\n");
    // }

    for (int i = 0; i != det.size(); ++i) {
        det.setFileWrite(prev_val[i], {i});
    }
}

TEST_CASE("rx_missingpackets", "[.cmd][.rx]") {
    Detector det;
    auto prev_val = det.getFileWrite();
    det.setFileWrite(false); // avoid writing or error on file creation
    CmdProxy proxy(&det);
    auto prev_frames =
        det.getNumberOfFrames().tsquash("inconsistent #frames in test");
    det.setNumberOfFrames(100);
    {
        // some missing packets
        det.startReceiver();
        det.stopReceiver();
        std::ostringstream oss;
        proxy.Call("rx_missingpackets", {}, -1, GET, oss);
        if (det.getNumberofUDPInterfaces().tsquash(
                "inconsistent number of interfaces") == 1) {
            REQUIRE(oss.str() != "rx_missingpackets [0]\n");
        } else {
            REQUIRE(oss.str() != "rx_missingpackets [0, 0]\n");
        }
    }
    auto det_type = det.getDetectorType().squash();
    if (det_type != defs::CHIPTESTBOARD && det_type != defs::MOENCH) {
        // 0 missing packets (takes into account that acquisition is stopped)
        det.startReceiver();
        det.startDetector();
        det.stopDetector();
        det.stopReceiver();
        std::ostringstream oss;
        proxy.Call("rx_missingpackets", {}, -1, GET, oss);
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

TEST_CASE("rx_frameindex", "[.cmd][.rx]") {
    Detector det;
    CmdProxy proxy(&det);
    proxy.Call("rx_frameindex", {}, -1, GET);

    // This is a get only command
    REQUIRE_THROWS(proxy.Call("rx_frameindex", {"2"}, -1, PUT));
}

/* Network Configuration (Detector<->Receiver) */

TEST_CASE("rx_printconfig", "[.cmd][.rx]") {
    Detector det;
    CmdProxy proxy(&det);
    REQUIRE_NOTHROW(proxy.Call("rx_printconfig", {}, -1, GET));
}

/* Receiver Config */

TEST_CASE("rx_hostname", "[.cmd][.rx]") {
    Detector det;
    CmdProxy proxy(&det);
    auto prev_val = det.getRxHostname();

    // Cannot set rx_hostname (will reset parameters in rxr and no shm variables
    // to update)
    // {
    //     // disable receiver
    //     std::ostringstream oss;
    //     proxy.Call("rx_hostname", {"none"}, -1, PUT, oss);
    //     REQUIRE(oss.str() == "rx_hostname [none]\n");
    // }
    // {
    //     std::ostringstream oss;
    //     proxy.Call("rx_hostname", {}, -1, GET, oss);
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
        proxy.Call("rx_hostname", {}, 0, GET, oss);
        REQUIRE(oss.str() == "rx_hostname " + prev_val[0] + "\n");
    }
}

TEST_CASE("rx_tcpport", "[.cmd][.rx]") {
    Detector det;
    CmdProxy proxy(&det);
    auto prev_val = det.getRxPort();

    int port = 3500;
    proxy.Call("rx_tcpport", {std::to_string(port)}, -1, PUT);
    for (int i = 0; i != det.size(); ++i) {
        std::ostringstream oss;
        proxy.Call("rx_tcpport", {}, i, GET, oss);
        REQUIRE(oss.str() == "rx_tcpport " + std::to_string(port + i) + '\n');
    }
    port = 5754;
    proxy.Call("rx_tcpport", {std::to_string(port)}, -1, PUT);
    for (int i = 0; i != det.size(); ++i) {
        std::ostringstream oss;
        proxy.Call("rx_tcpport", {}, i, GET, oss);
        REQUIRE(oss.str() == "rx_tcpport " + std::to_string(port + i) + '\n');
    }
    for (int i = 0; i != det.size(); ++i) {
        det.setRxPort(prev_val[i], i);
    }
}

TEST_CASE("rx_fifodepth", "[.cmd][.rx]") {
    Detector det;
    CmdProxy proxy(&det);
    auto prev_val = det.getRxFifoDepth();
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
    for (int i = 0; i != det.size(); ++i) {
        det.setRxFifoDepth(prev_val[i], {i});
    }
}

TEST_CASE("rx_silent", "[.cmd][.rx]") {
    Detector det;
    CmdProxy proxy(&det);
    auto prev_val = det.getRxSilentMode();
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
    for (int i = 0; i != det.size(); ++i) {
        det.setRxSilentMode(prev_val[i], {i});
    }
}

TEST_CASE("rx_discardpolicy", "[.cmd][.rx]") {
    Detector det;
    CmdProxy proxy(&det);
    auto prev_val = det.getRxFrameDiscardPolicy();
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
    for (int i = 0; i != det.size(); ++i) {
        det.setRxFrameDiscardPolicy(prev_val[i], {i});
    }
}

TEST_CASE("rx_padding", "[.cmd][.rx]") {
    Detector det;
    CmdProxy proxy(&det);
    auto prev_val = det.getPartialFramesPadding();
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
    for (int i = 0; i != det.size(); ++i) {
        det.setPartialFramesPadding(prev_val[i], {i});
    }
}

TEST_CASE("rx_udpsocksize", "[.cmd][.rx]") {
    Detector det;
    CmdProxy proxy(&det);
    int64_t prev_val = det.getRxUDPSocketBufferSize().tsquash(
        "Need same udp socket buffer size to test");
    std::string s_new_val = std::to_string(prev_val);
    /*std::string s_new_val = std::to_string(prev_val - 1000);
    { Need permissions
        std::ostringstream oss;
        proxy.Call("rx_udpsocksize", {s_new_val}, -1, PUT, oss);
        REQUIRE(oss.str() >= "rx_udpsocksize " + s_new_val + "\n");
    }*/
    {
        std::ostringstream oss;
        proxy.Call("rx_udpsocksize", {}, -1, GET, oss);
        REQUIRE(oss.str() >= "rx_udpsocksize " + s_new_val + "\n");
    }
    det.setRxUDPSocketBufferSize(prev_val);
}

TEST_CASE("rx_realudpsocksize", "[.cmd][.rx]") {
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

TEST_CASE("rx_lock", "[.cmd][.rx]") {
    Detector det;
    CmdProxy proxy(&det);
    auto prev_val = det.getRxLock();
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
    for (int i = 0; i != det.size(); ++i) {
        det.setRxLock(prev_val[i], {i});
    }
}

TEST_CASE("rx_lastclient", "[.cmd][.rx]") {
    Detector det;
    CmdProxy proxy(&det);
    std::ostringstream oss;
    REQUIRE_NOTHROW(proxy.Call("rx_lastclient", {}, -1, GET, oss));
    if (test::my_ip != "undefined") {
        REQUIRE(oss.str() == "rx_lastclient " + test::my_ip + "\n");
    }
}

TEST_CASE("rx_threads", "[.cmd][.rx]") {
    Detector det;
    CmdProxy proxy(&det);
    std::ostringstream oss;
    REQUIRE_NOTHROW(proxy.Call("rx_threads", {}, -1, GET, oss));
}

TEST_CASE("rx_arping", "[.cmd][.rx]") {
    Detector det;
    CmdProxy proxy(&det);
    auto prev_val = det.getRxArping();
    {
        std::ostringstream oss;
        proxy.Call("rx_arping", {"1"}, -1, PUT, oss);
        REQUIRE(oss.str() == "rx_arping 1\n");
    }
    {
        std::ostringstream oss;
        proxy.Call("rx_arping", {}, -1, GET, oss);
        REQUIRE(oss.str() == "rx_arping 1\n");
    }
    {
        std::ostringstream oss;
        proxy.Call("rx_arping", {"0"}, -1, PUT, oss);
        REQUIRE(oss.str() == "rx_arping 0\n");
    }
    for (int i = 0; i != det.size(); ++i) {
        det.setRxArping(prev_val[i], {i});
    }
}

TEST_CASE("rx_roi", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::CHIPTESTBOARD) {
        REQUIRE_THROWS(proxy.Call("rx_roi", {"5", "10"}, -1, PUT));
    } else {
        auto prev_val = det.getRxROI();
        defs::xy detsize = det.getDetectorSize();

        // 1d
        if (det_type == defs::GOTTHARD || det_type == defs::GOTTHARD2 ||
            det_type == defs::MYTHEN3) {
            {
                std::ostringstream oss;
                proxy.Call("rx_roi", {"5", "10"}, -1, PUT, oss);
                REQUIRE(oss.str() == "rx_roi [5, 10]\n");
            }
            {
                std::ostringstream oss;
                proxy.Call("rx_roi", {"10", "15"}, -1, PUT, oss);
                REQUIRE(oss.str() == "rx_roi [10, 15]\n");
            }
            REQUIRE_THROWS(proxy.Call("rx_roi", {"-1", "-1"}, -1, PUT));
            REQUIRE_THROWS(
                proxy.Call("rx_roi", {"10", "15", "25", "30"}, -1, PUT));
        }
        // 2d
        else {
            {
                std::ostringstream oss;
                proxy.Call("rx_roi", {"10", "15", "1", "5"}, -1, PUT, oss);
                REQUIRE(oss.str() == "rx_roi [10, 15, 1, 5]\n");
            }
            {
                std::ostringstream oss;
                proxy.Call("rx_roi", {"10", "22", "18", "19"}, -1, PUT, oss);
                REQUIRE(oss.str() == "rx_roi [10, 22, 18, 19]\n");
            }
            {
                std::ostringstream oss;
                proxy.Call("rx_roi",
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
                proxy.Call("rx_roi", {"-1", "-1", "-1", "-1"}, -1, PUT));
        }

        for (int i = 0; i != det.size(); ++i) {
            det.setRxROI(prev_val);
        }
    }
}

TEST_CASE("rx_clearroi", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::CHIPTESTBOARD) {
        REQUIRE_THROWS(proxy.Call("rx_clearroi", {}, -1, PUT));
    } else {
        auto prev_val = det.getRxROI();
        {
            std::ostringstream oss;
            proxy.Call("rx_clearroi", {}, -1, PUT, oss);
            REQUIRE(oss.str() == "rx_clearroi successful\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setRxROI(prev_val);
        }
    }
}

/* File */

TEST_CASE("fformat", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto prev_val = det.getFileFormat();
    {
        std::ostringstream oss;
        proxy.Call("fformat", {"binary"}, -1, PUT, oss);
        REQUIRE(oss.str() == "fformat binary\n");
    }
    {
        std::ostringstream oss;
        proxy.Call("fformat", {}, -1, GET, oss);
        REQUIRE(oss.str() == "fformat binary\n");
    }
    for (int i = 0; i != det.size(); ++i) {
        det.setFileFormat(prev_val[i], {i});
    }
}

TEST_CASE("fpath", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto prev_val = det.getFilePath();
    {
        std::ostringstream oss;
        proxy.Call("fpath", {"/tmp"}, -1, PUT, oss);
        REQUIRE(oss.str() == "fpath /tmp\n");
    }
    {
        std::ostringstream oss;
        proxy.Call("fpath", {}, -1, GET, oss);
        REQUIRE(oss.str() == "fpath /tmp\n");
    }
    for (int i = 0; i != det.size(); ++i) {
        det.setFilePath(prev_val[i], {i});
    }
}

TEST_CASE("fname", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto prev_val = det.getFileNamePrefix();
    {
        std::ostringstream oss;
        proxy.Call("fname", {"somename"}, -1, PUT, oss);
        REQUIRE(oss.str() == "fname somename\n");
    }
    {
        std::ostringstream oss;
        proxy.Call("fname", {}, -1, GET, oss);
        REQUIRE(oss.str() == "fname somename\n");
    }
    {
        std::ostringstream oss;
        proxy.Call("fname", {"run"}, -1, PUT, oss);
        REQUIRE(oss.str() == "fname run\n");
    }
    REQUIRE_THROWS(proxy.Call("fname", {"fdf/dfd"}, -1, PUT));
    REQUIRE_THROWS(proxy.Call("fname", {"fdf dfd"}, -1, PUT));

    for (int i = 0; i != det.size(); ++i) {
        det.setFileNamePrefix(prev_val[i], {i});
    }
}

TEST_CASE("findex", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto prev_val = det.getAcquisitionIndex();
    {
        std::ostringstream oss;
        proxy.Call("findex", {"57"}, -1, PUT, oss);
        REQUIRE(oss.str() == "findex 57\n");
    }
    {
        std::ostringstream oss;
        proxy.Call("findex", {}, -1, GET, oss);
        REQUIRE(oss.str() == "findex 57\n");
    }
    {
        std::ostringstream oss;
        proxy.Call("findex", {"0"}, -1, PUT, oss);
        REQUIRE(oss.str() == "findex 0\n");
    }
    for (int i = 0; i != det.size(); ++i) {
        det.setAcquisitionIndex(prev_val[i], {i});
    }
}

TEST_CASE("fwrite", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto prev_val = det.getFileWrite();
    {
        std::ostringstream oss;
        proxy.Call("fwrite", {"1"}, -1, PUT, oss);
        REQUIRE(oss.str() == "fwrite 1\n");
    }
    {
        std::ostringstream oss;
        proxy.Call("fwrite", {}, -1, GET, oss);
        REQUIRE(oss.str() == "fwrite 1\n");
    }
    {
        std::ostringstream oss;
        proxy.Call("fwrite", {"0"}, -1, PUT, oss);
        REQUIRE(oss.str() == "fwrite 0\n");
    }
    for (int i = 0; i != det.size(); ++i) {
        det.setFileWrite(prev_val[i], {i});
    }
}

TEST_CASE("fmaster", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto prev_val = det.getMasterFileWrite();
    {
        std::ostringstream oss;
        proxy.Call("fmaster", {"0"}, -1, PUT, oss);
        REQUIRE(oss.str() == "fmaster 0\n");
    }
    {
        std::ostringstream oss;
        proxy.Call("fmaster", {}, -1, GET, oss);
        REQUIRE(oss.str() == "fmaster 0\n");
    }
    {
        std::ostringstream oss;
        proxy.Call("fmaster", {"1"}, -1, PUT, oss);
        REQUIRE(oss.str() == "fmaster 1\n");
    }
    det.setMasterFileWrite(prev_val);
}

TEST_CASE("foverwrite", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto prev_val = det.getFileOverWrite();
    {
        std::ostringstream oss;
        proxy.Call("foverwrite", {"1"}, -1, PUT, oss);
        REQUIRE(oss.str() == "foverwrite 1\n");
    }
    {
        std::ostringstream oss;
        proxy.Call("foverwrite", {}, -1, GET, oss);
        REQUIRE(oss.str() == "foverwrite 1\n");
    }
    {
        std::ostringstream oss;
        proxy.Call("foverwrite", {"0"}, -1, PUT, oss);
        REQUIRE(oss.str() == "foverwrite 0\n");
    }
    for (int i = 0; i != det.size(); ++i) {
        det.setFileOverWrite(prev_val[i], {i});
    }
}

TEST_CASE("rx_framesperfile", "[.cmd][.rx]") {
    Detector det;
    CmdProxy proxy(&det);
    auto prev_val = det.getFramesPerFile();
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
    {
        std::ostringstream oss;
        proxy.Call("rx_framesperfile", {"0"}, -1, PUT, oss);
        REQUIRE(oss.str() == "rx_framesperfile 0\n");
    }
    for (int i = 0; i != det.size(); ++i) {
        det.setFramesPerFile(prev_val[i], {i});
    }
}

/* ZMQ Streaming Parameters (Receiver<->Client) */

TEST_CASE("rx_zmqstream", "[.cmd][.rx]") {
    Detector det;
    CmdProxy proxy(&det);
    auto prev_val = det.getRxZmqDataStream();
    {
        std::ostringstream oss;
        proxy.Call("rx_zmqstream", {"1"}, -1, PUT, oss);
        REQUIRE(oss.str() == "rx_zmqstream 1\n");
        REQUIRE(det.getRxZmqDataStream().squash() == true);
    }
    {
        std::ostringstream oss;
        proxy.Call("rx_zmqstream", {}, -1, GET, oss);
        REQUIRE(oss.str() == "rx_zmqstream 1\n");
    }
    {
        std::ostringstream oss;
        proxy.Call("rx_zmqstream", {"0"}, -1, PUT, oss);
        REQUIRE(oss.str() == "rx_zmqstream 0\n");
        REQUIRE(det.getRxZmqDataStream().squash() == false);
    }
    for (int i = 0; i != det.size(); ++i) {
        det.setRxZmqDataStream(prev_val[i], {i});
    }
}

TEST_CASE("rx_zmqfreq", "[.cmd][.rx]") {
    Detector det;
    CmdProxy proxy(&det);
    auto prev_val = det.getRxZmqFrequency();
    {
        std::ostringstream oss;
        proxy.Call("rx_zmqfreq", {"1"}, -1, PUT, oss);
        REQUIRE(oss.str() == "rx_zmqfreq 1\n");
    }
    {
        std::ostringstream oss;
        proxy.Call("rx_zmqfreq", {}, -1, GET, oss);
        REQUIRE(oss.str() == "rx_zmqfreq 1\n");
    }
    {
        std::ostringstream oss;
        proxy.Call("rx_zmqfreq", {"0"}, -1, PUT, oss);
        REQUIRE(oss.str() == "rx_zmqfreq 0\n");
    }
    for (int i = 0; i != det.size(); ++i) {
        det.setRxZmqFrequency(prev_val[i], {i});
    }
}

TEST_CASE("rx_zmqstartfnum", "[.cmd][.rx]") {
    Detector det;
    CmdProxy proxy(&det);
    auto prev_val = det.getRxZmqStartingFrame();
    {
        std::ostringstream oss;
        proxy.Call("rx_zmqstartfnum", {"5"}, -1, PUT, oss);
        REQUIRE(oss.str() == "rx_zmqstartfnum 5\n");
    }
    {
        std::ostringstream oss;
        proxy.Call("rx_zmqstartfnum", {}, -1, GET, oss);
        REQUIRE(oss.str() == "rx_zmqstartfnum 5\n");
    }
    {
        std::ostringstream oss;
        proxy.Call("rx_zmqstartfnum", {"0"}, -1, PUT, oss);
        REQUIRE(oss.str() == "rx_zmqstartfnum 0\n");
    }
    for (int i = 0; i != det.size(); ++i) {
        det.setRxZmqStartingFrame(prev_val[i], {i});
    }
}

TEST_CASE("rx_zmqport", "[.cmd][.rx]") {
    Detector det;
    CmdProxy proxy(&det);
    auto prev_val_zmqport = det.getRxZmqPort();
    auto prev_val_numinterfaces = det.getNumberofUDPInterfaces().tsquash(
        "inconsistent number of udp interfaces to test");

    int socketsperdetector = 1;
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::EIGER) {
        socketsperdetector *= 2;
    } else if (det_type == defs::JUNGFRAU || det_type == defs::MOENCH) {
        proxy.Call("numinterfaces", {"2"}, -1, PUT);
        socketsperdetector *= 2;
    }
    int port = 3500;
    proxy.Call("rx_zmqport", {std::to_string(port)}, -1, PUT);
    for (int i = 0; i != det.size(); ++i) {
        std::ostringstream oss;
        proxy.Call("rx_zmqport", {}, i, GET, oss);
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
    for (int i = 0; i != det.size(); ++i) {
        det.setRxZmqPort(prev_val_zmqport[i], i);
    }
    if (det_type == defs::JUNGFRAU || det_type == defs::MOENCH) {
        det.setNumberofUDPInterfaces(prev_val_numinterfaces);
    }
}

TEST_CASE("rx_zmqip", "[.cmd][.rx]") {
    Detector det;
    CmdProxy proxy(&det);
    auto prev_val = det.getRxZmqIP();
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
    for (int i = 0; i != det.size(); ++i) {
        det.setRxZmqIP(prev_val[i], {i});
    }
}

TEST_CASE("rx_zmqhwm", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto prev_val =
        det.getRxZmqHwm().tsquash("Inconsistent values for rx_zmqhwm to test");
    {
        std::ostringstream oss;
        proxy.Call("rx_zmqhwm", {"50"}, -1, PUT, oss);
        REQUIRE(oss.str() == "rx_zmqhwm 50\n");
    }
    {
        std::ostringstream oss;
        proxy.Call("rx_zmqhwm", {}, -1, GET, oss);
        REQUIRE(oss.str() == "rx_zmqhwm 50\n");
    }
    {
        std::ostringstream oss;
        proxy.Call("rx_zmqhwm", {"0"}, -1, PUT, oss);
        REQUIRE(oss.str() == "rx_zmqhwm 0\n");
    }
    {
        std::ostringstream oss;
        proxy.Call("rx_zmqhwm", {"-1"}, -1, PUT, oss);
        REQUIRE(oss.str() == "rx_zmqhwm -1\n");
    }
    det.setRxZmqHwm(prev_val);
}

/* CTB Specific */

TEST_CASE("rx_dbitlist", "[.cmd][.rx]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::CHIPTESTBOARD) {
        auto prev_val = det.getRxDbitList();
        {
            std::ostringstream oss;
            proxy.Call("rx_dbitlist",
                       {"0", "4", "5", "8", "9", "10", "52", "63"}, -1, PUT,
                       oss);
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
        for (int i = 0; i != det.size(); ++i) {
            det.setRxDbitList(prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(proxy.Call("rx_dbitlist", {}, -1, GET));
    }
}

TEST_CASE("rx_dbitoffset", "[.cmd][.rx]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::CHIPTESTBOARD) {
        auto prev_val = det.getRxDbitOffset();
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
        for (int i = 0; i != det.size(); ++i) {
            det.setRxDbitOffset(prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(proxy.Call("rx_dbitoffset", {}, -1, GET));
    }
}

TEST_CASE("rx_jsonaddheader", "[.cmd][.rx]") {
    Detector det;
    CmdProxy proxy(&det);
    auto prev_val = det.getAdditionalJsonHeader();

    {
        std::ostringstream oss;
        proxy.Call("rx_jsonaddheader", {"key1", "value1", "key2", "value2"}, -1,
                   PUT, oss);
        REQUIRE(oss.str() == "rx_jsonaddheader {key1: value1, key2: value2}\n");
    }
    {
        std::ostringstream oss;
        proxy.Call("rx_jsonaddheader", {}, -1, GET, oss);
        REQUIRE(oss.str() == "rx_jsonaddheader {key1: value1, key2: value2}\n");
    }
    {
        std::ostringstream oss;
        proxy.Call("rx_jsonaddheader", {}, -1, PUT, oss);
        REQUIRE(oss.str() == "rx_jsonaddheader {}\n");
    }
    for (int i = 0; i != det.size(); ++i) {
        det.setAdditionalJsonHeader(prev_val[i], {i});
    }
}

TEST_CASE("rx_jsonpara", "[.cmd][.rx]") {
    Detector det;
    CmdProxy proxy(&det);
    auto prev_val = det.getAdditionalJsonHeader();
    {
        std::ostringstream oss;
        proxy.Call("rx_jsonpara", {"key1", "value1"}, -1, PUT, oss);
        REQUIRE(oss.str() == "rx_jsonpara {key1: value1}\n");
    }
    {
        std::ostringstream oss;
        proxy.Call("rx_jsonpara", {"key1", "value2"}, -1, PUT, oss);
        REQUIRE(oss.str() == "rx_jsonpara {key1: value2}\n");
    }
    {
        std::ostringstream oss;
        proxy.Call("rx_jsonpara", {"key1"}, -1, GET, oss);
        REQUIRE(oss.str() == "rx_jsonpara value2\n");
    }
    {
        std::ostringstream oss;
        proxy.Call("rx_jsonpara", {"key1"}, -1, PUT, oss);
        REQUIRE(oss.str() == "rx_jsonpara key1 deleted\n");
    }
    REQUIRE_THROWS(proxy.Call("rx_jsonpara", {"key1"}, -1, GET));
    for (int i = 0; i != det.size(); ++i) {
        det.setAdditionalJsonHeader(prev_val[i], {i});
    }
}

/* Insignificant */

} // namespace sls
