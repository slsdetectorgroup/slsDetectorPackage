// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "Caller.h"
#include "catch.hpp"
#include "sls/Detector.h"
#include "sls/file_utils.h"
#include "sls/sls_detector_defs.h"
#include "test-Caller-global.h"

#include <chrono>
#include <sstream>
#include <thread>

#include "tests/globals.h"

namespace sls {

using test::GET;
using test::PUT;

TEST_CASE("Calling help doesn't throw or cause segfault") {
    // Dont add [.cmdcall] tag this should run with normal tests
    Caller caller(nullptr);
    std::ostringstream os;
    for (std::string cmd : caller.getAllCommands())
        REQUIRE_NOTHROW(
            caller.call(cmd, {}, -1, slsDetectorDefs::HELP_ACTION, os));
}

TEST_CASE("Unknown command", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    REQUIRE_THROWS(caller.call("vsaevrreavv", {}, -1, PUT));
}

/* configuration */

TEST_CASE("config", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    // put only
    REQUIRE_THROWS(caller.call("config", {}, -1, GET));
}

// free: not testing

TEST_CASE("parameters", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    // put only
    REQUIRE_THROWS(caller.call("parameters", {}, -1, GET));
    /*
        auto prev_val = det.getNumberOfFrames().tsquash("Number of frames has to
       be same to test");
        {
            system("echo 'frames 2' > /tmp/tempsetup.det ");
            std::ostringstream oss;
            caller.call("parameters", {"/tmp/tempsetup.det"}, -1, PUT, oss);
            REQUIRE(oss.str() == "parameters /tmp/tempsetup.det\n");
            REQUIRE(det.getNumberOfFrames().tsquash("failed") == 2);
        }
        {
            system("echo '0:frames 1' > /tmp/tempsetup.det ");
            std::ostringstream oss;
            caller.call("parameters", {"/tmp/tempsetup.det"}, -1, PUT, oss);
            REQUIRE(oss.str() == "parameters /tmp/tempsetup.det\n");
            REQUIRE(det.getNumberOfFrames({0}).tsquash("failed") == 1);
        }
        det.setNumberOfFrames(prev_val);
        */
}

TEST_CASE("hostname", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    REQUIRE_NOTHROW(caller.call("hostname", {}, -1, GET));
}

TEST_CASE("virtual", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    REQUIRE_THROWS(caller.call("virtual", {}, -1, GET));
    test_valid_port_caller("virtual", {"1"}, -1, PUT);
    REQUIRE_THROWS(caller.call("virtual", {"3", "65534"}, -1, PUT));
}

TEST_CASE("versions", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    REQUIRE_NOTHROW(caller.call("versions", {}, -1, GET));
    REQUIRE_THROWS(caller.call("versions", {"0"}, -1, PUT));
}

TEST_CASE("packageversion", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    REQUIRE_NOTHROW(caller.call("packageversion", {}, -1, GET));
    REQUIRE_THROWS(caller.call("packageversion", {"0"}, -1, PUT));
}

TEST_CASE("clientversion", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    REQUIRE_NOTHROW(caller.call("clientversion", {}, -1, GET));
    REQUIRE_THROWS(caller.call("clientversion", {"0"}, -1, PUT));
}

TEST_CASE("firmwareversion", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    REQUIRE_NOTHROW(caller.call("firmwareversion", {}, -1, GET));
    REQUIRE_THROWS(caller.call("firmwareversion", {"0"}, -1, PUT));
}

TEST_CASE("detectorserverversion", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    REQUIRE_NOTHROW(caller.call("detectorserverversion", {}, -1, GET));
    REQUIRE_THROWS(caller.call("detectorserverversion", {"0"}, -1, PUT));
}

TEST_CASE("hardwareversion", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    REQUIRE_NOTHROW(caller.call("hardwareversion", {}, -1, GET));
    REQUIRE_THROWS(caller.call("hardwareversion", {"0"}, -1, PUT));
}

TEST_CASE("kernelversion", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    REQUIRE_NOTHROW(caller.call("kernelversion", {}, -1, GET));
    REQUIRE_THROWS(caller.call("kernelversion", {"0"}, -1, PUT));
}

TEST_CASE("serialnumber", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::EIGER || det_type == defs::XILINX_CHIPTESTBOARD) {
        REQUIRE_THROWS(caller.call("serialnumber", {}, -1, GET));
    } else {
        REQUIRE_NOTHROW(caller.call("serialnumber", {}, -1, GET));
    }
}

TEST_CASE("moduleid", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::GOTTHARD2 || det_type == defs::MYTHEN3 ||
        det_type == defs::EIGER || det_type == defs::JUNGFRAU ||
        det_type == defs::MOENCH) {
        REQUIRE_NOTHROW(caller.call("moduleid", {}, -1, GET));
    } else {
        REQUIRE_THROWS(caller.call("moduleid", {}, -1, GET));
    }
}

TEST_CASE("type", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto dt = det.getDetectorType().squash();

    std::ostringstream oss;
    caller.call("type", {}, -1, GET, oss);
    auto ans = oss.str().erase(0, strlen("type "));
    REQUIRE(ans == ToString(dt) + '\n');
    // REQUIRE(dt == test::type);
}

TEST_CASE("detsize", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    REQUIRE_NOTHROW(caller.call("detsize", {}, -1, GET));
}

TEST_CASE("settingslist", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::CHIPTESTBOARD ||
        det_type == defs::XILINX_CHIPTESTBOARD) {
        REQUIRE_THROWS(caller.call("settingslist", {}, -1, GET));
    } else {
        REQUIRE_NOTHROW(caller.call("settingslist", {}, -1, GET));
        REQUIRE_THROWS(caller.call("settingslist", {}, -1, PUT));
    }
}

TEST_CASE("settings", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
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
    case defs::MYTHEN3:
        sett.push_back("standard");
        sett.push_back("fast");
        sett.push_back("highgain");
        break;
    default:
        if (det_type == defs::EIGER) {
            // FIXME: need to remove when settings removed
            REQUIRE_NOTHROW(caller.call("settings", {}, -1, GET));
            REQUIRE_THROWS(caller.call("settings", {"standard"}, -1, PUT));
        } else {
            REQUIRE_THROWS(caller.call("settings", {}, -1, GET));
        }
        return;
    }

    auto prev_val = det.getSettings();
    for (auto &it : sett) {
        {
            std::ostringstream oss;
            caller.call("settings", {it}, -1, PUT, oss);
            REQUIRE(oss.str() == "settings " + it + "\n");
        }
        {
            std::ostringstream oss;
            caller.call("settings", {}, -1, GET, oss);
            REQUIRE(oss.str() == "settings " + it + "\n");
        }
    }
    for (auto &it : allSett) {
        if (std::find(sett.begin(), sett.end(), it) == sett.end()) {
            REQUIRE_THROWS(caller.call("settings", {it}, -1, PUT));
        }
    }
    for (int i = 0; i != det.size(); ++i) {
        if (prev_val[i] != defs::UNDEFINED &&
            prev_val[i] != defs::UNINITIALIZED) {
            det.setSettings(prev_val[i], {i});
        }
    }
}

TEST_CASE("threshold", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);

    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::EIGER) {
        auto prev_threshold = det.getThresholdEnergy();
        auto prev_energies =
            det.getTrimEnergies().tsquash("inconsistent trim energies to test");
        if (!prev_energies.empty()) {
            std::string senergy = std::to_string(prev_energies[0]);
            std::ostringstream oss1, oss2;
            caller.call("threshold", {senergy, "standard"}, -1, PUT, oss1);
            REQUIRE(oss1.str() == "threshold [" + senergy + ", standard]\n");
            caller.call("threshold", {}, -1, GET, oss2);
            REQUIRE(oss2.str() == "threshold " + senergy + "\n");

            REQUIRE_THROWS(caller.call(
                "threshold", {senergy, senergy, senergy, "standard"}, -1, PUT));
            REQUIRE_THROWS(
                caller.call("threshold", {senergy, "undefined"}, -1, PUT));

            det.setTrimEnergies(prev_energies);
            for (int i = 0; i != det.size(); ++i) {
                if (prev_threshold[i] >= 0) {
                    det.setThresholdEnergy(prev_threshold[i], defs::STANDARD,
                                           true, {i});
                }
            }
        }
        REQUIRE_NOTHROW(caller.call("threshold", {}, -1, GET));
    } else if (det_type == defs::MYTHEN3) {
        auto prev_threshold = det.getAllThresholdEnergy();
        auto prev_settings =
            det.getSettings().tsquash("inconsistent settings to test");
        auto prev_energies =
            det.getTrimEnergies().tsquash("inconsistent trim energies to test");
        if (!prev_energies.empty()) {
            std::string senergy = std::to_string(prev_energies[0]);
            std::ostringstream oss1, oss2;
            caller.call("threshold", {senergy, "standard"}, -1, PUT, oss1);
            REQUIRE(oss1.str() == "threshold [" + senergy + ", standard]\n");
            caller.call("threshold", {}, -1, GET, oss2);
            REQUIRE(oss2.str() == "threshold [" + senergy + ", " + senergy +
                                      ", " + senergy + "]\n");
            std::string senergy2 = std::to_string(prev_energies[1]);
            std::string senergy3 = std::to_string(prev_energies[2]);
            std::ostringstream oss3, oss4;
            caller.call("threshold", {senergy, senergy2, senergy3, "standard"},
                        -1, PUT, oss3);
            REQUIRE(oss3.str() == "threshold [" + senergy + ", " + senergy2 +
                                      ", " + senergy3 + ", standard]\n");
            caller.call("threshold", {}, -1, GET, oss4);
            REQUIRE(oss4.str() == "threshold [" + senergy + ", " + senergy2 +
                                      ", " + senergy3 + "]\n");

            REQUIRE_THROWS(caller.call(
                "threshold", {senergy, senergy, "standard"}, -1, PUT));
            REQUIRE_THROWS(
                caller.call("threshold", {senergy, "undefined"}, -1, PUT));
            REQUIRE_NOTHROW(caller.call("threshold", {senergy}, -1, PUT));
            REQUIRE_NOTHROW(caller.call(
                "threshold", {senergy, senergy2, senergy3}, -1, PUT));
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
        REQUIRE_NOTHROW(caller.call("threshold", {}, -1, GET));
    } else {
        REQUIRE_THROWS(caller.call("threshold", {}, -1, GET));
    }
}

TEST_CASE("thresholdnotb", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);

    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::EIGER) {
        auto prev_threshold = det.getThresholdEnergy();
        auto prev_energies =
            det.getTrimEnergies().tsquash("inconsistent trim energies to test");
        if (!prev_energies.empty()) {
            std::string senergy = std::to_string(prev_energies[0]);
            std::ostringstream oss1, oss2;
            caller.call("thresholdnotb", {senergy, "standard"}, -1, PUT, oss1);
            REQUIRE(oss1.str() ==
                    "thresholdnotb [" + senergy + ", standard]\n");
            caller.call("threshold", {}, -1, GET, oss2);
            REQUIRE(oss2.str() == "threshold " + senergy + "\n");
            REQUIRE_THROWS(caller.call("thresholdnotb",
                                       {senergy, senergy, senergy, "standard"},
                                       -1, PUT));
            REQUIRE_THROWS(
                caller.call("thresholdnotb", {senergy, "undefined"}, -1, PUT));
            det.setTrimEnergies(prev_energies);
            for (int i = 0; i != det.size(); ++i) {
                if (prev_threshold[i] >= 0) {
                    det.setThresholdEnergy(prev_threshold[i], defs::STANDARD,
                                           false, {i});
                }
            }
        }
        REQUIRE_NOTHROW(caller.call("threshold", {}, -1, GET));
    } else if (det_type == defs::MYTHEN3) {
        auto prev_threshold = det.getAllThresholdEnergy();
        auto prev_settings =
            det.getSettings().tsquash("inconsistent settings to test");
        auto prev_energies =
            det.getTrimEnergies().tsquash("inconsistent trim energies to test");
        if (!prev_energies.empty()) {
            std::string senergy = std::to_string(prev_energies[0]);
            std::ostringstream oss1, oss2;
            caller.call("thresholdnotb", {senergy, "standard"}, -1, PUT, oss1);
            REQUIRE(oss1.str() ==
                    "thresholdnotb [" + senergy + ", standard]\n");
            caller.call("threshold", {}, -1, GET, oss2);
            REQUIRE(oss2.str() == "threshold [" + senergy + ", " + senergy +
                                      ", " + senergy + "]\n");
            std::string senergy2 = std::to_string(prev_energies[1]);
            std::string senergy3 = std::to_string(prev_energies[2]);
            std::ostringstream oss3, oss4;
            caller.call("thresholdnotb",
                        {senergy, senergy2, senergy3, "standard"}, -1, PUT,
                        oss3);
            REQUIRE(oss3.str() == "thresholdnotb [" + senergy + ", " +
                                      senergy2 + ", " + senergy3 +
                                      ", standard]\n");
            caller.call("threshold", {}, -1, GET, oss4);
            REQUIRE(oss4.str() == "threshold [" + senergy + ", " + senergy2 +
                                      ", " + senergy3 + "]\n");

            REQUIRE_THROWS(caller.call(
                "thresholdnotb", {senergy, senergy, "standard"}, -1, PUT));
            REQUIRE_THROWS(
                caller.call("thresholdnotb", {senergy, "undefined"}, -1, PUT));
            REQUIRE_NOTHROW(caller.call("thresholdnotb", {senergy}, -1, PUT));
            REQUIRE_NOTHROW(caller.call(
                "thresholdnotb", {senergy, senergy2, senergy3}, -1, PUT));
            det.setTrimEnergies(prev_energies);
            for (int i = 0; i != det.size(); ++i) {
                if (prev_threshold[i][0] >= 0) {
                    det.setThresholdEnergy(prev_threshold[i], prev_settings,
                                           true, {i});
                }
            }
        }
        REQUIRE_NOTHROW(caller.call("threshold", {}, -1, GET));
    } else {
        REQUIRE_THROWS(caller.call("thresholdnotb", {}, -1, GET));
    }
}

TEST_CASE("settingspath", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto prev_val = det.getSettingsPath();
    {
        std::ostringstream oss1, oss2;
        caller.call("settingspath", {"/tmp"}, -1, PUT, oss1);
        REQUIRE(oss1.str() == "settingspath /tmp\n");
        caller.call("settingspath", {}, -1, GET, oss2);
        REQUIRE(oss2.str() == "settingspath /tmp\n");
    }
    for (int i = 0; i != det.size(); ++i) {
        det.setSettingsPath(prev_val[i], {i});
    }
}

TEST_CASE("trimbits", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    REQUIRE_THROWS(caller.call("trimbits", {}, -1, GET));
}

TEST_CASE("trimval", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::MYTHEN3 || det_type == defs::EIGER) {
        auto prev_val = det.getAllTrimbits();
        {
            std::ostringstream oss;
            caller.call("trimval", {"63"}, -1, PUT, oss);
            REQUIRE(oss.str() == "trimval 63\n");
        }
        {
            std::ostringstream oss;
            caller.call("trimval", {"0"}, -1, PUT, oss);
            REQUIRE(oss.str() == "trimval 0\n");
        }
        {
            std::ostringstream oss;
            caller.call("trimval", {}, -1, GET, oss);
            REQUIRE(oss.str() == "trimval 0\n");
        }
        REQUIRE_THROWS(caller.call("trimval", {"64"}, -1, PUT));
        REQUIRE_THROWS(caller.call("trimval", {"-2"}, -1, PUT));
        for (int i = 0; i != det.size(); ++i) {
            if (prev_val[i] != -1) {
                det.setAllTrimbits(prev_val[i], {i});
            }
        }
    } else {
        REQUIRE_THROWS(caller.call("trimval", {}, -1, GET));
    }
}

