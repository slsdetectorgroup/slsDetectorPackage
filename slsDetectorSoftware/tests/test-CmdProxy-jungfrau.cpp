#include "CmdProxy.h"
#include "Detector.h"
#include "catch.hpp"
#include "sls_detector_defs.h"
#include <sstream>

#include "test-CmdProxy-global.h"
#include "tests/globals.h"
#include "versionAPI.h"

using sls::CmdProxy;
using sls::Detector;
using test::GET;
using test::PUT;

/* dacs */

TEST_CASE("Setting and reading back Jungfrau dacs", "[.cmd][.dacs][.new]") {
    // vb_comp, vdd_prot, vin_com, vref_prech, vb_pixbuf, vb_ds, vref_ds, vref_comp
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU) {
        SECTION("vb_comp") { test_dac(defs::VB_COMP, "vb_comp", 1220); }
        SECTION("vdd_prot") { test_dac(defs::VDD_PROT, "vdd_prot", 3000); }
        SECTION("vin_com") { test_dac(defs::VIN_COM, "vin_com", 1053); }
        SECTION("vref_prech") { test_dac(defs::VREF_PRECH, "vref_prech", 1450); }
        SECTION("vb_pixbuf") { test_dac(defs::VB_PIXBUF, "vb_pixbuf", 750); }
        SECTION("vb_ds") { test_dac(defs::VB_DS, "vb_ds", 1000); }
        SECTION("vref_ds") { test_dac(defs::VREF_DS, "vref_ds", 480); }
        SECTION("vref_comp") { test_dac(defs::VREF_COMP, "vref_comp", 420); }
        // eiger
        REQUIRE_THROWS(proxy.Call("vthreshold", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vsvp", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vsvn", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vtr", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vrf", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vrs", {}, -1, GET));
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
        REQUIRE_THROWS(proxy.Call("vis", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("iodelay", {}, -1, GET));
        // gotthard
        // REQUIRE_THROWS(proxy.Call("vref_ds", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vcascn_pb", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vcascp_pb", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vout_cm", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vcasc_out", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vin_cm", {}, -1, GET));
        // REQUIRE_THROWS(proxy.Call("vref_comp", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("ib_test_c", {}, -1, GET));
        // mythen3
        REQUIRE_THROWS(proxy.Call("vpreamp", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vshaper", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vshaperneg", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vipre", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("viinsh", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vdcsh", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vth1", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vth2", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vth3", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vpl", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vph", {}, -1, GET));
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

































TEST_CASE("nframes", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU || det_type == defs::CHIPTESTBOARD) {
        auto nframes = det.getNumberOfFramesFromStart().squash();
        std::ostringstream oss;
        proxy.Call("nframes", {}, -1, GET, oss);
        REQUIRE(oss.str() == "nframes " + std::to_string(nframes) + "\n");

    } else {
        REQUIRE_THROWS(proxy.Call("nframes", {}, -1, GET));
    }
}

TEST_CASE("now", "[.cmd]") {
    // TODO! can we test this?
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU || det_type == defs::CHIPTESTBOARD) {
        std::ostringstream oss;
        proxy.Call("now", {}, -1, GET, oss);

        // Get only
        REQUIRE_THROWS(proxy.Call("now", {"2019"}, -1, PUT));
    } else {
        REQUIRE_THROWS(proxy.Call("now", {}, -1, GET));
    }
}

TEST_CASE("timestamp", "[.cmd]") {
    // TODO! can we test this?
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU || det_type == defs::CHIPTESTBOARD) {
        std::ostringstream oss;
        proxy.Call("timestamp", {}, -1, GET, oss);

        // Get only
        REQUIRE_THROWS(proxy.Call("timestamp", {"2019"}, -1, PUT));
    } else {
        REQUIRE_THROWS(proxy.Call("timestamp", {}, -1, GET));
    }
}

TEST_CASE("adcreg", "[.cmd]") {
    //TODO! what is a safe value to use? 
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU || det_type == defs::CHIPTESTBOARD ||
        det_type == defs::GOTTHARD) {
        std::ostringstream oss;
        proxy.Call("adcreg", {"0x0", "0"}, -1, PUT, oss);
        REQUIRE(oss.str() == "adcreg [0x0, 0]\n");
        // This is a put only command
        REQUIRE_THROWS(proxy.Call("adcreg", {}, -1, GET));
    } else {
        REQUIRE_THROWS(proxy.Call("adcreg", {"0x0", "0"}, -1, PUT));
        REQUIRE_THROWS(proxy.Call("adcreg", {}, -1, GET));
    }
}

TEST_CASE("bustest", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU || det_type == defs::CHIPTESTBOARD ||
        det_type == defs::GOTTHARD) {
        std::ostringstream oss;
        proxy.Call("bustest", {}, -1, PUT, oss);
        REQUIRE(oss.str() == "bustest successful\n");
        REQUIRE_THROWS(proxy.Call("bustest", {}, -1, GET));
    } else {
        REQUIRE_THROWS(proxy.Call("bustest", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("bustest", {}, -1, PUT));
    }
}

TEST_CASE("firmwaretest", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU || det_type == defs::CHIPTESTBOARD ||
        det_type == defs::GOTTHARD) {
        std::ostringstream oss;
        proxy.Call("firmwaretest", {}, -1, PUT, oss);
        REQUIRE(oss.str() == "firmwaretest successful\n");
        REQUIRE_THROWS(proxy.Call("firmwaretest", {}, -1, GET));
    } else {
        REQUIRE_THROWS(proxy.Call("firmwaretest", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("firmwaretest", {}, -1, PUT));
    }
}

TEST_CASE("resetfpga", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU || det_type == defs::CHIPTESTBOARD) {
        std::ostringstream oss;
        proxy.Call("resetfpga", {}, -1, PUT, oss);
        REQUIRE(oss.str() == "resetfpga successful\n");
        REQUIRE_THROWS(proxy.Call("resetfpga", {}, -1, GET));
    } else {
        REQUIRE_THROWS(proxy.Call("resetfpga", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("resetfpga", {}, -1, PUT));
    }
}


