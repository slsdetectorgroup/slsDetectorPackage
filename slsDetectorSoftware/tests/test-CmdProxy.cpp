// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "CmdProxy.h"
#include "catch.hpp"
#include "sls/Detector.h"
#include "sls/file_utils.h"
#include "sls/sls_detector_defs.h"

#include <chrono>
#include <sstream>
#include <thread>

#include "tests/globals.h"

namespace sls {

using test::GET;
using test::PUT;

TEST_CASE("Calling help doesn't throw or cause segfault") {
    // Dont add [.cmd] tag this should run with normal tests
    CmdProxy proxy(nullptr);
    auto commands = proxy.GetProxyCommands();
    std::ostringstream os;
    for (const auto &cmd : commands)
        REQUIRE_NOTHROW(
            proxy.Call(cmd, {}, -1, slsDetectorDefs::HELP_ACTION, os));
}

TEST_CASE("Unknown command", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    REQUIRE_THROWS(proxy.Call("vsaevrreavv", {}, -1, PUT));
}

/* configuration */

TEST_CASE("config", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    // put only
    REQUIRE_THROWS(proxy.Call("config", {}, -1, GET));
}

// free: not testing

TEST_CASE("parameters", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    // put only
    REQUIRE_THROWS(proxy.Call("parameters", {}, -1, GET));
    /*
        auto prev_val = det.getNumberOfFrames().tsquash("Number of frames has to
       be same to test");
        {
            system("echo 'frames 2' > /tmp/tempsetup.det ");
            std::ostringstream oss;
            proxy.Call("parameters", {"/tmp/tempsetup.det"}, -1, PUT, oss);
            REQUIRE(oss.str() == "parameters /tmp/tempsetup.det\n");
            REQUIRE(det.getNumberOfFrames().tsquash("failed") == 2);
        }
        {
            system("echo '0:frames 1' > /tmp/tempsetup.det ");
            std::ostringstream oss;
            proxy.Call("parameters", {"/tmp/tempsetup.det"}, -1, PUT, oss);
            REQUIRE(oss.str() == "parameters /tmp/tempsetup.det\n");
            REQUIRE(det.getNumberOfFrames({0}).tsquash("failed") == 1);
        }
        det.setNumberOfFrames(prev_val);
        */
}

TEST_CASE("hostname", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    REQUIRE_NOTHROW(proxy.Call("hostname", {}, -1, GET));
}

// virtual: not testing

TEST_CASE("versions", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    REQUIRE_NOTHROW(proxy.Call("versions", {}, -1, GET));
    REQUIRE_THROWS(proxy.Call("versions", {"0"}, -1, PUT));
}

TEST_CASE("packageversion", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    REQUIRE_NOTHROW(proxy.Call("packageversion", {}, -1, GET));
    REQUIRE_THROWS(proxy.Call("packageversion", {"0"}, -1, PUT));
}

TEST_CASE("clientversion", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    REQUIRE_NOTHROW(proxy.Call("clientversion", {}, -1, GET));
    REQUIRE_THROWS(proxy.Call("clientversion", {"0"}, -1, PUT));
}

TEST_CASE("firmwareversion", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    REQUIRE_NOTHROW(proxy.Call("firmwareversion", {}, -1, GET));
    REQUIRE_THROWS(proxy.Call("firmwareversion", {"0"}, -1, PUT));
}

TEST_CASE("detectorserverversion", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    REQUIRE_NOTHROW(proxy.Call("detectorserverversion", {}, -1, GET));
    REQUIRE_THROWS(proxy.Call("detectorserverversion", {"0"}, -1, PUT));
}

TEST_CASE("hardwareversion", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type != defs::EIGER) {
        REQUIRE_NOTHROW(proxy.Call("hardwareversion", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("hardwareversion", {"0"}, -1, PUT));
    } else {
        REQUIRE_THROWS(proxy.Call("hardwareversion", {"0"}, -1, PUT));
        REQUIRE_THROWS(proxy.Call("hardwareversion", {}, -1, GET));
    }
}

TEST_CASE("kernelversion", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    REQUIRE_NOTHROW(proxy.Call("kernelversion", {}, -1, GET));
    REQUIRE_THROWS(proxy.Call("kernelversion", {"0"}, -1, PUT));
}

TEST_CASE("serialnumber", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::EIGER) {
        REQUIRE_THROWS(proxy.Call("serialnumber", {}, -1, GET));
    } else {
        REQUIRE_NOTHROW(proxy.Call("serialnumber", {}, -1, GET));
    }
}

TEST_CASE("moduleid", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::GOTTHARD2 || det_type == defs::MYTHEN3 ||
        det_type == defs::EIGER || det_type == defs::JUNGFRAU) {
        REQUIRE_NOTHROW(proxy.Call("moduleid", {}, -1, GET));
    } else {
        REQUIRE_THROWS(proxy.Call("moduleid", {}, -1, GET));
    }
}

TEST_CASE("type", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto dt = det.getDetectorType().squash();

    std::ostringstream oss;
    proxy.Call("type", {}, -1, GET, oss);
    auto ans = oss.str().erase(0, strlen("type "));
    REQUIRE(ans == ToString(dt) + '\n');
    // REQUIRE(dt == test::type);
}

TEST_CASE("detsize", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    REQUIRE_NOTHROW(proxy.Call("detsize", {}, -1, GET));
}

TEST_CASE("settingslist", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::CHIPTESTBOARD) {
        REQUIRE_THROWS(proxy.Call("settingslist", {}, -1, GET));
    } else {
        REQUIRE_NOTHROW(proxy.Call("settingslist", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("settingslist", {}, -1, PUT));
    }
}

TEST_CASE("settings", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    std::vector<std::string> allSett;
    allSett.push_back("standard");
    allSett.push_back("fast");
    allSett.push_back("highgain");
    allSett.push_back("dynamicgain");
    allSett.push_back("lowgain");
    allSett.push_back("mediumgain");
    allSett.push_back("veryhighgain");
    allSett.push_back("highgain0");
    allSett.push_back("fixgain1");
    allSett.push_back("fixgain2");
    allSett.push_back("verylowgain");
    allSett.push_back("g1_hg");
    allSett.push_back("g1_lg");
    allSett.push_back("g2_hc_hg");
    allSett.push_back("g2_hc_lg");
    allSett.push_back("g2_lc_hg");
    allSett.push_back("g2_lc_lg");
    allSett.push_back("g4_hg");
    allSett.push_back("g4_lg");
    allSett.push_back("forceswitchg1");
    allSett.push_back("forceswitchg2");
    allSett.push_back("gain0");

    std::vector<std::string> sett;
    switch (det_type) {
    case defs::JUNGFRAU:
        sett.push_back("gain0");
        sett.push_back("highgain0");
        break;
    case defs::GOTTHARD:
        sett.push_back("highgain");
        sett.push_back("dynamicgain");
        sett.push_back("lowgain");
        sett.push_back("mediumgain");
        sett.push_back("veryhighgain");
        break;
    case defs::GOTTHARD2:
        sett.push_back("dynamicgain");
        sett.push_back("fixgain1");
        sett.push_back("fixgain2");
        break;
    case defs::MOENCH:
        sett.push_back("g1_hg");
        sett.push_back("g1_lg");
        sett.push_back("g2_hc_hg");
        sett.push_back("g2_hc_lg");
        sett.push_back("g2_lc_hg");
        sett.push_back("g2_lc_lg");
        sett.push_back("g4_hg");
        sett.push_back("g4_lg");
        break;
    case defs::MYTHEN3:
        sett.push_back("standard");
        sett.push_back("fast");
        sett.push_back("highgain");
        break;
    default:
        if (det_type == defs::EIGER) {
            // FIXME: need to remove when settings removed
            REQUIRE_NOTHROW(proxy.Call("settings", {}, -1, GET));
            REQUIRE_THROWS(proxy.Call("settings", {"standard"}, -1, PUT));
        } else {
            REQUIRE_THROWS(proxy.Call("settings", {}, -1, GET));
        }
        return;
    }

    auto prev_val = det.getSettings();
    for (auto &it : sett) {
        {
            std::ostringstream oss;
            proxy.Call("settings", {it}, -1, PUT, oss);
            REQUIRE(oss.str() == "settings " + it + "\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("settings", {}, -1, GET, oss);
            REQUIRE(oss.str() == "settings " + it + "\n");
        }
    }
    for (auto &it : allSett) {
        if (std::find(sett.begin(), sett.end(), it) == sett.end()) {
            REQUIRE_THROWS(proxy.Call("settings", {it}, -1, PUT));
        }
    }
    for (int i = 0; i != det.size(); ++i) {
        if (prev_val[i] != defs::UNDEFINED &&
            prev_val[i] != defs::UNINITIALIZED) {
            det.setSettings(prev_val[i], {i});
        }
    }
}

TEST_CASE("threshold", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);

    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::EIGER) {
        auto prev_threshold = det.getThresholdEnergy();
        auto prev_energies =
            det.getTrimEnergies().tsquash("inconsistent trim energies to test");
        if (!prev_energies.empty()) {
            std::string senergy = std::to_string(prev_energies[0]);
            std::ostringstream oss1, oss2;
            proxy.Call("threshold", {senergy, "standard"}, -1, PUT, oss1);
            REQUIRE(oss1.str() == "threshold [" + senergy + ", standard]\n");
            proxy.Call("threshold", {}, -1, GET, oss2);
            REQUIRE(oss2.str() == "threshold " + senergy + "\n");

            REQUIRE_THROWS(proxy.Call(
                "threshold", {senergy, senergy, senergy, "standard"}, -1, PUT));
            REQUIRE_THROWS(
                proxy.Call("threshold", {senergy, "undefined"}, -1, PUT));

            det.setTrimEnergies(prev_energies);
            for (int i = 0; i != det.size(); ++i) {
                if (prev_threshold[i] >= 0) {
                    det.setThresholdEnergy(prev_threshold[i], defs::STANDARD,
                                           true, {i});
                }
            }
        }
        REQUIRE_NOTHROW(proxy.Call("threshold", {}, -1, GET));
    } else if (det_type == defs::MYTHEN3) {
        auto prev_threshold = det.getAllThresholdEnergy();
        auto prev_settings =
            det.getSettings().tsquash("inconsistent settings to test");
        auto prev_energies =
            det.getTrimEnergies().tsquash("inconsistent trim energies to test");
        if (!prev_energies.empty()) {
            std::string senergy = std::to_string(prev_energies[0]);
            std::ostringstream oss1, oss2;
            proxy.Call("threshold", {senergy, "standard"}, -1, PUT, oss1);
            REQUIRE(oss1.str() == "threshold [" + senergy + ", standard]\n");
            proxy.Call("threshold", {}, -1, GET, oss2);
            REQUIRE(oss2.str() == "threshold [" + senergy + ", " + senergy +
                                      ", " + senergy + "]\n");
            std::string senergy2 = std::to_string(prev_energies[1]);
            std::string senergy3 = std::to_string(prev_energies[2]);
            std::ostringstream oss3, oss4;
            proxy.Call("threshold", {senergy, senergy2, senergy3, "standard"},
                       -1, PUT, oss3);
            REQUIRE(oss3.str() == "threshold [" + senergy + ", " + senergy2 +
                                      ", " + senergy3 + ", standard]\n");
            proxy.Call("threshold", {}, -1, GET, oss4);
            REQUIRE(oss4.str() == "threshold [" + senergy + ", " + senergy2 +
                                      ", " + senergy3 + "]\n");

            REQUIRE_THROWS(proxy.Call("threshold",
                                      {senergy, senergy, "standard"}, -1, PUT));
            REQUIRE_THROWS(
                proxy.Call("threshold", {senergy, "undefined"}, -1, PUT));
            REQUIRE_NOTHROW(proxy.Call("threshold", {senergy}, -1, PUT));
            REQUIRE_NOTHROW(proxy.Call("threshold",
                                       {senergy, senergy2, senergy3}, -1, PUT));
            det.setTrimEnergies(prev_energies);
            for (int i = 0; i != det.size(); ++i) {
                if (prev_threshold[i][0] >= 0) {
                    std::cout << "prev cvalues:" << ToString(prev_threshold[i])
                              << std::endl;
                    det.setThresholdEnergy(prev_threshold[i], prev_settings,
                                           true, {i});
                }
            }
        }
        REQUIRE_NOTHROW(proxy.Call("threshold", {}, -1, GET));
    } else {
        REQUIRE_THROWS(proxy.Call("threshold", {}, -1, GET));
    }
}

TEST_CASE("thresholdnotb", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);

    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::EIGER) {
        auto prev_threshold = det.getThresholdEnergy();
        auto prev_energies =
            det.getTrimEnergies().tsquash("inconsistent trim energies to test");
        if (!prev_energies.empty()) {
            std::string senergy = std::to_string(prev_energies[0]);
            std::ostringstream oss1, oss2;
            proxy.Call("thresholdnotb", {senergy, "standard"}, -1, PUT, oss1);
            REQUIRE(oss1.str() ==
                    "thresholdnotb [" + senergy + ", standard]\n");
            proxy.Call("threshold", {}, -1, GET, oss2);
            REQUIRE(oss2.str() == "threshold " + senergy + "\n");
            REQUIRE_THROWS(proxy.Call("thresholdnotb",
                                      {senergy, senergy, senergy, "standard"},
                                      -1, PUT));
            REQUIRE_THROWS(
                proxy.Call("thresholdnotb", {senergy, "undefined"}, -1, PUT));
            det.setTrimEnergies(prev_energies);
            for (int i = 0; i != det.size(); ++i) {
                if (prev_threshold[i] >= 0) {
                    det.setThresholdEnergy(prev_threshold[i], defs::STANDARD,
                                           false, {i});
                }
            }
        }
        REQUIRE_NOTHROW(proxy.Call("threshold", {}, -1, GET));
    } else if (det_type == defs::MYTHEN3) {
        auto prev_threshold = det.getAllThresholdEnergy();
        auto prev_settings =
            det.getSettings().tsquash("inconsistent settings to test");
        auto prev_energies =
            det.getTrimEnergies().tsquash("inconsistent trim energies to test");
        if (!prev_energies.empty()) {
            std::string senergy = std::to_string(prev_energies[0]);
            std::ostringstream oss1, oss2;
            proxy.Call("thresholdnotb", {senergy, "standard"}, -1, PUT, oss1);
            REQUIRE(oss1.str() ==
                    "thresholdnotb [" + senergy + ", standard]\n");
            proxy.Call("threshold", {}, -1, GET, oss2);
            REQUIRE(oss2.str() == "threshold [" + senergy + ", " + senergy +
                                      ", " + senergy + "]\n");
            std::string senergy2 = std::to_string(prev_energies[1]);
            std::string senergy3 = std::to_string(prev_energies[2]);
            std::ostringstream oss3, oss4;
            proxy.Call("thresholdnotb",
                       {senergy, senergy2, senergy3, "standard"}, -1, PUT,
                       oss3);
            REQUIRE(oss3.str() == "thresholdnotb [" + senergy + ", " +
                                      senergy2 + ", " + senergy3 +
                                      ", standard]\n");
            proxy.Call("threshold", {}, -1, GET, oss4);
            REQUIRE(oss4.str() == "threshold [" + senergy + ", " + senergy2 +
                                      ", " + senergy3 + "]\n");

            REQUIRE_THROWS(proxy.Call("thresholdnotb",
                                      {senergy, senergy, "standard"}, -1, PUT));
            REQUIRE_THROWS(
                proxy.Call("thresholdnotb", {senergy, "undefined"}, -1, PUT));
            REQUIRE_NOTHROW(proxy.Call("thresholdnotb", {senergy}, -1, PUT));
            REQUIRE_NOTHROW(proxy.Call("thresholdnotb",
                                       {senergy, senergy2, senergy3}, -1, PUT));
            det.setTrimEnergies(prev_energies);
            for (int i = 0; i != det.size(); ++i) {
                if (prev_threshold[i][0] >= 0) {
                    det.setThresholdEnergy(prev_threshold[i], prev_settings,
                                           true, {i});
                }
            }
        }
        REQUIRE_NOTHROW(proxy.Call("threshold", {}, -1, GET));
    } else {
        REQUIRE_THROWS(proxy.Call("thresholdnotb", {}, -1, GET));
    }
}