TEST_CASE("trimen", "[.cmdcall][.this]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::EIGER || det_type == defs::MYTHEN3) {
        auto previous = det.getTrimEnergies();
        std::ostringstream oss1, oss2;
        caller.call("trimen", {"4500", "5400", "6400"}, -1, PUT, oss1);
        REQUIRE(oss1.str() == "trimen [4500, 5400, 6400]\n");
        caller.call("trimen", {}, -1, GET, oss2);
        REQUIRE(oss2.str() == "trimen [4500, 5400, 6400]\n");

        for (int i = 0; i != det.size(); ++i) {
            det.setTrimEnergies(previous[i], {i});
        }
    } else {
        REQUIRE_THROWS(
            caller.call("trimen", {"4500", "5400", "6400"}, -1, PUT));
        REQUIRE_THROWS(caller.call("trimen", {}, -1, GET));
    }
}

TEST_CASE("gappixels", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();

    // test eiger(quad or full module only)
    bool gapPixelTest = false;
    if (det_type == defs::JUNGFRAU || det_type == defs::MOENCH)
        gapPixelTest = true;
    else if (det_type == defs::EIGER) {
        bool quad = det.getQuad().squash(false);
        bool fullModule = (det.getModuleGeometry().y % 2 == 0);
        if (quad || fullModule) {
            gapPixelTest = true;
        }
    }

    if (gapPixelTest) {
        auto prev_val = det.getGapPixelsinCallback();
        {
            std::ostringstream oss;
            caller.call("gappixels", {"1"}, -1, PUT, oss);
            REQUIRE(oss.str() == "gappixels 1\n");
        }
        {
            std::ostringstream oss;
            caller.call("gappixels", {"0"}, -1, PUT, oss);
            REQUIRE(oss.str() == "gappixels 0\n");
        }
        {
            std::ostringstream oss;
            caller.call("gappixels", {}, -1, GET, oss);
            REQUIRE(oss.str() == "gappixels 0\n");
        }
        det.setGapPixelsinCallback(prev_val);
    } else {
        {
            std::ostringstream oss;
            caller.call("gappixels", {}, -1, GET, oss);
            REQUIRE(oss.str() == "gappixels 0\n");
        }
        {
            std::ostringstream oss;
            caller.call("gappixels", {"0"}, -1, PUT, oss);
            REQUIRE(oss.str() == "gappixels 0\n");
        }
        REQUIRE_THROWS(caller.call("gappixels", {"1"}, -1, PUT));
    }
}

TEST_CASE("fliprows", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    bool hw2 = false;
    if ((det_type == defs::JUNGFRAU || det_type == defs::MOENCH) &&
        ((det.getHardwareVersion().tsquash(
              "inconsistent serial number to test") == "2.0"))) {
        hw2 = true;
    }
    if (det_type == defs::EIGER || hw2) {
        auto previous = det.getFlipRows();
        auto previous_numudp = det.getNumberofUDPInterfaces().tsquash(
            "inconsistent number of udp interfaces to test");
        if (det_type == defs::JUNGFRAU || det_type == defs::MOENCH) {
            det.setNumberofUDPInterfaces(2);
        }
        std::ostringstream oss1, oss2, oss3;
        caller.call("fliprows", {"1"}, -1, PUT, oss1);
        REQUIRE(oss1.str() == "fliprows 1\n");
        caller.call("fliprows", {}, -1, GET, oss2);
        REQUIRE(oss2.str() == "fliprows 1\n");
        caller.call("fliprows", {"0"}, -1, PUT, oss3);
        REQUIRE(oss3.str() == "fliprows 0\n");
        for (int i = 0; i != det.size(); ++i) {
            det.setFlipRows(previous[i], {i});
        }
        if (det_type == defs::JUNGFRAU || det_type == defs::MOENCH) {
            det.setNumberofUDPInterfaces(previous_numudp);
        }
    } else {
        REQUIRE_THROWS(caller.call("fliprows", {}, -1, GET));
    }
}

TEST_CASE("master", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::EIGER || det_type == defs::MYTHEN3 ||
        det_type == defs::GOTTHARD || det_type == defs::GOTTHARD2 ||
        det_type == defs::JUNGFRAU || det_type == defs::MOENCH) {
        REQUIRE_NOTHROW(caller.call("master", {}, -1, GET));
        if (det_type == defs::EIGER || det_type == defs::GOTTHARD2 ||
            det_type == defs::JUNGFRAU || det_type == defs::MOENCH) {
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
                caller.call("master", {"0"}, 0, PUT, oss1);
                REQUIRE(oss1.str() == "master 0\n");
            }
            {
                std::ostringstream oss1;
                caller.call("master", {"1"}, 0, PUT, oss1);
                REQUIRE(oss1.str() == "master 1\n");
            }
            if (det.size() > 1) {
                REQUIRE_THROWS(caller.call("master", {"1"}, -1, PUT));
            }
            // set all to slaves, and then master
            for (int i = 0; i != det.size(); ++i) {
                det.setMaster(0, {i});
            }
            det.setMaster(1, prevMaster);
        }
    } else {
        REQUIRE_THROWS(caller.call("master", {}, -1, GET));
    }
}

TEST_CASE("badchannels", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::GOTTHARD2 || det_type == defs::MYTHEN3) {
        auto prev = det.getBadChannels();

        REQUIRE_THROWS(caller.call("badchannels", {}, -1, GET));

        std::string fname_put =
            getAbsolutePathFromCurrentProcess(TEST_FILE_NAME_BAD_CHANNELS);
        std::string fname_get = "/tmp/sls_test_channels.txt";

        REQUIRE_NOTHROW(caller.call("badchannels", {fname_put}, 0, PUT));
        REQUIRE_NOTHROW(caller.call("badchannels", {fname_get}, 0, GET));
        auto list = getChannelsFromFile(fname_get);
        std::vector<int> expected = {0, 12, 15, 40, 41, 42, 43, 44, 1279};
        REQUIRE(list == expected);

        REQUIRE_NOTHROW(caller.call("badchannels", {"none"}, 0, PUT));
        REQUIRE_NOTHROW(caller.call("badchannels", {fname_get}, 0, GET));
        list = getChannelsFromFile(fname_get);
        REQUIRE(list.empty());

        REQUIRE_NOTHROW(caller.call("badchannels", {fname_put}, 0, PUT));

        REQUIRE_NOTHROW(caller.call("badchannels", {"0"}, 0, PUT));
        REQUIRE_NOTHROW(caller.call("badchannels", {fname_get}, 0, GET));
        list = getChannelsFromFile(fname_get);
        REQUIRE(list.empty());

        REQUIRE_NOTHROW(caller.call("badchannels", {"12"}, 0, PUT));
        REQUIRE_NOTHROW(caller.call("badchannels", {fname_get}, 0, GET));
        list = getChannelsFromFile(fname_get);
        expected = {12};
        REQUIRE(list == expected);

        REQUIRE_NOTHROW(caller.call(
            "badchannels", {"0", "12,", "15", "43", "40:45", "1279"}, 0, PUT));
        REQUIRE_NOTHROW(caller.call("badchannels", {fname_get}, 0, GET));
        list = getChannelsFromFile(fname_get);
        expected = {0, 12, 15, 40, 41, 42, 43, 44, 1279};
        REQUIRE(list == expected);

        REQUIRE_NOTHROW(caller.call("badchannels", {"40:45"}, 0, PUT));
        REQUIRE_NOTHROW(caller.call("badchannels", {fname_get}, 0, GET));
        list = getChannelsFromFile(fname_get);
        expected = {40, 41, 42, 43, 44};
        REQUIRE(list == expected);

        REQUIRE_NOTHROW(caller.call("badchannels", {"5,6,7"}, 0, PUT));
        REQUIRE_NOTHROW(caller.call("badchannels", {fname_get}, 0, GET));
        list = getChannelsFromFile(fname_get);
        expected = {5, 6, 7};
        REQUIRE(list == expected);

        REQUIRE_NOTHROW(caller.call("badchannels", {"1:5,6,7"}, 0, PUT));
        REQUIRE_NOTHROW(caller.call("badchannels", {fname_get}, 0, GET));
        list = getChannelsFromFile(fname_get);
        expected = {1, 2, 3, 4, 6, 7};
        REQUIRE(list == expected);

        det.setBadChannels(prev);

    } else {
        REQUIRE_THROWS(caller.call("badchannels", {}, -1, GET));
    }
}

TEST_CASE("row", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto prev_val = det.getRow()[0];
    {
        std::ostringstream oss;
        caller.call("row", {"1"}, 0, PUT, oss);
        REQUIRE(oss.str() == "row 1\n");
    }
    {
        std::ostringstream oss;
        caller.call("row", {}, 0, GET, oss);
        REQUIRE(oss.str() == "row 1\n");
    }
    {
        std::ostringstream oss;
        caller.call("row", {"0"}, 0, PUT, oss);
        REQUIRE(oss.str() == "row 0\n");
    }
    REQUIRE_THROWS(caller.call("row", {"-5"}, -1, PUT));
    det.setRow(prev_val, {0});
}

TEST_CASE("column", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto prev_val = det.getColumn()[0];
    {
        std::ostringstream oss;
        caller.call("column", {"1"}, 0, PUT, oss);
        REQUIRE(oss.str() == "column 1\n");
    }
    {
        std::ostringstream oss;
        caller.call("column", {}, 0, GET, oss);
        REQUIRE(oss.str() == "column 1\n");
    }
    {
        std::ostringstream oss;
        caller.call("column", {"0"}, 0, PUT, oss);
        REQUIRE(oss.str() == "column 0\n");
    }
    REQUIRE_THROWS(caller.call("column", {"-5"}, -1, PUT));
    det.setColumn(prev_val, {0});
}

/* acquisition parameters */

// acquire: not testing

TEST_CASE("frames", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto prev_val =
        det.getNumberOfFrames().tsquash("#frames must be same to test");
    {
        std::ostringstream oss;
        caller.call("frames", {"1000"}, -1, PUT, oss);
        REQUIRE(oss.str() == "frames 1000\n");
    }
    {
        std::ostringstream oss;
        caller.call("frames", {}, -1, GET, oss);
        REQUIRE(oss.str() == "frames 1000\n");
    }
    {
        std::ostringstream oss;
        caller.call("frames", {"1"}, -1, PUT, oss);
        REQUIRE(oss.str() == "frames 1\n");
    }
    REQUIRE_THROWS(caller.call("frames", {"0"}, -1, PUT));
    det.setNumberOfFrames(prev_val);
}

TEST_CASE("triggers", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto prev_val =
        det.getNumberOfTriggers().tsquash("#triggers must be same to test");
    {
        std::ostringstream oss;
        caller.call("triggers", {"1000"}, -1, PUT, oss);
        REQUIRE(oss.str() == "triggers 1000\n");
    }
    {
        std::ostringstream oss;
        caller.call("triggers", {}, -1, GET, oss);
        REQUIRE(oss.str() == "triggers 1000\n");
    }
    {
        std::ostringstream oss;
        caller.call("triggers", {"1"}, -1, PUT, oss);
        REQUIRE(oss.str() == "triggers 1\n");
    }
    REQUIRE_THROWS(caller.call("triggers", {"0"}, -1, PUT));
    det.setNumberOfTriggers(prev_val);
}

TEST_CASE("exptime", "[.cmdcall][.time]") {
    Detector det;
    Caller caller(&det);
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
        caller.call("exptime", {"0.05"}, -1, PUT, oss);
        REQUIRE(oss.str() == "exptime 0.05\n");
    }
    if (det_type != defs::MYTHEN3) {
        std::ostringstream oss;
        caller.call("exptime", {}, -1, GET, oss);
        REQUIRE(oss.str() == "exptime 50ms\n");
    }
    {
        std::ostringstream oss;
        caller.call("exptime", {"1s"}, -1, PUT, oss);
        REQUIRE(oss.str() == "exptime 1s\n");
    }
    if (det_type != defs::JUNGFRAU && det_type != defs::MOENCH) {
        {
            std::ostringstream oss;
            caller.call("exptime", {"0"}, -1, PUT, oss);
            REQUIRE(oss.str() == "exptime 0\n");
        }
        {
            // Get exptime of single module
            std::ostringstream oss;
            caller.call("exptime", {}, 0, GET, oss);
            if (det_type == defs::MYTHEN3) {
                REQUIRE(oss.str() == "exptime [0ns, 0ns, 0ns]\n");
            } else {
                REQUIRE(oss.str() == "exptime 0ns\n");
            }
        }
    }
    det.setExptime(-1, prev_val);
}

TEST_CASE("period", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto prev_val = det.getPeriod();
    {
        std::ostringstream oss;
        caller.call("period", {"1.25s"}, -1, PUT, oss);
        REQUIRE(oss.str() == "period 1.25s\n");
    }
    {
        std::ostringstream oss;
        caller.call("period", {}, -1, GET, oss);
        REQUIRE(oss.str() == "period 1.25s\n");
    }
    {
        std::ostringstream oss;
        caller.call("period", {"0"}, -1, PUT, oss);
        REQUIRE(oss.str() == "period 0\n");
    }
    for (int i = 0; i != det.size(); ++i) {
        det.setPeriod(prev_val[i], {i});
    }
}

TEST_CASE("delay", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::EIGER) {
        REQUIRE_THROWS(caller.call("delay", {"1"}, -1, PUT));
        REQUIRE_THROWS(caller.call("delay", {}, -1, GET));
    } else if (det_type == defs::GOTTHARD) {
        // extra delays for master (can throw when setting)
        REQUIRE_NOTHROW(caller.call("delay", {}, -1, GET));
    } else {
        auto prev_val = det.getDelayAfterTrigger();
        {
            std::ostringstream oss;
            caller.call("delay", {"1.25s"}, -1, PUT, oss);
            REQUIRE(oss.str() == "delay 1.25s\n");
        }
        {
            std::ostringstream oss;
            caller.call("delay", {}, -1, GET, oss);
            REQUIRE(oss.str() == "delay 1.25s\n");
        }
        {
            std::ostringstream oss;
            caller.call("delay", {"0s"}, -1, PUT, oss);
            REQUIRE(oss.str() == "delay 0s\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setDelayAfterTrigger(prev_val[i], {i});
        }
    }
}

TEST_CASE("framesl", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::EIGER) {
        REQUIRE_THROWS(caller.call("framesl", {}, -1, GET));
    } else {
        REQUIRE_NOTHROW(caller.call("framesl", {}, -1, GET));
    }
}

TEST_CASE("triggersl", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::EIGER) {
        REQUIRE_THROWS(caller.call("triggersl", {}, -1, GET));
    } else {
        REQUIRE_NOTHROW(caller.call("triggersl", {}, -1, GET));
    }
}

