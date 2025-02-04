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

// time specific measurements for gotthard2
TEST_CASE("timegotthard2", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::GOTTHARD2) {
        // exptime
        auto prev_val = det.getExptime();
        {
            std::ostringstream oss;
            caller.call("exptime", {"220ns"}, -1, PUT, oss);
            REQUIRE(oss.str() == "exptime 220ns\n");
        }
        {
            std::ostringstream oss;
            caller.call("exptime", {}, -1, GET, oss);
            REQUIRE(oss.str() == "exptime 221ns\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setExptime(prev_val[i], {i});
        }
        // burst period
        prev_val = det.getBurstPeriod();
        {
            std::ostringstream oss;
            caller.call("burstperiod", {"220ns"}, -1, PUT, oss);
            REQUIRE(oss.str() == "burstperiod 220ns\n");
        }
        {
            std::ostringstream oss;
            caller.call("burstperiod", {}, -1, GET, oss);
            REQUIRE(oss.str() == "burstperiod 221ns\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setBurstPeriod(prev_val[i], {i});
        }
        // delay after trigger
        prev_val = det.getDelayAfterTrigger();
        {
            std::ostringstream oss;
            caller.call("delay", {"220ns"}, -1, PUT, oss);
            REQUIRE(oss.str() == "delay 220ns\n");
        }
        {
            std::ostringstream oss;
            caller.call("delay", {}, -1, GET, oss);
            REQUIRE(oss.str() == "delay 221ns\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setDelayAfterTrigger(prev_val[i], {i});
        }
        // period in burst mode
        auto burst_prev_val = det.getBurstMode();
        det.setBurstMode(defs::BURST_INTERNAL, {});
        prev_val = det.getPeriod();
        {
            std::ostringstream oss;
            caller.call("period", {"220ns"}, -1, PUT, oss);
            REQUIRE(oss.str() == "period 220ns\n");
        }
        {
            std::ostringstream oss;
            caller.call("period", {}, -1, GET, oss);
            REQUIRE(oss.str() == "period 221ns\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setPeriod(prev_val[i], {i});
        }
        // period in continuous mode
        det.setBurstMode(defs::CONTINUOUS_EXTERNAL, {});
        prev_val = det.getPeriod();
        {
            std::ostringstream oss;
            caller.call("period", {"220ns"}, -1, PUT, oss);
            REQUIRE(oss.str() == "period 220ns\n");
        }
        {
            std::ostringstream oss;
            caller.call("period", {}, -1, GET, oss);
            REQUIRE(oss.str() == "period 221ns\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setPeriod(prev_val[i], {i});
            det.setBurstMode(burst_prev_val[i], {i});
        }
    }
}
/* dacs */

TEST_CASE("Setting and reading back GOTTHARD2 dacs", "[.cmdcall][.dacs]") {
    // vref_h_adc,   vb_comp_fe, vb_comp_adc,  vcom_cds,
    // vref_restore, vb_opa_1st, vref_comp_fe, vcom_adc1,
    // vref_prech,   vref_l_adc, vref_cds,     vb_cs,
    // vb_opa_fd,    vcom_adc2

    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::GOTTHARD2) {
        SECTION("vref_h_adc") {
            test_dac_caller(defs::VREF_H_ADC, "vref_h_adc", 2099);
        }
        SECTION("vb_comp_fe") {
            test_dac_caller(defs::VB_COMP_FE, "vb_comp_fe", 0);
        }
        SECTION("vb_comp_adc") {
            test_dac_caller(defs::VB_COMP_ADC, "vb_comp_adc", 0);
        }
        SECTION("vcom_cds") {
            test_dac_caller(defs::VCOM_CDS, "vcom_cds", 1400);
        }
        SECTION("vref_rstore") {
            test_dac_caller(defs::VREF_RSTORE, "vref_rstore", 640);
        }
        SECTION("vb_opa_1st") {
            test_dac_caller(defs::VB_OPA_1ST, "vb_opa_1st", 0);
        }
        SECTION("vref_comp_fe") {
            test_dac_caller(defs::VREF_COMP_FE, "vref_comp_fe", 0);
        }
        SECTION("vcom_adc1") {
            test_dac_caller(defs::VCOM_ADC1, "vcom_adc1", 1400);
        }
        SECTION("vref_prech") {
            test_dac_caller(defs::VREF_PRECH, "vref_prech", 1720);
        }
        SECTION("vref_l_adc") {
            test_dac_caller(defs::VREF_L_ADC, "vref_l_adc", 700);
        }
        SECTION("vref_cds") {
            test_dac_caller(defs::VREF_CDS, "vref_cds", 1200);
        }
        SECTION("vb_cs") { test_dac_caller(defs::VB_CS, "vb_cs", 2799); }
        SECTION("vb_opa_fd") {
            test_dac_caller(defs::VB_OPA_FD, "vb_opa_fd", 0);
        }
        SECTION("vcom_adc2") {
            test_dac_caller(defs::VCOM_ADC2, "vcom_adc2", 1400);
        }
        // eiger
        REQUIRE_THROWS(caller.call("dac", {"vthreshold"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vsvp"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vsvn"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vtrim"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vrpreamp"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vrshaper"}, -1, GET));
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
        REQUIRE_THROWS(caller.call("dac", {"vishaper"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"iodelay"}, -1, GET));
        // gotthard
        REQUIRE_THROWS(caller.call("dac", {"vref_ds"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vcascn_pb"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vcascp_pb"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vout_cm"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vcasc_out"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vin_cm"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vref_comp"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"ib_test_c"}, -1, GET));
        // jungfrau
        REQUIRE_THROWS(caller.call("dac", {"vb_comp"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vdd_prot"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vin_com"}, -1, GET));
        // REQUIRE_THROWS(caller.call("dac", {"vref_prech"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vb_pixbuf"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vb_ds"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vref_ds"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vref_comp"}, -1, GET));
        // mythen3
        REQUIRE_THROWS(caller.call("dac", {"vrpreamp"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vrshaper"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vrshaper_n"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vipre"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vishaper"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vdcsh"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vth1"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vth2"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vth3"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vcal_n"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vcal_p"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vtrim"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vcassh"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vcas"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vicin"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vipre_out"}, -1, GET));
    }
}

/* on chip dacs */

TEST_CASE("vchip_comp_fe", "[.cmdcall][.onchipdacs]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::GOTTHARD2) {
        SECTION("vchip_comp_fe") {
            test_onchip_dac_caller(defs::VB_COMP_FE, "vchip_comp_fe", 0x137);
        }
    } else {
        REQUIRE_THROWS(caller.call("vchip_comp_fe", {}, -1, GET));
    }
}

TEST_CASE("vchip_opa_1st", "[.cmdcall][.onchipdacs]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::GOTTHARD2) {
        SECTION("vchip_opa_1st") {
            test_onchip_dac_caller(defs::VB_OPA_1ST, "vchip_opa_1st", 0x000);
        }
    } else {
        REQUIRE_THROWS(caller.call("vchip_opa_1st", {}, -1, GET));
    }
}

TEST_CASE("vchip_opa_fd", "[.cmdcall][.onchipdacs]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::GOTTHARD2) {
        SECTION("vchip_opa_fd") {
            test_onchip_dac_caller(defs::VB_OPA_FD, "vchip_opa_fd", 0x134);
        }
    } else {
        REQUIRE_THROWS(caller.call("vchip_opa_fd", {}, -1, GET));
    }
}

TEST_CASE("vchip_comp_adc", "[.cmdcall][.onchipdacs]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::GOTTHARD2) {
        SECTION("vchip_comp_adc") {
            test_onchip_dac_caller(defs::VB_COMP_ADC, "vchip_comp_adc", 0x3FF);
        }
    } else {
        REQUIRE_THROWS(caller.call("vchip_comp_adc", {}, -1, GET));
    }
}

TEST_CASE("vchip_ref_comp_fe", "[.cmdcall][.onchipdacs]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::GOTTHARD2) {
        SECTION("vchip_ref_comp_fe") {
            test_onchip_dac_caller(defs::VREF_COMP_FE, "vchip_ref_comp_fe",
                                   0x100);
        }
    } else {
        REQUIRE_THROWS(caller.call("vchip_ref_comp_fe", {}, -1, GET));
    }
}

TEST_CASE("vchip_cs", "[.cmdcall][.onchipdacs]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::GOTTHARD2) {
        SECTION("vchip_cs") {
            test_onchip_dac_caller(defs::VB_CS, "vchip_cs", 0x0D0);
        }
    } else {
        REQUIRE_THROWS(caller.call("vchip_cs", {}, -1, GET));
    }
}

/* Gotthard2 Specific */

TEST_CASE("bursts", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::GOTTHARD2) {
        auto prev_burst =
            det.getNumberOfBursts().tsquash("#bursts should be same to test");
        auto prev_trigger =
            det.getNumberOfFrames().tsquash("#frames should be same to test");
        auto prev_frames = det.getNumberOfTriggers().tsquash(
            "#triggers should be same to test");
        auto prev_timingMode = det.getTimingMode();
        auto prev_burstMode = det.getBurstMode();
        // changing continuous mode frames and bursts
        det.setBurstMode(defs::BURST_INTERNAL);
        det.setTimingMode(defs::AUTO_TIMING);
        {
            std::ostringstream oss;
            caller.call("bursts", {"3"}, -1, PUT, oss);
            REQUIRE(oss.str() == "bursts 3\n");
        }
        {
            std::ostringstream oss;
            caller.call("bursts", {}, -1, GET, oss);
            REQUIRE(oss.str() == "bursts 3\n");
        }
        REQUIRE_THROWS(caller.call("bursts", {"0"}, -1, PUT));
        // trigger mode: reg set to 1, but bursts must be same
        det.setTimingMode(defs::TRIGGER_EXPOSURE);
        {
            std::ostringstream oss;
            caller.call("bursts", {}, -1, GET, oss);
            REQUIRE(oss.str() == "bursts 3\n");
        }
        det.setTimingMode(defs::AUTO_TIMING);
        {
            std::ostringstream oss;
            caller.call("bursts", {}, -1, GET, oss);
            REQUIRE(oss.str() == "bursts 3\n");
        }
        // continuous mode: reg set to #frames,
        // but bursts should return same value
        det.setBurstMode(defs::CONTINUOUS_EXTERNAL);
        det.setNumberOfFrames(2);
        {
            std::ostringstream oss;
            caller.call("bursts", {}, -1, GET, oss);
            REQUIRE(oss.str() == "bursts 3\n");
        }
        det.setTimingMode(defs::TRIGGER_EXPOSURE);
        {
            std::ostringstream oss;
            caller.call("bursts", {}, -1, GET, oss);
            REQUIRE(oss.str() == "bursts 3\n");
        }
        det.setBurstMode(defs::BURST_INTERNAL);
        {
            std::ostringstream oss;
            caller.call("bursts", {}, -1, GET, oss);
            REQUIRE(oss.str() == "bursts 3\n");
        }
        // set to previous values
        det.setNumberOfBursts(prev_burst);
        det.setNumberOfFrames(prev_frames);
        det.setNumberOfTriggers(prev_trigger);
        for (int i = 0; i != det.size(); ++i) {
            det.setTimingMode(prev_timingMode[i], {i});
            det.setBurstMode(prev_burstMode[i], {i});
        }
    } else {
        REQUIRE_THROWS(caller.call("bursts", {}, -1, GET));
    }
}

TEST_CASE("burstperiod", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::GOTTHARD2) {
        auto previous = det.getBurstPeriod();

        std::ostringstream oss_set, oss_get;
        caller.call("burstperiod", {"30ms"}, -1, PUT, oss_set);
        REQUIRE(oss_set.str() == "burstperiod 30ms\n");
        caller.call("burstperiod", {}, -1, GET, oss_get);
        REQUIRE(oss_get.str() == "burstperiod 30ms\n");
        // Reset to previous value
        for (int i = 0; i != det.size(); ++i) {
            det.setBurstPeriod(previous[i], {i});
        }
    } else {
        REQUIRE_THROWS(caller.call("burstperiod", {}, -1, GET));
    }
}

TEST_CASE("burstsl", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::GOTTHARD2) {
        REQUIRE_NOTHROW(caller.call("burstsl", {}, -1, GET));
    } else {
        REQUIRE_THROWS(caller.call("burstsl", {}, -1, GET));
    }
}

