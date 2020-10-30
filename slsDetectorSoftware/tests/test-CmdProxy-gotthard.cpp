#include "CmdProxy.h"
#include "sls/Detector.h"
#include "catch.hpp"
#include "sls_detector_defs.h"
#include <sstream>

#include "sls/Result.h"
#include "ToString.h"
#include "test-CmdProxy-global.h"
#include "tests/globals.h"
#include "versionAPI.h"

using sls::CmdProxy;
using sls::Detector;
using test::GET;
using test::PUT;

/* dacs */

TEST_CASE("Setting and reading back GOTTHARD dacs", "[.cmd][.dacs][.new]") {
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
        REQUIRE_THROWS(proxy.Call("vthreshold", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vsvp", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vsvn", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vtrim", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vrpreamp", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vrshaper", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vtgstv", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vcmp_ll", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vcmp_lr", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vcal", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vcmp_rl", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vcmp_rr", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("rxb_rb", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("rxb_lb", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vcp", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vcn", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vishaper", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("iodelay", {}, -1, GET));
        // jungfrau
        REQUIRE_THROWS(proxy.Call("vb_comp", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vdd_prot", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vin_com", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vref_prech", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vb_pixbuf", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vb_ds", {}, -1, GET));
        // REQUIRE_THROWS(proxy.Call("vref_ds", {}, -1, GET));
        // REQUIRE_THROWS(proxy.Call("vref_comp", {}, -1, GET));
        // mythen3
        REQUIRE_THROWS(proxy.Call("vrpreamp", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vrshaper", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vrshaper_n", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vipre", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vishaper", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vdcsh", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vth1", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vth2", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vth3", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vcal_n", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vcal_p", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vtrim", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vcassh", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vcas", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vicin", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vipre_out", {}, -1, GET));
        // gotthard2
        REQUIRE_THROWS(proxy.Call("vref_h_adc", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vb_comp_fe", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vb_comp_adc", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vcom_cds", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vref_rstore", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vb_opa_1st", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vref_comp_fe", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vcom_adc1", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vref_l_adc", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vref_cds", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vb_cs", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vb_opa_fd", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vcom_adc2", {}, -1, GET));
    }
}

/* Gotthard Specific */

TEST_CASE("roi", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::GOTTHARD) {
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
    } else {
        REQUIRE_THROWS(proxy.Call("roi", {}, -1, GET));
    }
}

TEST_CASE("clearroi", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::GOTTHARD) {
        auto prev_val = det.getROI();
        {
            std::ostringstream oss;
            proxy.Call("clearroi", {}, -1, PUT, oss);
            REQUIRE(oss.str() == "clearroi [-1, -1]\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setROI(prev_val[i], i);
        }
    } else {
        REQUIRE_THROWS(proxy.Call("clearroi", {}, -1, PUT));
    }
}

TEST_CASE("exptimel", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::GOTTHARD) {
        REQUIRE_NOTHROW(proxy.Call("exptimel", {}, -1, GET));
    } else {
        REQUIRE_THROWS(proxy.Call("exptimel", {}, -1, GET));
    }
}