TEST_CASE("delayl", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    switch (det_type) {
    case defs::EIGER:
        REQUIRE_THROWS(caller.call("delayl", {}, -1, GET));
        break;
    default:
        REQUIRE_NOTHROW(caller.call("delayl", {}, -1, GET));
        break;
    }
}

TEST_CASE("periodl", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    switch (det_type) {
    case defs::EIGER:
        REQUIRE_THROWS(caller.call("periodl", {}, -1, GET));
        break;
    default:
        REQUIRE_NOTHROW(caller.call("periodl", {}, -1, GET));
        break;
    }
}

TEST_CASE("dr", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::EIGER) {
        auto dr = det.getDynamicRange().squash();
        std::array<int, 4> vals{4, 8, 16, 32};
        for (const auto val : vals) {
            std::ostringstream oss1, oss2;
            caller.call("dr", {std::to_string(val)}, -1, PUT, oss1);
            REQUIRE(oss1.str() == "dr " + std::to_string(val) + '\n');
            caller.call("dr", {}, -1, GET, oss2);
            REQUIRE(oss2.str() == "dr " + std::to_string(val) + '\n');
        }
        det.setDynamicRange(dr);
    } else if (det_type == defs::MYTHEN3) {
        auto dr = det.getDynamicRange().squash();
        // not updated in firmware to support dr 1
        std::array<int, 3> vals{8, 16, 32};
        for (const auto val : vals) {
            std::ostringstream oss1, oss2;
            caller.call("dr", {std::to_string(val)}, -1, PUT, oss1);
            REQUIRE(oss1.str() == "dr " + std::to_string(val) + '\n');
            caller.call("dr", {}, -1, GET, oss2);
            REQUIRE(oss2.str() == "dr " + std::to_string(val) + '\n');
        }
        det.setDynamicRange(dr);
    } else {
        // For the other detectors we should get an error message
        // except for dr 16
        REQUIRE_THROWS(caller.call("dr", {"4"}, -1, PUT));
        REQUIRE_THROWS(caller.call("dr", {"8"}, -1, PUT));
        REQUIRE_THROWS(caller.call("dr", {"32"}, -1, PUT));

        std::ostringstream oss1, oss2;
        caller.call("dr", {"16"}, -1, PUT, oss1);
        REQUIRE(oss1.str() == "dr 16\n");
        caller.call("dr", {"16"}, -1, PUT, oss2);
        REQUIRE(oss2.str() == "dr 16\n");
    }
}

TEST_CASE("drlist", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    REQUIRE_NOTHROW(caller.call("drlist", {}, -1, GET));
    REQUIRE_THROWS(caller.call("drlist", {}, -1, PUT));
}

TEST_CASE("timing", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    auto prev_val = det.getTimingMode();
    det.setTimingMode(defs::AUTO_TIMING);
    {
        std::ostringstream oss1, oss2;
        caller.call("timing", {"auto"}, -1, PUT, oss1);
        REQUIRE(oss1.str() == "timing auto\n");
        caller.call("timing", {}, -1, GET, oss2);
        REQUIRE(oss2.str() == "timing auto\n");
    }
    {
        std::ostringstream oss1, oss2;
        caller.call("timing", {"trigger"}, -1, PUT, oss1);
        REQUIRE(oss1.str() == "timing trigger\n");
        caller.call("timing", {}, -1, GET, oss2);
        REQUIRE(oss2.str() == "timing trigger\n");
    }
    if (det_type == defs::EIGER) {
        {
            std::ostringstream oss1, oss2;
            caller.call("timing", {"gating"}, -1, PUT, oss1);
            REQUIRE(oss1.str() == "timing gating\n");
            caller.call("timing", {}, -1, GET, oss2);
            REQUIRE(oss2.str() == "timing gating\n");
        }
        {
            std::ostringstream oss1, oss2;
            caller.call("timing", {"burst_trigger"}, -1, PUT, oss1);
            REQUIRE(oss1.str() == "timing burst_trigger\n");
            caller.call("timing", {}, -1, GET, oss2);
            REQUIRE(oss2.str() == "timing burst_trigger\n");
        }
        REQUIRE_THROWS(caller.call("timing", {"trigger_gating"}, -1, PUT));
    } else if (det_type == defs::MYTHEN3) {
        {
            std::ostringstream oss1, oss2;
            caller.call("timing", {"gating"}, -1, PUT, oss1);
            REQUIRE(oss1.str() == "timing gating\n");
            caller.call("timing", {}, -1, GET, oss2);
            REQUIRE(oss2.str() == "timing gating\n");
        }
        {
            std::ostringstream oss1, oss2;
            caller.call("timing", {"trigger_gating"}, -1, PUT, oss1);
            REQUIRE(oss1.str() == "timing trigger_gating\n");
            caller.call("timing", {}, -1, GET, oss2);
            REQUIRE(oss2.str() == "timing trigger_gating\n");
        }
        REQUIRE_THROWS(caller.call("timing", {"burst_trigger"}, -1, PUT));
    } else {
        REQUIRE_THROWS(caller.call("timing", {"gating"}, -1, PUT));
        REQUIRE_THROWS(caller.call("timing", {"burst_trigger"}, -1, PUT));
        REQUIRE_THROWS(caller.call("timing", {"trigger_gating"}, -1, PUT));
    }
    for (int i = 0; i != det.size(); ++i) {
        det.setTimingMode(prev_val[i], {i});
    }
}

TEST_CASE("timinglist", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    REQUIRE_NOTHROW(caller.call("timinglist", {}, -1, GET));
    REQUIRE_THROWS(caller.call("timinglist", {}, -1, PUT));
}

TEST_CASE("readoutspeed", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::EIGER || det_type == defs::JUNGFRAU ||
        det_type == defs::MOENCH || det_type == defs::GOTTHARD2 ||
        det_type == defs::MYTHEN3) {
        auto prev_val = det.getReadoutSpeed();

        // full speed for jungfrau/moench only works for new boards (chipv1.1 is
        // with new board [hw1.0 and chipv1.0 not tested here])
        if (((det_type == defs::JUNGFRAU) &&
             det.getChipVersion().squash() * 10 == 11) ||
            det_type == defs::EIGER || det_type == defs::MOENCH ||
            det_type == defs::MYTHEN3) {
            std::ostringstream oss1, oss2, oss3, oss4;
            caller.call("readoutspeed", {"0"}, -1, PUT, oss1);
            REQUIRE(oss1.str() == "readoutspeed full_speed\n");
            caller.call("readoutspeed", {}, -1, GET, oss2);
            REQUIRE(oss2.str() == "readoutspeed full_speed\n");
            caller.call("readoutspeed", {"full_speed"}, -1, PUT, oss3);
            REQUIRE(oss3.str() == "readoutspeed full_speed\n");
            caller.call("readoutspeed", {}, -1, GET, oss4);
            REQUIRE(oss4.str() == "readoutspeed full_speed\n");
        }

        if (det_type == defs::EIGER || det_type == defs::JUNGFRAU ||
            det_type == defs::MOENCH || det_type == defs::MYTHEN3) {
            {
                std::ostringstream oss1, oss2, oss3, oss4;
                caller.call("readoutspeed", {"1"}, -1, PUT, oss1);
                REQUIRE(oss1.str() == "readoutspeed half_speed\n");
                caller.call("readoutspeed", {}, -1, GET, oss2);
                REQUIRE(oss2.str() == "readoutspeed half_speed\n");
                caller.call("readoutspeed", {"half_speed"}, -1, PUT, oss3);
                REQUIRE(oss3.str() == "readoutspeed half_speed\n");
                caller.call("readoutspeed", {}, -1, GET, oss4);
                REQUIRE(oss4.str() == "readoutspeed half_speed\n");
            }
            {
                std::ostringstream oss1, oss2, oss3, oss4;
                caller.call("readoutspeed", {"2"}, -1, PUT, oss1);
                REQUIRE(oss1.str() == "readoutspeed quarter_speed\n");
                caller.call("readoutspeed", {}, -1, GET, oss2);
                REQUIRE(oss2.str() == "readoutspeed quarter_speed\n");
                caller.call("readoutspeed", {"quarter_speed"}, -1, PUT, oss3);
                REQUIRE(oss3.str() == "readoutspeed quarter_speed\n");
                caller.call("readoutspeed", {}, -1, GET, oss4);
                REQUIRE(oss4.str() == "readoutspeed quarter_speed\n");
            }
            REQUIRE_THROWS(caller.call("readoutspeed", {"108"}, -1, PUT));
            REQUIRE_THROWS(caller.call("readoutspeed", {"144"}, -1, PUT));
        }

        if (det_type == defs::GOTTHARD2) {
            {
                std::ostringstream oss1, oss2, oss3, oss4;
                caller.call("readoutspeed", {"108"}, -1, PUT, oss1);
                REQUIRE(oss1.str() == "readoutspeed 108\n");
                caller.call("readoutspeed", {}, -1, GET, oss2);
                REQUIRE(oss2.str() == "readoutspeed 108\n");
            }

            {
                std::ostringstream oss1, oss2, oss3, oss4;
                caller.call("readoutspeed", {"144"}, -1, PUT, oss1);
                REQUIRE(oss1.str() == "readoutspeed 144\n");
                caller.call("readoutspeed", {}, -1, GET, oss2);
                REQUIRE(oss2.str() == "readoutspeed 144\n");
            }
            REQUIRE_THROWS(
                caller.call("readoutspeed", {"full_speed"}, -1, PUT));
            REQUIRE_THROWS(
                caller.call("readoutspeed", {"half_speed"}, -1, PUT));
            REQUIRE_THROWS(
                caller.call("readoutspeed", {"quarter_speed"}, -1, PUT));
            REQUIRE_THROWS(caller.call("readoutspeed", {"0"}, -1, PUT));
            REQUIRE_THROWS(caller.call("readoutspeed", {"1"}, -1, PUT));
            REQUIRE_THROWS(caller.call("readoutspeed", {"2"}, -1, PUT));
        }

        for (int i = 0; i != det.size(); ++i) {
            det.setReadoutSpeed(prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(caller.call("readoutspeed", {"0"}, -1, PUT));
        REQUIRE_THROWS(caller.call("readoutspeed", {}, -1, GET));
    }
}

TEST_CASE("readoutspeedlist", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::GOTTHARD2 || det_type == defs::JUNGFRAU ||
        det_type == defs::MOENCH || det_type == defs::EIGER ||
        det_type == defs::MYTHEN3) {
        REQUIRE_NOTHROW(caller.call("readoutspeedlist", {}, -1, GET));
        REQUIRE_THROWS(caller.call("readoutspeedlist", {}, -1, PUT));
    } else {
        REQUIRE_THROWS(caller.call("readoutspeedlist", {}, -1, GET));
    }
}

TEST_CASE("adcphase", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::GOTTHARD || det_type == defs::JUNGFRAU ||
        det_type == defs::MOENCH || det_type == defs::CHIPTESTBOARD) {
        if (det_type == defs::GOTTHARD) {
            std::ostringstream oss1;
            caller.call("adcphase", {"20"}, -1, PUT, oss1);
            REQUIRE(oss1.str() == "adcphase 20\n");
            // cant get, cant use deg
            REQUIRE_THROWS(caller.call("adcphase", {}, -1, GET));
            REQUIRE_THROWS(caller.call("adcphase", {"20", "deg"}, -1, PUT));
        } else {
            auto prev_val = det.getADCPhase();
            {
                std::ostringstream oss1, oss2;
                caller.call("adcphase", {"20"}, -1, PUT, oss1);
                REQUIRE(oss1.str() == "adcphase 20\n");
                caller.call("adcphase", {}, -1, GET, oss2);
                REQUIRE(oss2.str() == "adcphase 20\n");
            }
            {
                std::ostringstream oss1, oss2;
                caller.call("adcphase", {"20", "deg"}, -1, PUT, oss1);
                REQUIRE(oss1.str() == "adcphase 20 deg\n");
                caller.call("adcphase", {"deg"}, -1, GET, oss2);
                REQUIRE(oss2.str() == "adcphase 20 deg\n");
            }
            for (int i = 0; i != det.size(); ++i) {
                det.setADCPhase(prev_val[i], {i});
            }
        }
    } else {
        REQUIRE_THROWS(caller.call("adcphase", {"0"}, -1, PUT));
        REQUIRE_THROWS(caller.call("adcphase", {}, -1, GET));
    }
}

TEST_CASE("maxadcphaseshift", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU || det_type == defs::MOENCH ||
        det_type == defs::CHIPTESTBOARD ||
        det_type == defs::MYTHEN3 ||   // only because clk index of 0 exists
        det_type == defs::GOTTHARD2) { // only because clk index of 0 exists
        REQUIRE_NOTHROW(caller.call("maxadcphaseshift", {}, -1, GET));
    } else {
        REQUIRE_THROWS(caller.call("maxadcphaseshift", {}, -1, GET));
    }
}

TEST_CASE("dbitphase", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU || det_type == defs::CHIPTESTBOARD) {
        auto prev_val = det.getDBITPhase();
        {
            std::ostringstream oss1, oss2;
            caller.call("dbitphase", {"20"}, -1, PUT, oss1);
            REQUIRE(oss1.str() == "dbitphase 20\n");
            caller.call("dbitphase", {}, -1, GET, oss2);
            REQUIRE(oss2.str() == "dbitphase 20\n");
        }
        {
            std::ostringstream oss1, oss2;
            caller.call("dbitphase", {"23", "deg"}, -1, PUT, oss1);
            REQUIRE(oss1.str() == "dbitphase 23 deg\n");
            caller.call("dbitphase", {"deg"}, -1, GET, oss2);
            REQUIRE(oss2.str() == "dbitphase 23 deg\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setDBITPhase(prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(caller.call("dbitphase", {"0"}, -1, PUT));
        REQUIRE_THROWS(caller.call("dbitphase", {}, -1, GET));
    }
}

TEST_CASE("maxdbitphaseshift", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU || det_type == defs::CHIPTESTBOARD ||
        det_type == defs::MYTHEN3 ||   // only because clk index of 0 exists
        det_type == defs::GOTTHARD2) { // only because clk index of 0 exists
        REQUIRE_NOTHROW(caller.call("maxdbitphaseshift", {}, -1, GET));
    } else {
        REQUIRE_THROWS(caller.call("maxdbitphaseshift", {}, -1, GET));
    }
}

TEST_CASE("clkfreq", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::MYTHEN3 || det_type == defs::GOTTHARD2) {
        REQUIRE_THROWS(caller.call("clkfreq", {"0", "2"}, -1, PUT));
        REQUIRE_THROWS(caller.call("clkfreq", {}, -1, GET));
        REQUIRE_THROWS(caller.call("clkfreq", {"7"}, -1, GET));
        REQUIRE_NOTHROW(caller.call("clkfreq", {"0"}, -1, GET));
        // other clocks removed for m3 (setting not supported)
        if (det_type == defs::MYTHEN3) {
            REQUIRE_NOTHROW(caller.call("clkfreq", {"1"}, -1, GET));
            REQUIRE_NOTHROW(caller.call("clkfreq", {"2"}, -1, GET));
            REQUIRE_THROWS(caller.call("clkfreq", {"3"}, -1, GET));
        }
    } else {
        REQUIRE_THROWS(caller.call("clkfreq", {"0"}, -1, GET));
    }
}

