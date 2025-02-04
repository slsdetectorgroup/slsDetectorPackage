// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "Caller.h"
#include "catch.hpp"
#include "sls/Detector.h"
#include "sls/sls_detector_defs.h"
#include <array>
#include <sstream>
#include <thread>

#include "sls/versionAPI.h"
#include "test-Caller-global.h"
#include "tests/globals.h"

namespace sls {

using test::GET;
using test::PUT;

/** temperature */

TEST_CASE("temp_fpgaext", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::EIGER) {
        REQUIRE_NOTHROW(caller.call("temp_fpgaext", {}, -1, GET));
        std::ostringstream oss;
        REQUIRE_NOTHROW(caller.call("temp_fpgaext", {}, 0, GET, oss));
        std::string s = (oss.str()).erase(0, strlen("temp_fpgaext "));
        REQUIRE(std::stoi(s) != -1);
    } else {
        REQUIRE_THROWS(caller.call("temp_fpgaext", {}, -1, GET));
    }
}

TEST_CASE("temp_10ge", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::EIGER) {
        REQUIRE_NOTHROW(caller.call("temp_10ge", {}, -1, GET));
        std::ostringstream oss;
        REQUIRE_NOTHROW(caller.call("temp_10ge", {}, 0, GET, oss));
        std::string s = (oss.str()).erase(0, strlen("temp_10ge "));
        REQUIRE(std::stoi(s) != -1);
    } else {
        REQUIRE_THROWS(caller.call("temp_10ge", {}, -1, GET));
    }
}

TEST_CASE("temp_dcdc", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::EIGER) {
        REQUIRE_NOTHROW(caller.call("temp_dcdc", {}, -1, GET));
        std::ostringstream oss;
        REQUIRE_NOTHROW(caller.call("temp_dcdc", {}, 0, GET, oss));
        std::string s = (oss.str()).erase(0, strlen("temp_dcdc "));
        REQUIRE(std::stoi(s) != -1);
    } else {
        REQUIRE_THROWS(caller.call("temp_dcdc", {}, -1, GET));
    }
}

TEST_CASE("temp_sodl", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::EIGER) {
        REQUIRE_NOTHROW(caller.call("temp_sodl", {}, -1, GET));
        std::ostringstream oss;
        REQUIRE_NOTHROW(caller.call("temp_sodl", {}, 0, GET, oss));
        std::string s = (oss.str()).erase(0, strlen("temp_sodl "));
        REQUIRE(std::stoi(s) != -1);
    } else {
        REQUIRE_THROWS(caller.call("temp_sodl", {}, -1, GET));
    }
}

TEST_CASE("temp_sodr", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::EIGER) {
        REQUIRE_NOTHROW(caller.call("temp_sodr", {}, -1, GET));
        std::ostringstream oss;
        REQUIRE_NOTHROW(caller.call("temp_sodr", {}, 0, GET, oss));
        std::string s = (oss.str()).erase(0, strlen("temp_sodr "));
        REQUIRE(std::stoi(s) != -1);
    } else {
        REQUIRE_THROWS(caller.call("temp_sodr", {}, -1, GET));
    }
}

TEST_CASE("temp_fpgafl", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::EIGER) {
        REQUIRE_NOTHROW(caller.call("temp_fpgafl", {}, -1, GET));
        std::ostringstream oss;
        REQUIRE_NOTHROW(caller.call("temp_fpgafl", {}, 0, GET, oss));
        std::string s = (oss.str()).erase(0, strlen("temp_fpgafl "));
        REQUIRE(std::stoi(s) != -1);
    } else {
        REQUIRE_THROWS(caller.call("temp_fpgafl", {}, -1, GET));
    }
}

