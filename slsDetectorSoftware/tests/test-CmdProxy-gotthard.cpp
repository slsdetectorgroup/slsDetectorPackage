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

TEST_CASE("Setting and reading back GOTTHARD dacs", "[.cmd][.dacs]") {
    // vref_ds, vcascn_pb, vcascp_pb, vout_cm, vcasc_out, vin_cm, vref_comp,
    // ib_test_c

    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::GOTTHARD) {
        SECTION("vref_ds") { test_dac(defs::VREF_DS, "vref_ds", 660); }
        SECTION("vcascn_pb") { test_dac(defs::VCASCN_PB, "vcascn_pb", 650); }
        SECTION("vcascp_pb") { test_dac(defs::VCASCP_PB, "vcascp_pb", 1480); }
        SECTION("vout_cm") { test_dac(defs::VOUT_CM, "vout_cm", 1520); }
        SECTION("vcasc_out") { test_dac(defs::VCASC_OUT, "vcasc_out", 1320); }
        SECTION("vin_cm") { test_dac(defs::VIN_CM, "vin_cm", 1350); }
        SECTION("vref_comp") { test_dac(defs::VREF_COMP, "vref_comp", 350); }
        SECTION("ib_test_c") { test_dac(defs::IB_TESTC, "ib_test_c", 2001); }
        // eiger
        REQUIRE_THROWS(proxy.Call("dac", {"vthreshold"}, -1, GET));
        REQUIRE_THROWS(proxy.Call("dac", {"vsvp"}, -1, GET));
        REQUIRE_THROWS(proxy.Call("dac", {"vsvn"}, -1, GET));
        REQUIRE_THROWS(proxy.Call("dac", {"vtrim"}, -1, GET));
        REQUIRE_THROWS(proxy.Call("dac", {"vrpreamp"}, -1, GET));
        REQUIRE_THROWS(proxy.Call("dac", {"vrshaper"}, -1, GET));
        REQUIRE_THROWS(proxy.Call("dac", {"vtgstv"}, -1, GET));
        REQUIRE_THROWS(proxy.Call("dac", {"vcmp_ll"}, -1, GET));
        REQUIRE_THROWS(proxy.Call("dac", {"vcmp_lr"}, -1, GET));
        REQUIRE_THROWS(proxy.Call("dac", {"vcal"}, -1, GET));
        REQUIRE_THROWS(proxy.Call("dac", {"vcmp_rl"}, -1, GET));
        REQUIRE_THROWS(proxy.Call("dac", {"vcmp_rr"}, -1, GET));
        REQUIRE_THROWS(proxy.Call("dac", {"rxb_rb"}, -1, GET));
        REQUIRE_THROWS(proxy.Call("dac", {"rxb_lb"}, -1, GET));
        REQUIRE_THROWS(proxy.Call("dac", {"vcp"}, -1, GET));
        REQUIRE_THROWS(proxy.Call("dac", {"vcn"}, -1, GET));
        REQUIRE_THROWS(proxy.Call("dac", {"vishaper"}, -1, GET));
        REQUIRE_THROWS(proxy.Call("dac", {"iodelay"}, -1, GET));
        // jungfrau
        REQUIRE_THROWS(proxy.Call("dac", {"vb_comp"}, -1, GET));
        REQUIRE_THROWS(proxy.Call("dac", {"vdd_prot"}, -1, GET));
        REQUIRE_THROWS(proxy.Call("dac", {"vin_com"}, -1, GET));
        REQUIRE_THROWS(proxy.Call("dac", {"vref_prech"}, -1, GET));
        REQUIRE_THROWS(proxy.Call("dac", {"vb_pixbuf"}, -1, GET));
        REQUIRE_THROWS(proxy.Call("dac", {"vb_ds"}, -1, GET));
        // REQUIRE_THROWS(proxy.Call("dac", {"vref_ds"}, -1, GET));
        // REQUIRE_THROWS(proxy.Call("dac", {"vref_comp"}, -1, GET));
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

/* Gotthard Specific */

TEST_CASE("roi", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::GOTTHARD) {
        if (det.size() > 1) {
            REQUIRE_THROWS(proxy.Call("roi", {"0", "255"}, -1, PUT));
            REQUIRE_NOTHROW(proxy.Call("roi", {}, -1, GET));
        } else {
            auto prev_val = det.getROI();
            {
                std::ostringstream oss;
                proxy.Call("roi", {"0", "255"}, -1, PUT, oss);
                REQUIRE(oss.str() == "roi [0, 255]\n");
            }
            {
                std::ostringstream oss;
                proxy.Call("roi", {"256", "511"}, -1, PUT, oss);
                REQUIRE(oss.str() == "roi [256, 511]\n");
            }
            REQUIRE_THROWS(proxy.Call("roi", {"0", "256"}, -1, PUT));
            for (int i = 0; i != det.size(); ++i) {
                det.setROI(prev_val[i], i);
            }
        }
    } else {
        REQUIRE_THROWS(proxy.Call("roi", {}, -1, GET));
    }
}

TEST_CASE("clearroi", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::GOTTHARD) {
        auto prev_val = det.getROI();
        {
            std::ostringstream oss;
            proxy.Call("clearroi", {}, -1, PUT, oss);
            REQUIRE(oss.str() == "clearroi successful\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setROI(prev_val[i], i);
        }
    } else {
        REQUIRE_THROWS(proxy.Call("clearroi", {}, -1, PUT));
    }
}

TEST_CASE("exptimel", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::GOTTHARD) {
        REQUIRE_NOTHROW(proxy.Call("exptimel", {}, -1, GET));
    } else {
        REQUIRE_THROWS(proxy.Call("exptimel", {}, -1, GET));
    }
}

} // namespace sls