TEST_CASE("clkphase", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::MYTHEN3 || det_type == defs::GOTTHARD2) {
        REQUIRE_THROWS(caller.call("clkphase", {}, -1, GET));
        REQUIRE_THROWS(caller.call("clkphase", {"7"}, -1, GET));
        REQUIRE_THROWS(caller.call("clkphase", {"4"}, -1, PUT));
        REQUIRE_THROWS(caller.call("clkphase", {"7", "4"}, -1, PUT));
        auto prev_val = det.getClockPhase(0);
        {
            std::ostringstream oss1, oss2;
            caller.call("clkphase", {"0", "20"}, -1, PUT, oss1);
            REQUIRE(oss1.str() == "clkphase 20\n");
            caller.call("clkphase", {"0"}, -1, GET, oss2);
            REQUIRE(oss2.str() == "clkphase 20\n");
        }
        std::string s_deg_val = "15";
        if (det_type == defs::MYTHEN3) {
            s_deg_val = "14";
        } else if (det_type == defs::GOTTHARD2) {
            s_deg_val = "23";
        }
        {
            std::ostringstream oss1, oss2;
            caller.call("clkphase", {"0", s_deg_val, "deg"}, -1, PUT, oss1);
            REQUIRE(oss1.str() == "clkphase " + s_deg_val + " deg\n");
            caller.call("clkphase", {"0", "deg"}, -1, GET, oss2);
            REQUIRE(oss2.str() == "clkphase " + s_deg_val + " deg\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setClockPhase(0, prev_val[i], {i});
        }
        // other clocks removed for m3 (setting not supported)
        if (det_type == defs::MYTHEN3) {
            REQUIRE_THROWS(
                caller.call("clkphase", {"1", s_deg_val, "deg"}, -1, PUT));
            REQUIRE_NOTHROW(caller.call("clkphase", {"1"}, -1, GET));
            REQUIRE_THROWS(
                caller.call("clkphase", {"2", s_deg_val, "deg"}, -1, PUT));
            REQUIRE_NOTHROW(caller.call("clkphase", {"2"}, -1, GET));
            REQUIRE_THROWS(
                caller.call("clkphase", {"3", s_deg_val, "deg"}, -1, PUT));
            REQUIRE_THROWS(caller.call("clkphase", {"3"}, -1, GET));
        }
    } else {
        REQUIRE_THROWS(caller.call("clkphase", {"0"}, -1, GET));
    }
}

TEST_CASE("clkdiv", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::MYTHEN3 || det_type == defs::GOTTHARD2) {
        REQUIRE_THROWS(caller.call("clkdiv", {}, -1, GET));
        REQUIRE_THROWS(caller.call("clkdiv", {"7"}, -1, GET));
        REQUIRE_THROWS(caller.call("clkdiv", {"7", "4"}, -1, PUT));
        REQUIRE_THROWS(caller.call("clkdiv", {"7", "4"}, -1, PUT));
        REQUIRE_THROWS(caller.call("clkdiv", {"0", "1"}, -1, PUT));
        auto prev_val = det.getClockDivider(0);
        {
            std::ostringstream oss1, oss2;
            caller.call("clkdiv", {"0", "3"}, -1, PUT, oss1);
            REQUIRE(oss1.str() == "clkdiv 0 3\n");
            caller.call("clkdiv", {"0"}, -1, GET, oss2);
            REQUIRE(oss2.str() == "clkdiv 0 3\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setClockDivider(0, prev_val[i], {i});
        }
        // other clocks removed for m3 (setting not supported)
        if (det_type == defs::MYTHEN3) {
            REQUIRE_THROWS(caller.call("clkdiv", {"1", "2"}, -1, PUT));
            REQUIRE_NOTHROW(caller.call("clkdiv", {"1"}, -1, GET));
            REQUIRE_THROWS(caller.call("clkdiv", {"2", "2"}, -1, PUT));
            REQUIRE_NOTHROW(caller.call("clkdiv", {"2"}, -1, GET));
            REQUIRE_THROWS(caller.call("clkdiv", {"3", "2"}, -1, PUT));
            REQUIRE_THROWS(caller.call("clkdiv", {"3"}, -1, GET));
        }
    } else {
        REQUIRE_THROWS(caller.call("clkdiv", {"0"}, -1, GET));
    }
}

TEST_CASE("maxclkphaseshift", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::MYTHEN3 || det_type == defs::GOTTHARD2) {
        REQUIRE_THROWS(caller.call("maxclkphaseshift", {"0", "2"}, -1, PUT));
        REQUIRE_THROWS(caller.call("maxclkphaseshift", {}, -1, GET));
        REQUIRE_THROWS(caller.call("maxclkphaseshift", {"7"}, -1, GET));
        REQUIRE_NOTHROW(caller.call("maxclkphaseshift", {"0"}, -1, GET));
        // other clocks removed for m3 (setting not supported)
        if (det_type == defs::MYTHEN3) {
            REQUIRE_NOTHROW(caller.call("maxclkphaseshift", {"1"}, -1, GET));
            REQUIRE_NOTHROW(caller.call("maxclkphaseshift", {"2"}, -1, GET));
            REQUIRE_THROWS(caller.call("maxclkphaseshift", {"3"}, -1, GET));
        }
    } else {
        REQUIRE_THROWS(caller.call("maxclkphaseshift", {"0"}, -1, GET));
    }
}