TEST_CASE("temp_fpgafr", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::EIGER) {
        REQUIRE_NOTHROW(caller.call("temp_fpgafr", {}, -1, GET));
        std::ostringstream oss;
        REQUIRE_NOTHROW(caller.call("temp_fpgafr", {}, 0, GET, oss));
        std::string s = (oss.str()).erase(0, strlen("temp_fpgafr "));
        REQUIRE(std::stoi(s) != -1);
    } else {
        REQUIRE_THROWS(caller.call("temp_fpgafr", {}, -1, GET));
    }
}

/* dacs */

TEST_CASE("Setting and reading back EIGER dacs", "[.cmdcall][.dacs]") {
    // vsvp, vtr, vrf, vrs, vsvn, vtgstv, vcmp_ll, vcmp_lr, vcal, vcmp_rl,
    // rxb_rb, rxb_lb, vcmp_rr, vcp, vcn, vis, vthreshold
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::EIGER) {
        SECTION("vsvp") { test_dac_caller(defs::VSVP, "vsvp", 5); }
        SECTION("vtrim") { test_dac_caller(defs::VTRIM, "vtrim", 1200); }
        SECTION("vrpreamp") {
            test_dac_caller(defs::VRPREAMP, "vrpreamp", 1500);
        }
        SECTION("vrshaper") {
            test_dac_caller(defs::VRSHAPER, "vrshaper", 1510);
        }
        SECTION("vsvn") { test_dac_caller(defs::VSVN, "vsvn", 3800); }
        SECTION("vtgstv") { test_dac_caller(defs::VTGSTV, "vtgstv", 2550); }
        SECTION("vcmp_ll") { test_dac_caller(defs::VCMP_LL, "vcmp_ll", 1400); }
        SECTION("vcmp_lr") { test_dac_caller(defs::VCMP_LR, "vcmp_lr", 1400); }
        SECTION("vcal") { test_dac_caller(defs::VCAL, "vcal", 1400); }
        SECTION("vcmp_rl") { test_dac_caller(defs::VCMP_RL, "vcmp_rl", 1400); }
        SECTION("rxb_rb") { test_dac_caller(defs::RXB_RB, "rxb_rb", 1400); }
        SECTION("rxb_lb") { test_dac_caller(defs::RXB_LB, "rxb_lb", 1400); }
        SECTION("vcmp_rr") { test_dac_caller(defs::VCMP_RR, "vcmp_rr", 1400); }
        SECTION("vcp") { test_dac_caller(defs::VCP, "vcp", 1400); }
        SECTION("vcn") { test_dac_caller(defs::VCN, "vcn", 1400); }
        SECTION("vishaper") {
            test_dac_caller(defs::VISHAPER, "vishaper", 1400);
        }
        SECTION("iodelay") { test_dac_caller(defs::IO_DELAY, "iodelay", 1400); }
        SECTION("vthreshold") {
            // Read out individual vcmp to be able to reset after
            // the test is done
            auto vcmp_ll = det.getDAC(defs::VCMP_LL, false);
            auto vcmp_lr = det.getDAC(defs::VCMP_LR, false);
            auto vcmp_rl = det.getDAC(defs::VCMP_RL, false);
            auto vcmp_rr = det.getDAC(defs::VCMP_RR, false);
            auto vcp = det.getDAC(defs::VCP, false);

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

            // Reset dacs after test
            for (int i = 0; i != det.size(); ++i) {
                det.setDAC(defs::VCMP_LL, vcmp_ll[i], false, {i});
                det.setDAC(defs::VCMP_LR, vcmp_lr[i], false, {i});
                det.setDAC(defs::VCMP_RL, vcmp_rl[i], false, {i});
                det.setDAC(defs::VCMP_RR, vcmp_rr[i], false, {i});
                det.setDAC(defs::VCP, vcp[i], false, {i});
            }
        }
        // gotthard
        REQUIRE_THROWS(caller.call("vref_ds", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vcascn_pb", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vcascp_pb", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vout_cm", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vcasc_out", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vin_cm", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vref_comp", {}, -1, GET));
        REQUIRE_THROWS(caller.call("ib_test_c", {}, -1, GET));
        // mythen3
        // REQUIRE_THROWS(caller.call("vrpreamp", {}, -1, GET));
        // REQUIRE_THROWS(caller.call("vrshaper", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vrshaper_n", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vipre", {}, -1, GET));
        // REQUIRE_THROWS(caller.call("vishaper", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vdcsh", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vth1", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vth2", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vth3", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vcal_n", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vcal_p", {}, -1, GET));
        // REQUIRE_THROWS(caller.call("vtrim", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vcassh", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vcas", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vicin", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vipre_out", {}, -1, GET));
        // gotthard2
        REQUIRE_THROWS(caller.call("vref_h_adc", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vb_comp_fe", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vb_comp_adc", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vcom_cds", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vref_rstore", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vb_opa_1st", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vref_comp_fe", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vcom_adc1", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vref_l_adc", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vref_cds", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vb_cs", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vb_opa_fd", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vcom_adc2", {}, -1, GET));
        // jungfrau
        REQUIRE_THROWS(caller.call("vb_comp", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vdd_prot", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vin_com", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vref_prech", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vb_pixbuf", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vb_ds", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vref_ds", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vref_comp", {}, -1, GET));
    }
}

/* acquisition */

/* Network Configuration (Detector<->Receiver) */

TEST_CASE("txdelay_left", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::EIGER) {
        auto prev_val = det.getTransmissionDelayLeft();
        {
            std::ostringstream oss1, oss2;
            caller.call("txdelay_left", {"5000"}, -1, PUT, oss1);
            REQUIRE(oss1.str() == "txdelay_left 5000\n");
            caller.call("txdelay_left", {}, -1, GET, oss2);
            REQUIRE(oss2.str() == "txdelay_left 5000\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setTransmissionDelayLeft(prev_val[i]);
        }
    } else {
        REQUIRE_THROWS(caller.call("txdelay_left", {}, -1, GET));
    }
}

TEST_CASE("txdelay_right", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::EIGER) {
        auto prev_val = det.getTransmissionDelayRight();
        {
            std::ostringstream oss1, oss2;
            caller.call("txdelay_right", {"5000"}, -1, PUT, oss1);
            REQUIRE(oss1.str() == "txdelay_right 5000\n");
            caller.call("txdelay_right", {}, -1, GET, oss2);
            REQUIRE(oss2.str() == "txdelay_right 5000\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setTransmissionDelayRight(prev_val[i]);
        }
    } else {
        REQUIRE_THROWS(caller.call("txdelay_right", {}, -1, GET));
    }
}

/* Eiger Specific */

TEST_CASE("subexptime", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);

    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::EIGER) {
        auto time = det.getSubExptime();
        std::ostringstream oss1, oss2;
        caller.call("subexptime", {"2.5us"}, -1, PUT, oss1);
        REQUIRE(oss1.str() == "subexptime 2.5us\n");
        caller.call("subexptime", {}, -1, GET, oss2);
        REQUIRE(oss2.str() == "subexptime 2.5us\n");
        for (int i = 0; i != det.size(); ++i) {
            det.setSubExptime(time[i], {i});
        }
    } else {
        REQUIRE_THROWS(caller.call("subexptime", {}, -1, GET));
        REQUIRE_THROWS(caller.call("subexptime", {"2.13"}, -1, PUT));
    }
}