TEST_CASE("inj_ch", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::GOTTHARD2) {
        REQUIRE_THROWS(
            caller.call("inj_ch", {"-1", "1"}, -1, PUT)); // invalid offset
        REQUIRE_THROWS(
            caller.call("inj_ch", {"0", "0"}, -1, PUT)); // invalid increment
        {
            std::ostringstream oss;
            caller.call("inj_ch", {"0", "1"}, -1, PUT, oss);
            REQUIRE(oss.str() == "inj_ch [0, 1]\n");
        }
        {
            std::ostringstream oss;
            caller.call("inj_ch", {}, -1, GET, oss);
            REQUIRE(oss.str() == "inj_ch [0, 1]\n");
        }
    } else {
        REQUIRE_THROWS(caller.call("inj_ch", {}, -1, GET));
    }
}

TEST_CASE("vetophoton", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::GOTTHARD2) {
        REQUIRE_THROWS(caller.call("vetophoton", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vetophoton", {"-1"}, -1, GET));
        REQUIRE_NOTHROW(
            caller.call("vetophoton", {"-1", "/tmp/bla.txt"}, -1, GET));
        REQUIRE_THROWS(caller.call("vetophoton", {"12", "1", "39950"}, -1,
                                   PUT)); // invalid chip index
        REQUIRE_THROWS(caller.call("vetophoton", {"-1", "0"}, -1,
                                   PUT)); // invalid photon number
        REQUIRE_THROWS(caller.call("vetophoton", {"-1", "1", "39950"}, -1,
                                   PUT)); // invald file
    } else {
        REQUIRE_THROWS(
            caller.call("vetophoton", {"-1", "/tmp/bla.txt"}, -1, GET));
    }
}

TEST_CASE("vetoref", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::GOTTHARD2) {
        REQUIRE_THROWS(caller.call("vetoref", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vetoref", {"3", "0x3ff"}, -1,
                                   PUT)); // invalid chip index
        REQUIRE_NOTHROW(caller.call("vetoref", {"1", "0x010"}, -1, PUT));
    } else {
        REQUIRE_THROWS(caller.call("vetoref", {"3", "0x0"}, -1, PUT));
    }
}