TEST_CASE("highvoltage", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type != defs::XILINX_CHIPTESTBOARD) {
        auto prev_val = det.getHighVoltage();
        // selected values
        if (det_type == defs::GOTTHARD) {
            REQUIRE_THROWS(caller.call("highvoltage", {"50"}, -1, PUT));
            {
                std::ostringstream oss1, oss2;
                caller.call("highvoltage", {"90"}, -1, PUT, oss1);
                REQUIRE(oss1.str() == "highvoltage 90\n");
                caller.call("highvoltage", {}, -1, GET, oss2);
                REQUIRE(oss2.str() == "highvoltage 90\n");
            }
            {
                std::ostringstream oss1, oss2;
                caller.call("highvoltage", {"0"}, -1, PUT, oss1);
                REQUIRE(oss1.str() == "highvoltage 0\n");
                caller.call("highvoltage", {}, -1, GET, oss2);
                REQUIRE(oss2.str() == "highvoltage 0\n");
            }
        }
        // range 0, 60 - 200
        else if (det_type == defs::JUNGFRAU || det_type == defs::MOENCH ||
                 det_type == defs::CHIPTESTBOARD) {
            REQUIRE_THROWS(caller.call("highvoltage", {"50"}, -1, PUT));
            {
                std::ostringstream oss1, oss2;
                caller.call("highvoltage", {"90"}, -1, PUT, oss1);
                REQUIRE(oss1.str() == "highvoltage 90\n");
                caller.call("highvoltage", {}, -1, GET, oss2);
                REQUIRE(oss2.str() == "highvoltage 90\n");
            }
            {
                std::ostringstream oss1, oss2;
                caller.call("highvoltage", {"0"}, -1, PUT, oss1);
                REQUIRE(oss1.str() == "highvoltage 0\n");
                caller.call("highvoltage", {}, -1, GET, oss2);
                REQUIRE(oss2.str() == "highvoltage 0\n");
            }
        }
        // full range 0 - 200 (get needs to wait)
        else if (det_type == defs::EIGER) {
            {
                std::ostringstream oss1, oss2;
                caller.call("highvoltage", {"50"}, 0, PUT, oss1);
                REQUIRE(oss1.str() == "highvoltage 50\n");
                std::this_thread::sleep_for(std::chrono::seconds(2));
                caller.call("highvoltage", {}, 0, GET, oss2);
                REQUIRE(oss2.str() == "highvoltage 50\n");
            }
            {
                std::ostringstream oss1, oss2;
                caller.call("highvoltage", {"120"}, 0, PUT, oss1);
                REQUIRE(oss1.str() == "highvoltage 120\n");
                std::this_thread::sleep_for(std::chrono::seconds(2));
                caller.call("highvoltage", {}, 0, GET, oss2);
                REQUIRE(oss2.str() == "highvoltage 120\n");
            }
            {
                std::ostringstream oss1, oss2;
                caller.call("highvoltage", {"0"}, 0, PUT, oss1);
                REQUIRE(oss1.str() == "highvoltage 0\n");
                std::this_thread::sleep_for(std::chrono::seconds(2));
                caller.call("highvoltage", {}, 0, GET, oss2);
                REQUIRE(oss2.str() == "highvoltage 0\n");
            }
        }
        // full range 0 - 200
        else {
            {
                std::ostringstream oss1, oss2;
                caller.call("highvoltage", {"50"}, -1, PUT, oss1);
                REQUIRE(oss1.str() == "highvoltage 50\n");
                caller.call("highvoltage", {}, -1, GET, oss2);
                REQUIRE(oss2.str() == "highvoltage 50\n");
            }
            {
                std::ostringstream oss1, oss2;
                caller.call("highvoltage", {"120"}, -1, PUT, oss1);
                REQUIRE(oss1.str() == "highvoltage 120\n");
                caller.call("highvoltage", {}, -1, GET, oss2);
                REQUIRE(oss2.str() == "highvoltage 120\n");
            }
            {
                std::ostringstream oss1, oss2;
                caller.call("highvoltage", {"0"}, -1, PUT, oss1);
                REQUIRE(oss1.str() == "highvoltage 0\n");
                caller.call("highvoltage", {}, -1, GET, oss2);
                REQUIRE(oss2.str() == "highvoltage 0\n");
            }
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setHighVoltage(prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(caller.call("highvoltage", {"0"}, -1, PUT));
        REQUIRE_THROWS(caller.call("highvoltage", {}, -1, GET));
    }
}

TEST_CASE("powerchip", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::JUNGFRAU || det_type == defs::MOENCH ||
        det_type == defs::MYTHEN3 || det_type == defs::GOTTHARD2 ||
        det_type == defs::XILINX_CHIPTESTBOARD) {
        auto prev_val = det.getPowerChip();
        {
            std::ostringstream oss;
            caller.call("powerchip", {"1"}, -1, PUT, oss);
            REQUIRE(oss.str() == "powerchip 1\n");
        }
        {
            std::ostringstream oss;
            caller.call("powerchip", {"0"}, -1, PUT, oss);
            REQUIRE(oss.str() == "powerchip 0\n");
        }
        // powering off chip throws if hv on (only test virtualserver - safety
        if (det_type == defs::GOTTHARD2 &&
            det.isVirtualDetectorServer().tsquash(
                "Inconsistent virtual detector "
                "server to test powerchip command")) {
            det.setPowerChip(1);
            int hv = det.getHighVoltage().tsquash(
                "Inconsistent high voltage to test "
                "powerchip command");

            det.setHighVoltage(100);
            REQUIRE_THROWS(caller.call("powerchip", {"0"}, -1, PUT));

            // previous settings
            det.setHighVoltage(hv);
            det.setPowerChip(0);
        }
        {
            std::ostringstream oss;
            caller.call("powerchip", {}, -1, GET, oss);
            REQUIRE(oss.str() == "powerchip 0\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setPowerChip(prev_val[i], {i});
            if (det_type == defs::XILINX_CHIPTESTBOARD) {
                det.configureTransceiver();
            }
        }
    } else {
        REQUIRE_THROWS(caller.call("powerchip", {}, -1, GET));
    }
}

TEST_CASE("imagetest", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    // cannot test only for virtual eiger/jungfrau
    if (det_type == defs::GOTTHARD) {
        auto prev_val = det.getImageTestMode();
        {
            std::ostringstream oss1, oss2;
            caller.call("imagetest", {"1"}, -1, PUT, oss1);
            REQUIRE(oss1.str() == "imagetest 1\n");
            caller.call("imagetest", {}, -1, GET, oss2);
            REQUIRE(oss2.str() == "imagetest 1\n");
        }
        {
            std::ostringstream oss1, oss2;
            caller.call("imagetest", {"0"}, -1, PUT, oss1);
            REQUIRE(oss1.str() == "imagetest 0\n");
            caller.call("imagetest", {}, -1, GET, oss2);
            REQUIRE(oss2.str() == "imagetest 0\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setImageTestMode(prev_val[i], {i});
        }
    } else if (det_type != defs::JUNGFRAU && det_type != defs::MOENCH &&
               det_type != defs::EIGER) {
        // wont fail for eiger and jungfrau/moench virtual servers
        REQUIRE_THROWS(caller.call("imagetest", {}, -1, GET));
    }
}

TEST_CASE("extsig", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::GOTTHARD) {
        auto prev_val = det.getExternalSignalFlags(0);
        REQUIRE_THROWS(caller.call("extsig", {}, -1, GET));
        REQUIRE_THROWS(caller.call("extsig", {"1"}, -1, GET));
        REQUIRE_THROWS(caller.call("extsig", {"0", "inversion_on"}, -1, PUT));
        REQUIRE_THROWS(caller.call("extsig", {"0", "inversion_off"}, -1, PUT));
        {
            std::ostringstream oss1, oss2;
            caller.call("extsig", {"0", "trigger_in_rising_edge"}, -1, PUT,
                        oss1);
            REQUIRE(oss1.str() == "extsig 0 trigger_in_rising_edge\n");
            caller.call("extsig", {"0"}, -1, GET, oss2);
            REQUIRE(oss2.str() == "extsig 0 trigger_in_rising_edge\n");
        }
        {
            std::ostringstream oss1, oss2;
            caller.call("extsig", {"0", "trigger_in_falling_edge"}, -1, PUT,
                        oss1);
            REQUIRE(oss1.str() == "extsig 0 trigger_in_falling_edge\n");
            caller.call("extsig", {"0"}, -1, GET, oss2);
            REQUIRE(oss2.str() == "extsig 0 trigger_in_falling_edge\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setExternalSignalFlags(0, prev_val[i], {i});
        }
    } else if (det_type == defs::MYTHEN3) {
        auto prev_val_0 = det.getExternalSignalFlags(0);
        auto prev_val_1 = det.getExternalSignalFlags(1);
        REQUIRE_THROWS(caller.call("extsig", {}, -1, GET));
        REQUIRE_THROWS(caller.call("extsig", {"8"}, -1, GET));
        REQUIRE_THROWS(caller.call("extsig", {"0", "inversion_on"}, -1, PUT));
        REQUIRE_THROWS(caller.call("extsig", {"0", "inversion_off"}, -1, PUT));
        REQUIRE_THROWS(
            caller.call("extsig", {"1", "trigger_in_rising_edge"}, -1, PUT));
        REQUIRE_THROWS(
            caller.call("extsig", {"1", "trigger_in_falling_edge"}, -1, PUT));
        {
            std::ostringstream oss1, oss2;
            caller.call("extsig", {"0", "trigger_in_rising_edge"}, -1, PUT,
                        oss1);
            REQUIRE(oss1.str() == "extsig 0 trigger_in_rising_edge\n");
            caller.call("extsig", {"0"}, -1, GET, oss2);
            REQUIRE(oss2.str() == "extsig 0 trigger_in_rising_edge\n");
        }
        {
            std::ostringstream oss1, oss2;
            caller.call("extsig", {"0", "trigger_in_falling_edge"}, -1, PUT,
                        oss1);
            REQUIRE(oss1.str() == "extsig 0 trigger_in_falling_edge\n");
            caller.call("extsig", {"0"}, -1, GET, oss2);
            REQUIRE(oss2.str() == "extsig 0 trigger_in_falling_edge\n");
        }
        {
            std::ostringstream oss1, oss2;
            caller.call("extsig", {"1", "inversion_off"}, -1, PUT, oss1);
            REQUIRE(oss1.str() == "extsig 1 inversion_off\n");
            caller.call("extsig", {"1"}, -1, GET, oss2);
            REQUIRE(oss2.str() == "extsig 1 inversion_off\n");
        }
        {
            std::ostringstream oss1, oss2;
            caller.call("extsig", {"1", "inversion_on"}, -1, PUT, oss1);
            REQUIRE(oss1.str() == "extsig 1 inversion_on\n");
            caller.call("extsig", {"1"}, -1, GET, oss2);
            REQUIRE(oss2.str() == "extsig 1 inversion_on\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setExternalSignalFlags(0, prev_val_0[i], {i});
            det.setExternalSignalFlags(1, prev_val_1[i], {i});
        }
    } else {
        REQUIRE_THROWS(caller.call("extsig", {}, -1, GET));
    }
}

TEST_CASE("parallel", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::EIGER || det_type == defs::MYTHEN3 ||
        det_type == defs::GOTTHARD2 || det_type == defs::MOENCH) {
        auto prev_val = det.getParallelMode();
        {
            std::ostringstream oss;
            caller.call("parallel", {"1"}, -1, PUT, oss);
            REQUIRE(oss.str() == "parallel 1\n");
        }
        {
            std::ostringstream oss;
            caller.call("parallel", {"0"}, -1, PUT, oss);
            REQUIRE(oss.str() == "parallel 0\n");
        }
        {
            std::ostringstream oss;
            caller.call("parallel", {}, -1, GET, oss);
            REQUIRE(oss.str() == "parallel 0\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setParallelMode(prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(caller.call("parallel", {}, -1, GET));
    }
}

TEST_CASE("filterresistor", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
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
            caller.call("filterresistor", {"1"}, -1, PUT, oss);
            REQUIRE(oss.str() == "filterresistor 1\n");
        }
        {
            std::ostringstream oss;
            caller.call("filterresistor", {"0"}, -1, PUT, oss);
            REQUIRE(oss.str() == "filterresistor 0\n");
        }
        {
            std::ostringstream oss;
            caller.call("filterresistor", {}, -1, GET, oss);
            REQUIRE(oss.str() == "filterresistor 0\n");
        }
        if (det_type == defs::GOTTHARD2) {
            REQUIRE_NOTHROW(caller.call("filterresistor", {"2"}, -1, PUT));
            REQUIRE_NOTHROW(caller.call("filterresistor", {"3"}, -1, PUT));
        } else {
            REQUIRE_THROWS(caller.call("filterresistor", {"2"}, -1, PUT));
            REQUIRE_THROWS(caller.call("filterresistor", {"3"}, -1, PUT));
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setFilterResistor(prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(caller.call("filterresistor", {}, -1, GET));
    }
}

TEST_CASE("dbitpipeline", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::CHIPTESTBOARD || det_type == defs::GOTTHARD2) {
        auto prev_val = det.getDBITPipeline();
        {
            std::ostringstream oss;
            caller.call("dbitpipeline", {"1"}, -1, PUT, oss);
            REQUIRE(oss.str() == "dbitpipeline 1\n");
        }
        {
            std::ostringstream oss;
            caller.call("dbitpipeline", {"0"}, -1, PUT, oss);
            REQUIRE(oss.str() == "dbitpipeline 0\n");
        }
        if (det_type == defs::CHIPTESTBOARD) {
            {
                std::ostringstream oss;
                caller.call("dbitpipeline", {"15"}, -1, PUT, oss);
                REQUIRE(oss.str() == "dbitpipeline 15\n");
            }
            {
                std::ostringstream oss;
                caller.call("dbitpipeline", {}, -1, GET, oss);
                REQUIRE(oss.str() == "dbitpipeline 15\n");
            }
            REQUIRE_THROWS(caller.call("dbitpipeline", {"256"}, -1, PUT));
        } else {
            {
                std::ostringstream oss;
                caller.call("dbitpipeline", {"7"}, -1, PUT, oss);
                REQUIRE(oss.str() == "dbitpipeline 7\n");
            }
            {
                std::ostringstream oss;
                caller.call("dbitpipeline", {}, -1, GET, oss);
                REQUIRE(oss.str() == "dbitpipeline 7\n");
            }
            REQUIRE_THROWS(caller.call("dbitpipeline", {"8"}, -1, PUT));
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setDBITPipeline(prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(caller.call("dbitpipeline", {}, -1, GET));
    }
}

TEST_CASE("readnrows", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::EIGER || det_type == defs::JUNGFRAU ||
        det_type == defs::MOENCH) {
        bool hw2 = false;
        if ((det_type == defs::JUNGFRAU || det_type == defs::MOENCH) &&
            ((det.getHardwareVersion().tsquash(
                  "inconsistent hardware version number to test") == "2.0"))) {
            hw2 = true;
        }
        if ((det_type == defs::JUNGFRAU || det_type == defs::MOENCH) && !hw2) {
            {
                std::ostringstream oss;
                caller.call("readnrows", {}, -1, GET, oss);
                REQUIRE(oss.str() == "readnrows 512\n");
            }
        } else {
            auto prev_val = det.getReadNRows();
            {
                std::ostringstream oss;
                caller.call("readnrows", {"256"}, -1, PUT, oss);
                REQUIRE(oss.str() == "readnrows 256\n");
            }
            {
                std::ostringstream oss;
                caller.call("readnrows", {}, -1, GET, oss);
                REQUIRE(oss.str() == "readnrows 256\n");
            }
            {
                std::ostringstream oss;
                caller.call("readnrows", {"16"}, -1, PUT, oss);
                REQUIRE(oss.str() == "readnrows 16\n");
            }
            if (det_type == defs::JUNGFRAU || det_type == defs::MOENCH) {
                REQUIRE_THROWS(caller.call("readnrows", {"7"}, -1, PUT));
                REQUIRE_THROWS(caller.call("readnrows", {"20"}, -1, PUT));
                REQUIRE_THROWS(caller.call("readnrows", {"44"}, -1, PUT));
                REQUIRE_THROWS(caller.call("readnrows", {"513"}, -1, PUT));
                REQUIRE_THROWS(caller.call("readnrows", {"1"}, -1, PUT));
            }
            REQUIRE_THROWS(caller.call("readnrows", {"0"}, -1, PUT));
            for (int i = 0; i != det.size(); ++i) {
                det.setReadNRows(prev_val[i], {i});
            }
        }
    } else {
        REQUIRE_THROWS(caller.call("readnrows", {}, -1, GET));
    }
}

TEST_CASE("currentsource", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::GOTTHARD2 || det_type == defs::JUNGFRAU) {
        auto prev_val = det.getCurrentSource();

        if (det_type == defs::GOTTHARD2) {
            {
                std::ostringstream oss;
                caller.call("currentsource", {"1"}, -1, PUT, oss);
                REQUIRE(oss.str() == "currentsource [1]\n");
            }
            {
                std::ostringstream oss;
                caller.call("currentsource", {"0"}, -1, PUT, oss);
                REQUIRE(oss.str() == "currentsource [0]\n");
            }
            {
                std::ostringstream oss;
                caller.call("currentsource", {}, -1, GET, oss);
                REQUIRE(oss.str() == "currentsource [disabled]\n");
            }
            REQUIRE_THROWS(
                caller.call("currentsource", {"1", "fix", "42"}, -1, PUT));
            REQUIRE_THROWS(caller.call("currentsource",
                                       {"1", "fix", "42", "normal"}, -1, PUT));
        }
        // jungfrau
        else {
            int chipVersion = 10;
            if (det_type == defs::JUNGFRAU) {
                chipVersion = det.getChipVersion().tsquash(
                                  "inconsistent chip versions to test") *
                              10;
            }
            if (chipVersion == 10) {
                REQUIRE_THROWS(caller.call("currentsource", {"1"}, -1, PUT));
                REQUIRE_THROWS(
                    caller.call("currentsource", {"1", "fix"}, -1, PUT));
                REQUIRE_THROWS(
                    caller.call("currentsource", {"1", "fix", "64"}, -1, PUT));
                REQUIRE_THROWS(
                    caller.call("currentsource", {"1", "dfg", "64"}, -1, PUT));
                REQUIRE_THROWS(caller.call(
                    "currentsource", {"1", "fix", "63", "normal"}, -1, PUT));
                {
                    std::ostringstream oss;
                    caller.call("currentsource", {"1", "fix", "63"}, -1, PUT,
                                oss);
                    REQUIRE(oss.str() == "currentsource [1, fix, 63]\n");
                }
                {
                    std::ostringstream oss;
                    caller.call("currentsource", {"0"}, -1, PUT, oss);
                    REQUIRE(oss.str() == "currentsource [0]\n");
                }
                {
                    std::ostringstream oss;
                    caller.call("currentsource", {}, -1, GET, oss);
                    REQUIRE(oss.str() == "currentsource [disabled]\n");
                }
                {
                    std::ostringstream oss;
                    caller.call("currentsource", {"1", "nofix", "63"}, -1, PUT,
                                oss);
                    REQUIRE(oss.str() == "currentsource [1, nofix, 63]\n");
                }
                {
                    std::ostringstream oss;
                    caller.call("currentsource", {}, -1, GET, oss);
                    REQUIRE(oss.str() ==
                            "currentsource [enabled, nofix, 63]\n");
                }
            }
            // chipv1.1
            else {
                REQUIRE_THROWS(caller.call("currentsource", {"1"}, -1, PUT));
                REQUIRE_THROWS(
                    caller.call("currentsource", {"1", "fix"}, -1, PUT));
                REQUIRE_THROWS(caller.call(
                    "currentsource", {"1", "ffgdfgix", "0x0000000000000041"},
                    -1, PUT));
                REQUIRE_THROWS(caller.call(
                    "currentsource",
                    {"1", "fix", "0x0000000000000041", "normaldgf"}, -1, PUT));

                {
                    std::ostringstream oss;
                    caller.call("currentsource",
                                {"1", "fix", "0x0000000000000041", "normal"},
                                -1, PUT, oss);
                    REQUIRE(
                        oss.str() ==
                        "currentsource [1, fix, 0x0000000000000041, normal]\n");
                }
                {
                    std::ostringstream oss;
                    caller.call("currentsource", {"0"}, -1, PUT, oss);
                    REQUIRE(oss.str() == "currentsource [0]\n");
                }
                {
                    std::ostringstream oss;
                    caller.call("currentsource", {}, -1, GET, oss);
                    REQUIRE(oss.str() == "currentsource [disabled]\n");
                }
                {
                    std::ostringstream oss;
                    caller.call("currentsource",
                                {"1", "nofix", "0x0000000000000041", "normal"},
                                -1, PUT, oss);
                    REQUIRE(oss.str() == "currentsource [1, nofix, "
                                         "0x0000000000000041, normal]\n");
                }
                {
                    std::ostringstream oss;
                    caller.call("currentsource", {}, -1, GET, oss);
                    REQUIRE(oss.str() == "currentsource [enabled, nofix, "
                                         "0x0000000000000041, normal]\n");
                }
                {
                    std::ostringstream oss;
                    caller.call("currentsource",
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
        REQUIRE_THROWS(caller.call("currentsource", {}, -1, GET));
    }
}

/** temperature */

TEST_CASE("templist", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    REQUIRE_NOTHROW(caller.call("templist", {}, -1, GET));
    REQUIRE_THROWS(caller.call("templist", {}, -1, PUT));
}

TEST_CASE("tempvalues", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    REQUIRE_NOTHROW(caller.call("tempvalues", {}, -1, GET));
    REQUIRE_THROWS(caller.call("tempvalues", {}, -1, PUT));
}

TEST_CASE("temp_adc", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU || det_type == defs::MOENCH ||
        det_type == defs::GOTTHARD) {
        REQUIRE_NOTHROW(caller.call("temp_adc", {}, -1, GET));
        std::ostringstream oss;
        REQUIRE_NOTHROW(caller.call("temp_adc", {}, 0, GET, oss));
        std::string s = (oss.str()).erase(0, strlen("temp_adc "));
        REQUIRE(std::stoi(s) != -1);
    } else {
        REQUIRE_THROWS(caller.call("temp_adc", {}, -1, GET));
    }
}

TEST_CASE("temp_fpga", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type != defs::CHIPTESTBOARD) {
        REQUIRE_NOTHROW(caller.call("temp_fpga", {}, -1, GET));
        std::ostringstream oss;
        REQUIRE_NOTHROW(caller.call("temp_fpga", {}, 0, GET, oss));
        std::string s = (oss.str()).erase(0, strlen("temp_fpga "));
        REQUIRE(std::stoi(s) != -1);
    } else {
        REQUIRE_THROWS(caller.call("temp_fpga", {}, -1, GET));
    }
}

/* list */

TEST_CASE("daclist", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::CHIPTESTBOARD ||
        det_type == defs::XILINX_CHIPTESTBOARD) {
        REQUIRE_NOTHROW(caller.call("daclist", {}, -1, GET));

        auto prev = det.getDacNames();
        REQUIRE_THROWS(caller.call("daclist", {"a", "s", "d"}, -1, PUT));

        std::vector<std::string> names;
        for (int iarg = 0; iarg != 18; ++iarg) {
            names.push_back("a");
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(caller.call("daclist", names, -1, PUT, oss));
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(caller.call("daclist", {}, -1, GET, oss));
            REQUIRE(oss.str() ==
                    std::string("daclist ") + ToString(names) + '\n');
        }
        det.setDacNames(prev);

    } else {
        REQUIRE_THROWS(caller.call("daclist", {"a", "b"}, -1, PUT));
        REQUIRE_NOTHROW(caller.call("daclist", {}, -1, GET));
    }
}

/* dacs */

TEST_CASE("dacvalues", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    REQUIRE_NOTHROW(caller.call("dacvalues", {}, -1, GET));
    REQUIRE_THROWS(caller.call("dacvalues", {}, -1, PUT));
}

TEST_CASE("defaultdac", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type != defs::CHIPTESTBOARD &&
        det_type != defs::XILINX_CHIPTESTBOARD) {
        REQUIRE_THROWS(caller.call("defaultdac", {}, -1, GET));
        REQUIRE_THROWS(caller.call("defaultdac", {"blabla"}, -1, PUT));
        auto daclist = det.getDacList();
        for (auto it : daclist) {
            if (it == defs::VTHRESHOLD) {
                continue;
            }
            auto dacname = ToString(it);
            auto prev_val = det.getDefaultDac(it);
            {
                std::ostringstream oss;
                caller.call("defaultdac", {dacname, "1000"}, -1, PUT, oss);
                REQUIRE(oss.str() == std::string("defaultdac ") + dacname +
                                         std::string(" 1000\n"));
            }
            {
                std::ostringstream oss;
                caller.call("defaultdac", {dacname}, -1, GET, oss);
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
                    caller.call("defaultdac", {dacname, "1000", "gain0"}, -1,
                                PUT, oss);
                    REQUIRE(oss.str() == std::string("defaultdac ") + dacname +
                                             std::string(" gain0 1000\n"));
                }
                {
                    std::ostringstream oss;
                    caller.call("defaultdac", {dacname, "gain0"}, -1, GET, oss);
                    REQUIRE(oss.str() == std::string("defaultdac ") + dacname +
                                             std::string(" gain0 1000\n"));
                }
                for (int i = 0; i != det.size(); ++i) {
                    det.setDefaultDac(it, prev_val[i], defs::GAIN0, {i});
                }
            }
        }
    } else {
        REQUIRE_THROWS(caller.call("defaultdac", {}, -1, GET));
    }
}

TEST_CASE("resetdacs", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type != defs::CHIPTESTBOARD &&
        det_type != defs::XILINX_CHIPTESTBOARD) {
        auto prev_val = det.getSettings();

        REQUIRE_THROWS(caller.call("resetdacs", {}, -1, GET));
        REQUIRE_NOTHROW(caller.call("resetdacs", {}, -1, PUT));
        REQUIRE_NOTHROW(caller.call("resetdacs", {"hard"}, -1, PUT));

        // settings should not change especially for jungfrau/moench and m3
        auto next_val = det.getSettings();
        for (int i = 0; i != det.size(); ++i) {
            REQUIRE(prev_val[i] == next_val[i]);
        }
    } else {
        REQUIRE_THROWS(caller.call("resetdacs", {}, -1, GET));
        REQUIRE_THROWS(caller.call("resetdacs", {}, -1, PUT));
    }
}

/* acquisition */

TEST_CASE("trigger", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    REQUIRE_THROWS(caller.call("trigger", {}, -1, GET));
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::MYTHEN3) {
        REQUIRE_NOTHROW(caller.call("trigger", {}, -1, PUT));
    } else if (det_type == defs::EIGER || det_type == defs::JUNGFRAU ||
               det_type == defs::MOENCH) {
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
            caller.call("trigger", {}, -1, PUT, oss);
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
        REQUIRE_THROWS(caller.call("trigger", {}, -1, PUT));
    }
}