TEST_CASE("subdeadtime", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);

    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::EIGER) {
        auto time = det.getSubDeadTime();
        std::ostringstream oss1, oss2;
        caller.call("subdeadtime", {"500us"}, -1, PUT, oss1);
        REQUIRE(oss1.str() == "subdeadtime 500us\n");
        caller.call("subdeadtime", {}, -1, GET, oss2);
        REQUIRE(oss2.str() == "subdeadtime 500us\n");
        for (int i = 0; i != det.size(); ++i) {
            det.setSubDeadTime(time[i], {i});
        }
    } else {
        REQUIRE_THROWS(caller.call("subdeadtime", {}, -1, GET));
        REQUIRE_THROWS(caller.call("subdeadtime", {"2.13"}, -1, PUT));
    }
}

TEST_CASE("overflow", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::EIGER) {
        auto previous = det.getOverFlowMode();
        std::ostringstream oss1, oss2, oss3;
        caller.call("overflow", {"1"}, -1, PUT, oss1);
        REQUIRE(oss1.str() == "overflow 1\n");
        caller.call("overflow", {}, -1, GET, oss2);
        REQUIRE(oss2.str() == "overflow 1\n");
        caller.call("overflow", {"0"}, -1, PUT, oss3);
        REQUIRE(oss3.str() == "overflow 0\n");
        for (int i = 0; i != det.size(); ++i) {
            det.setOverFlowMode(previous[i], {i});
        }
    } else {
        REQUIRE_THROWS(caller.call("overflow", {}, -1, GET));
    }
}