TEST_CASE("vetofile", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::GOTTHARD2) {
        REQUIRE_THROWS(caller.call("vetofile", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vetofile", {"12", "/tmp/bla.txt"}, -1,
                                   PUT)); // invalid chip index
    } else {
        REQUIRE_THROWS(caller.call("vetofile", {"-1"}, -1, GET));
    }
}

TEST_CASE("burstmode", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::GOTTHARD2) {
        auto burst = det.getBurstMode();
        auto burststr = ToString(burst);
        {
            std::ostringstream oss;
            caller.call("burstmode", {"burst_internal"}, -1, PUT, oss);
            REQUIRE(oss.str() == "burstmode burst_internal\n");
        }
        {
            std::ostringstream oss;
            caller.call("burstmode", {"cw_external"}, -1, PUT, oss);
            REQUIRE(oss.str() == "burstmode cw_external\n");
        }
        {
            std::ostringstream oss;
            caller.call("burstmode", {}, -1, GET, oss);
            REQUIRE(oss.str() == "burstmode cw_external\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setBurstMode(burst[i], {i});
        }
    } else {
        REQUIRE_THROWS(caller.call("burstmode", {}, -1, GET));
    }
}

TEST_CASE("cdsgain", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::GOTTHARD2) {
        auto prev_val = det.getCDSGain();
        {
            std::ostringstream oss;
            caller.call("cdsgain", {"1"}, -1, PUT, oss);
            REQUIRE(oss.str() == "cdsgain 1\n");
        }
        {
            std::ostringstream oss;
            caller.call("cdsgain", {"0"}, -1, PUT, oss);
            REQUIRE(oss.str() == "cdsgain 0\n");
        }
        {
            std::ostringstream oss;
            caller.call("cdsgain", {}, -1, GET, oss);
            REQUIRE(oss.str() == "cdsgain 0\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setCDSGain(prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(caller.call("cdsgain", {}, -1, GET));
    }
}

TEST_CASE("timingsource", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::GOTTHARD2) {
        auto prev_val = det.getTimingSource();
        /* { until its activated in fpga
             std::ostringstream oss;
             caller.call("timingsource", {"external"}, -1, PUT, oss);
             REQUIRE(oss.str() == "timingsource external\n");
         }*/
        {
            std::ostringstream oss;
            caller.call("timingsource", {"internal"}, -1, PUT, oss);
            REQUIRE(oss.str() == "timingsource internal\n");
        }
        {
            std::ostringstream oss;
            caller.call("timingsource", {}, -1, GET, oss);
            REQUIRE(oss.str() == "timingsource internal\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setTimingSource(prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(caller.call("timingsource", {}, -1, GET));
    }
}

TEST_CASE("veto", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::GOTTHARD2) {
        auto prev_val = det.getVeto();
        {
            std::ostringstream oss;
            caller.call("veto", {"1"}, -1, PUT, oss);
            REQUIRE(oss.str() == "veto 1\n");
        }
        {
            std::ostringstream oss;
            caller.call("veto", {"0"}, -1, PUT, oss);
            REQUIRE(oss.str() == "veto 0\n");
        }
        {
            std::ostringstream oss;
            caller.call("veto", {}, -1, GET, oss);
            REQUIRE(oss.str() == "veto 0\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setVeto(prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(caller.call("veto", {}, -1, GET));
    }
}

TEST_CASE("vetostream", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::GOTTHARD2) {
        auto prev_val =
            det.getVetoStream().tsquash("inconsistent veto stream to test");
        {
            std::ostringstream oss;
            caller.call("vetostream", {"none"}, -1, PUT, oss);
            REQUIRE(oss.str() == "vetostream none\n");
        }
        {
            std::ostringstream oss;
            caller.call("vetostream", {}, -1, GET, oss);
            REQUIRE(oss.str() == "vetostream none\n");
        }
        {
            std::ostringstream oss;
            caller.call("vetostream", {"lll"}, -1, PUT, oss);
            REQUIRE(oss.str() == "vetostream lll\n");
        }
        {
            std::ostringstream oss;
            caller.call("vetostream", {}, -1, GET, oss);
            REQUIRE(oss.str() == "vetostream lll\n");
        }
        {
            std::ostringstream oss;
            caller.call("vetostream", {"lll", "10gbe"}, -1, PUT, oss);
            REQUIRE(oss.str() == "vetostream lll, 10gbe\n");
        }
        {
            std::ostringstream oss;
            caller.call("vetostream", {}, -1, GET, oss);
            REQUIRE(oss.str() == "vetostream lll, 10gbe\n");
        }
        REQUIRE_THROWS(caller.call("vetostream", {"lll", "none"}, -1, PUT));
        det.setVetoStream(prev_val);
    } else {
        REQUIRE_THROWS(caller.call("vetostream", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vetostream", {"none"}, -1, PUT));
    }
    REQUIRE_THROWS(caller.call("vetostream", {"dfgd"}, -1, GET));
}

TEST_CASE("vetoalg", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::GOTTHARD2) {
        auto prev_val_lll =
            det.getVetoAlgorithm(defs::streamingInterface::LOW_LATENCY_LINK);
        auto prev_val_10g =
            det.getVetoAlgorithm(defs::streamingInterface::ETHERNET_10GB);
        {
            std::ostringstream oss;
            caller.call("vetoalg", {"hits", "lll"}, -1, PUT, oss);
            REQUIRE(oss.str() == "vetoalg hits lll\n");
        }
        {
            std::ostringstream oss;
            caller.call("vetoalg", {"lll"}, -1, GET, oss);
            REQUIRE(oss.str() == "vetoalg hits lll\n");
        }
        {
            std::ostringstream oss;
            caller.call("vetoalg", {"hits", "10gbe"}, -1, PUT, oss);
            REQUIRE(oss.str() == "vetoalg hits 10gbe\n");
        }
        {
            std::ostringstream oss;
            caller.call("vetoalg", {"10gbe"}, -1, GET, oss);
            REQUIRE(oss.str() == "vetoalg hits 10gbe\n");
        }
        {
            std::ostringstream oss;
            caller.call("vetoalg", {"raw", "lll"}, -1, PUT, oss);
            REQUIRE(oss.str() == "vetoalg raw lll\n");
        }
        {
            std::ostringstream oss;
            caller.call("vetoalg", {"raw", "10gbe"}, -1, PUT, oss);
            REQUIRE(oss.str() == "vetoalg raw 10gbe\n");
        }
        REQUIRE_THROWS(
            caller.call("vetoalg", {"default", "lll", "10gbe"}, -1, PUT));
        REQUIRE_THROWS(caller.call("vetoalg", {"hits", "none"}, -1, PUT));
        for (int i = 0; i != det.size(); ++i) {
            det.setVetoAlgorithm(prev_val_lll[i],
                                 defs::streamingInterface::LOW_LATENCY_LINK,
                                 {i});
            det.setVetoAlgorithm(prev_val_10g[i],
                                 defs::streamingInterface::ETHERNET_10GB, {i});
        }
    } else {
        REQUIRE_THROWS(caller.call("vetoalg", {"lll"}, -1, GET));
        REQUIRE_THROWS(caller.call("vetoalg", {"none"}, -1, PUT));
    }
    REQUIRE_THROWS(caller.call("vetoalg", {"dfgd"}, -1, GET));
}

TEST_CASE("confadc", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::GOTTHARD2) {
        const int ndet = det.size();
        const int nchip = 10;
        const int nadc = 32;
        std::vector<std::vector<std::vector<int>>> prev_val(
            ndet, std::vector<std::vector<int>>(nchip, std::vector<int>(nadc)));
        for (int i = 0; i != ndet; ++i) {
            for (int j = 0; j != nchip; ++j) {
                for (int k = 0; k != nadc; ++k) {
                    prev_val[i][j][k] =
                        det.getADCConfiguration(j, k, {i}).squash();
                }
            }
        }

        REQUIRE_THROWS(caller.call("confadc", {"11", "2", "0x7f"}, -1,
                                   PUT)); // invalid chip index
        REQUIRE_THROWS(caller.call("confadc", {"-1", "32", "0x7f"}, -1,
                                   PUT)); // invalid adc index
        REQUIRE_THROWS(caller.call("confadc", {"-1", "10", "0x80"}, -1,
                                   PUT)); // invalid value
        {
            std::ostringstream oss;
            caller.call("confadc", {"-1", "-1", "0x11"}, -1, PUT, oss);
            REQUIRE(oss.str() == "confadc [-1, -1, 0x11]\n");
        }
        {
            std::ostringstream oss;
            caller.call("confadc", {"2", "3"}, -1, GET, oss);
            REQUIRE(oss.str() == "confadc 0x11\n");
        }

        for (int i = 0; i != ndet; ++i) {
            for (int j = 0; j != nchip; ++j) {
                for (int k = 0; k != nadc; ++k) {
                    det.setADCConfiguration(-1, k, prev_val[i][j][k], {i});
                }
            }
        }
    } else {
        REQUIRE_THROWS(caller.call("confadc", {}, -1, GET));
    }
}

} // namespace sls