TEST_CASE("blockingtrigger", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    REQUIRE_THROWS(caller.call("blockingtrigger", {}, -1, GET));
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::EIGER || det_type == defs::JUNGFRAU ||
        det_type == defs::MOENCH) {
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
            caller.call("blockingtrigger", {}, -1, PUT, oss);
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
        REQUIRE_THROWS(caller.call("blockingtrigger", {}, -1, PUT));
    }
}

TEST_CASE("clearbusy", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    REQUIRE_NOTHROW(caller.call("clearbusy", {}, -1, PUT));
    REQUIRE_THROWS(caller.call("clearbusy", {"0"}, -1, PUT));
    REQUIRE_THROWS(caller.call("clearbusy", {}, -1, GET));
}

TEST_CASE("start", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    // PUT only command
    REQUIRE_THROWS(caller.call("start", {}, -1, GET));
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
        caller.call("start", {}, -1, PUT, oss);
        REQUIRE(oss.str() == "start successful\n");
    }
    if (det_type != defs::CHIPTESTBOARD && det_type != defs::MOENCH) {
        std::ostringstream oss;
        caller.call("status", {}, -1, GET, oss);
        REQUIRE(oss.str() == "status running\n");
    }
    det.stopDetector();
    det.setExptime(-1, prev_val);
    det.setPeriod(prev_period);
    det.setNumberOfFrames(prev_frames);
}

TEST_CASE("stop", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    // PUT only command
    REQUIRE_THROWS(caller.call("stop", {}, -1, GET));
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
        caller.call("status", {}, -1, GET, oss);
        REQUIRE(oss.str() == "status running\n");
    }
    {
        std::ostringstream oss;
        caller.call("stop", {}, -1, PUT, oss);
        REQUIRE(oss.str() == "stop successful\n");
    }
    {
        std::ostringstream oss;
        caller.call("status", {}, -1, GET, oss);
        REQUIRE(((oss.str() == "status stopped\n") ||
                 (oss.str() == "status idle\n")));
    }
    det.setExptime(-1, prev_val);
    det.setPeriod(prev_period);
    det.setNumberOfFrames(prev_frames);
}

TEST_CASE("status", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
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
        caller.call("status", {}, -1, GET, oss);
        REQUIRE(oss.str() == "status running\n");
    }
    det.stopDetector();
    {
        std::ostringstream oss;
        caller.call("status", {}, -1, GET, oss);
        REQUIRE(((oss.str() == "status stopped\n") ||
                 (oss.str() == "status idle\n")));
    }
    det.setExptime(-1, prev_val);
    det.setPeriod(prev_period);
    det.setNumberOfFrames(prev_frames);
}

TEST_CASE("nextframenumber", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::EIGER || det_type == defs::JUNGFRAU ||
        det_type == defs::MOENCH || det_type == defs::CHIPTESTBOARD ||
        det_type == defs::XILINX_CHIPTESTBOARD || det_type == defs::GOTTHARD2) {
        auto prev_sfnum = det.getNextFrameNumber();
        REQUIRE_THROWS(caller.call("nextframenumber", {"0"}, -1, PUT));
        {
            std::ostringstream oss;
            caller.call("nextframenumber", {"3"}, -1, PUT, oss);
            REQUIRE(oss.str() == "nextframenumber 3\n");
        }
        {
            std::ostringstream oss;
            caller.call("nextframenumber", {}, -1, GET, oss);
            REQUIRE(oss.str() == "nextframenumber 3\n");
        }
        {
            std::ostringstream oss;
            caller.call("nextframenumber", {"1"}, -1, PUT, oss);
            REQUIRE(oss.str() == "nextframenumber 1\n");
        }

        if (det_type == defs::GOTTHARD2) {
            auto prev_timing =
                det.getTimingMode().tsquash("inconsistent timing mode in test");
            auto prev_frames =
                det.getNumberOfFrames().tsquash("inconsistent #frames in test");
            auto prev_exptime =
                det.getExptime().tsquash("inconsistent exptime in test");
            auto prev_period =
                det.getPeriod().tsquash("inconsistent period in test");
            auto prev_burstmode =
                det.getBurstMode().tsquash("inconsistent burst mode in test");
            auto prev_bursts =
                det.getNumberOfBursts().tsquash("inconsistent #bursts in test");
            auto prev_burstperiod = det.getBurstPeriod().tsquash(
                "inconsistent burst period in test");

            det.setTimingMode(defs::AUTO_TIMING);
            det.setNumberOfFrames(1);
            det.setExptime(std::chrono::microseconds(200));
            det.setPeriod(std::chrono::milliseconds(1));
            det.setBurstMode(defs::CONTINUOUS_EXTERNAL);
            det.setNumberOfBursts(1);
            det.setBurstPeriod(std::chrono::milliseconds(0));

            det.startDetector();
            std::this_thread::sleep_for(std::chrono::seconds(2));
            auto currentfnum = det.getNextFrameNumber().tsquash(
                "inconsistent frame nr in test");
            REQUIRE(currentfnum == 2);

            det.setTimingMode(prev_timing);
            det.setNumberOfFrames(prev_frames);
            det.setExptime(prev_exptime);
            det.setPeriod(prev_period);
            det.setBurstMode(prev_burstmode);
            det.setNumberOfBursts(prev_bursts);
            det.setBurstPeriod(prev_burstperiod);
        } else {
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
            auto currentfnum = det.getNextFrameNumber().tsquash(
                "inconsistent frame nr in test");
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
        }

        for (int i = 0; i != det.size(); ++i) {
            det.setNextFrameNumber(prev_sfnum[i], {i});
        }
    } else {
        REQUIRE_THROWS(caller.call("nextframenumber", {}, -1, GET));
    }
}

TEST_CASE("scan", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    defs::dacIndex ind = defs::DAC_0;
    defs::dacIndex notImplementedInd = defs::DAC_0;
    auto det_type = det.getDetectorType().squash();
    switch (det_type) {
    case defs::CHIPTESTBOARD:
    case defs::XILINX_CHIPTESTBOARD:
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
    case defs::MOENCH:
        ind = defs::VIN_CM;
        notImplementedInd = defs::VSVP;
        break;
    case defs::GOTTHARD:
        ind = defs::VREF_DS;
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
            caller.call("scan", {ToString(ind), "500", "1500", "500"}, -1, PUT,
                        oss);
            CHECK(oss.str() ==
                  "scan [" + ToString(ind) + ", 500, 1500, 500]\n");
        }
        {
            std::ostringstream oss;
            caller.call("scan", {}, -1, GET, oss);
            CHECK(oss.str() == "scan [enabled\ndac " + ToString(ind) +
                                   "\nstart 500\nstop 1500\nstep "
                                   "500\nsettleTime 1ms\n]\n");
        }
        {
            std::ostringstream oss;
            caller.call("scan", {ToString(ind), "500", "1500", "500", "2s"}, -1,
                        PUT, oss);
            CHECK(oss.str() ==
                  "scan [" + ToString(ind) + ", 500, 1500, 500, 2s]\n");
        }
        {
            std::ostringstream oss;
            caller.call("scan", {}, -1, GET, oss);
            CHECK(oss.str() == "scan [enabled\ndac " + ToString(ind) +
                                   "\nstart 500\nstop 1500\nstep "
                                   "500\nsettleTime 2s\n]\n");
        }
        {
            std::ostringstream oss;
            caller.call("scan", {"0"}, -1, PUT, oss);
            CHECK(oss.str() == "scan [0]\n");
        }
        {
            std::ostringstream oss;
            caller.call("scan", {}, -1, GET, oss);
            CHECK(oss.str() == "scan [disabled]\n");
        }
        {
            std::ostringstream oss;
            caller.call("scan", {ToString(ind), "1500", "500", "-500"}, -1, PUT,
                        oss);
            CHECK(oss.str() ==
                  "scan [" + ToString(ind) + ", 1500, 500, -500]\n");
        }
        CHECK_THROWS(caller.call(
            "scan", {ToString(notImplementedInd), "500", "1500", "500"}, -1,
            PUT));
        CHECK_THROWS(caller.call("scan", {ToString(ind), "500", "1500", "-500"},
                                 -1, PUT));
        CHECK_THROWS(caller.call("scan", {ToString(ind), "1500", "500", "500"},
                                 -1, PUT));

        if (det_type == defs::MYTHEN3 || defs::EIGER) {
            {
                std::ostringstream oss;
                caller.call("scan", {"trimbits", "0", "63", "16", "2s"}, -1,
                            PUT, oss);
                CHECK(oss.str() == "scan [trimbits, 0, 63, 16, 2s]\n");
            }
            {
                std::ostringstream oss;
                caller.call("scan", {}, -1, GET, oss);
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
        //     det.setDAC(notImplementedInd, notImplementedPrevious[i],
        //     false, {i});
        // }
    }
}

TEST_CASE("scanerrmsg", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    REQUIRE_NOTHROW(caller.call("scanerrmsg", {}, -1, GET));
    REQUIRE_THROWS(caller.call("scanerrmsg", {""}, -1, PUT));
}

/* Network Configuration (Detector<->Receiver) */

TEST_CASE("numinterfaces", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU || det_type == defs::MOENCH) {
        auto prev_val = det.getNumberofUDPInterfaces().tsquash(
            "inconsistent numinterfaces to test");
        {
            std::ostringstream oss;
            caller.call("numinterfaces", {"2"}, -1, PUT, oss);
            REQUIRE(oss.str() == "numinterfaces 2\n");
        }
        {
            std::ostringstream oss;
            caller.call("numinterfaces", {"1"}, -1, PUT, oss);
            REQUIRE(oss.str() == "numinterfaces 1\n");
        }
        {
            std::ostringstream oss;
            caller.call("numinterfaces", {}, -1, GET, oss);
            REQUIRE(oss.str() == "numinterfaces 1\n");
        }
        det.setNumberofUDPInterfaces(prev_val);
    } else if (det_type == defs::EIGER) {
        REQUIRE_THROWS(caller.call("numinterfaces", {"1"}, -1, PUT));
        {
            std::ostringstream oss;
            caller.call("numinterfaces", {}, -1, GET, oss);
            REQUIRE(oss.str() == "numinterfaces 2\n");
        }
    } else {
        std::ostringstream oss;
        caller.call("numinterfaces", {}, -1, GET, oss);
        REQUIRE(oss.str() == "numinterfaces 1\n");
        REQUIRE_THROWS(caller.call("numinterfaces", {"1"}, -1, PUT));
    }
    REQUIRE_THROWS(caller.call("numinterfaces", {"3"}, -1, PUT));
    REQUIRE_THROWS(caller.call("numinterfaces", {"0"}, -1, PUT));
}

TEST_CASE("udp_srcip", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto prev_val = det.getSourceUDPIP();
    REQUIRE_THROWS(caller.call("udp_srcip", {"0.0.0.0"}, -1, PUT));
    {
        std::ostringstream oss;
        caller.call("udp_srcip", {"129.129.205.12"}, -1, PUT, oss);
        REQUIRE(oss.str() == "udp_srcip 129.129.205.12\n");
    }
    for (int i = 0; i != det.size(); ++i) {
        det.setSourceUDPIP(prev_val[i], {i});
    }
}

TEST_CASE("udp_dstlist", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU || det_type == defs::MOENCH ||
        det_type == defs::EIGER || det_type == defs::MYTHEN3 ||
        det_type == defs::GOTTHARD2) {
        REQUIRE_NOTHROW(caller.call("udp_dstlist", {}, 0, GET, std::cout, 0));
        REQUIRE_THROWS(caller.call(
            "udp_dstlist", {"ip=0.0.0.0", "mac=00:00:00:00:00:00", "port=1233"},
            -1, PUT, std::cout, 0));
    } else {
        REQUIRE_THROWS(caller.call("udp_dstlist", {}, -1, GET, std::cout, 0));
    }
}

TEST_CASE("udp_numdst", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU || det_type == defs::MOENCH ||
        det_type == defs::EIGER || det_type == defs::MYTHEN3 ||
        det_type == defs::GOTTHARD2) {
        REQUIRE_NOTHROW(caller.call("udp_numdst", {}, -1, GET));
    } else {
        REQUIRE_THROWS(caller.call("udp_numdst", {}, -1, GET));
    }
}

TEST_CASE("udp_cleardst", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    REQUIRE_THROWS(caller.call("udp_cleardst", {}, -1, GET));
    /* dont clear all udp destinations */
    /*REQUIRE_NOTHROW(caller.call("udp_cleardst", {}, -1, PUT));*/
}