TEST_CASE("ratecorr", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::EIGER) {
        auto prev_dr = det.getDynamicRange().tsquash("inconsistent dr to test");
        auto prev_tau = det.getRateCorrection();
        det.setDynamicRange(16);
        {
            std::ostringstream oss;
            caller.call("ratecorr", {"120"}, -1, PUT, oss);
            REQUIRE(oss.str() == "ratecorr 120ns\n");
        }
        {
            std::ostringstream oss;
            caller.call("ratecorr", {}, -1, GET, oss);
            REQUIRE(oss.str() == "ratecorr 120ns\n");
        }
        // may fail if default settings not loaded
        // REQUIRE_NOTHROW(caller.call("ratecorr", {"-1"}, -1, PUT));
        {
            std::ostringstream oss;
            caller.call("ratecorr", {"0"}, -1, PUT, oss);
            REQUIRE(oss.str() == "ratecorr 0ns\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setRateCorrection(prev_tau[i], {i});
        }
        det.setDynamicRange(prev_dr);
    } else {
        REQUIRE_THROWS(caller.call("ratecorr", {}, -1, GET));
    }
}

TEST_CASE("interruptsubframe", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::EIGER) {
        auto prev_val = det.getInterruptSubframe();

        std::ostringstream oss1, oss2, oss3;
        caller.call("interruptsubframe", {"1"}, -1, PUT, oss1);
        REQUIRE(oss1.str() == "interruptsubframe 1\n");
        caller.call("interruptsubframe", {}, -1, GET, oss2);
        REQUIRE(oss2.str() == "interruptsubframe 1\n");
        caller.call("interruptsubframe", {"0"}, -1, PUT, oss3);
        REQUIRE(oss3.str() == "interruptsubframe 0\n");
        for (int i = 0; i != det.size(); ++i) {
            det.setInterruptSubframe(prev_val[i], {i});
        }

    } else {
        REQUIRE_THROWS(caller.call("interruptsubframe", {}, -1, GET));
        REQUIRE_THROWS(caller.call("interruptsubframe", {"1"}, -1, PUT));
    }
}

TEST_CASE("measuredperiod", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::EIGER) {
        auto prev_frames = det.getNumberOfFrames().tsquash(
            "inconsistent number of frames to test");
        auto prev_timing =
            det.getTimingMode().tsquash("inconsistent timing mode to test");
        auto prev_period = det.getPeriod();
        det.setNumberOfFrames(2);
        det.setPeriod(std::chrono::seconds(1));
        det.setTimingMode(defs::AUTO_TIMING);
        det.startDetector();
        std::this_thread::sleep_for(std::chrono::seconds(3));
        std::ostringstream oss;
        caller.call("measuredperiod", {}, -1, GET, oss);
        std::string st = oss.str();
        std::string s;
        if (st.find('[') != std::string::npos) {
            s = st.erase(0, strlen("measuredperiod ["));
        } else {
            s = st.erase(0, strlen("measuredperiod "));
        }
        double val = std::stod(s);
        // REQUIRE(val >= 1.0);
        REQUIRE(val < 2.0);
        for (int i = 0; i != det.size(); ++i) {
            det.setPeriod(prev_period[i], {i});
        }
        det.setNumberOfFrames(prev_frames);
        det.setTimingMode(prev_timing);
    } else {
        REQUIRE_THROWS(caller.call("measuredperiod", {}, -1, GET));
    }
}

