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

TEST_CASE("Setting and reading back MYTHEN3 dacs", "[.cmd][.dacs][.new]") {
    // vcassh, vth2, vshaper, vshaperneg, vipre_out, vth3, vth1,
    // vicin, vcas, vpreamp, vpl, vipre, viinsh, vph, vtrim, vdcsh,

    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::MYTHEN3) {
        SECTION("vcassh") { test_dac(defs::CASSH, "vcassh", 1200); }
        SECTION("vth2") { test_dac(defs::VTH2, "vth2", 2800); }
        SECTION("vshaper") { test_dac(defs::SHAPER1, "vshaper", 1280); }
        SECTION("vshaperneg") { test_dac(defs::SHAPER2, "vshaperneg", 2800); }
        SECTION("vipre_out") { test_dac(defs::VIPRE_OUT, "vipre_out", 1220); }
        SECTION("vth3") { test_dac(defs::VTH3, "vth3", 2800); }
        SECTION("vth1") { test_dac(defs::THRESHOLD, "vth1", 2880); }
        SECTION("vicin") { test_dac(defs::VICIN, "vicin", 1708); }
        SECTION("vcas") { test_dac(defs::CAS, "vcas", 1800); }
        SECTION("vpreamp") { test_dac(defs::PREAMP, "vpreamp", 1100); }
        SECTION("vpl") { test_dac(defs::VPL, "vpl", 1100); }
        SECTION("vipre") { test_dac(defs::VIPRE, "vipre", 2624); }
        SECTION("viinsh") { test_dac(defs::VIINSH, "viinsh", 1708); }
        SECTION("vph") { test_dac(defs::CALIBRATION_PULSE, "vph", 1712); }
        SECTION("vtrim") { test_dac(defs::TRIMBIT_SIZE, "vtrim", 2800); }
        SECTION("vdcsh") { test_dac(defs::VDCSH, "vdcsh", 800); }

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
        REQUIRE_THROWS(proxy.Call("vref_ds", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vcascn_pb", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vcascp_pb", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vout_cm", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vcasc_out", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vin_cm", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vref_comp", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("ib_test_c", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vref_h_adc", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vb_comp_fe", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vb_comp_adc", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vcom_cds", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vref_rstore", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vb_opa_1st", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vref_comp_fe", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vcom_adc1", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vref_prech", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vref_l_adc", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vref_cds", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vb_cs", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vb_opa_fd", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vcom_adc2", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vb_ds", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vb_comp", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vb_pixbuf", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vin_com", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vdd_prot", {}, -1, GET));
    }
}

















TEST_CASE("counters", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::MYTHEN3) {
        REQUIRE_THROWS(proxy.Call("counters", {}, -1, PUT));
        REQUIRE_THROWS(proxy.Call("counters", {"3"}, -1, GET));
        REQUIRE_THROWS(proxy.Call("counters", {"0", "-1"}, -1, GET));
        REQUIRE_THROWS(proxy.Call("counters", {"0", "1", "1"}, -1, GET));

        auto mask = det.getCounterMask({0}).squash(-1);
        std::vector<std::string> list_str;
        for (int i = 0; i < 32; ++i) {
            if (mask & (1 << i)) {
                list_str.push_back(std::to_string(i));
            }
        }
        std::ostringstream oss_set, oss_set2, oss_set3, oss_get;
        proxy.Call("counters", {"0", "2", "1"}, -1, PUT, oss_set);
        REQUIRE(oss_set.str() == "counters [0, 2, 1]\n");
        proxy.Call("counters", {"0", "2"}, -1, PUT, oss_set2);
        REQUIRE(oss_set2.str() == "counters [0, 2]\n");        
        // put back old value
        proxy.Call("counters", list_str, -1, PUT, oss_set3);
        REQUIRE(oss_set3.str() == "counters " + sls::ToString(list_str) + "\n");
        proxy.Call("counters", {}, -1, GET, oss_get);
        REQUIRE(oss_get.str() == "counters " + sls::ToString(list_str) + "\n");
    } else {
        REQUIRE_THROWS(proxy.Call("counters", {}, -1, GET));
    }
}