TEST_CASE("settingspath", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto prev_val = det.getSettingsPath();
    {
        std::ostringstream oss1, oss2;
        proxy.Call("settingspath", {"/tmp"}, -1, PUT, oss1);
        REQUIRE(oss1.str() == "settingspath /tmp\n");
        proxy.Call("settingspath", {}, -1, GET, oss2);
        REQUIRE(oss2.str() == "settingspath /tmp\n");
    }
    for (int i = 0; i != det.size(); ++i) {
        det.setSettingsPath(prev_val[i], {i});
    }
}

TEST_CASE("trimbits", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    REQUIRE_THROWS(proxy.Call("trimbits", {}, -1, GET));
}

TEST_CASE("trimval", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::MYTHEN3 || det_type == defs::EIGER) {
        auto prev_val = det.getAllTrimbits();
        {
            std::ostringstream oss;
            proxy.Call("trimval", {"63"}, -1, PUT, oss);
            REQUIRE(oss.str() == "trimval 63\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("trimval", {"0"}, -1, PUT, oss);
            REQUIRE(oss.str() == "trimval 0\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("trimval", {}, -1, GET, oss);
            REQUIRE(oss.str() == "trimval 0\n");
        }
        REQUIRE_THROWS(proxy.Call("trimval", {"64"}, -1, PUT));
        REQUIRE_THROWS(proxy.Call("trimval", {"-2"}, -1, PUT));
        for (int i = 0; i != det.size(); ++i) {
            if (prev_val[i] != -1) {
                det.setAllTrimbits(prev_val[i], {i});
            }
        }
    } else {
        REQUIRE_THROWS(proxy.Call("trimval", {}, -1, GET));
    }
}

TEST_CASE("trimen", "[.cmd][.this]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::EIGER || det_type == defs::MYTHEN3) {
        auto previous = det.getTrimEnergies();
        std::ostringstream oss1, oss2;
        proxy.Call("trimen", {"4500", "5400", "6400"}, -1, PUT, oss1);
        REQUIRE(oss1.str() == "trimen [4500, 5400, 6400]\n");
        proxy.Call("trimen", {}, -1, GET, oss2);
        REQUIRE(oss2.str() == "trimen [4500, 5400, 6400]\n");

        for (int i = 0; i != det.size(); ++i) {
            det.setTrimEnergies(previous[i], {i});
        }
    } else {
        REQUIRE_THROWS(proxy.Call("trimen", {"4500", "5400", "6400"}, -1, PUT));
        REQUIRE_THROWS(proxy.Call("trimen", {}, -1, GET));
    }
}

TEST_CASE("gappixels", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::JUNGFRAU || det_type == defs::EIGER) {
        auto prev_val = det.getGapPixelsinCallback();
        {
            std::ostringstream oss;
            proxy.Call("gappixels", {"1"}, -1, PUT, oss);
            REQUIRE(oss.str() == "gappixels 1\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("gappixels", {"0"}, -1, PUT, oss);
            REQUIRE(oss.str() == "gappixels 0\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("gappixels", {}, -1, GET, oss);
            REQUIRE(oss.str() == "gappixels 0\n");
        }
        det.setGapPixelsinCallback(prev_val);
    } else {
        {
            std::ostringstream oss;
            proxy.Call("gappixels", {}, -1, GET, oss);
            REQUIRE(oss.str() == "gappixels 0\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("gappixels", {"0"}, -1, PUT, oss);
            REQUIRE(oss.str() == "gappixels 0\n");
        }
        REQUIRE_THROWS(proxy.Call("gappixels", {"1"}, -1, PUT));
    }
}

TEST_CASE("fliprows", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    bool jungfrauhw2 = false;
    if (det_type == defs::JUNGFRAU &&
        ((det.getHardwareVersion().tsquash(
              "inconsistent serial number to test") == "2.0"))) {
        jungfrauhw2 = true;
    }
    if (det_type == defs::EIGER || jungfrauhw2) {
        auto previous = det.getFlipRows();
        auto previous_numudp = det.getNumberofUDPInterfaces().tsquash(
            "inconsistent number of udp interfaces to test");
        if (det_type == defs::JUNGFRAU) {
            det.setNumberofUDPInterfaces(2);
        }
        std::ostringstream oss1, oss2, oss3;
        proxy.Call("fliprows", {"1"}, -1, PUT, oss1);
        REQUIRE(oss1.str() == "fliprows 1\n");
        proxy.Call("fliprows", {}, -1, GET, oss2);
        REQUIRE(oss2.str() == "fliprows 1\n");
        proxy.Call("fliprows", {"0"}, -1, PUT, oss3);
        REQUIRE(oss3.str() == "fliprows 0\n");
        for (int i = 0; i != det.size(); ++i) {
            det.setFlipRows(previous[i], {i});
        }
        if (det_type == defs::JUNGFRAU) {
            det.setNumberofUDPInterfaces(previous_numudp);
        }
    } else {
        REQUIRE_THROWS(proxy.Call("fliprows", {}, -1, GET));
    }
}

TEST_CASE("master", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::EIGER || det_type == defs::MYTHEN3 ||
        det_type == defs::GOTTHARD || det_type == defs::GOTTHARD2 ||
        det_type == defs::JUNGFRAU) {
        REQUIRE_NOTHROW(proxy.Call("master", {}, -1, GET));
        if (det_type == defs::EIGER || det_type == defs::GOTTHARD2 ||
            det_type == defs::JUNGFRAU) {
            // get previous master
            int prevMaster = 0;
            {
                auto previous = det.getMaster();
                for (int i = 0; i != det.size(); ++i) {
                    if (previous[i] == 1) {
                        prevMaster = i;
                        break;
                    }
                }
            }
            {
                std::ostringstream oss1;
                proxy.Call("master", {"0"}, 0, PUT, oss1);
                REQUIRE(oss1.str() == "master 0\n");
            }
            {
                std::ostringstream oss1;
                proxy.Call("master", {"1"}, 0, PUT, oss1);
                REQUIRE(oss1.str() == "master 1\n");
            }
            if (det.size() > 1) {
                REQUIRE_THROWS(proxy.Call("master", {"1"}, -1, PUT));
            }
            // set all to slaves, and then master
            for (int i = 0; i != det.size(); ++i) {
                det.setMaster(0, {i});
            }
            det.setMaster(1, prevMaster);
        }
    } else {
        REQUIRE_THROWS(proxy.Call("master", {}, -1, GET));
    }
}

TEST_CASE("badchannels", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::GOTTHARD2 || det_type == defs::MYTHEN3) {
        auto prev = det.getBadChannels();

        REQUIRE_THROWS(proxy.Call("badchannels", {}, -1, GET));

        std::string fname_put =
            getAbsolutePathFromCurrentProcess(TEST_FILE_NAME_BAD_CHANNELS);
        std::string fname_get = "/tmp/sls_test_channels.txt";

        REQUIRE_NOTHROW(proxy.Call("badchannels", {fname_put}, 0, PUT));
        REQUIRE_NOTHROW(proxy.Call("badchannels", {fname_get}, 0, GET));
        auto list = getChannelsFromFile(fname_get);
        std::vector<int> expected = {0, 12, 15, 40, 41, 42, 43, 44, 1279};
        REQUIRE(list == expected);

        REQUIRE_NOTHROW(proxy.Call("badchannels", {"none"}, 0, PUT));
        REQUIRE_NOTHROW(proxy.Call("badchannels", {fname_get}, 0, GET));
        list = getChannelsFromFile(fname_get);
        REQUIRE(list.empty());

        REQUIRE_NOTHROW(proxy.Call("badchannels", {fname_put}, 0, PUT));

        REQUIRE_NOTHROW(proxy.Call("badchannels", {"0"}, 0, PUT));
        REQUIRE_NOTHROW(proxy.Call("badchannels", {fname_get}, 0, GET));
        list = getChannelsFromFile(fname_get);
        REQUIRE(list.empty());

        REQUIRE_NOTHROW(proxy.Call("badchannels", {"12"}, 0, PUT));
        REQUIRE_NOTHROW(proxy.Call("badchannels", {fname_get}, 0, GET));
        list = getChannelsFromFile(fname_get);
        expected = {12};
        REQUIRE(list == expected);

        REQUIRE_NOTHROW(proxy.Call(
            "badchannels", {"0", "12,", "15", "43", "40:45", "1279"}, 0, PUT));
        REQUIRE_NOTHROW(proxy.Call("badchannels", {fname_get}, 0, GET));
        list = getChannelsFromFile(fname_get);
        expected = {0, 12, 15, 40, 41, 42, 43, 44, 1279};
        REQUIRE(list == expected);

        det.setBadChannels(prev);

    } else {
        REQUIRE_THROWS(proxy.Call("badchannels", {}, -1, GET));
    }
}

/* acquisition parameters */

// acquire: not testing

TEST_CASE("frames", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto prev_val =
        det.getNumberOfFrames().tsquash("#frames must be same to test");
    {
        std::ostringstream oss;
        proxy.Call("frames", {"1000"}, -1, PUT, oss);
        REQUIRE(oss.str() == "frames 1000\n");
    }
    {
        std::ostringstream oss;
        proxy.Call("frames", {}, -1, GET, oss);
        REQUIRE(oss.str() == "frames 1000\n");
    }
    {
        std::ostringstream oss;
        proxy.Call("frames", {"1"}, -1, PUT, oss);
        REQUIRE(oss.str() == "frames 1\n");
    }
    REQUIRE_THROWS(proxy.Call("frames", {"0"}, -1, PUT));
    det.setNumberOfFrames(prev_val);
}

TEST_CASE("triggers", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto prev_val =
        det.getNumberOfTriggers().tsquash("#triggers must be same to test");
    {
        std::ostringstream oss;
        proxy.Call("triggers", {"1000"}, -1, PUT, oss);
        REQUIRE(oss.str() == "triggers 1000\n");
    }
    {
        std::ostringstream oss;
        proxy.Call("triggers", {}, -1, GET, oss);
        REQUIRE(oss.str() == "triggers 1000\n");
    }
    {
        std::ostringstream oss;
        proxy.Call("triggers", {"1"}, -1, PUT, oss);
        REQUIRE(oss.str() == "triggers 1\n");
    }
    REQUIRE_THROWS(proxy.Call("triggers", {"0"}, -1, PUT));
    det.setNumberOfTriggers(prev_val);
}

TEST_CASE("exptime", "[.cmd][.time]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    std::chrono::nanoseconds prev_val;
    if (det_type != defs::MYTHEN3) {
        prev_val = det.getExptime().tsquash("inconsistent exptime to test");
    } else {
        auto t =
            det.getExptimeForAllGates().tsquash("inconsistent exptime to test");
        if (t[0] != t[1] || t[1] != t[2]) {
            throw RuntimeError("inconsistent exptime for all gates");
        }
        prev_val = t[0];
    }
    {
        std::ostringstream oss;
        proxy.Call("exptime", {"0.05"}, -1, PUT, oss);
        REQUIRE(oss.str() == "exptime 0.05\n");
    }
    if (det_type != defs::MYTHEN3) {
        std::ostringstream oss;
        proxy.Call("exptime", {}, -1, GET, oss);
        REQUIRE(oss.str() == "exptime 50ms\n");
    }
    {
        std::ostringstream oss;
        proxy.Call("exptime", {"1s"}, -1, PUT, oss);
        REQUIRE(oss.str() == "exptime 1s\n");
    }
    if (det_type != defs::JUNGFRAU) {
        {
            std::ostringstream oss;
            proxy.Call("exptime", {"0"}, -1, PUT, oss);
            REQUIRE(oss.str() == "exptime 0\n");
        }
        {
            // Get exptime of single module
            std::ostringstream oss;
            proxy.Call("exptime", {}, 0, GET, oss);
            if (det_type == defs::MYTHEN3) {
                REQUIRE(oss.str() == "exptime [0ns, 0ns, 0ns]\n");
            } else {
                REQUIRE(oss.str() == "exptime 0ns\n");
            }
        }
    }
    det.setExptime(-1, prev_val);
}

TEST_CASE("period", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto prev_val = det.getPeriod();
    {
        std::ostringstream oss;
        proxy.Call("period", {"1.25s"}, -1, PUT, oss);
        REQUIRE(oss.str() == "period 1.25s\n");
    }
    {
        std::ostringstream oss;
        proxy.Call("period", {}, -1, GET, oss);
        REQUIRE(oss.str() == "period 1.25s\n");
    }
    {
        std::ostringstream oss;
        proxy.Call("period", {"0"}, -1, PUT, oss);
        REQUIRE(oss.str() == "period 0\n");
    }
    for (int i = 0; i != det.size(); ++i) {
        det.setPeriod(prev_val[i], {i});
    }
}

TEST_CASE("delay", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::EIGER) {
        REQUIRE_THROWS(proxy.Call("delay", {"1"}, -1, PUT));
        REQUIRE_THROWS(proxy.Call("delay", {}, -1, GET));
    } else if (det_type == defs::GOTTHARD) {
        // extra delays for master (can throw when setting)
        REQUIRE_NOTHROW(proxy.Call("delay", {}, -1, GET));
    } else {
        auto prev_val = det.getDelayAfterTrigger();
        {
            std::ostringstream oss;
            proxy.Call("delay", {"1.25s"}, -1, PUT, oss);
            REQUIRE(oss.str() == "delay 1.25s\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("delay", {}, -1, GET, oss);
            REQUIRE(oss.str() == "delay 1.25s\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("delay", {"0s"}, -1, PUT, oss);
            REQUIRE(oss.str() == "delay 0s\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setDelayAfterTrigger(prev_val[i], {i});
        }
    }
}

TEST_CASE("framesl", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::EIGER) {
        REQUIRE_THROWS(proxy.Call("framesl", {}, -1, GET));
    } else {
        REQUIRE_NOTHROW(proxy.Call("framesl", {}, -1, GET));
    }
}

TEST_CASE("triggersl", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::EIGER) {
        REQUIRE_THROWS(proxy.Call("triggersl", {}, -1, GET));
    } else {
        REQUIRE_NOTHROW(proxy.Call("triggersl", {}, -1, GET));
    }
}