TEST_CASE("measuredsubperiod", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::EIGER) {
        auto prev_frames = det.getNumberOfFrames().tsquash(
            "inconsistent number of frames to test");
        auto prev_timing =
            det.getTimingMode().tsquash("inconsistent timing mode to test");
        auto prev_period = det.getPeriod();
        auto prev_dr = det.getDynamicRange().tsquash("inconsistent dr to test");
        det.setNumberOfFrames(1);
        det.setPeriod(std::chrono::seconds(1));
        det.setTimingMode(defs::AUTO_TIMING);
        det.setDynamicRange(32);
        det.startDetector();
        std::this_thread::sleep_for(std::chrono::seconds(3));
        std::ostringstream oss;
        caller.call("measuredsubperiod", {}, -1, GET, oss);
        std::string st = oss.str();
        std::string s;
        if (st.find('[') != std::string::npos) {
            s = st.erase(0, strlen("measuredsubperiod ["));
        } else {
            s = st.erase(0, strlen("measuredsubperiod "));
        }
        double val = std::stod(s);
        REQUIRE(val >= 0);
        REQUIRE(val < 1000);
        for (int i = 0; i != det.size(); ++i) {
            det.setPeriod(prev_period[i], {i});
        }
        det.setNumberOfFrames(prev_frames);
        det.setTimingMode(prev_timing);
        det.setDynamicRange(prev_dr);
    } else {
        REQUIRE_THROWS(caller.call("measuredsubperiod", {}, -1, GET));
    }
}

TEST_CASE("activate", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::EIGER) {
        auto prev_val = det.getActive();
        {
            std::ostringstream oss;
            caller.call("activate", {"1"}, -1, PUT, oss);
            REQUIRE(oss.str() == "activate 1\n");
        }
        {
            std::ostringstream oss;
            caller.call("activate", {}, -1, GET, oss);
            REQUIRE(oss.str() == "activate 1\n");
        }
        {
            std::ostringstream oss;
            caller.call("activate", {"0"}, -1, PUT, oss);
            REQUIRE(oss.str() == "activate 0\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setActive(prev_val[i], {i});
        }

    } else {
        REQUIRE_THROWS(caller.call("activate", {}, -1, GET));
        REQUIRE_THROWS(caller.call("activate", {"1"}, -1, PUT));
    }
}

TEST_CASE("partialreset", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::EIGER) {
        auto prev_val = det.getPartialReset();
        std::ostringstream oss1, oss2, oss3;
        caller.call("partialreset", {"1"}, -1, PUT, oss1);
        REQUIRE(oss1.str() == "partialreset 1\n");
        caller.call("partialreset", {}, -1, GET, oss2);
        REQUIRE(oss2.str() == "partialreset 1\n");
        caller.call("partialreset", {"0"}, -1, PUT, oss3);
        REQUIRE(oss3.str() == "partialreset 0\n");
        for (int i = 0; i != det.size(); ++i) {
            det.setPartialReset(prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(caller.call("partialreset", {}, -1, GET));
        REQUIRE_THROWS(caller.call("partialreset", {"1"}, -1, PUT));
    }
}

TEST_CASE("pulse", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::EIGER) {
        REQUIRE_THROWS(caller.call("pulse", {}, -1, GET));
        std::ostringstream oss;
        caller.call("pulse", {"1", "1", "5"}, -1, PUT, oss);
        REQUIRE(oss.str() == "pulse [1, 1, 5]\n");
    } else {
        REQUIRE_THROWS(caller.call("pulse", {}, -1, GET));
        REQUIRE_THROWS(caller.call("pulse", {"1", "1", "5"}, -1, PUT));
    }
}

