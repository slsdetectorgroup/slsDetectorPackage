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
        SECTION("vcassh") { test_dac(defs::VCASSH, "vcassh", 1200); }
        SECTION("vth2") { test_dac(defs::VTH2, "vth2", 2800); }
        SECTION("vrshaper") { test_dac(defs::VRSHAPER, "vrshaper", 1280); }
        SECTION("vrshaper_n") {
            test_dac(defs::VRSHAPER_N, "vrshaper_n", 2800);
        }
        SECTION("vipre_out") { test_dac(defs::VIPRE_OUT, "vipre_out", 1220); }
        SECTION("vth3") { test_dac(defs::VTH3, "vth3", 2800); }
        SECTION("vth1") { test_dac(defs::VTH1, "vth1", 2880); }
        SECTION("vicin") { test_dac(defs::VICIN, "vicin", 1708); }
        SECTION("vcas") { test_dac(defs::VCAS, "vcas", 1800); }
        SECTION("vrpreamp") { test_dac(defs::VRPREAMP, "vrpreamp", 1100); }
        SECTION("vcal_n") { test_dac(defs::VCAL_N, "vcal_n", 1100); }
        SECTION("vipre") { test_dac(defs::VIPRE, "vipre", 2624); }
        SECTION("vishaper") { test_dac(defs::VISHAPER, "vishaper", 1708); }
        SECTION("vcal_p") { test_dac(defs::VCAL_P, "vcal_p", 1712); }
        SECTION("vtrim") { test_dac(defs::VTRIM, "vtrim", 2800); }
        SECTION("vdcsh") { test_dac(defs::VDCSH, "vdcsh", 800); }
        SECTION("vthreshold") {
            // Read out individual vcmp to be able to reset after
            // the test is done
            auto vth1 = det.getDAC(defs::VTH1, false);
            auto vth2 = det.getDAC(defs::VTH2, false);
            auto vth3 = det.getDAC(defs::VTH3, false);

            {
                std::ostringstream oss;
                proxy.Call("vthreshold", {"1234"}, -1, PUT, oss);
                REQUIRE(oss.str() == "dac vthreshold 1234\n");
            }
            {
                std::ostringstream oss;
                proxy.Call("vthreshold", {}, -1, GET, oss);
                REQUIRE(oss.str() == "dac vthreshold 1234\n");
            }

            // Reset dacs after test
            for (int i = 0; i != det.size(); ++i) {
                det.setDAC(defs::VTH1, vth1[i], false, {i});
                det.setDAC(defs::VTH2, vth2[i], false, {i});
                det.setDAC(defs::VTH3, vth3[i], false, {i});
            }
        }
        REQUIRE_THROWS(proxy.Call("vsvp", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vsvn", {}, -1, GET));
        // REQUIRE_THROWS(proxy.Call("vtrim", {}, -1, GET));
        // REQUIRE_THROWS(proxy.Call("vrpreamp", {}, -1, GET));
        // REQUIRE_THROWS(proxy.Call("vrshaper", {}, -1, GET));
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
        // REQUIRE_THROWS(proxy.Call("vishaper", {}, -1, GET));
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

/* acquisition */

TEST_CASE("readout", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    // PUT only command
    REQUIRE_THROWS(proxy.Call("readout", {}, -1, GET));
    auto det_type = det.getDetectorType().squash();
    if (det_type != defs::MYTHEN3) {
        REQUIRE_THROWS(proxy.Call("readout", {}, -1, GET));
    } else {
        std::ostringstream oss;
        proxy.Call("readout", {}, -1, PUT, oss);
        REQUIRE(oss.str() == "readout successful\n");
    }
}

/* Mythen3 Specific */

TEST_CASE("counters", "[.cmd][.new]") {
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

TEST_CASE("gates", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::MYTHEN3) {
        auto prev_val = det.getNumberOfGates();
        {
            std::ostringstream oss;
            proxy.Call("gates", {"1000"}, -1, PUT, oss);
            REQUIRE(oss.str() == "gates 1000\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("gates", {}, -1, GET, oss);
            REQUIRE(oss.str() == "gates 1000\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("gates", {"1"}, -1, PUT, oss);
            REQUIRE(oss.str() == "gates 1\n");
        }
        REQUIRE_THROWS(proxy.Call("gates", {"0"}, -1, PUT));
        for (int i = 0; i != det.size(); ++i) {
            det.setNumberOfGates(prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(proxy.Call("gates", {}, -1, GET));
    }
}

TEST_CASE("exptime1", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::MYTHEN3) {
        auto prev_val = det.getExptime(0);
        {
            std::ostringstream oss;
            proxy.Call("exptime1", {"1.25s"}, -1, PUT, oss);
            REQUIRE(oss.str() == "exptime1 1.25s\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("exptime1", {}, -1, GET, oss);
            REQUIRE(oss.str() == "exptime1 1.25s\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("exptime1", {"0"}, -1, PUT, oss);
            REQUIRE(oss.str() == "exptime1 0\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setExptime(0, prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(proxy.Call("exptime1", {}, -1, GET));
    }
}

TEST_CASE("exptime2", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::MYTHEN3) {
        auto prev_val = det.getExptime(1);
        {
            std::ostringstream oss;
            proxy.Call("exptime2", {"1.25s"}, -1, PUT, oss);
            REQUIRE(oss.str() == "exptime2 1.25s\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("exptime2", {}, -1, GET, oss);
            REQUIRE(oss.str() == "exptime2 1.25s\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("exptime2", {"0"}, -1, PUT, oss);
            REQUIRE(oss.str() == "exptime2 0\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setExptime(1, prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(proxy.Call("exptime2", {}, -1, GET));
    }
}

TEST_CASE("exptime3", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::MYTHEN3) {
        auto prev_val = det.getExptime(2);
        {
            std::ostringstream oss;
            proxy.Call("exptime3", {"1.25s"}, -1, PUT, oss);
            REQUIRE(oss.str() == "exptime3 1.25s\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("exptime3", {}, -1, GET, oss);
            REQUIRE(oss.str() == "exptime3 1.25s\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("exptime3", {"0"}, -1, PUT, oss);
            REQUIRE(oss.str() == "exptime3 0\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setExptime(2, prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(proxy.Call("exptime3", {}, -1, GET));
    }
}

TEST_CASE("gatedelay", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::MYTHEN3) {
        auto prev_val = det.getExptimeForAllGates().tsquash(
            "inconsistent gatedelay to test");
        if (prev_val[0] != prev_val[1] || prev_val[1] != prev_val[2]) {
            throw sls::RuntimeError("inconsistent gatedelay for all gates");
        }
        {
            std::ostringstream oss;
            proxy.Call("gatedelay", {"0.05"}, -1, PUT, oss);
            REQUIRE(oss.str() == "gatedelay 0.05\n");
        }
        if (det_type != defs::MYTHEN3) {
            std::ostringstream oss;
            proxy.Call("gatedelay", {}, -1, GET, oss);
            REQUIRE(oss.str() == "gatedelay 50ms\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("gatedelay", {"1s"}, -1, PUT, oss);
            REQUIRE(oss.str() == "gatedelay 1s\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("gatedelay", {"0"}, -1, PUT, oss);
            REQUIRE(oss.str() == "gatedelay 0\n");
        }
        det.setGateDelay(-1, prev_val[0]);
    } else {
        REQUIRE_THROWS(proxy.Call("gatedelay", {}, -1, GET));
    }
}

TEST_CASE("gatedelay1", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::MYTHEN3) {
        auto prev_val = det.getGateDelay(0);
        {
            std::ostringstream oss;
            proxy.Call("gatedelay1", {"1.25s"}, -1, PUT, oss);
            REQUIRE(oss.str() == "gatedelay1 1.25s\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("gatedelay1", {}, -1, GET, oss);
            REQUIRE(oss.str() == "gatedelay1 1.25s\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("gatedelay1", {"0"}, -1, PUT, oss);
            REQUIRE(oss.str() == "gatedelay1 0\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setGateDelay(0, prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(proxy.Call("gatedelay1", {}, -1, GET));
    }
}

TEST_CASE("gatedelay2", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::MYTHEN3) {
        auto prev_val = det.getGateDelay(1);
        {
            std::ostringstream oss;
            proxy.Call("gatedelay2", {"1.25s"}, -1, PUT, oss);
            REQUIRE(oss.str() == "gatedelay2 1.25s\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("gatedelay2", {}, -1, GET, oss);
            REQUIRE(oss.str() == "gatedelay2 1.25s\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("gatedelay2", {"0"}, -1, PUT, oss);
            REQUIRE(oss.str() == "gatedelay2 0\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setGateDelay(1, prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(proxy.Call("gatedelay2", {}, -1, GET));
    }
}

TEST_CASE("gatedelay3", "[.cmd][.new]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::MYTHEN3) {
        auto prev_val = det.getGateDelay(2);
        {
            std::ostringstream oss;
            proxy.Call("gatedelay3", {"1.25s"}, -1, PUT, oss);
            REQUIRE(oss.str() == "gatedelay3 1.25s\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("gatedelay3", {}, -1, GET, oss);
            REQUIRE(oss.str() == "gatedelay3 1.25s\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("gatedelay3", {"0"}, -1, PUT, oss);
            REQUIRE(oss.str() == "gatedelay3 0\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setGateDelay(2, prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(proxy.Call("gatedelay3", {}, -1, GET));
    }
}