TEST_CASE("delayl", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    switch (det_type) {
    case defs::EIGER:
    case defs::CHIPTESTBOARD:
    case defs::MOENCH:
    case defs::GOTTHARD2:
    case defs::MYTHEN3:
        REQUIRE_THROWS(proxy.Call("delayl", {}, -1, GET));
        break;
    default:
        REQUIRE_NOTHROW(proxy.Call("delayl", {}, -1, GET));
        break;
    }
}

TEST_CASE("periodl", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    switch (det_type) {
    case defs::EIGER:
    case defs::CHIPTESTBOARD:
    case defs::MOENCH:
    case defs::GOTTHARD2:
    case defs::MYTHEN3:
        REQUIRE_THROWS(proxy.Call("periodl", {}, -1, GET));
        break;
    default:
        REQUIRE_NOTHROW(proxy.Call("periodl", {}, -1, GET));
        break;
    }
}

TEST_CASE("dr", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::EIGER) {
        auto dr = det.getDynamicRange().squash();
        std::array<int, 4> vals{4, 8, 16, 32};
        for (const auto val : vals) {
            std::ostringstream oss1, oss2;
            proxy.Call("dr", {std::to_string(val)}, -1, PUT, oss1);
            REQUIRE(oss1.str() == "dr " + std::to_string(val) + '\n');
            proxy.Call("dr", {}, -1, GET, oss2);
            REQUIRE(oss2.str() == "dr " + std::to_string(val) + '\n');
        }
        det.setDynamicRange(dr);
    } else if (det_type == defs::MYTHEN3) {
        auto dr = det.getDynamicRange().squash();
        // not updated in firmware to support dr 1
        std::array<int, 3> vals{8, 16, 32};
        for (const auto val : vals) {
            std::ostringstream oss1, oss2;
            proxy.Call("dr", {std::to_string(val)}, -1, PUT, oss1);
            REQUIRE(oss1.str() == "dr " + std::to_string(val) + '\n');
            proxy.Call("dr", {}, -1, GET, oss2);
            REQUIRE(oss2.str() == "dr " + std::to_string(val) + '\n');
        }
        det.setDynamicRange(dr);
    } else {
        // For the other detectors we should get an error message
        // except for dr 16
        REQUIRE_THROWS(proxy.Call("dr", {"4"}, -1, PUT));
        REQUIRE_THROWS(proxy.Call("dr", {"8"}, -1, PUT));
        REQUIRE_THROWS(proxy.Call("dr", {"32"}, -1, PUT));

        std::ostringstream oss1, oss2;
        proxy.Call("dr", {"16"}, -1, PUT, oss1);
        REQUIRE(oss1.str() == "dr 16\n");
        proxy.Call("dr", {"16"}, -1, PUT, oss2);
        REQUIRE(oss2.str() == "dr 16\n");
    }
}

TEST_CASE("drlist", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    REQUIRE_NOTHROW(proxy.Call("drlist", {}, -1, GET));
    REQUIRE_THROWS(proxy.Call("drlist", {}, -1, PUT));
}

TEST_CASE("timing", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto prev_val = det.getTimingMode();
    det.setTimingMode(defs::AUTO_TIMING);
    {
        std::ostringstream oss1, oss2;
        proxy.Call("timing", {"auto"}, -1, PUT, oss1);
        REQUIRE(oss1.str() == "timing auto\n");
        proxy.Call("timing", {}, -1, GET, oss2);
        REQUIRE(oss2.str() == "timing auto\n");
    }
    {
        std::ostringstream oss1, oss2;
        proxy.Call("timing", {"trigger"}, -1, PUT, oss1);
        REQUIRE(oss1.str() == "timing trigger\n");
        proxy.Call("timing", {}, -1, GET, oss2);
        REQUIRE(oss2.str() == "timing trigger\n");
    }
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::EIGER) {
        {
            std::ostringstream oss1, oss2;
            proxy.Call("timing", {"gating"}, -1, PUT, oss1);
            REQUIRE(oss1.str() == "timing gating\n");
            proxy.Call("timing", {}, -1, GET, oss2);
            REQUIRE(oss2.str() == "timing gating\n");
        }
        {
            std::ostringstream oss1, oss2;
            proxy.Call("timing", {"burst_trigger"}, -1, PUT, oss1);
            REQUIRE(oss1.str() == "timing burst_trigger\n");
            proxy.Call("timing", {}, -1, GET, oss2);
            REQUIRE(oss2.str() == "timing burst_trigger\n");
        }
        REQUIRE_THROWS(proxy.Call("timing", {"trigger_gating"}, -1, PUT));
    } else if (det_type == defs::MYTHEN3) {
        {
            std::ostringstream oss1, oss2;
            proxy.Call("timing", {"gating"}, -1, PUT, oss1);
            REQUIRE(oss1.str() == "timing gating\n");
            proxy.Call("timing", {}, -1, GET, oss2);
            REQUIRE(oss2.str() == "timing gating\n");
        }
        {
            std::ostringstream oss1, oss2;
            proxy.Call("timing", {"trigger_gating"}, -1, PUT, oss1);
            REQUIRE(oss1.str() == "timing trigger_gating\n");
            proxy.Call("timing", {}, -1, GET, oss2);
            REQUIRE(oss2.str() == "timing trigger_gating\n");
        }
        REQUIRE_THROWS(proxy.Call("timing", {"burst_trigger"}, -1, PUT));
    } else {
        REQUIRE_THROWS(proxy.Call("timing", {"gating"}, -1, PUT));
        REQUIRE_THROWS(proxy.Call("timing", {"burst_trigger"}, -1, PUT));
        REQUIRE_THROWS(proxy.Call("timing", {"trigger_gating"}, -1, PUT));
    }
    for (int i = 0; i != det.size(); ++i) {
        det.setTimingMode(prev_val[i], {i});
    }
}

TEST_CASE("timinglist", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    REQUIRE_NOTHROW(proxy.Call("timinglist", {}, -1, GET));
    REQUIRE_THROWS(proxy.Call("timinglist", {}, -1, PUT));
}

TEST_CASE("readoutspeed", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::EIGER || det_type == defs::JUNGFRAU ||
        det_type == defs::GOTTHARD2) {
        auto prev_val = det.getReadoutSpeed();

        // full speed for jungfrau only works for new boards (chipv1.1 is with
        // new board [hw1.0 and chipv1.0 not tested here])
        if ((det_type == defs::JUNGFRAU &&
             det.getChipVersion().squash() * 10 == 11) ||
            (det_type == defs::EIGER)) {
            std::ostringstream oss1, oss2, oss3, oss4;
            proxy.Call("readoutspeed", {"0"}, -1, PUT, oss1);
            REQUIRE(oss1.str() == "readoutspeed full_speed\n");
            proxy.Call("readoutspeed", {}, -1, GET, oss2);
            REQUIRE(oss2.str() == "readoutspeed full_speed\n");
            proxy.Call("readoutspeed", {"full_speed"}, -1, PUT, oss3);
            REQUIRE(oss3.str() == "readoutspeed full_speed\n");
            proxy.Call("readoutspeed", {}, -1, GET, oss4);
            REQUIRE(oss4.str() == "readoutspeed full_speed\n");
        }

        if (det_type == defs::EIGER || det_type == defs::JUNGFRAU) {
            {
                std::ostringstream oss1, oss2, oss3, oss4;
                proxy.Call("readoutspeed", {"1"}, -1, PUT, oss1);
                REQUIRE(oss1.str() == "readoutspeed half_speed\n");
                proxy.Call("readoutspeed", {}, -1, GET, oss2);
                REQUIRE(oss2.str() == "readoutspeed half_speed\n");
                proxy.Call("readoutspeed", {"half_speed"}, -1, PUT, oss3);
                REQUIRE(oss3.str() == "readoutspeed half_speed\n");
                proxy.Call("readoutspeed", {}, -1, GET, oss4);
                REQUIRE(oss4.str() == "readoutspeed half_speed\n");
            }
            {
                std::ostringstream oss1, oss2, oss3, oss4;
                proxy.Call("readoutspeed", {"2"}, -1, PUT, oss1);
                REQUIRE(oss1.str() == "readoutspeed quarter_speed\n");
                proxy.Call("readoutspeed", {}, -1, GET, oss2);
                REQUIRE(oss2.str() == "readoutspeed quarter_speed\n");
                proxy.Call("readoutspeed", {"quarter_speed"}, -1, PUT, oss3);
                REQUIRE(oss3.str() == "readoutspeed quarter_speed\n");
                proxy.Call("readoutspeed", {}, -1, GET, oss4);
                REQUIRE(oss4.str() == "readoutspeed quarter_speed\n");
            }
            REQUIRE_THROWS(proxy.Call("readoutspeed", {"108"}, -1, PUT));
            REQUIRE_THROWS(proxy.Call("readoutspeed", {"144"}, -1, PUT));
        }

        if (det_type == defs::GOTTHARD2) {
            {
                std::ostringstream oss1, oss2, oss3, oss4;
                proxy.Call("readoutspeed", {"108"}, -1, PUT, oss1);
                REQUIRE(oss1.str() == "readoutspeed 108\n");
                proxy.Call("readoutspeed", {}, -1, GET, oss2);
                REQUIRE(oss2.str() == "readoutspeed 108\n");
            }

            {
                std::ostringstream oss1, oss2, oss3, oss4;
                proxy.Call("readoutspeed", {"144"}, -1, PUT, oss1);
                REQUIRE(oss1.str() == "readoutspeed 144\n");
                proxy.Call("readoutspeed", {}, -1, GET, oss2);
                REQUIRE(oss2.str() == "readoutspeed 144\n");
            }
            REQUIRE_THROWS(proxy.Call("readoutspeed", {"full_speed"}, -1, PUT));
            REQUIRE_THROWS(proxy.Call("readoutspeed", {"half_speed"}, -1, PUT));
            REQUIRE_THROWS(
                proxy.Call("readoutspeed", {"quarter_speed"}, -1, PUT));
            REQUIRE_THROWS(proxy.Call("readoutspeed", {"0"}, -1, PUT));
            REQUIRE_THROWS(proxy.Call("readoutspeed", {"1"}, -1, PUT));
            REQUIRE_THROWS(proxy.Call("readoutspeed", {"2"}, -1, PUT));
        }

        for (int i = 0; i != det.size(); ++i) {
            det.setReadoutSpeed(prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(proxy.Call("readoutspeed", {"0"}, -1, PUT));
        REQUIRE_THROWS(proxy.Call("readoutspeed", {}, -1, GET));
    }
}

TEST_CASE("readoutspeedlist", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::GOTTHARD2 || det_type == defs::JUNGFRAU ||
        det_type == defs::EIGER) {
        REQUIRE_NOTHROW(proxy.Call("readoutspeedlist", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("readoutspeedlist", {}, -1, PUT));
    } else {
        REQUIRE_THROWS(proxy.Call("readoutspeedlist", {}, -1, GET));
    }
}

TEST_CASE("adcphase", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::GOTTHARD || det_type == defs::JUNGFRAU ||
        det_type == defs::CHIPTESTBOARD || det_type == defs::MOENCH) {
        if (det_type == defs::GOTTHARD) {
            std::ostringstream oss1;
            proxy.Call("adcphase", {"20"}, -1, PUT, oss1);
            REQUIRE(oss1.str() == "adcphase 20\n");
            // cant get, cant use deg
            REQUIRE_THROWS(proxy.Call("adcphase", {}, -1, GET));
            REQUIRE_THROWS(proxy.Call("adcphase", {"20", "deg"}, -1, PUT));
        } else {
            auto prev_val = det.getADCPhase();
            {
                std::ostringstream oss1, oss2;
                proxy.Call("adcphase", {"20"}, -1, PUT, oss1);
                REQUIRE(oss1.str() == "adcphase 20\n");
                proxy.Call("adcphase", {}, -1, GET, oss2);
                REQUIRE(oss2.str() == "adcphase 20\n");
            }
            {
                std::ostringstream oss1, oss2;
                proxy.Call("adcphase", {"20", "deg"}, -1, PUT, oss1);
                REQUIRE(oss1.str() == "adcphase 20 deg\n");
                proxy.Call("adcphase", {"deg"}, -1, GET, oss2);
                REQUIRE(oss2.str() == "adcphase 20 deg\n");
            }
            for (int i = 0; i != det.size(); ++i) {
                det.setADCPhase(prev_val[i], {i});
            }
        }
    } else {
        REQUIRE_THROWS(proxy.Call("adcphase", {"0"}, -1, PUT));
        REQUIRE_THROWS(proxy.Call("adcphase", {}, -1, GET));
    }
}

TEST_CASE("maxadcphaseshift", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU || det_type == defs::CHIPTESTBOARD ||
        det_type == defs::MOENCH ||
        det_type == defs::MYTHEN3 ||   // only because clk index of 0 exists
        det_type == defs::GOTTHARD2) { // only because clk index of 0 exists
        REQUIRE_NOTHROW(proxy.Call("maxadcphaseshift", {}, -1, GET));
    } else {
        REQUIRE_THROWS(proxy.Call("maxadcphaseshift", {}, -1, GET));
    }
}

TEST_CASE("dbitphase", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU || det_type == defs::CHIPTESTBOARD) {
        auto prev_val = det.getDBITPhase();
        {
            std::ostringstream oss1, oss2;
            proxy.Call("dbitphase", {"20"}, -1, PUT, oss1);
            REQUIRE(oss1.str() == "dbitphase 20\n");
            proxy.Call("dbitphase", {}, -1, GET, oss2);
            REQUIRE(oss2.str() == "dbitphase 20\n");
        }
        {
            std::ostringstream oss1, oss2;
            proxy.Call("dbitphase", {"20", "deg"}, -1, PUT, oss1);
            REQUIRE(oss1.str() == "dbitphase 20 deg\n");
            proxy.Call("dbitphase", {"deg"}, -1, GET, oss2);
            REQUIRE(oss2.str() == "dbitphase 20 deg\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setDBITPhase(prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(proxy.Call("dbitphase", {"0"}, -1, PUT));
        REQUIRE_THROWS(proxy.Call("dbitphase", {}, -1, GET));
    }
}

TEST_CASE("maxdbitphaseshift", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU || det_type == defs::CHIPTESTBOARD ||
        det_type == defs::MYTHEN3 ||   // only because clk index of 0 exists
        det_type == defs::GOTTHARD2) { // only because clk index of 0 exists
        REQUIRE_NOTHROW(proxy.Call("maxdbitphaseshift", {}, -1, GET));
    } else {
        REQUIRE_THROWS(proxy.Call("maxdbitphaseshift", {}, -1, GET));
    }
}

