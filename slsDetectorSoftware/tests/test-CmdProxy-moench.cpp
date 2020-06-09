#include "CmdProxy.h"
#include "Detector.h"
#include "catch.hpp"
#include "sls_detector_defs.h"
#include <sstream>

#include "Result.h"
#include "ToString.h"
#include "test-CmdProxy-global.h"
#include "tests/globals.h"
#include "versionAPI.h"

using sls::CmdProxy;
using sls::Detector;
using test::GET;
using test::PUT;

/* dacs */

TEST_CASE("Setting and reading back MOENCH dacs", "[.cmd][.dacs][.new]") {
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
        REQUIRE_THROWS(proxy.Call("vref_ds", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vref_comp", {}, -1, GET));
        // gotthard
        REQUIRE_THROWS(proxy.Call("vref_ds", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vcascn_pb", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vcascp_pb", {}, -1, GET));
        // REQUIRE_THROWS(proxy.Call("vout_cm", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vcasc_out", {}, -1, GET));
        // REQUIRE_THROWS(proxy.Call("vin_cm", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vref_comp", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("ib_test_c", {}, -1, GET));
        // mythen3
        REQUIRE_THROWS(proxy.Call("vrpreamp", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vrshaper", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vrshaper_n", {}, -1, GET));
        // REQUIRE_THROWS(proxy.Call("vipre", {}, -1, GET));
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

/* Moench */

TEST_CASE("emin", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::MOENCH) {
        {
            std::ostringstream oss;
            proxy.Call("emin", {"100"}, -1, PUT, oss);
            REQUIRE(oss.str() == "emin 100\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("emin", {"200"}, -1, PUT, oss);
            REQUIRE(oss.str() == "emin 200\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("emin", {}, -1, GET, oss);
            REQUIRE(oss.str() == "emin 200\n");
        }
    } else {
        REQUIRE_THROWS(proxy.Call("emin", {}, -1, GET));
    }
}

TEST_CASE("emax", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::MOENCH) {
        {
            std::ostringstream oss;
            proxy.Call("emax", {"100"}, -1, PUT, oss);
            REQUIRE(oss.str() == "emax 100\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("emax", {"200"}, -1, PUT, oss);
            REQUIRE(oss.str() == "emax 200\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("emax", {}, -1, GET, oss);
            REQUIRE(oss.str() == "emax 200\n");
        }
    } else {
        REQUIRE_THROWS(proxy.Call("emax", {}, -1, GET));
    }
}

TEST_CASE("framemode", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::MOENCH) {
        {
            std::ostringstream oss;
            proxy.Call("framemode", {"pedestal"}, -1, PUT, oss);
            REQUIRE(oss.str() == "framemode pedestal\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("framemode", {"newpedestal"}, -1, PUT, oss);
            REQUIRE(oss.str() == "framemode newpedestal\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("framemode", {"flatfield"}, -1, PUT, oss);
            REQUIRE(oss.str() == "framemode flatfield\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("framemode", {"newflatfield"}, -1, PUT, oss);
            REQUIRE(oss.str() == "framemode newflatfield\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("framemode", {}, -1, GET, oss);
            REQUIRE(oss.str() == "framemode newflatfield\n");
        }
        REQUIRE_THROWS(proxy.Call("framemode", {"counting"}, -1, PUT));
    } else {
        REQUIRE_THROWS(proxy.Call("framemode", {}, -1, GET));
    }
}

TEST_CASE("detectormode", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::MOENCH) {
        {
            std::ostringstream oss;
            proxy.Call("detectormode", {"counting"}, -1, PUT, oss);
            REQUIRE(oss.str() == "detectormode counting\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("detectormode", {"interpolating"}, -1, PUT, oss);
            REQUIRE(oss.str() == "detectormode interpolating\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("detectormode", {"analog"}, -1, PUT, oss);
            REQUIRE(oss.str() == "detectormode analog\n");
        }
        std::ostringstream oss;
        proxy.Call("detectormode", {}, -1, GET, oss);
        REQUIRE(oss.str() == "detectormode analog\n");

        REQUIRE_THROWS(proxy.Call("detectormode", {"pedestal"}, -1, PUT));
    } else {
        REQUIRE_THROWS(proxy.Call("detectormode", {}, -1, GET));
    }
}