TEST_CASE("udp_firstdst", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU || det_type == defs::MOENCH ||
        det_type == defs::MYTHEN3 || det_type == defs::GOTTHARD2) {
        auto prev_val = det.getFirstUDPDestination();
        {
            std::ostringstream oss;
            caller.call("udp_firstdst", {"0"}, -1, PUT, oss);
            REQUIRE(oss.str() == "udp_firstdst 0\n");
        }
        {
            std::ostringstream oss;
            caller.call("udp_firstdst", {}, -1, GET, oss);
            REQUIRE(oss.str() == "udp_firstdst 0\n");
        }
        /*
        {
            std::ostringstream oss;
            caller.call("udp_firstdst", {"1"}, -1, PUT, oss);
            REQUIRE(oss.str() == "udp_firstdst 1\n");
        }
        */
        REQUIRE_THROWS(caller.call("udp_firstdst", {"33"}, -1, PUT));

        for (int i = 0; i != det.size(); ++i) {
            det.setFirstUDPDestination(prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(caller.call("udp_firstdst", {}, -1, GET));
    }
}

TEST_CASE("udp_dstip", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    REQUIRE_THROWS(caller.call("udp_dstip", {"0.0.0.0"}, -1, PUT));
}

TEST_CASE("udp_srcmac", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto prev_val = det.getSourceUDPMAC();
    REQUIRE_THROWS(caller.call("udp_srcmac", {"00:00:00:00:00:00"}, -1, PUT));
    {
        std::ostringstream oss;
        caller.call("udp_srcmac", {"00:50:c2:42:34:12"}, -1, PUT, oss);
        REQUIRE(oss.str() == "udp_srcmac 00:50:c2:42:34:12\n");
    }
    for (int i = 0; i != det.size(); ++i) {
        if (prev_val[i].str() != "00:00:00:00:00:00") {
            det.setSourceUDPMAC(prev_val[i], {i});
        }
    }
}

TEST_CASE("udp_dstmac", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    REQUIRE_THROWS(caller.call("udp_dstmac", {"00:00:00:00:00:00"}, -1, PUT));
}

TEST_CASE("udp_dstport", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto prev_val = det.getDestinationUDPPort();
    {
        std::ostringstream oss;
        caller.call("udp_dstport", {"50084"}, -1, PUT, oss);
        REQUIRE(oss.str() == "udp_dstport 50084\n");
    }
    test_valid_port_caller("udp_dstport", {}, -1, PUT);
    test_valid_port_caller("udp_dstport", {}, 0, PUT);
    // should fail for the second module
    if (det.size() > 1) {
        REQUIRE_THROWS(caller.call("udp_dstport", {"65535"}, -1, PUT));
    }

    for (int i = 0; i != det.size(); ++i) {
        det.setDestinationUDPPort(prev_val[i], {i});
    }
}

TEST_CASE("udp_srcip2", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU || det_type == defs::MOENCH ||
        det_type == defs::GOTTHARD2) {
        auto prev_val = det.getSourceUDPIP2();
        REQUIRE_THROWS(caller.call("udp_srcip2", {"0.0.0.0"}, -1, PUT));
        {
            std::ostringstream oss;
            caller.call("udp_srcip2", {"129.129.205.12"}, -1, PUT, oss);
            REQUIRE(oss.str() == "udp_srcip2 129.129.205.12\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            if (prev_val[i] != IpAddr{"0.0.0.0"})
                det.setSourceUDPIP2(prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(caller.call("udp_srcip2", {}, -1, GET));
    }
}

TEST_CASE("udp_dstip2", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU || det_type == defs::MOENCH ||
        det_type == defs::GOTTHARD2) {
        REQUIRE_THROWS(caller.call("udp_dstip2", {"0.0.0.0"}, -1, PUT));
    } else {
        REQUIRE_THROWS(caller.call("udp_dstip2", {}, -1, GET));
    }
}

TEST_CASE("udp_srcmac2", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU || det_type == defs::MOENCH ||
        det_type == defs::GOTTHARD2) {
        auto prev_val = det.getSourceUDPMAC2();
        REQUIRE_THROWS(
            caller.call("udp_srcmac2", {"00:00:00:00:00:00"}, -1, PUT));
        {
            std::ostringstream oss;
            caller.call("udp_srcmac2", {"00:50:c2:42:34:12"}, -1, PUT, oss);
            REQUIRE(oss.str() == "udp_srcmac2 00:50:c2:42:34:12\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            if (prev_val[i].str() != "00:00:00:00:00:00") {
                det.setSourceUDPMAC2(prev_val[i], {i});
            }
        }
    } else {
        REQUIRE_THROWS(caller.call("udp_srcmac2", {}, -1, GET));
    }
}

TEST_CASE("udp_dstmac2", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU || det_type == defs::MOENCH ||
        det_type == defs::GOTTHARD2) {
        REQUIRE_THROWS(
            caller.call("udp_dstmac2", {"00:00:00:00:00:00"}, -1, PUT));
    } else {
        REQUIRE_THROWS(caller.call("udp_dstmac2", {}, -1, GET));
    }
}

TEST_CASE("udp_dstport2", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU || det_type == defs::MOENCH ||
        det_type == defs::GOTTHARD2 || det_type == defs::EIGER) {
        auto prev_val = det.getDestinationUDPPort2();
        {
            std::ostringstream oss;
            caller.call("udp_dstport2", {"50084"}, -1, PUT, oss);
            REQUIRE(oss.str() == "udp_dstport2 50084\n");
        }
        test_valid_port_caller("udp_dstport2", {}, -1, PUT);
        test_valid_port_caller("udp_dstport2", {}, 0, PUT);
        // should fail for the second module
        if (det.size() > 1) {
            REQUIRE_THROWS(caller.call("udp_dstport2", {"65535"}, -1, PUT));
        }

        for (int i = 0; i != det.size(); ++i) {
            if (prev_val[i] != 0) {
                det.setDestinationUDPPort2(prev_val[i], {i});
            }
        }
    } else {
        REQUIRE_THROWS(caller.call("udp_dstport2", {}, -1, GET));
    }
}

TEST_CASE("udp_reconfigure", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    REQUIRE_THROWS(caller.call("udp_reconfigure", {}, -1, GET));
    REQUIRE_NOTHROW(caller.call("udp_reconfigure", {}, -1, PUT));
}

TEST_CASE("udp_validate", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    REQUIRE_THROWS(caller.call("udp_validate", {}, -1, GET));
    REQUIRE_NOTHROW(caller.call("udp_validate", {}, -1, PUT));
}

TEST_CASE("tengiga", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);

    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::EIGER || det_type == defs::CHIPTESTBOARD ||
        det_type == defs::MYTHEN3) {
        auto tengiga = det.getTenGiga();
        det.setTenGiga(false);

        std::ostringstream oss1, oss2;
        caller.call("tengiga", {"1"}, -1, PUT, oss1);
        REQUIRE(oss1.str() == "tengiga 1\n");
        caller.call("tengiga", {}, -1, GET, oss2);
        REQUIRE(oss2.str() == "tengiga 1\n");

        for (int i = 0; i != det.size(); ++i) {
            det.setTenGiga(tengiga[i], {i});
        }
    } else {
        REQUIRE_THROWS(caller.call("tengiga", {}, -1, GET));
    }
}

TEST_CASE("flowcontrol10g", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::EIGER || det_type == defs::JUNGFRAU ||
        det_type == defs::MOENCH) {
        auto prev_val = det.getTenGigaFlowControl();
        {
            std::ostringstream oss;
            caller.call("flowcontrol10g", {"1"}, -1, PUT, oss);
            REQUIRE(oss.str() == "flowcontrol10g 1\n");
        }
        {
            std::ostringstream oss;
            caller.call("flowcontrol10g", {"0"}, -1, PUT, oss);
            REQUIRE(oss.str() == "flowcontrol10g 0\n");
        }
        {
            std::ostringstream oss;
            caller.call("flowcontrol10g", {}, -1, GET, oss);
            REQUIRE(oss.str() == "flowcontrol10g 0\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setTenGigaFlowControl(prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(caller.call("flowcontrol10g", {}, -1, GET));
    }
}

TEST_CASE("txdelay_frame", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::EIGER || det_type == defs::JUNGFRAU ||
        det_type == defs::MOENCH || det_type == defs::MYTHEN3) {
        auto prev_val = det.getTransmissionDelayFrame();
        auto val = 5000;
        if (det_type == defs::JUNGFRAU || det_type == defs::MOENCH ||
            det_type == defs::MYTHEN3) {
            val = 5;
        }
        std::string sval = std::to_string(val);
        {
            std::ostringstream oss1, oss2;
            caller.call("txdelay_frame", {sval}, -1, PUT, oss1);
            REQUIRE(oss1.str() == "txdelay_frame " + sval + "\n");
            caller.call("txdelay_frame", {}, -1, GET, oss2);
            REQUIRE(oss2.str() == "txdelay_frame " + sval + "\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setTransmissionDelayFrame(prev_val[i]);
        }
    } else {
        REQUIRE_THROWS(caller.call("txdelay_frame", {}, -1, GET));
    }
}

TEST_CASE("txdelay", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::EIGER || det_type == defs::JUNGFRAU ||
        det_type == defs::MOENCH || det_type == defs::MYTHEN3) {

        // cannot get transmission delay with just one module
        if ((det_type == defs::JUNGFRAU || det_type == defs::MOENCH ||
             det_type == defs::MYTHEN3) &&
            (det.size() < 2)) {
            REQUIRE_THROWS(caller.call("txdelay", {}, -1, GET));
            int val = 5;
            std::string sval = std::to_string(val);
            {
                std::ostringstream oss1;
                caller.call("txdelay", {sval}, -1, PUT, oss1);
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
            if (det_type == defs::JUNGFRAU || det_type == defs::MOENCH ||
                det_type == defs::MYTHEN3) {
                val = 5;
            }
            std::string sval = std::to_string(val);
            {
                std::ostringstream oss1, oss2;
                caller.call("txdelay", {sval}, -1, PUT, oss1);
                REQUIRE(oss1.str() == "txdelay " + sval + "\n");
                caller.call("txdelay", {}, -1, GET, oss2);
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
            REQUIRE_THROWS(caller.call("txdelay", {"5"}, 0, PUT));
            REQUIRE_THROWS(caller.call("txdelay", {}, 0, GET));

            for (int i = 0; i != det.size(); ++i) {
                if (eiger) {
                    det.setTransmissionDelayLeft(prev_left[i]);
                    det.setTransmissionDelayRight(prev_right[i]);
                }
                det.setTransmissionDelayFrame(prev_frame[i]);
            }
        }
    } else {
        REQUIRE_THROWS(caller.call("txdelay", {}, -1, GET));
    }
}

/* ZMQ Streaming Parameters (Receiver<->Client) */

TEST_CASE("zmqport", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);

    int socketsperdetector = 1;
    auto det_type = det.getDetectorType().squash();
    int prev = 1;
    if (det_type == defs::EIGER) {
        socketsperdetector *= 2;
    } else if (det_type == defs::JUNGFRAU || det_type == defs::MOENCH) {
        prev = det.getNumberofUDPInterfaces().squash();
        det.setNumberofUDPInterfaces(2);
        socketsperdetector *= 2;
    }
    uint16_t port = 3500;
    auto port_str = std::to_string(port);
    {
        std::ostringstream oss;
        caller.call("zmqport", {port_str}, -1, PUT, oss);
        REQUIRE(oss.str() == "zmqport " + port_str + '\n');
    }
    for (int i = 0; i != det.size(); ++i) {
        std::ostringstream oss;
        caller.call("zmqport", {}, i, GET, oss);
        REQUIRE(oss.str() == "zmqport " +
                                 std::to_string(port + i * socketsperdetector) +
                                 '\n');
    }

    port = 1954;
    port_str = std::to_string(port);
    {
        std::ostringstream oss;
        caller.call("zmqport", {port_str}, -1, PUT, oss);
        REQUIRE(oss.str() == "zmqport " + port_str + '\n');
    }
    for (int i = 0; i != det.size(); ++i) {
        std::ostringstream oss;
        caller.call("zmqport", {}, i, GET, oss);
        REQUIRE(oss.str() == "zmqport " +
                                 std::to_string(port + i * socketsperdetector) +
                                 '\n');
    }
    test_valid_port_caller("zmqport", {}, -1, PUT);
    test_valid_port_caller("zmqport", {}, 0, PUT);
    // should fail for the second module
    if (det.size() > 1) {
        REQUIRE_THROWS(caller.call("zmqport", {"65535"}, -1, PUT));
    }

    if (det_type == defs::JUNGFRAU || det_type == defs::MOENCH) {
        det.setNumberofUDPInterfaces(prev);
    }
}

TEST_CASE("zmqip", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    std::ostringstream oss1, oss2;
    auto zmqip = det.getClientZmqIp();
    caller.call("zmqip", {}, 0, GET, oss1);
    REQUIRE(oss1.str() == "zmqip " + zmqip[0].str() + '\n');

    caller.call("zmqip", {zmqip[0].str()}, 0, PUT, oss2);
    REQUIRE(oss2.str() == "zmqip " + zmqip[0].str() + '\n');

    for (int i = 0; i != det.size(); ++i) {
        det.setClientZmqIp(zmqip[i], {i});
    }
}

TEST_CASE("zmqhwm", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto prev_val = det.getClientZmqHwm();
    {
        std::ostringstream oss;
        caller.call("zmqhwm", {"50"}, -1, PUT, oss);
        REQUIRE(oss.str() == "zmqhwm 50\n");
    }
    {
        std::ostringstream oss;
        caller.call("zmqhwm", {}, -1, GET, oss);
        REQUIRE(oss.str() == "zmqhwm 50\n");
    }
    {
        std::ostringstream oss;
        caller.call("zmqhwm", {"0"}, -1, PUT, oss);
        REQUIRE(oss.str() == "zmqhwm 0\n");
    }
    {
        std::ostringstream oss;
        caller.call("zmqhwm", {"-1"}, -1, PUT, oss);
        REQUIRE(oss.str() == "zmqhwm -1\n");
    }
    det.setClientZmqHwm(prev_val);
}

/* Advanced */

TEST_CASE("adcpipeline", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::CHIPTESTBOARD || det_type == defs::MOENCH) {
        auto prev_val = det.getADCPipeline();
        {
            std::ostringstream oss;
            caller.call("adcpipeline", {"1"}, -1, PUT, oss);
            REQUIRE(oss.str() == "adcpipeline 1\n");
        }
        {
            std::ostringstream oss;
            caller.call("adcpipeline", {"0"}, -1, PUT, oss);
            REQUIRE(oss.str() == "adcpipeline 0\n");
        }
        {
            std::ostringstream oss;
            caller.call("adcpipeline", {"15"}, -1, PUT, oss);
            REQUIRE(oss.str() == "adcpipeline 15\n");
        }
        {
            std::ostringstream oss;
            caller.call("adcpipeline", {}, -1, GET, oss);
            REQUIRE(oss.str() == "adcpipeline 15\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setADCPipeline(prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(caller.call("adcpipeline", {}, -1, GET));
    }
}

TEST_CASE("programfpga", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::CHIPTESTBOARD || det_type == defs::JUNGFRAU ||
        det_type == defs::MOENCH || det_type == defs::MYTHEN3 ||
        det_type == defs::GOTTHARD2) {
        // TODO program a real board?
        /// afs/psi.ch/project/sls_det_firmware/jungfrau_firmware/cyclone_V/v0_8/Jungfrau_MCB.pof
        REQUIRE_THROWS(caller.call("programfpga", {}, -1, GET));
    } else {
        REQUIRE_THROWS(caller.call("programfpga", {}, -1, GET));
        REQUIRE_THROWS(caller.call("programfpga", {"/tmp/test.pof"}, -1, PUT));
    }
}

TEST_CASE("resetfpga", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU || det_type == defs::CHIPTESTBOARD ||
        det_type == defs::MOENCH || det_type == defs::XILINX_CHIPTESTBOARD) {
        // reset will also reset udp info from config file (comment out for
        // invdividual tests) std::ostringstream oss; caller.call("resetfpga",
        // {}, -1, PUT, oss); REQUIRE(oss.str() == "resetfpga successful\n");
        REQUIRE_THROWS(caller.call("resetfpga", {}, -1, GET));
    } else {
        REQUIRE_THROWS(caller.call("resetfpga", {}, -1, GET));
        REQUIRE_THROWS(caller.call("resetfpga", {}, -1, PUT));
    }
}

