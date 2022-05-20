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

TEST_CASE("Setting and reading back MOENCH dacs", "[.cmd][.dacs]") {
    // vbp_colbuf, vipre, vin_cm", vb_sda, vcasc_sfp, vout_cm, vipre_cds,
    // ibias_sfp

    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::MOENCH) {
        SECTION("vbp_colbuf") {
            test_dac(defs::VBP_COLBUF, "vbp_colbuf", 1300);
        }
        SECTION("vipre") { test_dac(defs::VIPRE, "vipre", 1000); }
        SECTION("vin_cm") { test_dac(defs::VIN_CM, "vin_cm", 1400); }
        SECTION("vb_sda") { test_dac(defs::VB_SDA, "vb_sda", 680); }
        SECTION("vcasc_sfp") { test_dac(defs::VCASC_SFP, "vcasc_sfp", 1428); }
        SECTION("vout_cm") { test_dac(defs::VOUT_CM, "vout_cm", 1200); }
        SECTION("vipre_cds") { test_dac(defs::VIPRE_CDS, "vipre_cds", 800); }
        SECTION("ibias_sfp") { test_dac(defs::IBIAS_SFP, "ibias_sfp", 900); }

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
        REQUIRE_THROWS(proxy.Call("dac", {"vref_ds"}, -1, GET));
        REQUIRE_THROWS(proxy.Call("dac", {"vref_comp"}, -1, GET));
        // gotthard
        REQUIRE_THROWS(proxy.Call("dac", {"vref_ds"}, -1, GET));
        REQUIRE_THROWS(proxy.Call("dac", {"vcascn_pb"}, -1, GET));
        REQUIRE_THROWS(proxy.Call("dac", {"vcascp_pb"}, -1, GET));
        // REQUIRE_THROWS(proxy.Call("dac", {"vout_cm"}, -1, GET));
        REQUIRE_THROWS(proxy.Call("dac", {"vcasc_out"}, -1, GET));
        // REQUIRE_THROWS(proxy.Call("dac", {"vin_cm"}, -1, GET));
        REQUIRE_THROWS(proxy.Call("dac", {"vref_comp"}, -1, GET));
        REQUIRE_THROWS(proxy.Call("dac", {"ib_test_c"}, -1, GET));
        // mythen3
        REQUIRE_THROWS(proxy.Call("dac", {"vrpreamp"}, -1, GET));
        REQUIRE_THROWS(proxy.Call("dac", {"vrshaper"}, -1, GET));
        REQUIRE_THROWS(proxy.Call("dac", {"vrshaper_n"}, -1, GET));
        // REQUIRE_THROWS(proxy.Call("dac", {"vipre"}, -1, GET));
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

} // namespace sls
