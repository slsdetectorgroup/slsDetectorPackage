// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "CmdProxy.h"
#include "catch.hpp"
#include "sls/Detector.h"
#include "sls/sls_detector_defs.h"
#include <sstream>

#include "sls/versionAPI.h"
#include "test-CmdProxy-global.h"
#include "tests/globals.h"

namespace sls {

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
    if (det_type == defs::JUNGFRAU || det_type == defs::MOENCH) {
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

/* Jungfrau/moench Specific */

TEST_CASE("temp_threshold", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU || det_type == defs::MOENCH) {
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

TEST_CASE("chipversion", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU) {
        REQUIRE_NOTHROW(proxy.Call("chipversion", {}, -1, GET));
    } else {
        REQUIRE_THROWS(proxy.Call("chipversion", {}, -1, GET));
    }
    REQUIRE_THROWS(proxy.Call("chipversion", {"0"}, -1, PUT));
}

TEST_CASE("temp_control", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU || det_type == defs::MOENCH) {
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
    if (det_type == defs::JUNGFRAU || det_type == defs::MOENCH) {
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

TEST_CASE("autocompdisable", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU) {
        auto prev_val = det.getAutoComparatorDisable();
        {
            std::ostringstream oss;
            proxy.Call("autocompdisable", {"0"}, -1, PUT, oss);
            REQUIRE(oss.str() == "autocompdisable 0\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("autocompdisable", {"1"}, -1, PUT, oss);
            REQUIRE(oss.str() == "autocompdisable 1\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("autocompdisable", {}, -1, GET, oss);
            REQUIRE(oss.str() == "autocompdisable 1\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setAutoComparatorDisable(prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(proxy.Call("autocompdisable", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("autocompdisable", {"0"}, -1, PUT));
    }
}

TEST_CASE("compdisabletime", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU &&
        det.getChipVersion().squash() * 10 == 11) {
        auto prev_val = det.getComparatorDisableTime();
        {
            std::ostringstream oss;
            proxy.Call("compdisabletime", {"125ns"}, -1, PUT, oss);
            REQUIRE(oss.str() == "compdisabletime 125ns\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("compdisabletime", {}, -1, GET, oss);
            REQUIRE(oss.str() == "compdisabletime 125ns\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("compdisabletime", {"0"}, -1, PUT, oss);
            REQUIRE(oss.str() == "compdisabletime 0\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setComparatorDisableTime(prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(proxy.Call("compdisabletime", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("compdisabletime", {"0"}, -1, PUT));
    }
}

TEST_CASE("extrastoragecells", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU) {
        // chip version 1.0
        if (det.getChipVersion().squash() * 10 == 10) {
            auto prev_val = det.getNumberOfAdditionalStorageCells().tsquash(
                "inconsistent #additional storage cells to test");
            {
                std::ostringstream oss;
                proxy.Call("extrastoragecells", {"1"}, -1, PUT, oss);
                REQUIRE(oss.str() == "extrastoragecells 1\n");
            }
            {
                std::ostringstream oss;
                proxy.Call("extrastoragecells", {"15"}, -1, PUT, oss);
                REQUIRE(oss.str() == "extrastoragecells 15\n");
            }
            {
                std::ostringstream oss;
                proxy.Call("extrastoragecells", {"0"}, -1, PUT, oss);
                REQUIRE(oss.str() == "extrastoragecells 0\n");
            }
            {
                std::ostringstream oss;
                proxy.Call("extrastoragecells", {}, -1, GET, oss);
                REQUIRE(oss.str() == "extrastoragecells 0\n");
            }
            REQUIRE_THROWS(proxy.Call("extrastoragecells", {"16"}, -1, PUT));
            det.setNumberOfAdditionalStorageCells(prev_val);
        }
        // chip version 1.1
        else {
            // cannot set number of addl. storage cells
            REQUIRE_THROWS(proxy.Call("extrastoragecells", {"1"}, -1, PUT));
        }
    } else {
        REQUIRE_THROWS(proxy.Call("extrastoragecells", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("extrastoragecells", {"0"}, -1, PUT));
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
        // chip version 1.0
        if (det.getChipVersion().squash() * 10 == 10) {
            std::ostringstream oss;
            proxy.Call("storagecell_start", {"15"}, -1, PUT, oss);
            REQUIRE(oss.str() == "storagecell_start 15\n");
        }
        // chip version 1.1
        else {
            // max is 3
            REQUIRE_THROWS(proxy.Call("storagecell_start", {"15"}, -1, PUT));
            std::ostringstream oss;
            proxy.Call("storagecell_start", {"3"}, -1, PUT, oss);
            REQUIRE(oss.str() == "storagecell_start 3\n");
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
        // chip version 1.0
        if (det.getChipVersion().squash() * 10 == 10) {
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
            REQUIRE_THROWS(
                proxy.Call("storagecell_delay", {"1638376ns"}, -1, PUT));
            for (int i = 0; i != det.size(); ++i) {
                det.setStorageCellDelay(prev_val[i], {i});
            }
        }
        // chip version 1.1
        else {
            // cannot set storage cell delay
            REQUIRE_THROWS(
                proxy.Call("storagecell_delay", {"1.62ms"}, -1, PUT));
        }
    } else {
        REQUIRE_THROWS(proxy.Call("storagecell_delay", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("storagecell_delay", {"0"}, -1, PUT));
    }
}

TEST_CASE("gainmode", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU) {
        auto prev_val = det.getGainMode();
        {
            std::ostringstream oss;
            proxy.Call("gainmode", {"forceswitchg1"}, -1, PUT, oss);
            REQUIRE(oss.str() == "gainmode forceswitchg1\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("gainmode", {}, -1, GET, oss);
            REQUIRE(oss.str() == "gainmode forceswitchg1\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("gainmode", {"dynamic"}, -1, PUT, oss);
            REQUIRE(oss.str() == "gainmode dynamic\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("gainmode", {"forceswitchg2"}, -1, PUT, oss);
            REQUIRE(oss.str() == "gainmode forceswitchg2\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("gainmode", {"fixg1"}, -1, PUT, oss);
            REQUIRE(oss.str() == "gainmode fixg1\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("gainmode", {"fixg2"}, -1, PUT, oss);
            REQUIRE(oss.str() == "gainmode fixg2\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("gainmode", {"fixg0"}, -1, PUT, oss);
            REQUIRE(oss.str() == "gainmode fixg0\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setGainMode(prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(proxy.Call("gainmode", {}, -1, GET));
    }
}

TEST_CASE("filtercells", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU) {
        // chip version 1.1
        if (det.getChipVersion().squash() * 10 == 11) {
            auto prev_val = det.getNumberOfFilterCells();
            {
                std::ostringstream oss;
                proxy.Call("filtercells", {"1"}, -1, PUT, oss);
                REQUIRE(oss.str() == "filtercells 1\n");
            }
            {
                std::ostringstream oss;
                proxy.Call("filtercells", {"12"}, -1, PUT, oss);
                REQUIRE(oss.str() == "filtercells 12\n");
            }
            {
                std::ostringstream oss;
                proxy.Call("filtercells", {"0"}, -1, PUT, oss);
                REQUIRE(oss.str() == "filtercells 0\n");
            }
            {
                std::ostringstream oss;
                proxy.Call("filtercells", {}, -1, GET, oss);
                REQUIRE(oss.str() == "filtercells 0\n");
            }
            REQUIRE_THROWS(proxy.Call("filtercells", {"13"}, -1, PUT));
            for (int i = 0; i != det.size(); ++i) {
                det.setNumberOfFilterCells(prev_val[i], {i});
            }
        }
        // chip version 1.0
        else {
            // cannot set/get filter cell
            REQUIRE_THROWS(proxy.Call("filtercells", {"1"}, -1, PUT));
            REQUIRE_THROWS(proxy.Call("filtercells", {}, -1, GET));
        }
    } else {
        REQUIRE_THROWS(proxy.Call("filtercells", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("filtercells", {"0"}, -1, PUT));
    }
}

TEST_CASE("pedestalmode", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU) {
        auto prev_val = det.getPedestalMode();
        auto prev_frames = det.getNumberOfFrames().tsquash(
            "Inconsistent number of frames to test");
        auto prev_triggers = det.getNumberOfTriggers().tsquash(
            "Inconsistent number of triggers to test");
        auto prev_timingmode =
            det.getTimingMode().tsquash("Inconsistent timing mode to test");

        REQUIRE_NOTHROW(proxy.Call("pedestalmode", {}, 0, GET));
        REQUIRE_NOTHROW(proxy.Call("pedestalmode", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("pedestalmode", {"0"}, -1, GET));

        REQUIRE_THROWS(proxy.Call("pedestalmode", {"256", "10"}, -1, PUT));
        REQUIRE_THROWS(proxy.Call("pedestalmode", {"-1", "10"}, 0, PUT));
        REQUIRE_THROWS(proxy.Call("pedestalmode", {"20", "65536"}, 0, PUT));
        REQUIRE_THROWS(proxy.Call("pedestalmode", {"20", "-1"}, 0, PUT));

        {
            std::ostringstream oss;
            proxy.Call("pedestalmode", {"30", "1000"}, -1, PUT, oss);
            REQUIRE(oss.str() == "pedestalmode [30, 1000]\n");
        }
        // cannot change any of these in pedestal mode
        REQUIRE_THROWS_WITH(proxy.Call("frames", {"200"}, -1, PUT),
                            "Detector returned: Cannot set frames in pedestal "
                            "mode. It is overwritten anyway.\n");
        REQUIRE_THROWS_WITH(proxy.Call("triggers", {"200"}, -1, PUT),
                            "Detector returned: Cannot set triggers in "
                            "pedestal mode. It is overwritten anyway.\n");
        REQUIRE_THROWS_WITH(
            proxy.Call("timing", {"auto"}, -1, PUT),
            "Detector returned: Cannot set timing mode in pedestal mode. "
            "Switch off pedestal mode to change timing mode.\n");
        REQUIRE_THROWS_WITH(
            proxy.Call("scan", {"vb_comp", "500", "1500", "10"}, -1, PUT),
            "Detector returned: Cannot set scan when in pedestal mode.\n");
        REQUIRE_THROWS_WITH(
            proxy.Call("scan", {"0"}, -1, PUT),
            "Detector returned: Cannot set scan when in pedestal mode.\n");
        // should not throw to get these values though
        REQUIRE_NOTHROW(proxy.Call("frames", {}, -1, GET));
        REQUIRE_NOTHROW(proxy.Call("triggers", {}, -1, GET));
        REQUIRE_NOTHROW(proxy.Call("timing", {}, -1, GET));
        REQUIRE_NOTHROW(proxy.Call("scan", {}, -1, GET));

        {
            std::ostringstream oss;
            proxy.Call("pedestalmode", {"50", "500"}, -1, PUT, oss);
            REQUIRE(oss.str() == "pedestalmode [50, 500]\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("pedestalmode", {}, -1, GET, oss);
            REQUIRE(oss.str() == "pedestalmode [enabled, 50, 500]\n");
        }
        {
            auto pedemode = det.getPedestalMode().tsquash(
                "Inconsistent pedestal mode to test");
            REQUIRE(pedemode.enable == true);
            REQUIRE(pedemode.frames == 50);
            REQUIRE(pedemode.loops == 500);
        }
        {
            std::ostringstream oss;
            proxy.Call("pedestalmode", {"0"}, -1, PUT, oss);
            REQUIRE(oss.str() == "pedestalmode [0]\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("pedestalmode", {}, -1, GET, oss);
            REQUIRE(oss.str() == "pedestalmode [disabled]\n");
        }

        uint8_t pedestalFrames = 50;
        uint16_t pedestalLoops = 1000;
        int64_t expNumFrames = pedestalFrames * pedestalLoops * 2;
        auto origFrames = det.getNumberOfFrames().squash(-1);
        auto origTriggers = det.getNumberOfTriggers().squash(-1);

        // auto mode
        det.setTimingMode(defs::AUTO_TIMING);
        REQUIRE_NOTHROW(proxy.Call(
            "pedestalmode",
            {std::to_string(pedestalFrames), std::to_string(pedestalLoops)}, -1,
            PUT));
        auto numTriggers = det.getNumberOfTriggers().squash(-1);
        auto numFrames = det.getNumberOfFrames().squash(-1);
        REQUIRE(numFrames == expNumFrames);
        REQUIRE(numTriggers == 1);

        // pedestal mode off
        REQUIRE_NOTHROW(proxy.Call("pedestalmode", {"0"}, -1, PUT));
        numTriggers = det.getNumberOfTriggers().squash(-1);
        numFrames = det.getNumberOfFrames().squash(-1);
        REQUIRE(numFrames == origFrames);
        REQUIRE(numTriggers == origTriggers);

        // trigger mode (frames > 1)
        REQUIRE_NOTHROW(det.setTimingMode(defs::TRIGGER_EXPOSURE));
        origFrames = 5;
        REQUIRE_NOTHROW(det.setNumberOfFrames(origFrames));
        REQUIRE_NOTHROW(proxy.Call(
            "pedestalmode",
            {std::to_string(pedestalFrames), std::to_string(pedestalLoops)}, -1,
            PUT));
        numTriggers = det.getNumberOfTriggers().squash(-1);
        numFrames = det.getNumberOfFrames().squash(-1);
        REQUIRE(numFrames == expNumFrames);
        REQUIRE(numTriggers == 1);

        // pedestal mode off
        REQUIRE_NOTHROW(proxy.Call("pedestalmode", {"0"}, -1, PUT));
        numTriggers = det.getNumberOfTriggers().squash(-1);
        numFrames = det.getNumberOfFrames().squash(-1);
        REQUIRE(numFrames == origFrames);
        REQUIRE(numTriggers == origTriggers);

        // trigger mode (frames = 1)
        origFrames = 1;
        REQUIRE_NOTHROW(det.setNumberOfFrames(origFrames));
        origTriggers = 10;
        REQUIRE_NOTHROW(det.setNumberOfTriggers(origTriggers));
        REQUIRE_NOTHROW(proxy.Call(
            "pedestalmode",
            {std::to_string(pedestalFrames), std::to_string(pedestalLoops)}, -1,
            PUT));
        numTriggers = det.getNumberOfTriggers().squash(-1);
        numFrames = det.getNumberOfFrames().squash(-1);
        REQUIRE(numFrames == 1);
        REQUIRE(numTriggers == expNumFrames);

        // pedestal mode off
        REQUIRE_NOTHROW(proxy.Call("pedestalmode", {"0"}, -1, PUT));
        numTriggers = det.getNumberOfTriggers().squash(-1);
        numFrames = det.getNumberOfFrames().squash(-1);
        REQUIRE(numFrames == origFrames);
        REQUIRE(numTriggers == origTriggers);

        det.setNumberOfFrames(prev_frames);
        det.setNumberOfTriggers(prev_triggers);
        det.setTimingMode(prev_timingmode);
        for (int i = 0; i != det.size(); ++i) {
            det.setPedestalMode(prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(proxy.Call("pedestalmode", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("pedestalmode", {"0"}, -1, PUT));
    }
}

TEST_CASE("sync", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU || det_type == defs::MOENCH) {
        auto prev_val = det.getSynchronization().tsquash(
            "inconsistent synchronization to test");
        {
            std::ostringstream oss;
            proxy.Call("sync", {"0"}, -1, PUT, oss);
            REQUIRE(oss.str() == "sync 0\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("sync", {"1"}, -1, PUT, oss);
            REQUIRE(oss.str() == "sync 1\n");
        }
        // setting to master or slave when synced
        {
            // get previous master
            int prevMaster = 0;
            auto previous = det.getMaster();
            for (int i = 0; i != det.size(); ++i) {
                if (previous[i] == 1) {
                    prevMaster = i;
                    break;
                }
            }
            proxy.Call("master", {"1"}, 0, PUT);
            proxy.Call("master", {"0"}, 0, PUT);
            std::ostringstream oss;
            proxy.Call("status", {}, -1, GET, oss);
            REQUIRE(oss.str() != "status running\n");
            // set all to slaves, and then master
            for (int i = 0; i != det.size(); ++i) {
                det.setMaster(0, {i});
            }
            det.setMaster(1, prevMaster);
        }
        {
            std::ostringstream oss;
            proxy.Call("sync", {}, -1, GET, oss);
            REQUIRE(oss.str() == "sync 1\n");
        }
        // setting sync when running
        {
            auto prev_timing =
                det.getTimingMode().tsquash("inconsistent timing mode in test");
            auto prev_frames =
                det.getNumberOfFrames().tsquash("inconsistent #frames in test");
            auto prev_exptime =
                det.getExptime().tsquash("inconsistent exptime in test");
            auto prev_period =
                det.getPeriod().tsquash("inconsistent period in test");
            det.setTimingMode(defs::AUTO_TIMING);
            det.setNumberOfFrames(10000);
            det.setExptime(std::chrono::microseconds(200));
            det.setPeriod(std::chrono::milliseconds(1000));
            det.setSynchronization(1);
            det.startDetector();
            REQUIRE_THROWS(proxy.Call("sync", {"0"}, -1, PUT));
            {
                std::ostringstream oss;
                proxy.Call("sync", {}, -1, GET, oss);
                REQUIRE(oss.str() == "sync 1\n");
            }
            det.stopDetector();
            det.setTimingMode(prev_timing);
            det.setNumberOfFrames(prev_frames);
            det.setExptime(prev_exptime);
            det.setPeriod(prev_period);
        }
        det.setSynchronization(prev_val);
    } else {
        REQUIRE_THROWS(proxy.Call("sync", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("sync", {"0"}, -1, PUT));
    }
}

} // namespace sls