TEST_CASE("updatekernel", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU || det_type == defs::MOENCH ||
        det_type == defs::CHIPTESTBOARD || det_type == defs::MYTHEN3 ||
        det_type == defs::GOTTHARD2) {
        // TODO: send real server?
        // std::ostringstream oss;
        // caller.call("updatekernel",{"juImage_detector.lzma",
        // "pc13784"}, -1, PUT, oss);
        // REQUIRE(oss.str() == "updatekernel successful\n");
        REQUIRE_THROWS(caller.call("updatekernel", {}, -1, GET));
    } else {
        REQUIRE_THROWS(caller.call("updatekernel", {}, -1, GET));
        REQUIRE_THROWS(caller.call("updatekernel", {}, -1, PUT));
    }
}

TEST_CASE("rebootcontroller", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU || det_type == defs::MOENCH ||
        det_type == defs::CHIPTESTBOARD || det_type == defs::MYTHEN3 ||
        det_type == defs::GOTTHARD2 || det_type == defs::GOTTHARD ||
        det_type == defs::XILINX_CHIPTESTBOARD) {
        // TODO: reboot real server?
        // REQUIRE_NOTHROW(caller.call("rebootcontroller", {}, -1, PUT));
        REQUIRE_THROWS(caller.call("rebootcontroller", {}, -1, GET));
    } else {
        REQUIRE_THROWS(caller.call("rebootcontroller", {}, -1, GET));
        REQUIRE_THROWS(caller.call("rebootcontroller", {}, -1, PUT));
    }
}

TEST_CASE("update", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU || det_type == defs::MOENCH ||
        det_type == defs::CHIPTESTBOARD) {
        // TODO: update real server and firmware?
        // REQUIRE_NOTHROW(caller.call("update",
        // {"jungfrauDetectorServerv4.0.1.0", "pc13784",
        // "/afs/psi.ch/project/sls_det_firmware/jungfrau_firmware/cyclone_V/v0_8/Jungfrau_MCB.pof"},
        // -1, PUT));
        REQUIRE_THROWS(caller.call("update", {}, -1, GET));
    } else {
        REQUIRE_THROWS(caller.call("update", {}, -1, GET));
        REQUIRE_THROWS(caller.call("update", {}, -1, PUT));
    }
}

TEST_CASE("reg", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type != defs::EIGER) {
        uint32_t addr = 0x64;
        if (det_type == defs::MYTHEN3) {
            addr = 0x80;
        }
        if (det_type == defs::GOTTHARD2) {
            addr = 0x298;
        }
        std::string saddr = ToStringHex(addr);
        auto prev_val = det.readRegister(addr);
        {
            std::ostringstream oss1, oss2;
            caller.call("reg", {saddr, "0x6", "--validate"}, -1, PUT, oss1);
            REQUIRE(oss1.str() == "reg [" + saddr + ", 0x6]\n");
            caller.call("reg", {saddr}, -1, GET, oss2);
            REQUIRE(oss2.str() == "reg 0x6\n");
        }
        {
            std::ostringstream oss1, oss2;
            caller.call("reg", {saddr, "0x5"}, -1, PUT, oss1);
            REQUIRE(oss1.str() == "reg [" + saddr + ", 0x5]\n");
            caller.call("reg", {saddr}, -1, GET, oss2);
            REQUIRE(oss2.str() == "reg 0x5\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.writeRegister(addr, prev_val[i], false, {i});
        }
    }
    // cannot check for eiger virtual server
    else {
        REQUIRE_NOTHROW(caller.call("reg", {"0x64"}, -1, GET));
    }
}

TEST_CASE("adcreg", "[.cmdcall]") {
    // TODO! what is a safe value to use?
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU || det_type == defs::MOENCH ||
        det_type == defs::CHIPTESTBOARD || det_type == defs::GOTTHARD) {
        std::ostringstream oss;
        caller.call("adcreg", {"0x8", "0x3"}, -1, PUT, oss);
        REQUIRE(oss.str() == "adcreg [0x8, 0x3]\n");
        // This is a put only command
        REQUIRE_THROWS(caller.call("adcreg", {}, -1, GET));
    } else {
        REQUIRE_THROWS(caller.call("adcreg", {"0x0", "0"}, -1, PUT));
        REQUIRE_THROWS(caller.call("adcreg", {}, -1, GET));
    }
}

TEST_CASE("setbit", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type != defs::EIGER) {
        uint32_t addr = 0x64;
        if (det_type == defs::MYTHEN3) {
            addr = 0x80;
        }
        if (det_type == defs::GOTTHARD2) {
            addr = 0x298;
        }
        std::string saddr = ToStringHex(addr);
        auto prev_val = det.readRegister(addr);
        {
            std::ostringstream oss1, oss2, oss3;
            caller.call("reg", {saddr, "0x0"}, -1, PUT);
            caller.call("setbit", {saddr, "1"}, -1, PUT, oss1);
            REQUIRE(oss1.str() == "setbit [" + saddr + ", 1]\n");
            caller.call("setbit", {saddr, "2", "--validate"}, -1, PUT, oss2);
            REQUIRE(oss2.str() == "setbit [" + saddr + ", 2]\n");
            caller.call("reg", {saddr}, -1, GET, oss3);
            REQUIRE(oss3.str() == "reg 0x6\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.writeRegister(addr, prev_val[i], false, {i});
        }
    }
}

TEST_CASE("clearbit", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type != defs::EIGER) {
        uint32_t addr = 0x64;
        if (det_type == defs::MYTHEN3) {
            addr = 0x80;
        }
        if (det_type == defs::GOTTHARD2) {
            addr = 0x298;
        }
        std::string saddr = ToStringHex(addr);
        auto prev_val = det.readRegister(addr);
        {
            std::ostringstream oss1, oss2, oss3;
            caller.call("reg", {saddr, "0x7"}, -1, PUT);
            caller.call("clearbit", {saddr, "1"}, -1, PUT, oss1);
            REQUIRE(oss1.str() == "clearbit [" + saddr + ", 1]\n");
            caller.call("clearbit", {saddr, "2", "--validate"}, -1, PUT, oss2);
            REQUIRE(oss2.str() == "clearbit [" + saddr + ", 2]\n");
            caller.call("reg", {saddr}, -1, GET, oss3);
            REQUIRE(oss3.str() == "reg 0x1\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.writeRegister(addr, prev_val[i], false, {i});
        }
    }
}

TEST_CASE("getbit", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type != defs::EIGER) {
        uint32_t addr = 0x64;
        if (det_type == defs::MYTHEN3) {
            addr = 0x80;
        }
        if (det_type == defs::GOTTHARD2) {
            addr = 0x298;
        }
        std::string saddr = ToStringHex(addr);
        auto prev_val = det.readRegister(addr);
        {
            std::ostringstream oss1, oss2;
            caller.call("reg", {saddr, "0x3"}, -1, PUT);
            caller.call("getbit", {saddr, "1"}, -1, GET, oss1);
            REQUIRE(oss1.str() == "getbit 1\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.writeRegister(addr, prev_val[i], false, {i});
        }
    }
    // cannot check for eiger virtual server
    else {
        REQUIRE_NOTHROW(caller.call("getbit", {"0x64", "1"}, -1, GET));
    }
}

TEST_CASE("firmwaretest", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU || det_type == defs::MOENCH ||
        det_type == defs::CHIPTESTBOARD || det_type == defs::GOTTHARD ||
        det_type == defs::MYTHEN3 || det_type == defs::GOTTHARD2 ||
        det_type == defs::XILINX_CHIPTESTBOARD) {
        std::ostringstream oss;
        caller.call("firmwaretest", {}, -1, PUT, oss);
        REQUIRE(oss.str() == "firmwaretest successful\n");
        REQUIRE_THROWS(caller.call("firmwaretest", {}, -1, GET));
    } else {
        REQUIRE_THROWS(caller.call("firmwaretest", {}, -1, GET));
        REQUIRE_THROWS(caller.call("firmwaretest", {}, -1, PUT));
    }
}

TEST_CASE("bustest", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU || det_type == defs::MOENCH ||
        det_type == defs::CHIPTESTBOARD || det_type == defs::GOTTHARD ||
        det_type == defs::MYTHEN3 || det_type == defs::GOTTHARD2) {
        std::ostringstream oss;
        caller.call("bustest", {}, -1, PUT, oss);
        REQUIRE(oss.str() == "bustest successful\n");
        REQUIRE_THROWS(caller.call("bustest", {}, -1, GET));
    } else {
        REQUIRE_THROWS(caller.call("bustest", {}, -1, GET));
        REQUIRE_THROWS(caller.call("bustest", {}, -1, PUT));
    }
}

TEST_CASE("initialchecks", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto check = det.getInitialChecks();
    {
        std::ostringstream oss;
        caller.call("initialchecks", {"0"}, -1, PUT, oss);
        REQUIRE(oss.str() == "initialchecks 0\n");
    }
    {
        std::ostringstream oss;
        caller.call("initialchecks", {}, -1, GET, oss);
        REQUIRE(oss.str() == "initialchecks 0\n");
    }
    {
        std::ostringstream oss;
        caller.call("initialchecks", {}, -1, GET, oss);
        REQUIRE(oss.str() == "initialchecks 0\n");
    }
    det.setInitialChecks(check);
}

TEST_CASE("adcinvert", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::CHIPTESTBOARD || det_type == defs::JUNGFRAU ||
        det_type == defs::MOENCH) {
        auto prev_val = det.getADCInvert();
        {
            std::ostringstream oss;
            caller.call("adcinvert", {"0x8d0a21d4"}, -1, PUT, oss);
            REQUIRE(oss.str() == "adcinvert 0x8d0a21d4\n");
        }
        {
            std::ostringstream oss;
            caller.call("adcinvert", {}, -1, GET, oss);
            REQUIRE(oss.str() == "adcinvert 0x8d0a21d4\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setADCInvert(prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(caller.call("adcinvert", {}, -1, GET));
    }
}

/* Insignificant */

TEST_CASE("port", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto prev_val = det.getControlPort({0}).squash();
    {
        std::ostringstream oss;
        caller.call("port", {"1942"}, 0, PUT, oss);
        REQUIRE(oss.str() == "port 1942\n");
    }
    {
        std::ostringstream oss;
        caller.call("port", {}, 0, GET, oss);
        REQUIRE(oss.str() == "port 1942\n");
    }
    test_valid_port_caller("port", {}, -1, PUT);
    test_valid_port_caller("port", {}, 0, PUT);
    // should fail for the second module
    if (det.size() > 1) {
        REQUIRE_THROWS(caller.call("port", {"65536"}, -1, PUT));
    }

    det.setControlPort(prev_val, {0});
}

TEST_CASE("stopport", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto prev_val = det.getStopPort({0}).squash();
    {
        std::ostringstream oss;
        caller.call("stopport", {"1942"}, 0, PUT, oss);
        REQUIRE(oss.str() == "stopport 1942\n");
    }
    {
        std::ostringstream oss;
        caller.call("stopport", {}, 0, GET, oss);
        REQUIRE(oss.str() == "stopport 1942\n");
    }
    test_valid_port_caller("stopport", {}, -1, PUT);
    test_valid_port_caller("stopport", {}, 0, PUT);
    // should fail for the second module
    if (det.size() > 1) {
        REQUIRE_THROWS(caller.call("stopport", {"65536"}, -1, PUT));
    }

    det.setStopPort(prev_val, {0});
}

TEST_CASE("lock", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto prev_val = det.getDetectorLock();
    {
        std::ostringstream oss;
        caller.call("lock", {"1"}, -1, PUT, oss);
        REQUIRE(oss.str() == "lock 1\n");
    }
    {
        std::ostringstream oss;
        caller.call("lock", {}, -1, GET, oss);
        REQUIRE(oss.str() == "lock 1\n");
    }
    {
        std::ostringstream oss;
        caller.call("lock", {"0"}, -1, PUT, oss);
        REQUIRE(oss.str() == "lock 0\n");
    }
    for (int i = 0; i != det.size(); ++i) {
        det.setDetectorLock(prev_val[i], {i});
    }
}

TEST_CASE("execcommand", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    REQUIRE_NOTHROW(caller.call("execcommand", {"ls *.txt"}, -1, PUT));
}

TEST_CASE("framecounter", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU || det_type == defs::MOENCH ||
        det_type == defs::CHIPTESTBOARD || det_type == defs::MYTHEN3 ||
        det_type == defs::GOTTHARD2 || det_type == defs::XILINX_CHIPTESTBOARD) {
        auto framecounter = det.getNumberOfFramesFromStart().squash();
        std::ostringstream oss;
        caller.call("framecounter", {}, -1, GET, oss);
        REQUIRE(oss.str() ==
                "framecounter " + std::to_string(framecounter) + "\n");
        REQUIRE_NOTHROW(caller.call("framecounter", {}, -1, GET));
    } else {
        REQUIRE_THROWS(caller.call("framecounter", {}, -1, GET));
    }
}

TEST_CASE("runtime", "[.cmdcall]") {
    // TODO! can we test this?
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU || det_type == defs::MOENCH ||
        det_type == defs::CHIPTESTBOARD || det_type == defs::MYTHEN3 ||
        det_type == defs::GOTTHARD2 || det_type == defs::XILINX_CHIPTESTBOARD) {
        std::ostringstream oss;
        caller.call("runtime", {}, -1, GET, oss);
        // Get only
        REQUIRE_THROWS(caller.call("runtime", {"2019"}, -1, PUT));
        REQUIRE_NOTHROW(caller.call("runtime", {}, -1, GET));
    } else {
        REQUIRE_THROWS(caller.call("runtime", {}, -1, GET));
    }
}

TEST_CASE("frametime", "[.cmdcall]") {
    // TODO! can we test this?
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU || det_type == defs::MOENCH ||
        det_type == defs::CHIPTESTBOARD || det_type == defs::MYTHEN3 ||
        det_type == defs::GOTTHARD2 || det_type == defs::XILINX_CHIPTESTBOARD) {
        std::ostringstream oss;
        caller.call("frametime", {}, -1, GET, oss);
        // Get only
        REQUIRE_THROWS(caller.call("frametime", {"2019"}, -1, PUT));
        REQUIRE_NOTHROW(caller.call("frametime", {}, -1, GET));
    } else {
        REQUIRE_THROWS(caller.call("frametime", {}, -1, GET));
    }
}

TEST_CASE("user", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    caller.call("user", {}, -1, GET);

    // This is a get only command
    REQUIRE_THROWS(caller.call("user", {}, -1, PUT));
    REQUIRE_NOTHROW(caller.call("user", {}, -1, GET));
}

TEST_CASE("sleep", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    REQUIRE_NOTHROW(caller.call("sleep", {"1"}, -1, PUT));
    REQUIRE_NOTHROW(caller.call("sleep", {"100", "ms"}, -1, PUT));
    REQUIRE_NOTHROW(caller.call("sleep", {"1000", "ns"}, -1, PUT));
    // This is a put only command
    REQUIRE_THROWS(caller.call("sleep", {}, -1, GET));
}

} // namespace sls