// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "Caller.h"
#include "catch.hpp"
#include "sls/Detector.h"
#include "sls/sls_detector_defs.h"
#include <sstream>

#include "sls/Result.h"
#include "sls/ToString.h"
#include "sls/versionAPI.h"
#include "test-Caller-global.h"
#include "tests/globals.h"

namespace sls {

using test::GET;
using test::PUT;

/* dacs */

TEST_CASE("Setting and reading back MYTHEN3 dacs", "[.cmdcall][.dacs]") {
    // vcassh, vth2, vshaper, vshaperneg, vipre_out, vth3, vth1,
    // vicin, vcas, vpreamp, vpl, vipre, viinsh, vph, vtrim, vdcsh,

    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::MYTHEN3) {
        SECTION("vcassh") { test_dac_caller(defs::VCASSH, "vcassh", 1200); }
        SECTION("vth2") { test_dac_caller(defs::VTH2, "vth2", 2800); }
        SECTION("vrshaper") {
            test_dac_caller(defs::VRSHAPER, "vrshaper", 1280);
        }
        SECTION("vrshaper_n") {
            test_dac_caller(defs::VRSHAPER_N, "vrshaper_n", 2800);
        }
        SECTION("vipre_out") {
            test_dac_caller(defs::VIPRE_OUT, "vipre_out", 1220);
        }
        SECTION("vth3") { test_dac_caller(defs::VTH3, "vth3", 2800); }
        SECTION("vth1") { test_dac_caller(defs::VTH1, "vth1", 2880); }
        SECTION("vicin") { test_dac_caller(defs::VICIN, "vicin", 1708); }
        SECTION("vcas") { test_dac_caller(defs::VCAS, "vcas", 1800); }
        SECTION("vrpreamp") {
            test_dac_caller(defs::VRPREAMP, "vrpreamp", 1100);
        }
        SECTION("vcal_n") { test_dac_caller(defs::VCAL_N, "vcal_n", 1100); }
        SECTION("vipre") { test_dac_caller(defs::VIPRE, "vipre", 2624); }
        SECTION("vishaper") {
            test_dac_caller(defs::VISHAPER, "vishaper", 1708);
        }
        SECTION("vcal_p") { test_dac_caller(defs::VCAL_P, "vcal_p", 1712); }
        SECTION("vtrim") { test_dac_caller(defs::VTRIM, "vtrim", 2800); }
        SECTION("vdcsh") { test_dac_caller(defs::VDCSH, "vdcsh", 800); }
        SECTION("vthreshold") {
            // Read out individual vcmp to be able to reset after
            // the test is done
            auto vth1 = det.getDAC(defs::VTH1, false);
            auto vth2 = det.getDAC(defs::VTH2, false);
            auto vth3 = det.getDAC(defs::VTH3, false);
            auto mask = det.getCounterMask();

            {
                std::ostringstream oss;
                caller.call("dac", {"vthreshold", "1234"}, -1, PUT, oss);
                REQUIRE(oss.str() == "dac vthreshold 1234\n");
            }
            {
                std::ostringstream oss;
                caller.call("dac", {"vthreshold"}, -1, GET, oss);
                REQUIRE(oss.str() == "dac vthreshold 1234\n");
            }

            // disabling counters change vth values
            caller.call("counters", {"0"}, -1, PUT);
            {
                std::ostringstream oss1, oss2, oss3;
                caller.call("dac", {"vth1"}, -1, GET, oss1);
                REQUIRE(oss1.str() == "dac vth1 1234\n");
                caller.call("dac", {"vth2"}, -1, GET, oss2);
                REQUIRE(oss2.str() == "dac vth2 2800\n");
                caller.call("dac", {"vth3"}, -1, GET, oss3);
                REQUIRE(oss3.str() == "dac vth3 2800\n");
            }
            // vthreshold changes vth for only enabled counters
            REQUIRE_NOTHROW(
                caller.call("dac", {"vthreshold", "2100"}, -1, PUT));
            {
                std::ostringstream oss;
                caller.call("dac", {"vthreshold"}, -1, GET, oss);
                REQUIRE(oss.str() == "dac vthreshold 2100\n");
                std::ostringstream oss1, oss2, oss3;
                caller.call("dac", {"vth1"}, -1, GET, oss1);
                REQUIRE(oss1.str() == "dac vth1 2100\n");
                caller.call("dac", {"vth2"}, -1, GET, oss2);
                REQUIRE(oss2.str() == "dac vth2 2800\n");
                caller.call("dac", {"vth3"}, -1, GET, oss3);
                REQUIRE(oss3.str() == "dac vth3 2800\n");
            }
            // vth overwrite vth even if counter disabled
            {
                std::ostringstream oss;
                caller.call("dac", {"vth2", "2200"}, -1, PUT);
                caller.call("dac", {"vth2"}, -1, GET, oss);
                REQUIRE(oss.str() == "dac vth2 2200\n");
            }
            // counters enabled, sets remembered values
            caller.call("counters", {"0", "1", "2"}, -1, PUT);
            {
                std::ostringstream oss1, oss2, oss3;
                caller.call("dac", {"vth1"}, -1, GET, oss1);
                REQUIRE(oss1.str() == "dac vth1 2100\n");
                caller.call("dac", {"vth2"}, -1, GET, oss2);
                REQUIRE(oss2.str() == "dac vth2 2200\n");
                caller.call("dac", {"vth3"}, -1, GET, oss3);
                REQUIRE(oss3.str() == "dac vth3 2100\n");
            }
            // counters enabled, sets remembered values
            caller.call("counters", {"0", "1"}, -1, PUT);
            {
                std::ostringstream oss1, oss2, oss3;
                caller.call("dac", {"vth1"}, -1, GET, oss1);
                REQUIRE(oss1.str() == "dac vth1 2100\n");
                caller.call("dac", {"vth2"}, -1, GET, oss2);
                REQUIRE(oss2.str() == "dac vth2 2200\n");
                caller.call("dac", {"vth3"}, -1, GET, oss3);
                REQUIRE(oss3.str() == "dac vth3 2800\n");
            }
            // Reset dacs after test
            for (int i = 0; i != det.size(); ++i) {
                det.setCounterMask(mask[i], {i});
                det.setDAC(defs::VTH1, vth1[i], false, {i});
                det.setDAC(defs::VTH2, vth2[i], false, {i});
                det.setDAC(defs::VTH3, vth3[i], false, {i});
            }
        }
        REQUIRE_THROWS(caller.call("dac", {"vsvp"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vsvn"}, -1, GET));
        // REQUIRE_THROWS(caller.call("dac", {"vtrim"}, -1, GET));
        // REQUIRE_THROWS(caller.call("dac", {"vrpreamp"}, -1, GET));
        // REQUIRE_THROWS(caller.call("dac", {"vrshaper"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vtgstv"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vcmp_ll"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vcmp_lr"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vcal"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vcmp_rl"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vcmp_rr"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"rxb_rb"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"rxb_lb"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vcp"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vcn"}, -1, GET));
        // REQUIRE_THROWS(caller.call("dac", {"vishaper"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"iodelay"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vref_ds"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vcascn_pb"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vcascp_pb"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vout_cm"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vcasc_out"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vin_cm"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vref_comp"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"ib_test_c"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vref_h_adc"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vb_comp_fe"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vb_comp_adc"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vcom_cds"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vref_rstore"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vb_opa_1st"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vref_comp_fe"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vcom_adc1"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vref_prech"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vref_l_adc"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vref_cds"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vb_cs"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vb_opa_fd"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vcom_adc2"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vb_ds"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vb_comp"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vb_pixbuf"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vin_com"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vdd_prot"}, -1, GET));
    }
}

/* acquisition */

TEST_CASE("readout", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    // PUT only command
    REQUIRE_THROWS(caller.call("readout", {}, -1, GET));
    auto det_type = det.getDetectorType().squash();
    if (det_type != defs::MYTHEN3) {
        REQUIRE_THROWS(caller.call("readout", {}, -1, GET));
    } else {
        std::ostringstream oss;
        caller.call("readout", {}, -1, PUT, oss);
        REQUIRE(oss.str() == "readout successful\n");
    }
}

/* Mythen3 Specific */

TEST_CASE("counters", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::MYTHEN3) {
        REQUIRE_THROWS(caller.call("counters", {}, -1, PUT));
        REQUIRE_THROWS(caller.call("counters", {"3"}, -1, GET));
        REQUIRE_THROWS(caller.call("counters", {"0", "-1"}, -1, GET));
        REQUIRE_THROWS(caller.call("counters", {"0", "1", "1"}, -1, GET));

        auto mask = det.getCounterMask({0}).squash(-1);
        std::vector<std::string> list_str;
        for (int i = 0; i < 32; ++i) {
            if (mask & (1 << i)) {
                list_str.push_back(std::to_string(i));
            }
        }
        std::ostringstream oss_set, oss_set2, oss_set3, oss_get;
        caller.call("counters", {"0", "2", "1"}, -1, PUT, oss_set);
        REQUIRE(oss_set.str() == "counters [0, 2, 1]\n");
        caller.call("counters", {"0", "2"}, -1, PUT, oss_set2);
        REQUIRE(oss_set2.str() == "counters [0, 2]\n");
        // put back old value
        caller.call("counters", list_str, -1, PUT, oss_set3);
        REQUIRE(oss_set3.str() == "counters " + ToString(list_str) + "\n");
        caller.call("counters", {}, -1, GET, oss_get);
        REQUIRE(oss_get.str() == "counters " + ToString(list_str) + "\n");
    } else {
        REQUIRE_THROWS(caller.call("counters", {}, -1, GET));
    }
}

TEST_CASE("gates", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::MYTHEN3) {
        auto prev_val = det.getNumberOfGates();
        {
            std::ostringstream oss;
            caller.call("gates", {"1000"}, -1, PUT, oss);
            REQUIRE(oss.str() == "gates 1000\n");
        }
        {
            std::ostringstream oss;
            caller.call("gates", {}, -1, GET, oss);
            REQUIRE(oss.str() == "gates 1000\n");
        }
        {
            std::ostringstream oss;
            caller.call("gates", {"1"}, -1, PUT, oss);
            REQUIRE(oss.str() == "gates 1\n");
        }
        REQUIRE_THROWS(caller.call("gates", {"0"}, -1, PUT));
        for (int i = 0; i != det.size(); ++i) {
            det.setNumberOfGates(prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(caller.call("gates", {}, -1, GET));
    }
}

TEST_CASE("exptime1", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::MYTHEN3) {
        auto prev_val = det.getExptime(0);
        {
            std::ostringstream oss;
            caller.call("exptime1", {"1.25s"}, -1, PUT, oss);
            REQUIRE(oss.str() == "exptime1 1.25s\n");
        }
        {
            std::ostringstream oss;
            caller.call("exptime1", {}, -1, GET, oss);
            REQUIRE(oss.str() == "exptime1 1.25s\n");
        }
        {
            std::ostringstream oss;
            caller.call("exptime1", {"0"}, -1, PUT, oss);
            REQUIRE(oss.str() == "exptime1 0\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setExptime(0, prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(caller.call("exptime1", {}, -1, GET));
    }
}

TEST_CASE("exptime2", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::MYTHEN3) {
        auto prev_val = det.getExptime(1);
        {
            std::ostringstream oss;
            caller.call("exptime2", {"1.25s"}, -1, PUT, oss);
            REQUIRE(oss.str() == "exptime2 1.25s\n");
        }
        {
            std::ostringstream oss;
            caller.call("exptime2", {}, -1, GET, oss);
            REQUIRE(oss.str() == "exptime2 1.25s\n");
        }
        {
            std::ostringstream oss;
            caller.call("exptime2", {"0"}, -1, PUT, oss);
            REQUIRE(oss.str() == "exptime2 0\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setExptime(1, prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(caller.call("exptime2", {}, -1, GET));
    }
}

TEST_CASE("exptime3", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::MYTHEN3) {
        auto prev_val = det.getExptime(2);
        {
            std::ostringstream oss;
            caller.call("exptime3", {"1.25s"}, -1, PUT, oss);
            REQUIRE(oss.str() == "exptime3 1.25s\n");
        }
        {
            std::ostringstream oss;
            caller.call("exptime3", {}, -1, GET, oss);
            REQUIRE(oss.str() == "exptime3 1.25s\n");
        }
        {
            std::ostringstream oss;
            caller.call("exptime3", {"0"}, -1, PUT, oss);
            REQUIRE(oss.str() == "exptime3 0\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setExptime(2, prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(caller.call("exptime3", {}, -1, GET));
    }
}

TEST_CASE("gatedelay", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::MYTHEN3) {
        auto prev_val = det.getExptimeForAllGates().tsquash(
            "inconsistent gatedelay to test");
        if (prev_val[0] != prev_val[1] || prev_val[1] != prev_val[2]) {
            throw RuntimeError("inconsistent gatedelay for all gates");
        }
        {
            std::ostringstream oss;
            caller.call("gatedelay", {"0.05"}, -1, PUT, oss);
            REQUIRE(oss.str() == "gatedelay 0.05\n");
        }
        if (det_type != defs::MYTHEN3) {
            std::ostringstream oss;
            caller.call("gatedelay", {}, -1, GET, oss);
            REQUIRE(oss.str() == "gatedelay 50ms\n");
        }
        {
            std::ostringstream oss;
            caller.call("gatedelay", {"1s"}, -1, PUT, oss);
            REQUIRE(oss.str() == "gatedelay 1s\n");
        }
        {
            std::ostringstream oss;
            caller.call("gatedelay", {"0"}, -1, PUT, oss);
            REQUIRE(oss.str() == "gatedelay 0\n");
        }
        det.setGateDelay(-1, prev_val[0]);
    } else {
        REQUIRE_THROWS(caller.call("gatedelay", {}, -1, GET));
    }
}

TEST_CASE("gatedelay1", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::MYTHEN3) {
        auto prev_val = det.getGateDelay(0);
        {
            std::ostringstream oss;
            caller.call("gatedelay1", {"1.25s"}, -1, PUT, oss);
            REQUIRE(oss.str() == "gatedelay1 1.25s\n");
        }
        {
            std::ostringstream oss;
            caller.call("gatedelay1", {}, -1, GET, oss);
            REQUIRE(oss.str() == "gatedelay1 1.25s\n");
        }
        {
            std::ostringstream oss;
            caller.call("gatedelay1", {"0"}, -1, PUT, oss);
            REQUIRE(oss.str() == "gatedelay1 0\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setGateDelay(0, prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(caller.call("gatedelay1", {}, -1, GET));
    }
}

TEST_CASE("gatedelay2", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::MYTHEN3) {
        auto prev_val = det.getGateDelay(1);
        {
            std::ostringstream oss;
            caller.call("gatedelay2", {"1.25s"}, -1, PUT, oss);
            REQUIRE(oss.str() == "gatedelay2 1.25s\n");
        }
        {
            std::ostringstream oss;
            caller.call("gatedelay2", {}, -1, GET, oss);
            REQUIRE(oss.str() == "gatedelay2 1.25s\n");
        }
        {
            std::ostringstream oss;
            caller.call("gatedelay2", {"0"}, -1, PUT, oss);
            REQUIRE(oss.str() == "gatedelay2 0\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setGateDelay(1, prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(caller.call("gatedelay2", {}, -1, GET));
    }
}

TEST_CASE("gatedelay3", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::MYTHEN3) {
        auto prev_val = det.getGateDelay(2);
        {
            std::ostringstream oss;
            caller.call("gatedelay3", {"1.25s"}, -1, PUT, oss);
            REQUIRE(oss.str() == "gatedelay3 1.25s\n");
        }
        {
            std::ostringstream oss;
            caller.call("gatedelay3", {}, -1, GET, oss);
            REQUIRE(oss.str() == "gatedelay3 1.25s\n");
        }
        {
            std::ostringstream oss;
            caller.call("gatedelay3", {"0"}, -1, PUT, oss);
            REQUIRE(oss.str() == "gatedelay3 0\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setGateDelay(2, prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(caller.call("gatedelay3", {}, -1, GET));
    }
}

TEST_CASE("polarity", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    if (det.getDetectorType().squash() == defs::MYTHEN3) {
        auto prev_val = det.getPolarity();
        {
            std::ostringstream oss;
            caller.call("polarity", {"pos"}, -1, PUT, oss);
            REQUIRE(oss.str() == "polarity pos\n");
        }
        {
            std::ostringstream oss;
            caller.call("polarity", {"neg"}, -1, PUT, oss);
            REQUIRE(oss.str() == "polarity neg\n");
        }
        {
            std::ostringstream oss;
            caller.call("polarity", {}, -1, GET, oss);
            REQUIRE(oss.str() == "polarity neg\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setPolarity(prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(caller.call("polarity", {}, -1, GET));
    }
}

TEST_CASE("interpolation", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    if (det.getDetectorType().squash() == defs::MYTHEN3) {
        auto prev_interpolation = det.getInterpolation();
        auto prev_mask = det.getCounterMask();
        auto prev_vth3DacVal = det.getDAC(defs::VTH3, 0, {});

        int disabledDacValue = 2800;
        auto fixedVth3DacVal = 1000;
        det.setDAC(defs::VTH3, fixedVth3DacVal, 0, {});
        // mask with counter 3 disabled and enabled(to test vth3)
        uint32_t fixedMask[2] = {0x2, 0x4};
        for (int i = 0; i != 2; ++i) {
            det.setCounterMask(fixedMask[i]);
            {
                std::ostringstream oss;
                caller.call("interpolation", {"1"}, -1, PUT, oss);
                REQUIRE(oss.str() == "interpolation 1\n");
                REQUIRE(det.getCounterMask().tsquash(
                            "inconsistent counter mask") == 7);
                REQUIRE(det.getDAC(defs::VTH3, 0, {0})
                            .tsquash("inconsistent vth3 dac value") ==
                        disabledDacValue);
            }
            {
                std::ostringstream oss;
                caller.call("interpolation", {"0"}, -1, PUT, oss);
                REQUIRE(oss.str() == "interpolation 0\n");
                REQUIRE(det.getCounterMask().tsquash(
                            "inconsistent counter mask") == fixedMask[i]);
                int expectedVth3DacVal =
                    (fixedMask[i] & 0x4 ? fixedVth3DacVal : disabledDacValue);
                REQUIRE(det.getDAC(defs::VTH3, 0, {0})
                            .tsquash("inconsistent vth3 dac value") ==
                        expectedVth3DacVal);
            }
        }

        {
            std::ostringstream oss;
            caller.call("interpolation", {}, -1, GET, oss);
            REQUIRE(oss.str() == "interpolation 0\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setCounterMask(prev_mask[i], {i});
            det.setInterpolation(prev_interpolation[i], {i});
            det.setDAC(defs::VTH3, prev_vth3DacVal[i], 0, {i});
        }
    } else {
        REQUIRE_THROWS(caller.call("interpolation", {}, -1, GET));
    }
}

TEST_CASE("pumpprobe", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    if (det.getDetectorType().squash() == defs::MYTHEN3) {
        auto prev_val = det.getPumpProbe();
        auto prev_interpolation = det.getInterpolation();
        auto prev_mask = det.getCounterMask();
        auto prev_vth1DacVal = det.getDAC(defs::VTH1, 0, {});
        auto prev_vth2DacVal = det.getDAC(defs::VTH2, 0, {});
        auto prev_vth3DacVal = det.getDAC(defs::VTH3, 0, {});

        int disabledDacValue = 2800;
        auto fixedVthDacVal = 1000;
        det.setDAC(defs::VTH1, fixedVthDacVal, 0, {});
        det.setDAC(defs::VTH2, fixedVthDacVal, 0, {});
        det.setDAC(defs::VTH3, fixedVthDacVal, 0, {});
        // mask with counter 2 disabled and enabled(to test vth2)
        uint32_t fixedMask[2] = {0x4, 0x3};
        for (int i = 0; i != 2; ++i) {
            std::cout << "i:" << i << std::endl;
            det.setCounterMask(fixedMask[i]);
            {
                // pump probe
                std::ostringstream oss;
                caller.call("pumpprobe", {"1"}, -1, PUT, oss);
                REQUIRE(oss.str() == "pumpprobe 1\n");
                REQUIRE(det.getDAC(defs::VTH1, 0, {0})
                            .tsquash("inconsistent vth2 dac value") ==
                        disabledDacValue);
                REQUIRE(det.getDAC(defs::VTH2, 0, {0})
                            .tsquash("inconsistent vth2 dac value") ==
                        fixedVthDacVal);
                REQUIRE(det.getDAC(defs::VTH3, 0, {0})
                            .tsquash("inconsistent vth2 dac value") ==
                        disabledDacValue);
            }
            // interpolation and pump probe
            REQUIRE_THROWS(caller.call("interpolation", {"1"}, -1, PUT));
            {
                // none
                std::ostringstream oss;
                caller.call("pumpprobe", {"0"}, -1, PUT, oss);
                REQUIRE(oss.str() == "pumpprobe 0\n");
                REQUIRE(det.getCounterMask().tsquash(
                            "inconsistent counter mask") == fixedMask[i]);
                REQUIRE(
                    det.getDAC(defs::VTH1, 0, {0})
                        .tsquash("inconsistent vth1 dac value") ==
                    (fixedMask[i] & 0x1 ? fixedVthDacVal : disabledDacValue));
                REQUIRE(
                    det.getDAC(defs::VTH2, 0, {0})
                        .tsquash("inconsistent vth2 dac value") ==
                    (fixedMask[i] & 0x2 ? fixedVthDacVal : disabledDacValue));
                REQUIRE(
                    det.getDAC(defs::VTH3, 0, {0})
                        .tsquash("inconsistent vth3 dac value") ==
                    (fixedMask[i] & 0x4 ? fixedVthDacVal : disabledDacValue));
            }
        }
        {
            std::ostringstream oss;
            caller.call("pumpprobe", {}, -1, GET, oss);
            REQUIRE(oss.str() == "pumpprobe 0\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setCounterMask(prev_mask[i], {i});
            det.setPumpProbe(prev_val[i], {i});
            det.setInterpolation(prev_interpolation[i], {i});
            det.setDAC(defs::VTH1, prev_vth1DacVal[i], 0, {i});
            det.setDAC(defs::VTH2, prev_vth2DacVal[i], 0, {i});
            det.setDAC(defs::VTH3, prev_vth3DacVal[i], 0, {i});
        }
    } else {
        REQUIRE_THROWS(caller.call("pumpprobe", {}, -1, GET));
    }
}

TEST_CASE("apulse", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    if (det.getDetectorType().squash() == defs::MYTHEN3) {
        auto prev_val = det.getAnalogPulsing();
        {
            std::ostringstream oss;
            caller.call("apulse", {"1"}, -1, PUT, oss);
            REQUIRE(oss.str() == "apulse 1\n");
        }
        {
            std::ostringstream oss;
            caller.call("apulse", {"0"}, -1, PUT, oss);
            REQUIRE(oss.str() == "apulse 0\n");
        }
        {
            std::ostringstream oss;
            caller.call("apulse", {}, -1, GET, oss);
            REQUIRE(oss.str() == "apulse 0\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setAnalogPulsing(prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(caller.call("apulse", {}, -1, GET));
    }
}

TEST_CASE("dpulse", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    if (det.getDetectorType().squash() == defs::MYTHEN3) {
        auto prev_val = det.getDigitalPulsing();
        {
            std::ostringstream oss;
            caller.call("dpulse", {"1"}, -1, PUT, oss);
            REQUIRE(oss.str() == "dpulse 1\n");
        }
        {
            std::ostringstream oss;
            caller.call("dpulse", {"0"}, -1, PUT, oss);
            REQUIRE(oss.str() == "dpulse 0\n");
        }
        {
            std::ostringstream oss;
            caller.call("dpulse", {}, -1, GET, oss);
            REQUIRE(oss.str() == "dpulse 0\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setDigitalPulsing(prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(caller.call("dpulse", {}, -1, GET));
    }
}

} // namespace sls