TEST_CASE("pulsenmove", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::EIGER) {
        REQUIRE_THROWS(caller.call("pulsenmove", {}, -1, GET));
        std::ostringstream oss;
        caller.call("pulsenmove", {"1", "1", "5"}, -1, PUT, oss);
        REQUIRE(oss.str() == "pulsenmove [1, 1, 5]\n");
    } else {
        REQUIRE_THROWS(caller.call("pulsenmove", {}, -1, GET));
        REQUIRE_THROWS(caller.call("pulsenmove", {"1", "1", "5"}, -1, PUT));
    }
}

TEST_CASE("pulsechip", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::EIGER) {
        REQUIRE_THROWS(caller.call("pulsechip", {}, -1, GET));
        std::ostringstream oss;
        caller.call("pulsechip", {"1"}, -1, PUT, oss);
        REQUIRE(oss.str() == "pulsechip 1\n");
    } else {
        REQUIRE_THROWS(caller.call("pulsechip", {}, -1, GET));
        REQUIRE_THROWS(caller.call("pulsechip", {"1"}, -1, PUT));
    }
}

TEST_CASE("quad", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::EIGER) {
        auto prev_val = det.getQuad().tsquash("inconsistent quad to test");
        // Quad only works with a single half module EIGER
        std::ostringstream oss;
        caller.call("quad", {}, -1, GET, oss);
        REQUIRE(oss.str() == "quad 0\n");
        det.setQuad(prev_val);
    } else {
        REQUIRE_THROWS(caller.call("quad", {}, -1, GET));
    }
}

TEST_CASE("datastream", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::EIGER) {
        auto prev_val_left = det.getDataStream(defs::LEFT);
        auto prev_val_right = det.getDataStream(defs::RIGHT);
        // no "left" or "right"
        REQUIRE_THROWS(caller.call("datastream", {"1"}, -1, PUT));
        {
            std::ostringstream oss;
            caller.call("datastream", {"left", "0"}, -1, PUT, oss);
            REQUIRE(oss.str() == "datastream [left, 0]\n");
        }
        {
            std::ostringstream oss;
            caller.call("datastream", {"right", "0"}, -1, PUT, oss);
            REQUIRE(oss.str() == "datastream [right, 0]\n");
        }
        {
            std::ostringstream oss;
            caller.call("datastream", {"left", "1"}, -1, PUT, oss);
            REQUIRE(oss.str() == "datastream [left, 1]\n");
        }
        {
            std::ostringstream oss;
            caller.call("datastream", {"right", "1"}, -1, PUT, oss);
            REQUIRE(oss.str() == "datastream [right, 1]\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setDataStream(defs::LEFT, prev_val_left[i], {i});
            det.setDataStream(defs::RIGHT, prev_val_right[i], {i});
        }
    } else {
        REQUIRE_THROWS(caller.call("datastream", {}, -1, GET));
        REQUIRE_THROWS(caller.call("datastream", {"1"}, -1, PUT));
        REQUIRE_THROWS(caller.call("datastream", {"left", "1"}, -1, PUT));
    }
}

TEST_CASE("top", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::EIGER) {
        auto prev_val = det.getTop();
        int numModulesTested = 1;
        if (det.size() > 1) {
            numModulesTested = 2;
        }
        for (int i = 0; i != numModulesTested; ++i) {
            std::ostringstream oss1, oss2, oss3;
            caller.call("top", {"1"}, i, PUT, oss1);
            REQUIRE(oss1.str() == "top 1\n");
            caller.call("top", {}, i, GET, oss2);
            REQUIRE(oss2.str() == "top 1\n");
            caller.call("top", {"0"}, i, PUT, oss3);
            REQUIRE(oss3.str() == "top 0\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setTop(prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(caller.call("top", {}, -1, GET));
        REQUIRE_THROWS(caller.call("top", {"1"}, -1, PUT));
    }
}

} // namespace sls
