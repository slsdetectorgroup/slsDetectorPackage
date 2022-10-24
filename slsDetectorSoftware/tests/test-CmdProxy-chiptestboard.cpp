// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "CmdProxy.h"
#include "catch.hpp"
#include "sls/Detector.h"
#include "sls/sls_detector_defs.h"
#include <sstream>

#include "sls/Result.h"
#include "sls/ToString.h"
#include "sls/versionAPI.h"
#include "test-CmdProxy-global.h"
#include "tests/globals.h"

namespace sls {

using test::GET;
using test::PUT;

/* dacs */

TEST_CASE("dac", "[.cmd][.dacs]") {
    // dac 0 to dac 17

    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::CHIPTESTBOARD) {
        for (int i = 0; i < 18; ++i) {
            SECTION("dac " + std::to_string(i)) {
                test_dac(static_cast<defs::dacIndex>(i), "dac", 0);
            }
        }
        // eiger
        // REQUIRE_THROWS(proxy.Call("dac", {"vthreshold"}, -1, GET));
        // REQUIRE_THROWS(proxy.Call("dac", {"vsvp"}, -1, GET));
        // REQUIRE_THROWS(proxy.Call("dac", {"vsvn"}, -1, GET));
        // REQUIRE_THROWS(proxy.Call("dac", {"vtrim"}, -1, GET));
        // REQUIRE_THROWS(proxy.Call("dac", {"vrpreamp"}, -1, GET));
        // REQUIRE_THROWS(proxy.Call("dac", {"vrshaper"}, -1, GET));
        // REQUIRE_THROWS(proxy.Call("dac", {"vtgstv"}, -1, GET));
        // REQUIRE_THROWS(proxy.Call("dac", {"vcmp_ll"}, -1, GET));
        // REQUIRE_THROWS(proxy.Call("dac", {"vcmp_lr"}, -1, GET));
        // REQUIRE_THROWS(proxy.Call("dac", {"vcal"}, -1, GET));
        // REQUIRE_THROWS(proxy.Call("dac", {"vcmp_rl"}, -1, GET));
        // REQUIRE_THROWS(proxy.Call("dac", {"vcmp_rr"}, -1, GET));
        // REQUIRE_THROWS(proxy.Call("dac", {"rxb_rb"}, -1, GET));
        // REQUIRE_THROWS(proxy.Call("dac", {"rxb_lb"}, -1, GET));
        // REQUIRE_THROWS(proxy.Call("dac", {"vcp"}, -1, GET));
        // REQUIRE_THROWS(proxy.Call("dac", {"vcn"}, -1, GET));
        // REQUIRE_THROWS(proxy.Call("dac", {"vishaper"}, -1, GET));
        // REQUIRE_THROWS(proxy.Call("dac", {"iodelay"}, -1, GET));
        // jungfrau
        REQUIRE_THROWS(proxy.Call("dac", {"vb_comp"}, -1, GET));
        REQUIRE_THROWS(proxy.Call("dac", {"vdd_prot"}, -1, GET));
        REQUIRE_THROWS(proxy.Call("dac", {"vin_com"}, -1, GET));
        REQUIRE_THROWS(proxy.Call("dac", {"vref_prech"}, -1, GET));
        REQUIRE_THROWS(proxy.Call("dac", {"vb_pixbuf"}, -1, GET));
        REQUIRE_THROWS(proxy.Call("dac", {"vb_ds"}, -1, GET));
        REQUIRE_THROWS(proxy.Call("dac", {"vref_ds"}, -1, GET));
        REQUIRE_THROWS(proxy.Call("dac", {"vref_comp"}, -1, GET));
        // gotthard
        REQUIRE_THROWS(proxy.Call("dac", {"vref_ds"}, -1, GET));
        REQUIRE_THROWS(proxy.Call("dac", {"vcascn_pb"}, -1, GET));
        REQUIRE_THROWS(proxy.Call("dac", {"vcascp_pb"}, -1, GET));
        REQUIRE_THROWS(proxy.Call("dac", {"vout_cm"}, -1, GET));
        REQUIRE_THROWS(proxy.Call("dac", {"vcasc_out"}, -1, GET));
        REQUIRE_THROWS(proxy.Call("dac", {"vin_cm"}, -1, GET));
        REQUIRE_THROWS(proxy.Call("dac", {"vref_comp"}, -1, GET));
        REQUIRE_THROWS(proxy.Call("dac", {"ib_test_c"}, -1, GET));
        // mythen3
        REQUIRE_THROWS(proxy.Call("dac", {"vrpreamp"}, -1, GET));
        REQUIRE_THROWS(proxy.Call("dac", {"vrshaper"}, -1, GET));
        REQUIRE_THROWS(proxy.Call("dac", {"vrshaper_n"}, -1, GET));
        REQUIRE_THROWS(proxy.Call("dac", {"vipre"}, -1, GET));
        REQUIRE_THROWS(proxy.Call("dac", {"vishaper"}, -1, GET));
        REQUIRE_THROWS(proxy.Call("dac", {"vdcsh"}, -1, GET));
        REQUIRE_THROWS(proxy.Call("dac", {"vth1"}, -1, GET));
        REQUIRE_THROWS(proxy.Call("dac", {"vth2"}, -1, GET));
        REQUIRE_THROWS(proxy.Call("dac", {"vth3"}, -1, GET));
        REQUIRE_THROWS(proxy.Call("dac", {"vcal_n"}, -1, GET));
        REQUIRE_THROWS(proxy.Call("dac", {"vcal_p"}, -1, GET));
        REQUIRE_THROWS(proxy.Call("dac", {"vtrim"}, -1, GET));
        REQUIRE_THROWS(proxy.Call("dac", {"vcassh"}, -1, GET));
        REQUIRE_THROWS(proxy.Call("dac", {"vcas"}, -1, GET));
        REQUIRE_THROWS(proxy.Call("dac", {"vicin"}, -1, GET));
        REQUIRE_THROWS(proxy.Call("dac", {"vipre_out"}, -1, GET));
        // gotthard2
        REQUIRE_THROWS(proxy.Call("dac", {"vref_h_adc"}, -1, GET));
        REQUIRE_THROWS(proxy.Call("dac", {"vb_comp_fe"}, -1, GET));
        REQUIRE_THROWS(proxy.Call("dac", {"vb_comp_adc"}, -1, GET));
        REQUIRE_THROWS(proxy.Call("dac", {"vcom_cds"}, -1, GET));
        REQUIRE_THROWS(proxy.Call("dac", {"vref_rstore"}, -1, GET));
        REQUIRE_THROWS(proxy.Call("dac", {"vb_opa_1st"}, -1, GET));
        REQUIRE_THROWS(proxy.Call("dac", {"vref_comp_fe"}, -1, GET));
        REQUIRE_THROWS(proxy.Call("dac", {"vcom_adc1"}, -1, GET));
        REQUIRE_THROWS(proxy.Call("dac", {"vref_l_adc"}, -1, GET));
        REQUIRE_THROWS(proxy.Call("dac", {"vref_cds"}, -1, GET));
        REQUIRE_THROWS(proxy.Call("dac", {"vb_cs"}, -1, GET));
        REQUIRE_THROWS(proxy.Call("dac", {"vb_opa_fd"}, -1, GET));
        REQUIRE_THROWS(proxy.Call("dac", {"vcom_adc2"}, -1, GET));
    }
}