TEST_CASE("clkfreq", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::MYTHEN3 || det_type == defs::GOTTHARD2) {
        REQUIRE_THROWS(proxy.Call("clkfreq", {"0", "2"}, -1, PUT));
        REQUIRE_THROWS(proxy.Call("clkfreq", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("clkfreq", {"7"}, -1, GET));
        REQUIRE_NOTHROW(proxy.Call("clkfreq", {"0"}, -1, GET));
    } else {
        REQUIRE_THROWS(proxy.Call("clkfreq", {"0"}, -1, GET));
    }
}

TEST_CASE("clkphase", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::MYTHEN3 || det_type == defs::GOTTHARD2) {
        REQUIRE_THROWS(proxy.Call("clkphase", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("clkphase", {"7"}, -1, GET));
        REQUIRE_THROWS(proxy.Call("clkphase", {"4"}, -1, PUT));
        REQUIRE_THROWS(proxy.Call("clkphase", {"7", "4"}, -1, PUT));
        auto prev_val = det.getClockPhase(0);
        {
            std::ostringstream oss1, oss2;
            proxy.Call("clkphase", {"0", "20"}, -1, PUT, oss1);
            REQUIRE(oss1.str() == "clkphase 20\n");
            proxy.Call("clkphase", {"0"}, -1, GET, oss2);
            REQUIRE(oss2.str() == "clkphase 20\n");
        }
        std::string s_deg_val = "15";
        if (det_type == defs::MYTHEN3) {
            s_deg_val = "15";
        } else if (det_type == defs::GOTTHARD2) {
            s_deg_val = "23";
        }
        {
            std::ostringstream oss1, oss2;
            proxy.Call("clkphase", {"0", s_deg_val, "deg"}, -1, PUT, oss1);
            REQUIRE(oss1.str() == "clkphase " + s_deg_val + " deg\n");
            proxy.Call("clkphase", {"0", "deg"}, -1, GET, oss2);
            REQUIRE(oss2.str() == "clkphase " + s_deg_val + " deg\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setClockPhase(0, prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(proxy.Call("clkphase", {"0"}, -1, GET));
    }
}

TEST_CASE("clkdiv", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::MYTHEN3 || det_type == defs::GOTTHARD2) {
        REQUIRE_THROWS(proxy.Call("clkdiv", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("clkdiv", {"7"}, -1, GET));
        REQUIRE_THROWS(proxy.Call("clkdiv", {"7", "4"}, -1, PUT));
        REQUIRE_THROWS(proxy.Call("clkdiv", {"7", "4"}, -1, PUT));
        REQUIRE_THROWS(proxy.Call("clkdiv", {"0", "1"}, -1, PUT));
        auto prev_val = det.getClockDivider(0);
        {
            std::ostringstream oss1, oss2;
            proxy.Call("clkdiv", {"0", "3"}, -1, PUT, oss1);
            REQUIRE(oss1.str() == "clkdiv 3\n");
            proxy.Call("clkdiv", {"0"}, -1, GET, oss2);
            REQUIRE(oss2.str() == "clkdiv 3\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setClockDivider(0, prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(proxy.Call("clkdiv", {"0"}, -1, GET));
    }
}

TEST_CASE("maxclkphaseshift", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::MYTHEN3 || det_type == defs::GOTTHARD2) {
        REQUIRE_THROWS(proxy.Call("maxclkphaseshift", {"0", "2"}, -1, PUT));
        REQUIRE_THROWS(proxy.Call("maxclkphaseshift", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("maxclkphaseshift", {"7"}, -1, GET));
        REQUIRE_NOTHROW(proxy.Call("maxclkphaseshift", {"0"}, -1, GET));
    } else {
        REQUIRE_THROWS(proxy.Call("maxclkphaseshift", {"0"}, -1, GET));
    }
}

TEST_CASE("highvoltage", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    auto prev_val = det.getHighVoltage();
    // selected values
    if (det_type == defs::GOTTHARD) {
        REQUIRE_THROWS(proxy.Call("highvoltage", {"50"}, -1, PUT));
        {
            std::ostringstream oss1, oss2;
            proxy.Call("highvoltage", {"90"}, -1, PUT, oss1);
            REQUIRE(oss1.str() == "highvoltage 90\n");
            proxy.Call("highvoltage", {}, -1, GET, oss2);
            REQUIRE(oss2.str() == "highvoltage 90\n");
        }
        {
            std::ostringstream oss1, oss2;
            proxy.Call("highvoltage", {"0"}, -1, PUT, oss1);
            REQUIRE(oss1.str() == "highvoltage 0\n");
            proxy.Call("highvoltage", {}, -1, GET, oss2);
            REQUIRE(oss2.str() == "highvoltage 0\n");
        }
    }
    // range 0, 60 - 200
    else if (det_type == defs::JUNGFRAU || det_type == defs::CHIPTESTBOARD ||
             det_type == defs::MOENCH) {
        REQUIRE_THROWS(proxy.Call("highvoltage", {"50"}, -1, PUT));
        {
            std::ostringstream oss1, oss2;
            proxy.Call("highvoltage", {"90"}, -1, PUT, oss1);
            REQUIRE(oss1.str() == "highvoltage 90\n");
            proxy.Call("highvoltage", {}, -1, GET, oss2);
            REQUIRE(oss2.str() == "highvoltage 90\n");
        }
        {
            std::ostringstream oss1, oss2;
            proxy.Call("highvoltage", {"0"}, -1, PUT, oss1);
            REQUIRE(oss1.str() == "highvoltage 0\n");
            proxy.Call("highvoltage", {}, -1, GET, oss2);
            REQUIRE(oss2.str() == "highvoltage 0\n");
        }
    }
    // full range 0 - 200 (get needs to wait)
    else if (det_type == defs::EIGER) {
        {
            std::ostringstream oss1, oss2;
            proxy.Call("highvoltage", {"50"}, 0, PUT, oss1);
            REQUIRE(oss1.str() == "highvoltage 50\n");
            std::this_thread::sleep_for(std::chrono::seconds(2));
            proxy.Call("highvoltage", {}, 0, GET, oss2);
            REQUIRE(oss2.str() == "highvoltage 50\n");
        }
        {
            std::ostringstream oss1, oss2;
            proxy.Call("highvoltage", {"120"}, 0, PUT, oss1);
            REQUIRE(oss1.str() == "highvoltage 120\n");
            std::this_thread::sleep_for(std::chrono::seconds(2));
            proxy.Call("highvoltage", {}, 0, GET, oss2);
            REQUIRE(oss2.str() == "highvoltage 120\n");
        }
        {
            std::ostringstream oss1, oss2;
            proxy.Call("highvoltage", {"0"}, 0, PUT, oss1);
            REQUIRE(oss1.str() == "highvoltage 0\n");
            std::this_thread::sleep_for(std::chrono::seconds(2));
            proxy.Call("highvoltage", {}, 0, GET, oss2);
            REQUIRE(oss2.str() == "highvoltage 0\n");
        }
    }
    // full range 0 - 200
    else {
        {
            std::ostringstream oss1, oss2;
            proxy.Call("highvoltage", {"50"}, -1, PUT, oss1);
            REQUIRE(oss1.str() == "highvoltage 50\n");
            proxy.Call("highvoltage", {}, -1, GET, oss2);
            REQUIRE(oss2.str() == "highvoltage 50\n");
        }
        {
            std::ostringstream oss1, oss2;
            proxy.Call("highvoltage", {"120"}, -1, PUT, oss1);
            REQUIRE(oss1.str() == "highvoltage 120\n");
            proxy.Call("highvoltage", {}, -1, GET, oss2);
            REQUIRE(oss2.str() == "highvoltage 120\n");
        }
        {
            std::ostringstream oss1, oss2;
            proxy.Call("highvoltage", {"0"}, -1, PUT, oss1);
            REQUIRE(oss1.str() == "highvoltage 0\n");
            proxy.Call("highvoltage", {}, -1, GET, oss2);
            REQUIRE(oss2.str() == "highvoltage 0\n");
        }
    }
    for (int i = 0; i != det.size(); ++i) {
        det.setHighVoltage(prev_val[i], {i});
    }
}

TEST_CASE("powerchip", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::JUNGFRAU || det_type == defs::MYTHEN3 ||
        det_type == defs::GOTTHARD2 || det_type == defs::MOENCH) {
        auto prev_val = det.getPowerChip();
        {
            std::ostringstream oss;
            proxy.Call("powerchip", {"1"}, -1, PUT, oss);
            REQUIRE(oss.str() == "powerchip 1\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("powerchip", {"0"}, -1, PUT, oss);
            REQUIRE(oss.str() == "powerchip 0\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("powerchip", {}, -1, GET, oss);
            REQUIRE(oss.str() == "powerchip 0\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setPowerChip(prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(proxy.Call("powerchip", {}, -1, GET));
    }
}

TEST_CASE("imagetest", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    // cannot test only for virtual eiger/jungfrau
    if (det_type == defs::GOTTHARD) {
        auto prev_val = det.getImageTestMode();
        {
            std::ostringstream oss1, oss2;
            proxy.Call("imagetest", {"1"}, -1, PUT, oss1);
            REQUIRE(oss1.str() == "imagetest 1\n");
            proxy.Call("imagetest", {}, -1, GET, oss2);
            REQUIRE(oss2.str() == "imagetest 1\n");
        }
        {
            std::ostringstream oss1, oss2;
            proxy.Call("imagetest", {"0"}, -1, PUT, oss1);
            REQUIRE(oss1.str() == "imagetest 0\n");
            proxy.Call("imagetest", {}, -1, GET, oss2);
            REQUIRE(oss2.str() == "imagetest 0\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setImageTestMode(prev_val[i], {i});
        }
    } else if (det_type != defs::JUNGFRAU && det_type != defs::EIGER) {
        // wont fail for eiger and jungfrau virtual servers
        REQUIRE_THROWS(proxy.Call("imagetest", {}, -1, GET));
    }
}

TEST_CASE("extsig", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::GOTTHARD) {
        auto prev_val = det.getExternalSignalFlags(0);
        REQUIRE_THROWS(proxy.Call("extsig", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("extsig", {"1"}, -1, GET));
        REQUIRE_THROWS(proxy.Call("extsig", {"0", "inversion_on"}, -1, PUT));
        REQUIRE_THROWS(proxy.Call("extsig", {"0", "inversion_off"}, -1, PUT));
        {
            std::ostringstream oss1, oss2;
            proxy.Call("extsig", {"0", "trigger_in_rising_edge"}, -1, PUT,
                       oss1);
            REQUIRE(oss1.str() == "extsig 0 trigger_in_rising_edge\n");
            proxy.Call("extsig", {"0"}, -1, GET, oss2);
            REQUIRE(oss2.str() == "extsig 0 trigger_in_rising_edge\n");
        }
        {
            std::ostringstream oss1, oss2;
            proxy.Call("extsig", {"0", "trigger_in_falling_edge"}, -1, PUT,
                       oss1);
            REQUIRE(oss1.str() == "extsig 0 trigger_in_falling_edge\n");
            proxy.Call("extsig", {"0"}, -1, GET, oss2);
            REQUIRE(oss2.str() == "extsig 0 trigger_in_falling_edge\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setExternalSignalFlags(0, prev_val[i], {i});
        }
    } else if (det_type == defs::MYTHEN3) {
        auto prev_val_0 = det.getExternalSignalFlags(0);
        auto prev_val_1 = det.getExternalSignalFlags(1);
        REQUIRE_THROWS(proxy.Call("extsig", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("extsig", {"8"}, -1, GET));
        REQUIRE_THROWS(proxy.Call("extsig", {"0", "inversion_on"}, -1, PUT));
        REQUIRE_THROWS(proxy.Call("extsig", {"0", "inversion_off"}, -1, PUT));
        REQUIRE_THROWS(
            proxy.Call("extsig", {"1", "trigger_in_rising_edge"}, -1, PUT));
        REQUIRE_THROWS(
            proxy.Call("extsig", {"1", "trigger_in_falling_edge"}, -1, PUT));
        {
            std::ostringstream oss1, oss2;
            proxy.Call("extsig", {"0", "trigger_in_rising_edge"}, -1, PUT,
                       oss1);
            REQUIRE(oss1.str() == "extsig 0 trigger_in_rising_edge\n");
            proxy.Call("extsig", {"0"}, -1, GET, oss2);
            REQUIRE(oss2.str() == "extsig 0 trigger_in_rising_edge\n");
        }
        {
            std::ostringstream oss1, oss2;
            proxy.Call("extsig", {"0", "trigger_in_falling_edge"}, -1, PUT,
                       oss1);
            REQUIRE(oss1.str() == "extsig 0 trigger_in_falling_edge\n");
            proxy.Call("extsig", {"0"}, -1, GET, oss2);
            REQUIRE(oss2.str() == "extsig 0 trigger_in_falling_edge\n");
        }
        {
            std::ostringstream oss1, oss2;
            proxy.Call("extsig", {"1", "inversion_off"}, -1, PUT, oss1);
            REQUIRE(oss1.str() == "extsig 1 inversion_off\n");
            proxy.Call("extsig", {"1"}, -1, GET, oss2);
            REQUIRE(oss2.str() == "extsig 1 inversion_off\n");
        }
        {
            std::ostringstream oss1, oss2;
            proxy.Call("extsig", {"1", "inversion_on"}, -1, PUT, oss1);
            REQUIRE(oss1.str() == "extsig 1 inversion_on\n");
            proxy.Call("extsig", {"1"}, -1, GET, oss2);
            REQUIRE(oss2.str() == "extsig 1 inversion_on\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setExternalSignalFlags(0, prev_val_0[i], {i});
            det.setExternalSignalFlags(1, prev_val_1[i], {i});
        }
    } else {
        REQUIRE_THROWS(proxy.Call("extsig", {}, -1, GET));
    }
}

TEST_CASE("parallel", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::EIGER || det_type == defs::MYTHEN3 ||
        det_type == defs::GOTTHARD2) {
        auto prev_val = det.getParallelMode();
        {
            std::ostringstream oss;
            proxy.Call("parallel", {"1"}, -1, PUT, oss);
            REQUIRE(oss.str() == "parallel 1\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("parallel", {"0"}, -1, PUT, oss);
            REQUIRE(oss.str() == "parallel 0\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("parallel", {}, -1, GET, oss);
            REQUIRE(oss.str() == "parallel 0\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setParallelMode(prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(proxy.Call("parallel", {}, -1, GET));
    }
}

TEST_CASE("filterresistor", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();

    // only for chipv1.1
    bool chip11 = false;
    if (det_type == defs::JUNGFRAU &&
        det.getChipVersion().squash() * 10 == 11) {
        chip11 = true;
    }

    if (det_type == defs::GOTTHARD2 || chip11) {
        auto prev_val = det.getFilterResistor();
        {
            std::ostringstream oss;
            proxy.Call("filterresistor", {"1"}, -1, PUT, oss);
            REQUIRE(oss.str() == "filterresistor 1\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("filterresistor", {"0"}, -1, PUT, oss);
            REQUIRE(oss.str() == "filterresistor 0\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("filterresistor", {}, -1, GET, oss);
            REQUIRE(oss.str() == "filterresistor 0\n");
        }
        if (det_type == defs::GOTTHARD2) {
            REQUIRE_NOTHROW(proxy.Call("filterresistor", {"2"}, -1, PUT));
            REQUIRE_NOTHROW(proxy.Call("filterresistor", {"3"}, -1, PUT));
        } else {
            REQUIRE_THROWS(proxy.Call("filterresistor", {"2"}, -1, PUT));
            REQUIRE_THROWS(proxy.Call("filterresistor", {"3"}, -1, PUT));
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setFilterResistor(prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(proxy.Call("filterresistor", {}, -1, GET));
    }
}

TEST_CASE("dbitpipeline", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::CHIPTESTBOARD || det_type == defs::GOTTHARD2) {
        auto prev_val = det.getDBITPipeline();
        {
            std::ostringstream oss;
            proxy.Call("dbitpipeline", {"1"}, -1, PUT, oss);
            REQUIRE(oss.str() == "dbitpipeline 1\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("dbitpipeline", {"0"}, -1, PUT, oss);
            REQUIRE(oss.str() == "dbitpipeline 0\n");
        }
        if (det_type == defs::CHIPTESTBOARD) {
            {
                std::ostringstream oss;
                proxy.Call("dbitpipeline", {"15"}, -1, PUT, oss);
                REQUIRE(oss.str() == "dbitpipeline 15\n");
            }
            {
                std::ostringstream oss;
                proxy.Call("dbitpipeline", {}, -1, GET, oss);
                REQUIRE(oss.str() == "dbitpipeline 15\n");
            }
            REQUIRE_THROWS(proxy.Call("dbitpipeline", {"256"}, -1, PUT));
        } else {
            {
                std::ostringstream oss;
                proxy.Call("dbitpipeline", {"7"}, -1, PUT, oss);
                REQUIRE(oss.str() == "dbitpipeline 7\n");
            }
            {
                std::ostringstream oss;
                proxy.Call("dbitpipeline", {}, -1, GET, oss);
                REQUIRE(oss.str() == "dbitpipeline 7\n");
            }
            REQUIRE_THROWS(proxy.Call("dbitpipeline", {"8"}, -1, PUT));
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setDBITPipeline(prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(proxy.Call("dbitpipeline", {}, -1, GET));
    }
}

TEST_CASE("readnrows", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::EIGER || det_type == defs::JUNGFRAU) {
        bool jungfrauhw2 = false;
        if (det_type == defs::JUNGFRAU &&
            ((det.getHardwareVersion().tsquash(
                  "inconsistent hardware version number to test") == "2.0"))) {
            jungfrauhw2 = true;
        }
        if (det_type == defs::JUNGFRAU && !jungfrauhw2) {
            {
                std::ostringstream oss;
                proxy.Call("readnrows", {}, -1, GET, oss);
                REQUIRE(oss.str() == "readnrows 512\n");
            }
        } else {
            auto prev_val = det.getReadNRows();
            {
                std::ostringstream oss;
                proxy.Call("readnrows", {"256"}, -1, PUT, oss);
                REQUIRE(oss.str() == "readnrows 256\n");
            }
            {
                std::ostringstream oss;
                proxy.Call("readnrows", {}, -1, GET, oss);
                REQUIRE(oss.str() == "readnrows 256\n");
            }
            {
                std::ostringstream oss;
                proxy.Call("readnrows", {"16"}, -1, PUT, oss);
                REQUIRE(oss.str() == "readnrows 16\n");
            }
            if (det_type == defs::JUNGFRAU) {
                REQUIRE_THROWS(proxy.Call("readnrows", {"7"}, -1, PUT));
                REQUIRE_THROWS(proxy.Call("readnrows", {"20"}, -1, PUT));
                REQUIRE_THROWS(proxy.Call("readnrows", {"44"}, -1, PUT));
                REQUIRE_THROWS(proxy.Call("readnrows", {"513"}, -1, PUT));
                REQUIRE_THROWS(proxy.Call("readnrows", {"1"}, -1, PUT));
            }
            REQUIRE_THROWS(proxy.Call("readnrows", {"0"}, -1, PUT));
            for (int i = 0; i != det.size(); ++i) {
                det.setReadNRows(prev_val[i], {i});
            }
        }
    } else {
        REQUIRE_THROWS(proxy.Call("readnrows", {}, -1, GET));
    }
}

TEST_CASE("currentsource", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::GOTTHARD2 || det_type == defs::JUNGFRAU) {
        auto prev_val = det.getCurrentSource();

        if (det_type == defs::GOTTHARD2) {
            {
                std::ostringstream oss;
                proxy.Call("currentsource", {"1"}, -1, PUT, oss);
                REQUIRE(oss.str() == "currentsource [1]\n");
            }
            {
                std::ostringstream oss;
                proxy.Call("currentsource", {"0"}, -1, PUT, oss);
                REQUIRE(oss.str() == "currentsource [0]\n");
            }
            {
                std::ostringstream oss;
                proxy.Call("currentsource", {}, -1, GET, oss);
                REQUIRE(oss.str() == "currentsource [disabled]\n");
            }
            REQUIRE_THROWS(
                proxy.Call("currentsource", {"1", "fix", "42"}, -1, PUT));
            REQUIRE_THROWS(proxy.Call("currentsource",
                                      {"1", "fix", "42", "normal"}, -1, PUT));
        }
        // jungfrau
        else {
            int chipVersion = det.getChipVersion().tsquash(
                                  "inconsistent chip versions to test") *
                              10;
            if (chipVersion == 10) {
                REQUIRE_THROWS(proxy.Call("currentsource", {"1"}, -1, PUT));
                REQUIRE_THROWS(
                    proxy.Call("currentsource", {"1", "fix"}, -1, PUT));
                REQUIRE_THROWS(
                    proxy.Call("currentsource", {"1", "fix", "64"}, -1, PUT));
                REQUIRE_THROWS(
                    proxy.Call("currentsource", {"1", "dfg", "64"}, -1, PUT));
                REQUIRE_THROWS(proxy.Call(
                    "currentsource", {"1", "fix", "63", "normal"}, -1, PUT));
                {
                    std::ostringstream oss;
                    proxy.Call("currentsource", {"1", "fix", "63"}, -1, PUT,
                               oss);
                    REQUIRE(oss.str() == "currentsource [1, fix, 63]\n");
                }
                {
                    std::ostringstream oss;
                    proxy.Call("currentsource", {"0"}, -1, PUT, oss);
                    REQUIRE(oss.str() == "currentsource [0]\n");
                }
                {
                    std::ostringstream oss;
                    proxy.Call("currentsource", {}, -1, GET, oss);
                    REQUIRE(oss.str() == "currentsource [disabled]\n");
                }
                {
                    std::ostringstream oss;
                    proxy.Call("currentsource", {"1", "nofix", "63"}, -1, PUT,
                               oss);
                    REQUIRE(oss.str() == "currentsource [1, nofix, 63]\n");
                }
                {
                    std::ostringstream oss;
                    proxy.Call("currentsource", {}, -1, GET, oss);
                    REQUIRE(oss.str() ==
                            "currentsource [enabled, nofix, 63]\n");
                }
            }
            // chipv1.1
            else {
                REQUIRE_THROWS(proxy.Call("currentsource", {"1"}, -1, PUT));
                REQUIRE_THROWS(
                    proxy.Call("currentsource", {"1", "fix"}, -1, PUT));
                REQUIRE_THROWS(proxy.Call(
                    "currentsource", {"1", "ffgdfgix", "0x0000000000000041"},
                    -1, PUT));
                REQUIRE_THROWS(proxy.Call(
                    "currentsource",
                    {"1", "fix", "0x0000000000000041", "normaldgf"}, -1, PUT));

                {
                    std::ostringstream oss;
                    proxy.Call("currentsource",
                               {"1", "fix", "0x0000000000000041", "normal"}, -1,
                               PUT, oss);
                    REQUIRE(
                        oss.str() ==
                        "currentsource [1, fix, 0x0000000000000041, normal]\n");
                }
                {
                    std::ostringstream oss;
                    proxy.Call("currentsource", {"0"}, -1, PUT, oss);
                    REQUIRE(oss.str() == "currentsource [0]\n");
                }
                {
                    std::ostringstream oss;
                    proxy.Call("currentsource", {}, -1, GET, oss);
                    REQUIRE(oss.str() == "currentsource [disabled]\n");
                }
                {
                    std::ostringstream oss;
                    proxy.Call("currentsource",
                               {"1", "nofix", "0x0000000000000041", "normal"},
                               -1, PUT, oss);
                    REQUIRE(oss.str() == "currentsource [1, nofix, "
                                         "0x0000000000000041, normal]\n");
                }
                {
                    std::ostringstream oss;
                    proxy.Call("currentsource", {}, -1, GET, oss);
                    REQUIRE(oss.str() == "currentsource [enabled, nofix, "
                                         "0x0000000000000041, normal]\n");
                }
                {
                    std::ostringstream oss;
                    proxy.Call("currentsource",
                               {"1", "nofix", "0x0000000000000041", "low"}, -1,
                               PUT, oss);
                    REQUIRE(
                        oss.str() ==
                        "currentsource [1, nofix, 0x0000000000000041, low]\n");
                }
            }
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setCurrentSource(prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(proxy.Call("currentsource", {}, -1, GET));
    }
}

/** temperature */

TEST_CASE("templist", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    REQUIRE_NOTHROW(proxy.Call("templist", {}, -1, GET));
    REQUIRE_THROWS(proxy.Call("templist", {}, -1, PUT));
}

TEST_CASE("tempvalues", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    REQUIRE_NOTHROW(proxy.Call("tempvalues", {}, -1, GET));
    REQUIRE_THROWS(proxy.Call("tempvalues", {}, -1, PUT));
}

TEST_CASE("temp_adc", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU || det_type == defs::GOTTHARD) {
        REQUIRE_NOTHROW(proxy.Call("temp_adc", {}, -1, GET));
        std::ostringstream oss;
        REQUIRE_NOTHROW(proxy.Call("temp_adc", {}, 0, GET, oss));
        std::string s = (oss.str()).erase(0, strlen("temp_adc "));
        REQUIRE(std::stoi(s) != -1);
    } else {
        REQUIRE_THROWS(proxy.Call("temp_adc", {}, -1, GET));
    }
}

TEST_CASE("temp_fpga", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type != defs::MOENCH && det_type != defs::CHIPTESTBOARD) {
        REQUIRE_NOTHROW(proxy.Call("temp_fpga", {}, -1, GET));
        std::ostringstream oss;
        REQUIRE_NOTHROW(proxy.Call("temp_fpga", {}, 0, GET, oss));
        std::string s = (oss.str()).erase(0, strlen("temp_fpga "));
        REQUIRE(std::stoi(s) != -1);
    } else {
        REQUIRE_THROWS(proxy.Call("temp_fpga", {}, -1, GET));
    }
}

/* dacs */

TEST_CASE("daclist", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    REQUIRE_NOTHROW(proxy.Call("daclist", {}, -1, GET));
    REQUIRE_THROWS(proxy.Call("daclist", {}, -1, PUT));
}

TEST_CASE("dacvalues", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    REQUIRE_NOTHROW(proxy.Call("dacvalues", {}, -1, GET));
    REQUIRE_THROWS(proxy.Call("dacvalues", {}, -1, PUT));
}

TEST_CASE("defaultdac", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type != defs::CHIPTESTBOARD) {
        REQUIRE_THROWS(proxy.Call("defaultdac", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("defaultdac", {"blabla"}, -1, PUT));
        auto daclist = det.getDacList();
        for (auto it : daclist) {
            if (it == defs::VTHRESHOLD) {
                continue;
            }
            auto dacname = ToString(it);
            auto prev_val = det.getDefaultDac(it);
            {
                std::ostringstream oss;
                proxy.Call("defaultdac", {dacname, "1000"}, -1, PUT, oss);
                REQUIRE(oss.str() == std::string("defaultdac ") + dacname +
                                         std::string(" 1000\n"));
            }
            {
                std::ostringstream oss;
                proxy.Call("defaultdac", {dacname}, -1, GET, oss);
                REQUIRE(oss.str() == std::string("defaultdac ") + dacname +
                                         std::string(" 1000\n"));
            }
            for (int i = 0; i != det.size(); ++i) {
                det.setDefaultDac(it, prev_val[i], {i});
            }
        }
        if (det_type == defs::JUNGFRAU) {
            std::vector<defs::dacIndex> daclist = {
                defs::VREF_PRECH, defs::VREF_DS, defs::VREF_COMP};
            for (auto it : daclist) {
                auto dacname = ToString(it);
                auto prev_val = det.getDefaultDac(it, defs::GAIN0);
                {
                    std::ostringstream oss;
                    proxy.Call("defaultdac", {dacname, "1000", "gain0"}, -1,
                               PUT, oss);
                    REQUIRE(oss.str() == std::string("defaultdac ") + dacname +
                                             std::string(" gain0 1000\n"));
                }
                {
                    std::ostringstream oss;
                    proxy.Call("defaultdac", {dacname, "gain0"}, -1, GET, oss);
                    REQUIRE(oss.str() == std::string("defaultdac ") + dacname +
                                             std::string(" gain0 1000\n"));
                }
                for (int i = 0; i != det.size(); ++i) {
                    det.setDefaultDac(it, prev_val[i], defs::GAIN0, {i});
                }
            }
        }
    } else {
        REQUIRE_THROWS(proxy.Call("defaultdac", {}, -1, GET));
    }
}

TEST_CASE("resetdacs", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type != defs::CHIPTESTBOARD) {
        auto prev_val = det.getSettings();

        REQUIRE_THROWS(proxy.Call("resetdacs", {}, -1, GET));
        REQUIRE_NOTHROW(proxy.Call("resetdacs", {}, -1, PUT));
        REQUIRE_NOTHROW(proxy.Call("resetdacs", {"hard"}, -1, PUT));

        // settings should not change especially for jungfrau and m3
        auto next_val = det.getSettings();
        for (int i = 0; i != det.size(); ++i) {
            REQUIRE(prev_val[i] == next_val[i]);
        }
    } else {
        REQUIRE_THROWS(proxy.Call("resetdacs", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("resetdacs", {}, -1, PUT));
    }
}

/* acquisition */

TEST_CASE("trigger", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    REQUIRE_THROWS(proxy.Call("trigger", {}, -1, GET));
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::MYTHEN3) {
        REQUIRE_NOTHROW(proxy.Call("trigger", {}, -1, PUT));
    } else if (det_type == defs::EIGER || det_type == defs::JUNGFRAU) {
        auto prev_timing =
            det.getTimingMode().tsquash("inconsistent timing mode in test");
        auto prev_frames =
            det.getNumberOfFrames().tsquash("inconsistent #frames in test");
        auto prev_exptime =
            det.getExptime().tsquash("inconsistent exptime in test");
        auto prev_period =
            det.getPeriod().tsquash("inconsistent period in test");
        det.setTimingMode(defs::TRIGGER_EXPOSURE);
        det.setNumberOfFrames(1);
        det.setExptime(std::chrono::microseconds(200));
        det.setPeriod(std::chrono::milliseconds(1));
        auto nextframenumber =
            det.getNextFrameNumber().tsquash("inconsistent frame nr in test");
        det.startDetector();
        {
            std::ostringstream oss;
            proxy.Call("trigger", {}, -1, PUT, oss);
            REQUIRE(oss.str() == "trigger successful\n");
        }
        std::this_thread::sleep_for(std::chrono::seconds(2));
        auto currentfnum =
            det.getNextFrameNumber().tsquash("inconsistent frame nr in test");
        REQUIRE(nextframenumber + 1 == currentfnum);
        det.stopDetector();
        det.setTimingMode(prev_timing);
        det.setNumberOfFrames(prev_frames);
        det.setExptime(prev_exptime);
        det.setPeriod(prev_period);
    } else {
        REQUIRE_THROWS(proxy.Call("trigger", {}, -1, PUT));
    }
}

TEST_CASE("blockingtrigger", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    REQUIRE_THROWS(proxy.Call("blockingtrigger", {}, -1, GET));
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::EIGER || det_type == defs::JUNGFRAU) {
        auto prev_timing =
            det.getTimingMode().tsquash("inconsistent timing mode in test");
        auto prev_frames =
            det.getNumberOfFrames().tsquash("inconsistent #frames in test");
        auto prev_exptime =
            det.getExptime().tsquash("inconsistent exptime in test");
        auto prev_period =
            det.getPeriod().tsquash("inconsistent period in test");
        det.setTimingMode(defs::TRIGGER_EXPOSURE);
        det.setNumberOfFrames(1);
        det.setExptime(std::chrono::microseconds(200));
        det.setPeriod(std::chrono::milliseconds(1));
        auto nextframenumber =
            det.getNextFrameNumber().tsquash("inconsistent frame nr in test");
        det.startDetector();
        {
            std::ostringstream oss;
            proxy.Call("blockingtrigger", {}, -1, PUT, oss);
            REQUIRE(oss.str() == "blockingtrigger successful\n");
        }
        if (det.isVirtualDetectorServer().tsquash(
                "inconsistent virtual detectors")) {
            std::this_thread::sleep_for(std::chrono::seconds(2));
        }
        auto currentfnum =
            det.getNextFrameNumber().tsquash("inconsistent frame nr in test");
        REQUIRE(nextframenumber + 1 == currentfnum);
        det.stopDetector();
        det.setTimingMode(prev_timing);
        det.setNumberOfFrames(prev_frames);
        det.setExptime(prev_exptime);
        det.setPeriod(prev_period);
    } else {
        REQUIRE_THROWS(proxy.Call("blockingtrigger", {}, -1, PUT));
    }
}

TEST_CASE("clearbusy", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    REQUIRE_NOTHROW(proxy.Call("clearbusy", {}, -1, PUT));
    REQUIRE_THROWS(proxy.Call("clearbusy", {"0"}, -1, PUT));
    REQUIRE_THROWS(proxy.Call("clearbusy", {}, -1, GET));
}

TEST_CASE("start", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    // PUT only command
    REQUIRE_THROWS(proxy.Call("start", {}, -1, GET));
    auto det_type = det.getDetectorType().squash();
    std::chrono::nanoseconds prev_val;
    if (det_type != defs::MYTHEN3) {
        prev_val = det.getExptime().tsquash("inconsistent exptime to test");
    } else {
        auto t =
            det.getExptimeForAllGates().tsquash("inconsistent exptime to test");
        if (t[0] != t[1] || t[1] != t[2]) {
            throw RuntimeError("inconsistent exptime for all gates");
        }
        prev_val = t[0];
    }
    auto prev_frames =
        det.getNumberOfFrames().tsquash("inconsistent #frames in test");
    auto prev_period = det.getPeriod().tsquash("inconsistent period in test");
    det.setExptime(-1, std::chrono::microseconds(200));
    det.setPeriod(std::chrono::milliseconds(1));
    det.setNumberOfFrames(2000);
    {
        std::ostringstream oss;
        proxy.Call("start", {}, -1, PUT, oss);
        REQUIRE(oss.str() == "start successful\n");
    }
    if (det_type != defs::CHIPTESTBOARD && det_type != defs::MOENCH) {
        std::ostringstream oss;
        proxy.Call("status", {}, -1, GET, oss);
        REQUIRE(oss.str() == "status running\n");
    }
    det.stopDetector();
    det.setExptime(-1, prev_val);
    det.setPeriod(prev_period);
    det.setNumberOfFrames(prev_frames);
}

TEST_CASE("stop", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    // PUT only command
    REQUIRE_THROWS(proxy.Call("stop", {}, -1, GET));
    auto det_type = det.getDetectorType().squash();
    std::chrono::nanoseconds prev_val;
    if (det_type != defs::MYTHEN3) {
        prev_val = det.getExptime().tsquash("inconsistent exptime to test");
    } else {
        auto t =
            det.getExptimeForAllGates().tsquash("inconsistent exptime to test");
        if (t[0] != t[1] || t[1] != t[2]) {
            throw RuntimeError("inconsistent exptime for all gates");
        }
        prev_val = t[0];
    }
    auto prev_frames =
        det.getNumberOfFrames().tsquash("inconsistent #frames in test");
    auto prev_period = det.getPeriod().tsquash("inconsistent period in test");
    det.setExptime(-1, std::chrono::microseconds(200));
    det.setPeriod(std::chrono::milliseconds(1));
    det.setNumberOfFrames(2000);
    det.startDetector();
    if (det_type != defs::CHIPTESTBOARD && det_type != defs::MOENCH) {
        std::ostringstream oss;
        proxy.Call("status", {}, -1, GET, oss);
        REQUIRE(oss.str() == "status running\n");
    }
    {
        std::ostringstream oss;
        proxy.Call("stop", {}, -1, PUT, oss);
        REQUIRE(oss.str() == "stop successful\n");
    }
    {
        std::ostringstream oss;
        proxy.Call("status", {}, -1, GET, oss);
        REQUIRE(((oss.str() == "status stopped\n") ||
                 (oss.str() == "status idle\n")));
    }
    det.setExptime(-1, prev_val);
    det.setPeriod(prev_period);
    det.setNumberOfFrames(prev_frames);
}

TEST_CASE("status", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    std::chrono::nanoseconds prev_val;
    if (det_type != defs::MYTHEN3) {
        prev_val = det.getExptime().tsquash("inconsistent exptime to test");
    } else {
        auto t =
            det.getExptimeForAllGates().tsquash("inconsistent exptime to test");
        if (t[0] != t[1] || t[1] != t[2]) {
            throw RuntimeError("inconsistent exptime for all gates");
        }
        prev_val = t[0];
    }
    auto prev_frames =
        det.getNumberOfFrames().tsquash("inconsistent #frames in test");
    auto prev_period = det.getPeriod().tsquash("inconsistent period in test");
    det.setExptime(-1, std::chrono::microseconds(200));
    det.setPeriod(std::chrono::milliseconds(1));
    det.setNumberOfFrames(2000);
    det.startDetector();
    if (det_type != defs::CHIPTESTBOARD && det_type != defs::MOENCH) {
        std::ostringstream oss;
        proxy.Call("status", {}, -1, GET, oss);
        REQUIRE(oss.str() == "status running\n");
    }
    det.stopDetector();
    {
        std::ostringstream oss;
        proxy.Call("status", {}, -1, GET, oss);
        REQUIRE(((oss.str() == "status stopped\n") ||
                 (oss.str() == "status idle\n")));
    }
    det.setExptime(-1, prev_val);
    det.setPeriod(prev_period);
    det.setNumberOfFrames(prev_frames);
}

TEST_CASE("nextframenumber", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::EIGER || det_type == defs::JUNGFRAU ||
        det_type == defs::MOENCH || det_type == defs::CHIPTESTBOARD) {
        auto prev_sfnum = det.getNextFrameNumber();
        REQUIRE_THROWS(proxy.Call("nextframenumber", {"0"}, -1, PUT));
        {
            std::ostringstream oss;
            proxy.Call("nextframenumber", {"3"}, -1, PUT, oss);
            REQUIRE(oss.str() == "nextframenumber 3\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("nextframenumber", {}, -1, GET, oss);
            REQUIRE(oss.str() == "nextframenumber 3\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("nextframenumber", {"1"}, -1, PUT, oss);
            REQUIRE(oss.str() == "nextframenumber 1\n");
        }

        auto prev_timing =
            det.getTimingMode().tsquash("inconsistent timing mode in test");
        auto prev_frames =
            det.getNumberOfFrames().tsquash("inconsistent #frames in test");
        auto prev_exptime =
            det.getExptime().tsquash("inconsistent exptime in test");
        auto prev_period =
            det.getPeriod().tsquash("inconsistent period in test");
        det.setTimingMode(defs::AUTO_TIMING);
        det.setNumberOfFrames(1);
        det.setExptime(std::chrono::microseconds(200));
        det.setPeriod(std::chrono::milliseconds(1));
        det.startDetector();
        std::this_thread::sleep_for(std::chrono::seconds(2));
        auto currentfnum =
            det.getNextFrameNumber().tsquash("inconsistent frame nr in test");
        REQUIRE(currentfnum == 2);
        if (det_type == defs::EIGER) {
            auto prev_tengiga =
                det.getTenGiga().tsquash("inconsistent ten giga enable");
            det.setTenGiga(true);
            det.setNextFrameNumber(1);
            det.startDetector();
            std::this_thread::sleep_for(std::chrono::seconds(2));
            auto currentfnum = det.getNextFrameNumber().tsquash(
                "inconsistent frame nr in test");
            REQUIRE(currentfnum == 2);
            det.setTenGiga(prev_tengiga);
        }

        det.setTimingMode(prev_timing);
        det.setNumberOfFrames(prev_frames);
        det.setExptime(prev_exptime);
        det.setPeriod(prev_period);
        for (int i = 0; i != det.size(); ++i) {
            det.setNextFrameNumber(prev_sfnum[i], {i});
        }
    } else {
        REQUIRE_THROWS(proxy.Call("nextframenumber", {}, -1, GET));
    }
}

TEST_CASE("scan", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    defs::dacIndex ind = defs::DAC_0;
    defs::dacIndex notImplementedInd = defs::DAC_0;
    auto det_type = det.getDetectorType().squash();
    switch (det_type) {
    case defs::CHIPTESTBOARD:
        ind = defs::DAC_0;
        notImplementedInd = defs::VSVP;
        break;
    case defs::EIGER:
        ind = defs::VCMP_LL;
        notImplementedInd = defs::VCASCP_PB;
        break;
    case defs::JUNGFRAU:
        ind = defs::VB_COMP;
        notImplementedInd = defs::VSVP;
        break;
    case defs::GOTTHARD:
        ind = defs::VREF_DS;
        notImplementedInd = defs::VSVP;
        break;
    case defs::MOENCH:
        ind = defs::VBP_COLBUF;
        notImplementedInd = defs::VSVP;
        break;
    case defs::GOTTHARD2:
        ind = defs::VB_COMP_FE;
        notImplementedInd = defs::VSVP;
        break;
    case defs::MYTHEN3:
        ind = defs::VTH2;
        notImplementedInd = defs::VSVP;
        break;
    default:
        break;
    }

    // when taking acquisition
    // auto previous = det.getDAC(ind, false);
    // auto notImplementedPrevious = det.getDAC(notImplementedInd, false);

    if (det_type == defs::MYTHEN3 && det.size() > 1) {
        ; // scan only allowed for single module due to sync
    } else {
        {
            std::ostringstream oss;
            proxy.Call("scan", {ToString(ind), "500", "1500", "500"}, -1, PUT,
                       oss);
            CHECK(oss.str() ==
                  "scan [" + ToString(ind) + ", 500, 1500, 500]\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("scan", {}, -1, GET, oss);
            CHECK(oss.str() == "scan [enabled\ndac " + ToString(ind) +
                                   "\nstart 500\nstop 1500\nstep "
                                   "500\nsettleTime 1ms\n]\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("scan", {ToString(ind), "500", "1500", "500", "2s"}, -1,
                       PUT, oss);
            CHECK(oss.str() ==
                  "scan [" + ToString(ind) + ", 500, 1500, 500, 2s]\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("scan", {}, -1, GET, oss);
            CHECK(oss.str() == "scan [enabled\ndac " + ToString(ind) +
                                   "\nstart 500\nstop 1500\nstep "
                                   "500\nsettleTime 2s\n]\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("scan", {"0"}, -1, PUT, oss);
            CHECK(oss.str() == "scan [0]\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("scan", {}, -1, GET, oss);
            CHECK(oss.str() == "scan [disabled]\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("scan", {ToString(ind), "1500", "500", "-500"}, -1, PUT,
                       oss);
            CHECK(oss.str() ==
                  "scan [" + ToString(ind) + ", 1500, 500, -500]\n");
        }
        CHECK_THROWS(proxy.Call(
            "scan", {ToString(notImplementedInd), "500", "1500", "500"}, -1,
            PUT));
        CHECK_THROWS(proxy.Call("scan", {ToString(ind), "500", "1500", "-500"},
                                -1, PUT));
        CHECK_THROWS(
            proxy.Call("scan", {ToString(ind), "1500", "500", "500"}, -1, PUT));

        if (det_type == defs::MYTHEN3 || defs::EIGER) {
            {
                std::ostringstream oss;
                proxy.Call("scan", {"trimbits", "0", "63", "16", "2s"}, -1, PUT,
                           oss);
                CHECK(oss.str() == "scan [trimbits, 0, 63, 16, 2s]\n");
            }
            {
                std::ostringstream oss;
                proxy.Call("scan", {}, -1, GET, oss);
                CHECK(oss.str() ==
                      "scan [enabled\ndac trimbits\nstart 0\nstop 48\nstep "
                      "16\nsettleTime 2s\n]\n");
            }
        }

        // Switch off scan for future tests
        det.setScan(defs::scanParameters());
        // acquire for each?

        // when taking acquisition
        // Reset all dacs to previous value
        // for (int i = 0; i != det.size(); ++i) {
        //     det.setDAC(ind, previous[i], false, {i});
        //     det.setDAC(notImplementedInd, notImplementedPrevious[i], false,
        //     {i});
        // }
    }
}

TEST_CASE("scanerrmsg", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    REQUIRE_NOTHROW(proxy.Call("scanerrmsg", {}, -1, GET));
    REQUIRE_THROWS(proxy.Call("scanerrmsg", {""}, -1, PUT));
}

/* Network Configuration (Detector<->Receiver) */

TEST_CASE("numinterfaces", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU) {
        auto prev_val = det.getNumberofUDPInterfaces().tsquash(
            "inconsistent numinterfaces to test");
        {
            std::ostringstream oss;
            proxy.Call("numinterfaces", {"2"}, -1, PUT, oss);
            REQUIRE(oss.str() == "numinterfaces 2\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("numinterfaces", {"1"}, -1, PUT, oss);
            REQUIRE(oss.str() == "numinterfaces 1\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("numinterfaces", {}, -1, GET, oss);
            REQUIRE(oss.str() == "numinterfaces 1\n");
        }
        det.setNumberofUDPInterfaces(prev_val);
    } else if (det_type == defs::EIGER) {
        REQUIRE_THROWS(proxy.Call("numinterfaces", {"1"}, -1, PUT));
        {
            std::ostringstream oss;
            proxy.Call("numinterfaces", {}, -1, GET, oss);
            REQUIRE(oss.str() == "numinterfaces 2\n");
        }
    } else {
        std::ostringstream oss;
        proxy.Call("numinterfaces", {}, -1, GET, oss);
        REQUIRE(oss.str() == "numinterfaces 1\n");
        REQUIRE_THROWS(proxy.Call("numinterfaces", {"1"}, -1, PUT));
    }
    REQUIRE_THROWS(proxy.Call("numinterfaces", {"3"}, -1, PUT));
    REQUIRE_THROWS(proxy.Call("numinterfaces", {"0"}, -1, PUT));
}

TEST_CASE("udp_srcip", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto prev_val = det.getSourceUDPIP();
    REQUIRE_THROWS(proxy.Call("udp_srcip", {"0.0.0.0"}, -1, PUT));
    {
        std::ostringstream oss;
        proxy.Call("udp_srcip", {"129.129.205.12"}, -1, PUT, oss);
        REQUIRE(oss.str() == "udp_srcip 129.129.205.12\n");
    }
    for (int i = 0; i != det.size(); ++i) {
        det.setSourceUDPIP(prev_val[i], {i});
    }
}

TEST_CASE("udp_dstlist", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU || det_type == defs::EIGER ||
        det_type == defs::MYTHEN3 || det_type == defs::GOTTHARD2) {
        REQUIRE_NOTHROW(proxy.Call("udp_dstlist", {}, 0, GET, std::cout, 0));
        REQUIRE_THROWS(proxy.Call(
            "udp_dstlist", {"ip=0.0.0.0", "mac=00:00:00:00:00:00", "port=1233"},
            -1, PUT, std::cout, 0));
    } else {
        REQUIRE_THROWS(proxy.Call("udp_dstlist", {}, -1, GET, std::cout, 0));
    }
}

TEST_CASE("udp_numdst", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU || det_type == defs::EIGER ||
        det_type == defs::MYTHEN3 || det_type == defs::GOTTHARD2) {
        REQUIRE_NOTHROW(proxy.Call("udp_numdst", {}, -1, GET));
    } else {
        REQUIRE_THROWS(proxy.Call("udp_numdst", {}, -1, GET));
    }
}

TEST_CASE("udp_cleardst", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    REQUIRE_THROWS(proxy.Call("udp_cleardst", {}, -1, GET));
    /* dont clear all udp destinations */
    /*REQUIRE_NOTHROW(proxy.Call("udp_cleardst", {}, -1, PUT));*/
}

TEST_CASE("udp_firstdst", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU || det_type == defs::MYTHEN3 ||
        det_type == defs::GOTTHARD2) {
        auto prev_val = det.getFirstUDPDestination();
        {
            std::ostringstream oss;
            proxy.Call("udp_firstdst", {"0"}, -1, PUT, oss);
            REQUIRE(oss.str() == "udp_firstdst 0\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("udp_firstdst", {}, -1, GET, oss);
            REQUIRE(oss.str() == "udp_firstdst 0\n");
        }
        /*
        {
            std::ostringstream oss;
            proxy.Call("udp_firstdst", {"1"}, -1, PUT, oss);
            REQUIRE(oss.str() == "udp_firstdst 1\n");
        }
        */
        REQUIRE_THROWS(proxy.Call("udp_firstdst", {"33"}, -1, PUT));

        for (int i = 0; i != det.size(); ++i) {
            det.setFirstUDPDestination(prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(proxy.Call("udp_firstdst", {}, -1, GET));
    }
}

TEST_CASE("udp_dstip", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    REQUIRE_THROWS(proxy.Call("udp_dstip", {"0.0.0.0"}, -1, PUT));
}

TEST_CASE("udp_srcmac", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto prev_val = det.getSourceUDPMAC();
    REQUIRE_THROWS(proxy.Call("udp_srcmac", {"00:00:00:00:00:00"}, -1, PUT));
    {
        std::ostringstream oss;
        proxy.Call("udp_srcmac", {"00:50:c2:42:34:12"}, -1, PUT, oss);
        REQUIRE(oss.str() == "udp_srcmac 00:50:c2:42:34:12\n");
    }
    for (int i = 0; i != det.size(); ++i) {
        if (prev_val[i].str() != "00:00:00:00:00:00") {
            det.setSourceUDPMAC(prev_val[i], {i});
        }
    }
}

TEST_CASE("udp_dstmac", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    REQUIRE_THROWS(proxy.Call("udp_dstmac", {"00:00:00:00:00:00"}, -1, PUT));
}

TEST_CASE("udp_dstport", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto prev_val = det.getDestinationUDPPort();
    {
        std::ostringstream oss;
        proxy.Call("udp_dstport", {"50084"}, -1, PUT, oss);
        REQUIRE(oss.str() == "udp_dstport 50084\n");
    }
    for (int i = 0; i != det.size(); ++i) {
        det.setDestinationUDPPort(prev_val[i], {i});
    }
}

TEST_CASE("udp_srcip2", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU || det_type == defs::GOTTHARD2) {
        auto prev_val = det.getSourceUDPIP2();
        REQUIRE_THROWS(proxy.Call("udp_srcip2", {"0.0.0.0"}, -1, PUT));
        {
            std::ostringstream oss;
            proxy.Call("udp_srcip2", {"129.129.205.12"}, -1, PUT, oss);
            REQUIRE(oss.str() == "udp_srcip2 129.129.205.12\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            if (prev_val[i] != IpAddr{"0.0.0.0"})
                det.setSourceUDPIP2(prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(proxy.Call("udp_srcip2", {}, -1, GET));
    }
}

TEST_CASE("udp_dstip2", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU || det_type == defs::GOTTHARD2) {
        REQUIRE_THROWS(proxy.Call("udp_dstip2", {"0.0.0.0"}, -1, PUT));
    } else {
        REQUIRE_THROWS(proxy.Call("udp_dstip2", {}, -1, GET));
    }
}

TEST_CASE("udp_srcmac2", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU || det_type == defs::GOTTHARD2) {
        auto prev_val = det.getSourceUDPMAC2();
        REQUIRE_THROWS(
            proxy.Call("udp_srcmac2", {"00:00:00:00:00:00"}, -1, PUT));
        {
            std::ostringstream oss;
            proxy.Call("udp_srcmac2", {"00:50:c2:42:34:12"}, -1, PUT, oss);
            REQUIRE(oss.str() == "udp_srcmac2 00:50:c2:42:34:12\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            if (prev_val[i].str() != "00:00:00:00:00:00") {
                det.setSourceUDPMAC2(prev_val[i], {i});
            }
        }
    } else {
        REQUIRE_THROWS(proxy.Call("udp_srcmac2", {}, -1, GET));
    }
}

TEST_CASE("udp_dstmac2", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU || det_type == defs::GOTTHARD2) {
        REQUIRE_THROWS(
            proxy.Call("udp_dstmac2", {"00:00:00:00:00:00"}, -1, PUT));
    } else {
        REQUIRE_THROWS(proxy.Call("udp_dstmac2", {}, -1, GET));
    }
}

TEST_CASE("udp_dstport2", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU || det_type == defs::GOTTHARD2 ||
        det_type == defs::EIGER) {
        auto prev_val = det.getDestinationUDPPort2();
        {
            std::ostringstream oss;
            proxy.Call("udp_dstport2", {"50084"}, -1, PUT, oss);
            REQUIRE(oss.str() == "udp_dstport2 50084\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setDestinationUDPPort2(prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(proxy.Call("udp_dstport2", {}, -1, GET));
    }
}

TEST_CASE("udp_reconfigure", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    REQUIRE_THROWS(proxy.Call("udp_reconfigure", {}, -1, GET));
    REQUIRE_NOTHROW(proxy.Call("udp_reconfigure", {}, -1, PUT));
}

TEST_CASE("udp_validate", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    REQUIRE_THROWS(proxy.Call("udp_validate", {}, -1, GET));
    REQUIRE_NOTHROW(proxy.Call("udp_validate", {}, -1, PUT));
}

TEST_CASE("tengiga", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);

    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::EIGER || det_type == defs::CHIPTESTBOARD ||
        det_type == defs::MOENCH || det_type == defs::MYTHEN3) {
        auto tengiga = det.getTenGiga();
        det.setTenGiga(false);

        std::ostringstream oss1, oss2;
        proxy.Call("tengiga", {"1"}, -1, PUT, oss1);
        REQUIRE(oss1.str() == "tengiga 1\n");
        proxy.Call("tengiga", {}, -1, GET, oss2);
        REQUIRE(oss2.str() == "tengiga 1\n");

        for (int i = 0; i != det.size(); ++i) {
            det.setTenGiga(tengiga[i], {i});
        }
    } else {
        REQUIRE_THROWS(proxy.Call("tengiga", {}, -1, GET));
    }
}

TEST_CASE("flowcontrol10g", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::EIGER || det_type == defs::JUNGFRAU) {
        auto prev_val = det.getTenGigaFlowControl();
        {
            std::ostringstream oss;
            proxy.Call("flowcontrol10g", {"1"}, -1, PUT, oss);
            REQUIRE(oss.str() == "flowcontrol10g 1\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("flowcontrol10g", {"0"}, -1, PUT, oss);
            REQUIRE(oss.str() == "flowcontrol10g 0\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("flowcontrol10g", {}, -1, GET, oss);
            REQUIRE(oss.str() == "flowcontrol10g 0\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setTenGigaFlowControl(prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(proxy.Call("flowcontrol10g", {}, -1, GET));
    }
}

TEST_CASE("txdelay_frame", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::EIGER || det_type == defs::JUNGFRAU ||
        det_type == defs::MYTHEN3) {
        auto prev_val = det.getTransmissionDelayFrame();
        auto val = 5000;
        if (det_type == defs::JUNGFRAU || det_type == defs::MYTHEN3) {
            val = 5;
        }
        std::string sval = std::to_string(val);
        {
            std::ostringstream oss1, oss2;
            proxy.Call("txdelay_frame", {sval}, -1, PUT, oss1);
            REQUIRE(oss1.str() == "txdelay_frame " + sval + "\n");
            proxy.Call("txdelay_frame", {}, -1, GET, oss2);
            REQUIRE(oss2.str() == "txdelay_frame " + sval + "\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setTransmissionDelayFrame(prev_val[i]);
        }
    } else {
        REQUIRE_THROWS(proxy.Call("txdelay_frame", {}, -1, GET));
    }
}

TEST_CASE("txdelay", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::EIGER || det_type == defs::JUNGFRAU ||
        det_type == defs::MYTHEN3) {

        // cannot get transmission delay with just one module
        if ((det_type == defs::JUNGFRAU || det_type == defs::MYTHEN3) &&
            (det.size() < 2)) {
            REQUIRE_THROWS(proxy.Call("txdelay", {}, -1, GET));
            int val = 5;
            std::string sval = std::to_string(val);
            {
                std::ostringstream oss1;
                proxy.Call("txdelay", {sval}, -1, PUT, oss1);
                REQUIRE(oss1.str() == "txdelay " + sval + "\n");
            }
        }

        else {
            Result<int> prev_left, prev_right;
            bool eiger = false;
            if (det_type == defs::EIGER) {
                eiger = true;
                prev_left = det.getTransmissionDelayLeft();
                prev_right = det.getTransmissionDelayRight();
            }
            auto prev_frame = det.getTransmissionDelayFrame();
            auto val = 5000;
            if (det_type == defs::JUNGFRAU || det_type == defs::MYTHEN3) {
                val = 5;
            }
            std::string sval = std::to_string(val);
            {
                std::ostringstream oss1, oss2;
                proxy.Call("txdelay", {sval}, -1, PUT, oss1);
                REQUIRE(oss1.str() == "txdelay " + sval + "\n");
                proxy.Call("txdelay", {}, -1, GET, oss2);
                REQUIRE(oss2.str() == "txdelay " + sval + "\n");
            }
            // test other mods
            for (int i = 0; i != det.size(); ++i) {
                if (eiger) {
                    REQUIRE(det.getTransmissionDelayLeft({i}).squash(-1) ==
                            (2 * i * val));
                    REQUIRE(det.getTransmissionDelayRight({i}).squash(-1) ==
                            ((2 * i + 1) * val));
                    REQUIRE(det.getTransmissionDelayFrame({i}).squash(-1) ==
                            (2 * det.size() * val));
                } else {
                    REQUIRE(det.getTransmissionDelayFrame({i}).squash(-1) ==
                            (i * val));
                }
            }
            // not a module level command
            REQUIRE_THROWS(proxy.Call("txdelay", {"5"}, 0, PUT));
            REQUIRE_THROWS(proxy.Call("txdelay", {}, 0, GET));

            for (int i = 0; i != det.size(); ++i) {
                if (eiger) {
                    det.setTransmissionDelayLeft(prev_left[i]);
                    det.setTransmissionDelayRight(prev_right[i]);
                }
                det.setTransmissionDelayFrame(prev_frame[i]);
            }
        }
    } else {
        REQUIRE_THROWS(proxy.Call("txdelay", {}, -1, GET));
    }
}

/* ZMQ Streaming Parameters (Receiver<->Client) */

TEST_CASE("zmqport", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);

    int socketsperdetector = 1;
    auto det_type = det.getDetectorType().squash();
    int prev = 1;
    if (det_type == defs::EIGER) {
        socketsperdetector *= 2;
    } else if (det_type == defs::JUNGFRAU) {
        prev = det.getNumberofUDPInterfaces().squash();
        det.setNumberofUDPInterfaces(2);
        socketsperdetector *= 2;
    }
    int port = 3500;
    auto port_str = std::to_string(port);
    {
        std::ostringstream oss;
        proxy.Call("zmqport", {port_str}, -1, PUT, oss);
        REQUIRE(oss.str() == "zmqport " + port_str + '\n');
    }
    for (int i = 0; i != det.size(); ++i) {
        std::ostringstream oss;
        proxy.Call("zmqport", {}, i, GET, oss);
        REQUIRE(oss.str() == "zmqport " +
                                 std::to_string(port + i * socketsperdetector) +
                                 '\n');
    }

    port = 1954;
    port_str = std::to_string(port);
    {
        std::ostringstream oss;
        proxy.Call("zmqport", {port_str}, -1, PUT, oss);
        REQUIRE(oss.str() == "zmqport " + port_str + '\n');
    }
    for (int i = 0; i != det.size(); ++i) {
        std::ostringstream oss;
        proxy.Call("zmqport", {}, i, GET, oss);
        REQUIRE(oss.str() == "zmqport " +
                                 std::to_string(port + i * socketsperdetector) +
                                 '\n');
    }
    if (det_type == defs::JUNGFRAU) {
        det.setNumberofUDPInterfaces(prev);
    }
}

TEST_CASE("zmqip", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    std::ostringstream oss1, oss2;
    auto zmqip = det.getClientZmqIp();
    proxy.Call("zmqip", {}, 0, GET, oss1);
    REQUIRE(oss1.str() == "zmqip " + zmqip[0].str() + '\n');

    proxy.Call("zmqip", {zmqip[0].str()}, 0, PUT, oss2);
    REQUIRE(oss2.str() == "zmqip " + zmqip[0].str() + '\n');

    for (int i = 0; i != det.size(); ++i) {
        det.setRxZmqIP(zmqip[i], {i});
    }
}

TEST_CASE("zmqhwm", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto prev_val = det.getClientZmqHwm();
    {
        std::ostringstream oss;
        proxy.Call("zmqhwm", {"50"}, -1, PUT, oss);
        REQUIRE(oss.str() == "zmqhwm 50\n");
    }
    {
        std::ostringstream oss;
        proxy.Call("zmqhwm", {}, -1, GET, oss);
        REQUIRE(oss.str() == "zmqhwm 50\n");
    }
    {
        std::ostringstream oss;
        proxy.Call("zmqhwm", {"0"}, -1, PUT, oss);
        REQUIRE(oss.str() == "zmqhwm 0\n");
    }
    {
        std::ostringstream oss;
        proxy.Call("zmqhwm", {"-1"}, -1, PUT, oss);
        REQUIRE(oss.str() == "zmqhwm -1\n");
    }
    det.setClientZmqHwm(prev_val);
}

/* Advanced */

TEST_CASE("programfpga", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::CHIPTESTBOARD || det_type == defs::MOENCH ||
        det_type == defs::JUNGFRAU || det_type == defs::MYTHEN3 ||
        det_type == defs::GOTTHARD2) {
        // TODO program a real board?
        /// afs/psi.ch/project/sls_det_firmware/jungfrau_firmware/cyclone_V/v0_8/Jungfrau_MCB.pof
        REQUIRE_THROWS(proxy.Call("programfpga", {}, -1, GET));
    } else {
        REQUIRE_THROWS(proxy.Call("programfpga", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("programfpga", {"/tmp/test.pof"}, -1, PUT));
    }
}

TEST_CASE("resetfpga", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU || det_type == defs::CHIPTESTBOARD ||
        det_type == defs::MOENCH) {
        // reset will also reset udp info from config file (comment out for
        // invdividual tests) std::ostringstream oss; proxy.Call("resetfpga",
        // {}, -1, PUT, oss); REQUIRE(oss.str() == "resetfpga successful\n");
        REQUIRE_THROWS(proxy.Call("resetfpga", {}, -1, GET));
    } else {
        REQUIRE_THROWS(proxy.Call("resetfpga", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("resetfpga", {}, -1, PUT));
    }
}

TEST_CASE("updatekernel", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU || det_type == defs::CHIPTESTBOARD ||
        det_type == defs::MOENCH || det_type == defs::MYTHEN3 ||
        det_type == defs::GOTTHARD2) {
        // TODO: send real server?
        // std::ostringstream oss;
        // proxy.Call("updatekernel",{"juImage_detector.lzma",
        // "pc13784"}, -1, PUT, oss);
        // REQUIRE(oss.str() == "updatekernel successful\n");
        REQUIRE_THROWS(proxy.Call("updatekernel", {}, -1, GET));
    } else {
        REQUIRE_THROWS(proxy.Call("updatekernel", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("updatekernel", {}, -1, PUT));
    }
}

TEST_CASE("rebootcontroller", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU || det_type == defs::CHIPTESTBOARD ||
        det_type == defs::MOENCH || det_type == defs::MYTHEN3 ||
        det_type == defs::GOTTHARD2 || det_type == defs::GOTTHARD) {
        // TODO: reboot real server?
        // REQUIRE_NOTHROW(proxy.Call("rebootcontroller", {}, -1, PUT));
        REQUIRE_THROWS(proxy.Call("rebootcontroller", {}, -1, GET));
    } else {
        REQUIRE_THROWS(proxy.Call("rebootcontroller", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("rebootcontroller", {}, -1, PUT));
    }
}

TEST_CASE("update", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU || det_type == defs::CHIPTESTBOARD ||
        det_type == defs::MOENCH) {
        // TODO: update real server and firmware?
        // REQUIRE_NOTHROW(proxy.Call("update",
        // {"jungfrauDetectorServerv4.0.1.0", "pc13784",
        // "/afs/psi.ch/project/sls_det_firmware/jungfrau_firmware/cyclone_V/v0_8/Jungfrau_MCB.pof"},
        // -1, PUT));
        REQUIRE_THROWS(proxy.Call("update", {}, -1, GET));
    } else {
        REQUIRE_THROWS(proxy.Call("update", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("update", {}, -1, PUT));
    }
}

TEST_CASE("reg", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type != defs::EIGER) {
        uint32_t addr = 0x64;
        std::string saddr = ToStringHex(addr);
        auto prev_val = det.readRegister(addr);
        {
            std::ostringstream oss1, oss2;
            proxy.Call("reg", {saddr, "0x5"}, -1, PUT, oss1);
            REQUIRE(oss1.str() == "reg [" + saddr + ", 0x5]\n");
            proxy.Call("reg", {saddr}, -1, GET, oss2);
            REQUIRE(oss2.str() == "reg 0x5\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.writeRegister(addr, prev_val[i], {i});
        }
    }
    // cannot check for eiger virtual server
    else {
        REQUIRE_NOTHROW(proxy.Call("reg", {"0x64"}, -1, GET));
    }
}

TEST_CASE("adcreg", "[.cmd]") {
    // TODO! what is a safe value to use?
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU || det_type == defs::CHIPTESTBOARD ||
        det_type == defs::MOENCH || det_type == defs::GOTTHARD) {
        std::ostringstream oss;
        proxy.Call("adcreg", {"0x8", "0x3"}, -1, PUT, oss);
        REQUIRE(oss.str() == "adcreg [0x8, 0x3]\n");
        // This is a put only command
        REQUIRE_THROWS(proxy.Call("adcreg", {}, -1, GET));
    } else {
        REQUIRE_THROWS(proxy.Call("adcreg", {"0x0", "0"}, -1, PUT));
        REQUIRE_THROWS(proxy.Call("adcreg", {}, -1, GET));
    }
}

TEST_CASE("setbit", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type != defs::EIGER) {
        uint32_t addr = 0x64;
        std::string saddr = ToStringHex(addr);
        auto prev_val = det.readRegister(addr);
        {
            std::ostringstream oss1, oss2;
            proxy.Call("reg", {saddr, "0x0"}, -1, PUT);
            proxy.Call("setbit", {saddr, "1"}, -1, PUT, oss1);
            REQUIRE(oss1.str() == "setbit [" + saddr + ", 1]\n");
            proxy.Call("reg", {saddr}, -1, GET, oss2);
            REQUIRE(oss2.str() == "reg 0x2\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.writeRegister(addr, prev_val[i], {i});
        }
    }
}

TEST_CASE("clearbit", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type != defs::EIGER) {
        uint32_t addr = 0x64;
        std::string saddr = ToStringHex(addr);
        auto prev_val = det.readRegister(addr);
        {
            std::ostringstream oss1, oss2;
            proxy.Call("reg", {saddr, "0x3"}, -1, PUT);
            proxy.Call("clearbit", {saddr, "1"}, -1, PUT, oss1);
            REQUIRE(oss1.str() == "clearbit [" + saddr + ", 1]\n");
            proxy.Call("reg", {saddr}, -1, GET, oss2);
            REQUIRE(oss2.str() == "reg 0x1\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.writeRegister(addr, prev_val[i], {i});
        }
    }
}

TEST_CASE("getbit", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type != defs::EIGER) {
        uint32_t addr = 0x64;
        std::string saddr = ToStringHex(addr);
        auto prev_val = det.readRegister(addr);
        {
            std::ostringstream oss1, oss2;
            proxy.Call("reg", {saddr, "0x3"}, -1, PUT);
            proxy.Call("getbit", {saddr, "1"}, -1, GET, oss1);
            REQUIRE(oss1.str() == "getbit 1\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.writeRegister(addr, prev_val[i], {i});
        }
    }
    // cannot check for eiger virtual server
    else {
        REQUIRE_NOTHROW(proxy.Call("getbit", {"0x64", "1"}, -1, GET));
    }
}

TEST_CASE("firmwaretest", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU || det_type == defs::CHIPTESTBOARD ||
        det_type == defs::MOENCH || det_type == defs::GOTTHARD ||
        det_type == defs::MYTHEN3 || det_type == defs::GOTTHARD2) {
        std::ostringstream oss;
        proxy.Call("firmwaretest", {}, -1, PUT, oss);
        REQUIRE(oss.str() == "firmwaretest successful\n");
        REQUIRE_THROWS(proxy.Call("firmwaretest", {}, -1, GET));
    } else {
        REQUIRE_THROWS(proxy.Call("firmwaretest", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("firmwaretest", {}, -1, PUT));
    }
}

TEST_CASE("bustest", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU || det_type == defs::CHIPTESTBOARD ||
        det_type == defs::MOENCH || det_type == defs::GOTTHARD ||
        det_type == defs::MYTHEN3 || det_type == defs::GOTTHARD2) {
        std::ostringstream oss;
        proxy.Call("bustest", {}, -1, PUT, oss);
        REQUIRE(oss.str() == "bustest successful\n");
        REQUIRE_THROWS(proxy.Call("bustest", {}, -1, GET));
    } else {
        REQUIRE_THROWS(proxy.Call("bustest", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("bustest", {}, -1, PUT));
    }
}

TEST_CASE("initialchecks", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto check = det.getInitialChecks();
    {
        std::ostringstream oss;
        proxy.Call("initialchecks", {"0"}, -1, PUT, oss);
        REQUIRE(oss.str() == "initialchecks 0\n");
    }
    {
        std::ostringstream oss;
        proxy.Call("initialchecks", {}, -1, GET, oss);
        REQUIRE(oss.str() == "initialchecks 0\n");
    }
    {
        std::ostringstream oss;
        proxy.Call("initialchecks", {}, -1, GET, oss);
        REQUIRE(oss.str() == "initialchecks 0\n");
    }
    det.setInitialChecks(check);
}

TEST_CASE("adcinvert", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::CHIPTESTBOARD || det_type == defs::MOENCH ||
        det_type == defs::JUNGFRAU) {
        auto prev_val = det.getADCInvert();
        {
            std::ostringstream oss;
            proxy.Call("adcinvert", {"0x8d0a21d4"}, -1, PUT, oss);
            REQUIRE(oss.str() == "adcinvert 0x8d0a21d4\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("adcinvert", {}, -1, GET, oss);
            REQUIRE(oss.str() == "adcinvert 0x8d0a21d4\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setADCInvert(prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(proxy.Call("adcinvert", {}, -1, GET));
    }
}

/* Insignificant */

TEST_CASE("port", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto prev_val = det.getControlPort({0}).squash();
    {
        std::ostringstream oss;
        proxy.Call("port", {"1942"}, 0, PUT, oss);
        REQUIRE(oss.str() == "port 1942\n");
    }
    {
        std::ostringstream oss;
        proxy.Call("port", {}, 0, GET, oss);
        REQUIRE(oss.str() == "port 1942\n");
    }
    det.setControlPort(prev_val, {0});
}

TEST_CASE("stopport", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto prev_val = det.getStopPort({0}).squash();
    {
        std::ostringstream oss;
        proxy.Call("stopport", {"1942"}, 0, PUT, oss);
        REQUIRE(oss.str() == "stopport 1942\n");
    }
    {
        std::ostringstream oss;
        proxy.Call("stopport", {}, 0, GET, oss);
        REQUIRE(oss.str() == "stopport 1942\n");
    }
    det.setStopPort(prev_val, {0});
}

TEST_CASE("lock", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto prev_val = det.getDetectorLock();
    {
        std::ostringstream oss;
        proxy.Call("lock", {"1"}, -1, PUT, oss);
        REQUIRE(oss.str() == "lock 1\n");
    }
    {
        std::ostringstream oss;
        proxy.Call("lock", {}, -1, GET, oss);
        REQUIRE(oss.str() == "lock 1\n");
    }
    {
        std::ostringstream oss;
        proxy.Call("lock", {"0"}, -1, PUT, oss);
        REQUIRE(oss.str() == "lock 0\n");
    }
    for (int i = 0; i != det.size(); ++i) {
        det.setDetectorLock(prev_val[i], {i});
    }
}

TEST_CASE("execcommand", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    REQUIRE_NOTHROW(proxy.Call("execcommand", {"ls"}, -1, PUT));
}

TEST_CASE("framecounter", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU || det_type == defs::CHIPTESTBOARD ||
        det_type == defs::MOENCH || det_type == defs::MYTHEN3 ||
        det_type == defs::GOTTHARD2) {
        auto framecounter = det.getNumberOfFramesFromStart().squash();
        std::ostringstream oss;
        proxy.Call("framecounter", {}, -1, GET, oss);
        REQUIRE(oss.str() ==
                "framecounter " + std::to_string(framecounter) + "\n");
        REQUIRE_NOTHROW(proxy.Call("framecounter", {}, -1, GET));
    } else {
        REQUIRE_THROWS(proxy.Call("framecounter", {}, -1, GET));
    }
}

TEST_CASE("runtime", "[.cmd]") {
    // TODO! can we test this?
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU || det_type == defs::CHIPTESTBOARD ||
        det_type == defs::MOENCH || det_type == defs::MYTHEN3 ||
        det_type == defs::GOTTHARD2) {
        std::ostringstream oss;
        proxy.Call("runtime", {}, -1, GET, oss);
        // Get only
        REQUIRE_THROWS(proxy.Call("runtime", {"2019"}, -1, PUT));
        REQUIRE_NOTHROW(proxy.Call("runtime", {}, -1, GET));
    } else {
        REQUIRE_THROWS(proxy.Call("runtime", {}, -1, GET));
    }
}

TEST_CASE("frametime", "[.cmd]") {
    // TODO! can we test this?
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU || det_type == defs::CHIPTESTBOARD ||
        det_type == defs::MOENCH || det_type == defs::MYTHEN3 ||
        det_type == defs::GOTTHARD2) {
        std::ostringstream oss;
        proxy.Call("frametime", {}, -1, GET, oss);
        // Get only
        REQUIRE_THROWS(proxy.Call("frametime", {"2019"}, -1, PUT));
        REQUIRE_NOTHROW(proxy.Call("frametime", {}, -1, GET));
    } else {
        REQUIRE_THROWS(proxy.Call("frametime", {}, -1, GET));
    }
}

TEST_CASE("user", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    proxy.Call("user", {}, -1, GET);

    // This is a get only command
    REQUIRE_THROWS(proxy.Call("user", {}, -1, PUT));
    REQUIRE_NOTHROW(proxy.Call("user", {}, -1, GET));
}

} // namespace sls
