#include "CmdProxy.h"
#include "catch.hpp"
#include "sls/Detector.h"
#include "sls/sls_detector_defs.h"
#include <sstream>

#include "sls/versionAPI.h"
#include "test-CmdProxy-global.h"
#include "tests/globals.h"

using sls::CmdProxy;
using sls::Detector;
using test::GET;
using test::PUT;

/* dacs */

TEST_CASE("Setting and reading back Jungfrau dacs", "[.cmd][.dacs]") {
    // vb_comp, vdd_prot, vin_com, vref_prech, vb_pixbuf, vb_ds, vref_ds,
    // vref_comp
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU) {
        SECTION("vb_comp") { test_dac(defs::VB_COMP, "vb_comp", 1220); }
        SECTION("vdd_prot") { test_dac(defs::VDD_PROT, "vdd_prot", 3000); }
        SECTION("vin_com") { test_dac(defs::VIN_COM, "vin_com", 1053); }
        SECTION("vref_prech") {
            test_dac(defs::VREF_PRECH, "vref_prech", 1450);
        }
        SECTION("vb_pixbuf") { test_dac(defs::VB_PIXBUF, "vb_pixbuf", 750); }
        SECTION("vb_ds") { test_dac(defs::VB_DS, "vb_ds", 1000); }
        SECTION("vref_ds") { test_dac(defs::VREF_DS, "vref_ds", 480); }
        SECTION("vref_comp") { test_dac(defs::VREF_COMP, "vref_comp", 420); }
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

/* Network Configuration (Detector<->Receiver) */

TEST_CASE("selinterface", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU) {
        auto prev_val = det.getSelectedUDPInterface().tsquash(
            "inconsistent selected interface to test");
        {
            std::ostringstream oss;
            proxy.Call("selinterface", {"1"}, -1, PUT, oss);
            REQUIRE(oss.str() == "selinterface 1\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("selinterface", {"0"}, -1, PUT, oss);
            REQUIRE(oss.str() == "selinterface 0\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("selinterface", {}, -1, GET, oss);
            REQUIRE(oss.str() == "selinterface 0\n");
        }
        det.selectUDPInterface(prev_val);
        REQUIRE_THROWS(proxy.Call("selinterface", {"2"}, -1, PUT));
    } else {
        REQUIRE_THROWS(proxy.Call("selinterface", {}, -1, GET));
    }
}

/* Jungfrau Specific */

TEST_CASE("temp_threshold", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU) {
        auto prev_val = det.getThresholdTemperature();
        {
            std::ostringstream oss;
            proxy.Call("temp_threshold", {"65"}, -1, PUT, oss);
            REQUIRE(oss.str() == "temp_threshold 65\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("temp_threshold", {"70"}, -1, PUT, oss);
            REQUIRE(oss.str() == "temp_threshold 70\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("temp_threshold", {}, -1, GET, oss);
            REQUIRE(oss.str() == "temp_threshold 70\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setThresholdTemperature(prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(proxy.Call("temp_threshold", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("temp_threshold", {"70"}, -1, PUT));
    }
}

TEST_CASE("temp_control", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU) {
        auto prev_val = det.getTemperatureControl();
        {
            std::ostringstream oss;
            proxy.Call("temp_control", {"0"}, -1, PUT, oss);
            REQUIRE(oss.str() == "temp_control 0\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("temp_control", {"1"}, -1, PUT, oss);
            REQUIRE(oss.str() == "temp_control 1\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("temp_control", {}, -1, GET, oss);
            REQUIRE(oss.str() == "temp_control 1\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setTemperatureControl(prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(proxy.Call("temp_control", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("temp_control", {"0"}, -1, PUT));
    }
}

TEST_CASE("temp_event", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU) {
        {
            std::ostringstream oss;
            proxy.Call("temp_event", {"0"}, -1, PUT, oss);
            REQUIRE(oss.str() == "temp_event cleared\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("temp_event", {}, -1, GET, oss);
            REQUIRE(oss.str() == "temp_event 0\n");
        }
    } else {
        REQUIRE_THROWS(proxy.Call("temp_event", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("temp_event", {"0"}, -1, PUT));
    }
}

TEST_CASE("auto_comp_disable", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU) {
        auto prev_val = det.getAutoCompDisable();
        {
            std::ostringstream oss;
            proxy.Call("auto_comp_disable", {"0"}, -1, PUT, oss);
            REQUIRE(oss.str() == "auto_comp_disable 0\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("auto_comp_disable", {"1"}, -1, PUT, oss);
            REQUIRE(oss.str() == "auto_comp_disable 1\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("auto_comp_disable", {}, -1, GET, oss);
            REQUIRE(oss.str() == "auto_comp_disable 1\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setAutoCompDisable(prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(proxy.Call("auto_comp_disable", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("auto_comp_disable", {"0"}, -1, PUT));
    }
}

TEST_CASE("storagecells", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU) {
        auto prev_val = det.getNumberOfAdditionalStorageCells().tsquash(
            "inconsistent #additional storage cells to test");
        {
            std::ostringstream oss;
            proxy.Call("storagecells", {"1"}, -1, PUT, oss);
            REQUIRE(oss.str() == "storagecells 1\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("storagecells", {"15"}, -1, PUT, oss);
            REQUIRE(oss.str() == "storagecells 15\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("storagecells", {"0"}, -1, PUT, oss);
            REQUIRE(oss.str() == "storagecells 0\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("storagecells", {}, -1, GET, oss);
            REQUIRE(oss.str() == "storagecells 0\n");
        }
        REQUIRE_THROWS(proxy.Call("storagecells", {"16"}, -1, PUT));
        det.setNumberOfAdditionalStorageCells(prev_val);
    } else {
        REQUIRE_THROWS(proxy.Call("storagecells", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("storagecells", {"0"}, -1, PUT));
    }
}

TEST_CASE("storagecell_start", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU) {
        auto prev_val = det.getStorageCellStart();
        {
            std::ostringstream oss;
            proxy.Call("storagecell_start", {"1"}, -1, PUT, oss);
            REQUIRE(oss.str() == "storagecell_start 1\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("storagecell_start", {"15"}, -1, PUT, oss);
            REQUIRE(oss.str() == "storagecell_start 15\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("storagecell_start", {"0"}, -1, PUT, oss);
            REQUIRE(oss.str() == "storagecell_start 0\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("storagecell_start", {}, -1, GET, oss);
            REQUIRE(oss.str() == "storagecell_start 0\n");
        }
        REQUIRE_THROWS(proxy.Call("storagecell_start", {"16"}, -1, PUT));
        for (int i = 0; i != det.size(); ++i) {
            det.setStorageCellStart(prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(proxy.Call("storagecell_start", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("storagecell_start", {"0"}, -1, PUT));
    }
}

TEST_CASE("storagecell_delay", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU) {
        auto prev_val = det.getStorageCellDelay();
        {
            std::ostringstream oss;
            proxy.Call("storagecell_delay", {"1.62ms"}, -1, PUT, oss);
            REQUIRE(oss.str() == "storagecell_delay 1.62ms\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("storagecell_delay", {}, -1, GET, oss);
            REQUIRE(oss.str() == "storagecell_delay 1.62ms\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("storagecell_delay", {"0"}, -1, PUT, oss);
            REQUIRE(oss.str() == "storagecell_delay 0\n");
        }
        REQUIRE_THROWS(proxy.Call("storagecell_delay", {"1638376ns"}, -1, PUT));
        for (int i = 0; i != det.size(); ++i) {
            det.setStorageCellDelay(prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(proxy.Call("storagecell_delay", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("storagecell_delay", {"0"}, -1, PUT));
    }
}