TEST_CASE("adcvpp", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::CHIPTESTBOARD || det_type == defs::MOENCH) {
        auto prev_val = det.getADCVpp(false);
        {
            std::ostringstream oss;
            proxy.Call("adcvpp", {"1"}, -1, PUT, oss);
            REQUIRE(oss.str() == "dac adcvpp 1\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("adcvpp", {"1140", "mv"}, -1, PUT, oss);
            REQUIRE(oss.str() == "dac adcvpp 1140 mV\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("adcvpp", {"mv"}, -1, GET, oss);
            REQUIRE(oss.str() == "dac adcvpp 1140 mV\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setADCVpp(prev_val[i], false, {i});
        }
    } else {
        REQUIRE_THROWS(proxy.Call("dac adcvpp", {}, -1, GET));
    }
}

/* CTB/ Moench Specific */

TEST_CASE("samples", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::CHIPTESTBOARD || det_type == defs::MOENCH) {
        auto prev_asamples = det.getNumberOfAnalogSamples();
        Result<int> prev_dsamples = 0;
        if (det_type == defs::CHIPTESTBOARD) {
            prev_dsamples = det.getNumberOfDigitalSamples();
        }
        {
            std::ostringstream oss;
            proxy.Call("samples", {"25"}, -1, PUT, oss);
            REQUIRE(oss.str() == "samples 25\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("samples", {"450"}, -1, PUT, oss);
            REQUIRE(oss.str() == "samples 450\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("samples", {}, -1, GET, oss);
            REQUIRE(oss.str() == "samples 450\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("asamples", {}, -1, GET, oss);
            REQUIRE(oss.str() == "asamples 450\n");
        }
        if (det_type == defs::CHIPTESTBOARD) {
            std::ostringstream oss;
            proxy.Call("dsamples", {}, -1, GET, oss);
            REQUIRE(oss.str() == "dsamples 450\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setNumberOfAnalogSamples(prev_asamples[i], {i});
            if (det_type == defs::CHIPTESTBOARD) {
                det.setNumberOfDigitalSamples(prev_dsamples[i], {i});
            }
        }
    } else {
        REQUIRE_THROWS(proxy.Call("samples", {}, -1, GET));
    }
}

TEST_CASE("asamples", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::CHIPTESTBOARD || det_type == defs::MOENCH) {
        auto prev_val = det.getNumberOfAnalogSamples();
        {
            std::ostringstream oss;
            proxy.Call("asamples", {"25"}, -1, PUT, oss);
            REQUIRE(oss.str() == "asamples 25\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("asamples", {"450"}, -1, PUT, oss);
            REQUIRE(oss.str() == "asamples 450\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("asamples", {}, -1, GET, oss);
            REQUIRE(oss.str() == "asamples 450\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setNumberOfAnalogSamples(prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(proxy.Call("asamples", {}, -1, GET));
    }
}

TEST_CASE("adcclk", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::CHIPTESTBOARD || det_type == defs::MOENCH) {
        auto prev_val = det.getADCClock();
        {
            std::ostringstream oss;
            proxy.Call("adcclk", {"20"}, -1, PUT, oss);
            REQUIRE(oss.str() == "adcclk 20\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("adcclk", {"10"}, -1, PUT, oss);
            REQUIRE(oss.str() == "adcclk 10\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("adcclk", {}, -1, GET, oss);
            REQUIRE(oss.str() == "adcclk 10\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setADCClock(prev_val[i], {i});
        }
    } else {
        // clock index might work
        // REQUIRE_THROWS(proxy.Call("adcclk", {}, -1, GET));
    }
}

TEST_CASE("runclk", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::CHIPTESTBOARD || det_type == defs::MOENCH) {
        auto prev_val = det.getRUNClock();
        {
            std::ostringstream oss;
            proxy.Call("runclk", {"20"}, -1, PUT, oss);
            REQUIRE(oss.str() == "runclk 20\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("runclk", {"10"}, -1, PUT, oss);
            REQUIRE(oss.str() == "runclk 10\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("runclk", {}, -1, GET, oss);
            REQUIRE(oss.str() == "runclk 10\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setRUNClock(prev_val[i], {i});
        }
    } else {
        // clock index might work
        // REQUIRE_THROWS(proxy.Call("runclk", {}, -1, GET));
    }
}

TEST_CASE("syncclk", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::CHIPTESTBOARD || det_type == defs::MOENCH) {
        REQUIRE_NOTHROW(proxy.Call("syncclk", {}, -1, GET));
    } else {
        // clock index might work
        // REQUIRE_THROWS(proxy.Call("syncclk", {}, -1, GET));
    }
}

TEST_CASE("adcpipeline", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::CHIPTESTBOARD || det_type == defs::MOENCH) {
        auto prev_val = det.getADCPipeline();
        {
            std::ostringstream oss;
            proxy.Call("adcpipeline", {"1"}, -1, PUT, oss);
            REQUIRE(oss.str() == "adcpipeline 1\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("adcpipeline", {"0"}, -1, PUT, oss);
            REQUIRE(oss.str() == "adcpipeline 0\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("adcpipeline", {"15"}, -1, PUT, oss);
            REQUIRE(oss.str() == "adcpipeline 15\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("adcpipeline", {}, -1, GET, oss);
            REQUIRE(oss.str() == "adcpipeline 15\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setADCPipeline(prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(proxy.Call("adcpipeline", {}, -1, GET));
    }
}

TEST_CASE("v_limit", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::CHIPTESTBOARD || det_type == defs::MOENCH) {
        auto prev_val = det.getVoltage(defs::V_LIMIT);
        {
            std::ostringstream oss;
            proxy.Call("v_limit", {"1500"}, -1, PUT, oss);
            REQUIRE(oss.str() == "v_limit 1500\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("v_limit", {"0"}, -1, PUT, oss);
            REQUIRE(oss.str() == "v_limit 0\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("v_limit", {"0"}, -1, PUT, oss);
            REQUIRE(oss.str() == "v_limit 0\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("v_limit", {}, -1, GET, oss);
            REQUIRE(oss.str() == "v_limit 0\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            if (prev_val[i] == -100) {
                prev_val[i] = 0;
            }
            det.setVoltage(defs::V_LIMIT, prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(proxy.Call("v_limit", {}, -1, GET));
    }
}

TEST_CASE("adcenable", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::CHIPTESTBOARD || det_type == defs::MOENCH) {
        auto prev_val = det.getADCEnableMask();
        {
            std::ostringstream oss;
            proxy.Call("adcenable", {"0x8d0aa0d8"}, -1, PUT, oss);
            REQUIRE(oss.str() == "adcenable 0x8d0aa0d8\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("adcenable", {"0xffffffff"}, -1, PUT, oss);
            REQUIRE(oss.str() == "adcenable 0xffffffff\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("adcenable", {}, -1, GET, oss);
            REQUIRE(oss.str() == "adcenable 0xffffffff\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setADCEnableMask(prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(proxy.Call("adcenable", {}, -1, GET));
    }
}

TEST_CASE("adcenable10g", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::CHIPTESTBOARD || det_type == defs::MOENCH) {
        auto prev_val = det.getTenGigaADCEnableMask();
        {
            std::ostringstream oss;
            proxy.Call("adcenable10g", {"0xff0000ff"}, -1, PUT, oss);
            REQUIRE(oss.str() == "adcenable10g 0xff0000ff\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("adcenable10g", {"0xffffffff"}, -1, PUT, oss);
            REQUIRE(oss.str() == "adcenable10g 0xffffffff\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("adcenable10g", {}, -1, GET, oss);
            REQUIRE(oss.str() == "adcenable10g 0xffffffff\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setTenGigaADCEnableMask(prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(proxy.Call("adcenable10g", {}, -1, GET));
    }
}

/* CTB Specific */

TEST_CASE("dsamples", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::CHIPTESTBOARD) {
        auto prev_val = det.getNumberOfDigitalSamples();
        {
            std::ostringstream oss;
            proxy.Call("dsamples", {"1"}, -1, PUT, oss);
            REQUIRE(oss.str() == "dsamples 1\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("dsamples", {"450"}, -1, PUT, oss);
            REQUIRE(oss.str() == "dsamples 450\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("dsamples", {}, -1, GET, oss);
            REQUIRE(oss.str() == "dsamples 450\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setNumberOfDigitalSamples(prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(proxy.Call("dsamples", {}, -1, GET));
    }
}

TEST_CASE("romode", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::CHIPTESTBOARD) {
        auto prev_romode = det.getReadoutMode();
        auto prev_asamples = det.getNumberOfAnalogSamples();
        auto prev_dsamples = det.getNumberOfDigitalSamples();
        det.setNumberOfAnalogSamples(5000);
        det.setNumberOfDigitalSamples(5000);
        {
            std::ostringstream oss;
            proxy.Call("romode", {"digital"}, -1, PUT, oss);
            REQUIRE(oss.str() == "romode digital\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("romode", {"analog_digital"}, -1, PUT, oss);
            REQUIRE(oss.str() == "romode analog_digital\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("romode", {"analog"}, -1, PUT, oss);
            REQUIRE(oss.str() == "romode analog\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("romode", {}, -1, GET, oss);
            REQUIRE(oss.str() == "romode analog\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setReadoutMode(prev_romode[i], {i});
            det.setNumberOfAnalogSamples(prev_asamples[i], {i});
            det.setNumberOfDigitalSamples(prev_dsamples[i], {i});
        }
    } else {
        REQUIRE_THROWS(proxy.Call("romode", {}, -1, GET));
    }
}

TEST_CASE("dbitclk", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::CHIPTESTBOARD) {
        auto prev_val = det.getRUNClock();
        {
            std::ostringstream oss;
            proxy.Call("dbitclk", {"20"}, -1, PUT, oss);
            REQUIRE(oss.str() == "dbitclk 20\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("dbitclk", {"10"}, -1, PUT, oss);
            REQUIRE(oss.str() == "dbitclk 10\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("dbitclk", {}, -1, GET, oss);
            REQUIRE(oss.str() == "dbitclk 10\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setRUNClock(prev_val[i], {i});
        }
    } else {
        // clock index might work
        // REQUIRE_THROWS(proxy.Call("dbitclk", {}, -1, GET));
    }
}

TEST_CASE("v_a", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::CHIPTESTBOARD) {
        auto prev_val = det.getVoltage(defs::V_POWER_A);
        {
            std::ostringstream oss1, oss2;
            proxy.Call("v_a", {"700"}, -1, PUT, oss1);
            REQUIRE(oss1.str() == "v_a 700\n");
            proxy.Call("v_a", {}, -1, GET, oss2);
            REQUIRE(oss2.str() == "v_a 700\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setVoltage(defs::V_POWER_A, prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(proxy.Call("v_a", {}, -1, GET));
    }
}

TEST_CASE("v_b", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::CHIPTESTBOARD) {
        auto prev_val = det.getVoltage(defs::V_POWER_B);
        {
            std::ostringstream oss1, oss2;
            proxy.Call("v_b", {"700"}, -1, PUT, oss1);
            REQUIRE(oss1.str() == "v_b 700\n");
            proxy.Call("v_b", {}, -1, GET, oss2);
            REQUIRE(oss2.str() == "v_b 700\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setVoltage(defs::V_POWER_B, prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(proxy.Call("v_b", {}, -1, GET));
    }
}

TEST_CASE("v_c", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::CHIPTESTBOARD) {
        auto prev_val = det.getVoltage(defs::V_POWER_C);
        {
            std::ostringstream oss1, oss2;
            proxy.Call("v_c", {"700"}, -1, PUT, oss1);
            REQUIRE(oss1.str() == "v_c 700\n");
            proxy.Call("v_c", {}, -1, GET, oss2);
            REQUIRE(oss2.str() == "v_c 700\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setVoltage(defs::V_POWER_C, prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(proxy.Call("v_c", {}, -1, GET));
    }
}

TEST_CASE("v_d", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::CHIPTESTBOARD) {
        auto prev_val = det.getVoltage(defs::V_POWER_D);
        {
            std::ostringstream oss1, oss2;
            proxy.Call("v_d", {"700"}, -1, PUT, oss1);
            REQUIRE(oss1.str() == "v_d 700\n");
            proxy.Call("v_d", {}, -1, GET, oss2);
            REQUIRE(oss2.str() == "v_d 700\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setVoltage(defs::V_POWER_D, prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(proxy.Call("v_d", {}, -1, GET));
    }
}

TEST_CASE("v_io", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::CHIPTESTBOARD) {
        // better not to play with setting it
        REQUIRE_NOTHROW(proxy.Call("v_io", {}, -1, GET));
    } else {
        REQUIRE_THROWS(proxy.Call("v_io", {}, -1, GET));
    }
}

TEST_CASE("v_chip", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::CHIPTESTBOARD) {
        // better not to play with setting it
        REQUIRE_NOTHROW(proxy.Call("v_chip", {}, -1, GET));
    } else {
        REQUIRE_THROWS(proxy.Call("v_chip", {}, -1, GET));
    }
}

TEST_CASE("vm_a", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::CHIPTESTBOARD) {
        REQUIRE_NOTHROW(proxy.Call("vm_a", {}, -1, GET));
    } else {
        REQUIRE_THROWS(proxy.Call("vm_a", {}, -1, GET));
    }
}

TEST_CASE("vm_b", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::CHIPTESTBOARD) {
        REQUIRE_NOTHROW(proxy.Call("vm_b", {}, -1, GET));
    } else {
        REQUIRE_THROWS(proxy.Call("vm_b", {}, -1, GET));
    }
}

TEST_CASE("vm_c", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::CHIPTESTBOARD) {
        REQUIRE_NOTHROW(proxy.Call("vm_c", {}, -1, GET));
    } else {
        REQUIRE_THROWS(proxy.Call("vm_c", {}, -1, GET));
    }
}

TEST_CASE("vm_d", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::CHIPTESTBOARD) {
        REQUIRE_NOTHROW(proxy.Call("vm_d", {}, -1, GET));
    } else {
        REQUIRE_THROWS(proxy.Call("vm_d", {}, -1, GET));
    }
}

TEST_CASE("vm_io", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::CHIPTESTBOARD) {
        REQUIRE_NOTHROW(proxy.Call("vm_io", {}, -1, GET));
    } else {
        REQUIRE_THROWS(proxy.Call("vm_io", {}, -1, GET));
    }
}

TEST_CASE("im_a", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::CHIPTESTBOARD) {
        REQUIRE_NOTHROW(proxy.Call("im_a", {}, -1, GET));
    } else {
        REQUIRE_THROWS(proxy.Call("im_a", {}, -1, GET));
    }
}

TEST_CASE("im_b", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::CHIPTESTBOARD) {
        REQUIRE_NOTHROW(proxy.Call("im_b", {}, -1, GET));
    } else {
        REQUIRE_THROWS(proxy.Call("im_b", {}, -1, GET));
    }
}

TEST_CASE("im_c", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::CHIPTESTBOARD) {
        REQUIRE_NOTHROW(proxy.Call("im_c", {}, -1, GET));
    } else {
        REQUIRE_THROWS(proxy.Call("im_c", {}, -1, GET));
    }
}

TEST_CASE("im_d", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::CHIPTESTBOARD) {
        REQUIRE_NOTHROW(proxy.Call("im_d", {}, -1, GET));
    } else {
        REQUIRE_THROWS(proxy.Call("im_d", {}, -1, GET));
    }
}

TEST_CASE("im_io", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::CHIPTESTBOARD) {
        REQUIRE_NOTHROW(proxy.Call("im_io", {}, -1, GET));
    } else {
        REQUIRE_THROWS(proxy.Call("im_io", {}, -1, GET));
    }
}

TEST_CASE("adc", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::CHIPTESTBOARD) {
        for (int i = 0; i <= 7; ++i) {
            REQUIRE_NOTHROW(proxy.Call("adc", {std::to_string(i)}, -1, GET));
            REQUIRE_THROWS(proxy.Call("adc", {"0"}, -1, PUT));
        }
    } else {
        REQUIRE_THROWS(proxy.Call("adc", {"0"}, -1, GET));
    }
}

TEST_CASE("extsampling", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::CHIPTESTBOARD) {
        auto prev_val = det.getExternalSampling();
        {
            std::ostringstream oss;
            proxy.Call("extsampling", {"1"}, -1, PUT, oss);
            REQUIRE(oss.str() == "extsampling 1\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("extsampling", {"0"}, -1, PUT, oss);
            REQUIRE(oss.str() == "extsampling 0\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("extsampling", {}, -1, GET, oss);
            REQUIRE(oss.str() == "extsampling 0\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setExternalSampling(prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(proxy.Call("extsampling", {}, -1, GET));
    }
}

TEST_CASE("extsamplingsrc", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::CHIPTESTBOARD) {
        auto prev_val = det.getExternalSamplingSource();
        {
            std::ostringstream oss;
            proxy.Call("extsamplingsrc", {"63"}, -1, PUT, oss);
            REQUIRE(oss.str() == "extsamplingsrc 63\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("extsamplingsrc", {"0"}, -1, PUT, oss);
            REQUIRE(oss.str() == "extsamplingsrc 0\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("extsamplingsrc", {}, -1, GET, oss);
            REQUIRE(oss.str() == "extsamplingsrc 0\n");
        }
        REQUIRE_THROWS(proxy.Call("extsamplingsrc", {"64"}, -1, PUT));
        for (int i = 0; i != det.size(); ++i) {
            det.setExternalSamplingSource(prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(proxy.Call("extsamplingsrc", {}, -1, GET));
    }
}

TEST_CASE("diodelay", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::CHIPTESTBOARD) {
        {
            std::ostringstream oss;
            proxy.Call("diodelay", {"0x01010", "0"}, -1, PUT, oss);
            REQUIRE(oss.str() == "diodelay [0x01010, 0]\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("diodelay", {"0x01010", "775"}, -1, PUT, oss);
            REQUIRE(oss.str() == "diodelay [0x01010, 775]\n");
        }
        REQUIRE_THROWS(proxy.Call("diodelay", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("diodelay", {"0x01010", "776"}, -1, GET));
    } else {
        REQUIRE_THROWS(proxy.Call("diodelay", {"0x01010", "775"}, -1, PUT));
    }
}

TEST_CASE("led", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::CHIPTESTBOARD) {
        auto prev_val = det.getLEDEnable();
        {
            std::ostringstream oss;
            proxy.Call("led", {"1"}, -1, PUT, oss);
            REQUIRE(oss.str() == "led 1\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("led", {"0"}, -1, PUT, oss);
            REQUIRE(oss.str() == "led 0\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("led", {}, -1, GET, oss);
            REQUIRE(oss.str() == "led 0\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setLEDEnable(prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(proxy.Call("led", {}, -1, GET));
    }
}

} // namespace sls
