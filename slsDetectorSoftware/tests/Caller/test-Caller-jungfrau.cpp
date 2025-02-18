// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "Caller.h"
#include "catch.hpp"
#include "sls/Detector.h"
#include "sls/sls_detector_defs.h"
#include <sstream>

#include "sls/versionAPI.h"
#include "test-Caller-global.h"
#include "tests/globals.h"

namespace sls {

using test::GET;
using test::PUT;

/* dacs */

TEST_CASE("Setting and reading back Jungfrau dacs", "[.cmdcall][.dacs]") {
    // vb_comp, vdd_prot, vin_com, vref_prech, vb_pixbuf, vb_ds, vref_ds,
    // vref_comp
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU) {
        SECTION("vb_comp") { test_dac_caller(defs::VB_COMP, "vb_comp", 1220); }
        SECTION("vdd_prot") {
            test_dac_caller(defs::VDD_PROT, "vdd_prot", 3000);
        }
        SECTION("vin_com") { test_dac_caller(defs::VIN_COM, "vin_com", 1053); }
        SECTION("vref_prech") {
            test_dac_caller(defs::VREF_PRECH, "vref_prech", 1450);
        }
        SECTION("vb_pixbuf") {
            test_dac_caller(defs::VB_PIXBUF, "vb_pixbuf", 750);
        }
        SECTION("vb_ds") { test_dac_caller(defs::VB_DS, "vb_ds", 1000); }
        SECTION("vref_ds") { test_dac_caller(defs::VREF_DS, "vref_ds", 480); }
        SECTION("vref_comp") {
            test_dac_caller(defs::VREF_COMP, "vref_comp", 420);
        }
        // eiger
        REQUIRE_THROWS(caller.call("vthreshold", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vsvp", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vsvn", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vtrim", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vrpreamp", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vrshaper", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vtgstv", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vcmp_ll", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vcmp_lr", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vcal", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vcmp_rl", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vcmp_rr", {}, -1, GET));
        REQUIRE_THROWS(caller.call("rxb_rb", {}, -1, GET));
        REQUIRE_THROWS(caller.call("rxb_lb", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vcp", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vcn", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vishaper", {}, -1, GET));
        REQUIRE_THROWS(caller.call("iodelay", {}, -1, GET));
        // gotthard
        // REQUIRE_THROWS(caller.call("vref_ds", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vcascn_pb", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vcascp_pb", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vout_cm", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vcasc_out", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vin_cm", {}, -1, GET));
        // REQUIRE_THROWS(caller.call("vref_comp", {}, -1, GET));
        REQUIRE_THROWS(caller.call("ib_test_c", {}, -1, GET));
        // mythen3
        REQUIRE_THROWS(caller.call("vrpreamp", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vrshaper", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vrshaper_n", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vipre", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vishaper", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vdcsh", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vth1", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vth2", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vth3", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vcal_n", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vcal_p", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vtrim", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vcassh", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vcas", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vicin", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vipre_out", {}, -1, GET));
        // gotthard2
        REQUIRE_THROWS(caller.call("vref_h_adc", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vb_comp_fe", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vb_comp_adc", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vcom_cds", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vref_rstore", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vb_opa_1st", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vref_comp_fe", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vcom_adc1", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vref_l_adc", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vref_cds", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vb_cs", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vb_opa_fd", {}, -1, GET));
        REQUIRE_THROWS(caller.call("vcom_adc2", {}, -1, GET));
    }
}

/* Network Configuration (Detector<->Receiver) */

TEST_CASE("selinterface", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU || det_type == defs::MOENCH) {
        auto prev_val = det.getSelectedUDPInterface().tsquash(
            "inconsistent selected interface to test");
        {
            std::ostringstream oss;
            caller.call("selinterface", {"1"}, -1, PUT, oss);
            REQUIRE(oss.str() == "selinterface 1\n");
        }
        {
            std::ostringstream oss;
            caller.call("selinterface", {"0"}, -1, PUT, oss);
            REQUIRE(oss.str() == "selinterface 0\n");
        }
        {
            std::ostringstream oss;
            caller.call("selinterface", {}, -1, GET, oss);
            REQUIRE(oss.str() == "selinterface 0\n");
        }
        det.selectUDPInterface(prev_val);
        REQUIRE_THROWS(caller.call("selinterface", {"2"}, -1, PUT));
    } else {
        REQUIRE_THROWS(caller.call("selinterface", {}, -1, GET));
    }
}

/* Jungfrau/moench Specific */

TEST_CASE("temp_threshold", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU || det_type == defs::MOENCH) {
        auto prev_val = det.getThresholdTemperature();
        {
            std::ostringstream oss;
            caller.call("temp_threshold", {"65"}, -1, PUT, oss);
            REQUIRE(oss.str() == "temp_threshold 65\n");
        }
        {
            std::ostringstream oss;
            caller.call("temp_threshold", {"70"}, -1, PUT, oss);
            REQUIRE(oss.str() == "temp_threshold 70\n");
        }
        {
            std::ostringstream oss;
            caller.call("temp_threshold", {}, -1, GET, oss);
            REQUIRE(oss.str() == "temp_threshold 70\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setThresholdTemperature(prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(caller.call("temp_threshold", {}, -1, GET));
        REQUIRE_THROWS(caller.call("temp_threshold", {"70"}, -1, PUT));
    }
}

TEST_CASE("chipversion", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU) {
        REQUIRE_NOTHROW(caller.call("chipversion", {}, -1, GET));
    } else {
        REQUIRE_THROWS(caller.call("chipversion", {}, -1, GET));
    }
    REQUIRE_THROWS(caller.call("chipversion", {"0"}, -1, PUT));
}

TEST_CASE("temp_control", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU || det_type == defs::MOENCH) {
        auto prev_val = det.getTemperatureControl();
        {
            std::ostringstream oss;
            caller.call("temp_control", {"0"}, -1, PUT, oss);
            REQUIRE(oss.str() == "temp_control 0\n");
        }
        {
            std::ostringstream oss;
            caller.call("temp_control", {"1"}, -1, PUT, oss);
            REQUIRE(oss.str() == "temp_control 1\n");
        }
        {
            std::ostringstream oss;
            caller.call("temp_control", {}, -1, GET, oss);
            REQUIRE(oss.str() == "temp_control 1\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setTemperatureControl(prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(caller.call("temp_control", {}, -1, GET));
        REQUIRE_THROWS(caller.call("temp_control", {"0"}, -1, PUT));
    }
}

TEST_CASE("temp_event", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU || det_type == defs::MOENCH) {
        {
            std::ostringstream oss;
            caller.call("temp_event", {"0"}, -1, PUT, oss);
            REQUIRE(oss.str() == "temp_event cleared\n");
        }
        {
            std::ostringstream oss;
            caller.call("temp_event", {}, -1, GET, oss);
            REQUIRE(oss.str() == "temp_event 0\n");
        }
    } else {
        REQUIRE_THROWS(caller.call("temp_event", {}, -1, GET));
        REQUIRE_THROWS(caller.call("temp_event", {"0"}, -1, PUT));
    }
}

TEST_CASE("autocompdisable", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU) {
        auto prev_val = det.getAutoComparatorDisable();
        {
            std::ostringstream oss;
            caller.call("autocompdisable", {"0"}, -1, PUT, oss);
            REQUIRE(oss.str() == "autocompdisable 0\n");
        }
        {
            std::ostringstream oss;
            caller.call("autocompdisable", {"1"}, -1, PUT, oss);
            REQUIRE(oss.str() == "autocompdisable 1\n");
        }
        {
            std::ostringstream oss;
            caller.call("autocompdisable", {}, -1, GET, oss);
            REQUIRE(oss.str() == "autocompdisable 1\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setAutoComparatorDisable(prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(caller.call("autocompdisable", {}, -1, GET));
        REQUIRE_THROWS(caller.call("autocompdisable", {"0"}, -1, PUT));
    }
}

TEST_CASE("compdisabletime", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU) {
        auto prev_val = det.getComparatorDisableTime();
        {
            std::ostringstream oss;
            caller.call("compdisabletime", {"125ns"}, -1, PUT, oss);
            REQUIRE(oss.str() == "compdisabletime 125ns\n");
        }
        {
            std::ostringstream oss;
            caller.call("compdisabletime", {}, -1, GET, oss);
            REQUIRE(oss.str() == "compdisabletime 125ns\n");
        }
        {
            std::ostringstream oss;
            caller.call("compdisabletime", {"0"}, -1, PUT, oss);
            REQUIRE(oss.str() == "compdisabletime 0\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setComparatorDisableTime(prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(caller.call("compdisabletime", {}, -1, GET));
        REQUIRE_THROWS(caller.call("compdisabletime", {"0"}, -1, PUT));
    }
}

TEST_CASE("extrastoragecells", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU) {
        // chip version 1.0
        if (det.getChipVersion().squash() * 10 == 10) {
            auto prev_val = det.getNumberOfAdditionalStorageCells().tsquash(
                "inconsistent #additional storage cells to test");
            {
                std::ostringstream oss;
                caller.call("extrastoragecells", {"1"}, -1, PUT, oss);
                REQUIRE(oss.str() == "extrastoragecells 1\n");
            }
            {
                std::ostringstream oss;
                caller.call("extrastoragecells", {"15"}, -1, PUT, oss);
                REQUIRE(oss.str() == "extrastoragecells 15\n");
            }
            {
                std::ostringstream oss;
                caller.call("extrastoragecells", {"0"}, -1, PUT, oss);
                REQUIRE(oss.str() == "extrastoragecells 0\n");
            }
            {
                std::ostringstream oss;
                caller.call("extrastoragecells", {}, -1, GET, oss);
                REQUIRE(oss.str() == "extrastoragecells 0\n");
            }
            REQUIRE_THROWS(caller.call("extrastoragecells", {"16"}, -1, PUT));
            det.setNumberOfAdditionalStorageCells(prev_val);
        }
        // chip version 1.1
        else {
            // cannot set number of addl. storage cells
            REQUIRE_THROWS(caller.call("extrastoragecells", {"1"}, -1, PUT));
        }
    } else {
        REQUIRE_THROWS(caller.call("extrastoragecells", {}, -1, GET));
        REQUIRE_THROWS(caller.call("extrastoragecells", {"0"}, -1, PUT));
    }
}

TEST_CASE("storagecell_start", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU) {
        auto prev_val = det.getStorageCellStart();
        {
            std::ostringstream oss;
            caller.call("storagecell_start", {"1"}, -1, PUT, oss);
            REQUIRE(oss.str() == "storagecell_start 1\n");
        }
        // chip version 1.0
        if (det.getChipVersion().squash() * 10 == 10) {
            std::ostringstream oss;
            caller.call("storagecell_start", {"15"}, -1, PUT, oss);
            REQUIRE(oss.str() == "storagecell_start 15\n");
        }
        // chip version 1.1
        else {
            // max is 3
            REQUIRE_THROWS(caller.call("storagecell_start", {"15"}, -1, PUT));
            std::ostringstream oss;
            caller.call("storagecell_start", {"3"}, -1, PUT, oss);
            REQUIRE(oss.str() == "storagecell_start 3\n");
        }
        {
            std::ostringstream oss;
            caller.call("storagecell_start", {"0"}, -1, PUT, oss);
            REQUIRE(oss.str() == "storagecell_start 0\n");
        }
        {
            std::ostringstream oss;
            caller.call("storagecell_start", {}, -1, GET, oss);
            REQUIRE(oss.str() == "storagecell_start 0\n");
        }
        REQUIRE_THROWS(caller.call("storagecell_start", {"16"}, -1, PUT));
        for (int i = 0; i != det.size(); ++i) {
            det.setStorageCellStart(prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(caller.call("storagecell_start", {}, -1, GET));
        REQUIRE_THROWS(caller.call("storagecell_start", {"0"}, -1, PUT));
    }
}

TEST_CASE("storagecell_delay", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU) {
        // chip version 1.0
        if (det.getChipVersion().squash() * 10 == 10) {
            auto prev_val = det.getStorageCellDelay();
            {
                std::ostringstream oss;
                caller.call("storagecell_delay", {"1.62ms"}, -1, PUT, oss);
                REQUIRE(oss.str() == "storagecell_delay 1.62ms\n");
            }
            {
                std::ostringstream oss;
                caller.call("storagecell_delay", {}, -1, GET, oss);
                REQUIRE(oss.str() == "storagecell_delay 1.62ms\n");
            }
            {
                std::ostringstream oss;
                caller.call("storagecell_delay", {"0"}, -1, PUT, oss);
                REQUIRE(oss.str() == "storagecell_delay 0\n");
            }
            REQUIRE_THROWS(
                caller.call("storagecell_delay", {"1638376ns"}, -1, PUT));
            for (int i = 0; i != det.size(); ++i) {
                det.setStorageCellDelay(prev_val[i], {i});
            }
        }
        // chip version 1.1
        else {
            // cannot set storage cell delay
            REQUIRE_THROWS(
                caller.call("storagecell_delay", {"1.62ms"}, -1, PUT));
        }
    } else {
        REQUIRE_THROWS(caller.call("storagecell_delay", {}, -1, GET));
        REQUIRE_THROWS(caller.call("storagecell_delay", {"0"}, -1, PUT));
    }
}

TEST_CASE("gainmode", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU) {
        auto prev_val = det.getGainMode();
        {
            std::ostringstream oss;
            caller.call("gainmode", {"forceswitchg1"}, -1, PUT, oss);
            REQUIRE(oss.str() == "gainmode forceswitchg1\n");
        }
        {
            std::ostringstream oss;
            caller.call("gainmode", {}, -1, GET, oss);
            REQUIRE(oss.str() == "gainmode forceswitchg1\n");
        }
        {
            std::ostringstream oss;
            caller.call("gainmode", {"dynamic"}, -1, PUT, oss);
            REQUIRE(oss.str() == "gainmode dynamic\n");
        }
        {
            std::ostringstream oss;
            caller.call("gainmode", {"forceswitchg2"}, -1, PUT, oss);
            REQUIRE(oss.str() == "gainmode forceswitchg2\n");
        }
        {
            std::ostringstream oss;
            caller.call("gainmode", {"fixg1"}, -1, PUT, oss);
            REQUIRE(oss.str() == "gainmode fixg1\n");
        }
        {
            std::ostringstream oss;
            caller.call("gainmode", {"fixg2"}, -1, PUT, oss);
            REQUIRE(oss.str() == "gainmode fixg2\n");
        }
        {
            std::ostringstream oss;
            caller.call("gainmode", {"fixg0"}, -1, PUT, oss);
            REQUIRE(oss.str() == "gainmode fixg0\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setGainMode(prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(caller.call("gainmode", {}, -1, GET));
    }
}

TEST_CASE("filtercells", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU) {
        // chip version 1.1
        if (det.getChipVersion().squash() * 10 == 11) {
            auto prev_val = det.getNumberOfFilterCells();
            {
                std::ostringstream oss;
                caller.call("filtercells", {"1"}, -1, PUT, oss);
                REQUIRE(oss.str() == "filtercells 1\n");
            }
            {
                std::ostringstream oss;
                caller.call("filtercells", {"12"}, -1, PUT, oss);
                REQUIRE(oss.str() == "filtercells 12\n");
            }
            {
                std::ostringstream oss;
                caller.call("filtercells", {"0"}, -1, PUT, oss);
                REQUIRE(oss.str() == "filtercells 0\n");
            }
            {
                std::ostringstream oss;
                caller.call("filtercells", {}, -1, GET, oss);
                REQUIRE(oss.str() == "filtercells 0\n");
            }
            REQUIRE_THROWS(caller.call("filtercells", {"13"}, -1, PUT));
            for (int i = 0; i != det.size(); ++i) {
                det.setNumberOfFilterCells(prev_val[i], {i});
            }
        }
        // chip version 1.0
        else {
            // cannot set/get filter cell
            REQUIRE_THROWS(caller.call("filtercells", {"1"}, -1, PUT));
            REQUIRE_THROWS(caller.call("filtercells", {}, -1, GET));
        }
    } else {
        REQUIRE_THROWS(caller.call("filtercells", {}, -1, GET));
        REQUIRE_THROWS(caller.call("filtercells", {"0"}, -1, PUT));
    }
}
TEST_CASE("pedestalmode", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU) {
        auto prev_val = det.getPedestalMode();
        auto prev_frames = det.getNumberOfFrames().tsquash(
            "Inconsistent number of frames to test");
        auto prev_triggers = det.getNumberOfTriggers().tsquash(
            "Inconsistent number of triggers to test");
        auto prev_timingmode =
            det.getTimingMode().tsquash("Inconsistent timing mode to test");

        REQUIRE_NOTHROW(caller.call("pedestalmode", {}, 0, GET));
        REQUIRE_NOTHROW(caller.call("pedestalmode", {}, -1, GET));
        REQUIRE_THROWS(caller.call("pedestalmode", {"0"}, -1, GET));

        REQUIRE_THROWS(caller.call("pedestalmode", {"256", "10"}, -1, PUT));
        REQUIRE_THROWS(caller.call("pedestalmode", {"-1", "10"}, 0, PUT));
        REQUIRE_THROWS(caller.call("pedestalmode", {"20", "1000"}, 0, PUT));
        REQUIRE_THROWS(caller.call("pedestalmode", {"2000", "100"}, 0, PUT));
        REQUIRE_THROWS(caller.call("pedestalmode", {"20", "-1"}, 0, PUT));

        {
            std::ostringstream oss;
            caller.call("pedestalmode", {"30", "100"}, -1, PUT, oss);
            REQUIRE(oss.str() == "pedestalmode [30, 100]\n");
        }
        // cannot change any of these in pedestal mode
        REQUIRE_THROWS_WITH(caller.call("frames", {"200"}, -1, PUT),
                            "Detector returned: Cannot set frames in pedestal "
                            "mode. It is overwritten anyway.\n");
        REQUIRE_THROWS_WITH(caller.call("triggers", {"200"}, -1, PUT),
                            "Detector returned: Cannot set triggers in "
                            "pedestal mode. It is overwritten anyway.\n");
        REQUIRE_THROWS_WITH(
            caller.call("timing", {"auto"}, -1, PUT),
            "Detector returned: Cannot set timing mode in pedestal mode. "
            "Switch off pedestal mode to change timing mode.\n");
        REQUIRE_THROWS_WITH(
            caller.call("scan", {"vb_comp", "500", "1500", "10"}, -1, PUT),
            "Detector returned: Cannot set scan when in pedestal mode.\n");
        REQUIRE_THROWS_WITH(
            caller.call("scan", {"0"}, -1, PUT),
            "Detector returned: Cannot set scan when in pedestal mode.\n");
        // should not throw to get these values though
        REQUIRE_NOTHROW(caller.call("frames", {}, -1, GET));
        REQUIRE_NOTHROW(caller.call("triggers", {}, -1, GET));
        REQUIRE_NOTHROW(caller.call("timing", {}, -1, GET));
        REQUIRE_NOTHROW(caller.call("scan", {}, -1, GET));

        {
            std::ostringstream oss;
            caller.call("pedestalmode", {"50", "100"}, -1, PUT, oss);
            REQUIRE(oss.str() == "pedestalmode [50, 100]\n");
        }
        {
            std::ostringstream oss;
            caller.call("pedestalmode", {}, -1, GET, oss);
            REQUIRE(oss.str() == "pedestalmode [enabled, 50, 100]\n");
        }
        {
            auto pedemode = det.getPedestalMode().tsquash(
                "Inconsistent pedestal mode to test");
            REQUIRE(pedemode.enable == true);
            REQUIRE(pedemode.frames == 50);
            REQUIRE(pedemode.loops == 100);
        }
        {
            std::ostringstream oss;
            caller.call("pedestalmode", {"0"}, -1, PUT, oss);
            REQUIRE(oss.str() == "pedestalmode [0]\n");
        }
        {
            std::ostringstream oss;
            caller.call("pedestalmode", {}, -1, GET, oss);
            REQUIRE(oss.str() == "pedestalmode [disabled]\n");
        }

        uint8_t pedestalFrames = 50;
        uint16_t pedestalLoops = 100;
        int64_t expNumFrames = pedestalFrames * pedestalLoops * 2;
        auto origFrames = det.getNumberOfFrames().squash(-1);
        auto origTriggers = det.getNumberOfTriggers().squash(-1);

        // auto mode
        det.setTimingMode(defs::AUTO_TIMING);
        REQUIRE_NOTHROW(caller.call(
            "pedestalmode",
            {std::to_string(pedestalFrames), std::to_string(pedestalLoops)}, -1,
            PUT));
        auto numTriggers = det.getNumberOfTriggers().squash(-1);
        auto numFrames = det.getNumberOfFrames().squash(-1);
        REQUIRE(numFrames == expNumFrames);
        REQUIRE(numTriggers == 1);

        // pedestal mode off
        REQUIRE_NOTHROW(caller.call("pedestalmode", {"0"}, -1, PUT));
        numTriggers = det.getNumberOfTriggers().squash(-1);
        numFrames = det.getNumberOfFrames().squash(-1);
        REQUIRE(numFrames == origFrames);
        REQUIRE(numTriggers == origTriggers);

        // trigger mode (frames > 1)
        REQUIRE_NOTHROW(det.setTimingMode(defs::TRIGGER_EXPOSURE));
        origFrames = 5;
        REQUIRE_NOTHROW(det.setNumberOfFrames(origFrames));
        REQUIRE_NOTHROW(caller.call(
            "pedestalmode",
            {std::to_string(pedestalFrames), std::to_string(pedestalLoops)}, -1,
            PUT));
        numTriggers = det.getNumberOfTriggers().squash(-1);
        numFrames = det.getNumberOfFrames().squash(-1);
        REQUIRE(numFrames == expNumFrames);
        REQUIRE(numTriggers == 1);

        // pedestal mode off
        REQUIRE_NOTHROW(caller.call("pedestalmode", {"0"}, -1, PUT));
        numTriggers = det.getNumberOfTriggers().squash(-1);
        numFrames = det.getNumberOfFrames().squash(-1);
        REQUIRE(numFrames == origFrames);
        REQUIRE(numTriggers == origTriggers);

        // trigger mode (frames = 1)
        origFrames = 1;
        REQUIRE_NOTHROW(det.setNumberOfFrames(origFrames));
        origTriggers = 10;
        REQUIRE_NOTHROW(det.setNumberOfTriggers(origTriggers));
        REQUIRE_NOTHROW(caller.call(
            "pedestalmode",
            {std::to_string(pedestalFrames), std::to_string(pedestalLoops)}, -1,
            PUT));
        numTriggers = det.getNumberOfTriggers().squash(-1);
        numFrames = det.getNumberOfFrames().squash(-1);
        REQUIRE(numFrames == 1);
        REQUIRE(numTriggers == expNumFrames);

        // pedestal mode off
        REQUIRE_NOTHROW(caller.call("pedestalmode", {"0"}, -1, PUT));
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
        REQUIRE_THROWS(caller.call("pedestalmode", {}, -1, GET));
        REQUIRE_THROWS(caller.call("pedestalmode", {"0"}, -1, PUT));
    }
}

TEST_CASE("timing_info_decoder", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    if (det.getDetectorType().squash() == defs::JUNGFRAU &&
        det.getHardwareVersion().squash() == "2.0") {
        auto prev_val = det.getTimingInfoDecoder();
        /*{
            std::ostringstream oss;
            caller.call("timing_info_decoder", {"shine"}, -1, PUT, oss);
            REQUIRE(oss.str() == "timing_info_decoder shine\n");
        }*/
        {
            std::ostringstream oss;
            caller.call("timing_info_decoder", {"swissfel"}, -1, PUT, oss);
            REQUIRE(oss.str() == "timing_info_decoder swissfel\n");
        }
        {
            std::ostringstream oss;
            caller.call("timing_info_decoder", {}, -1, GET, oss);
            REQUIRE(oss.str() == "timing_info_decoder swissfel\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setTimingInfoDecoder(prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(caller.call("timing_info_decoder", {}, -1, GET));
    }
}

TEST_CASE("collectionmode", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    if (det.getDetectorType().squash() == defs::JUNGFRAU) {
        auto prev_val = det.getCollectionMode();
        {
            std::ostringstream oss;
            caller.call("collectionmode", {"electron"}, -1, PUT, oss);
            REQUIRE(oss.str() == "collectionmode electron\n");
        }
        {
            std::ostringstream oss;
            caller.call("collectionmode", {"hole"}, -1, PUT, oss);
            REQUIRE(oss.str() == "collectionmode hole\n");
        }
        {
            std::ostringstream oss;
            caller.call("collectionmode", {}, -1, GET, oss);
            REQUIRE(oss.str() == "collectionmode hole\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setCollectionMode(prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(caller.call("collectionmode", {}, -1, GET));
    }
}

TEST_CASE("sync", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU || det_type == defs::MOENCH) {
        auto prev_val = det.getSynchronization().tsquash(
            "inconsistent synchronization to test");
        {
            std::ostringstream oss;
            caller.call("sync", {"0"}, -1, PUT, oss);
            REQUIRE(oss.str() == "sync 0\n");
        }
        {
            std::ostringstream oss;
            caller.call("sync", {"1"}, -1, PUT, oss);
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
            caller.call("master", {"1"}, 0, PUT);
            caller.call("master", {"0"}, 0, PUT);
            std::ostringstream oss;
            caller.call("status", {}, -1, GET, oss);
            REQUIRE(oss.str() != "status running\n");
            // set all to slaves, and then master
            for (int i = 0; i != det.size(); ++i) {
                det.setMaster(0, {i});
            }
            det.setMaster(1, prevMaster);
        }
        {
            std::ostringstream oss;
            caller.call("sync", {}, -1, GET, oss);
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
            REQUIRE_THROWS(caller.call("sync", {"0"}, -1, PUT));
            {
                std::ostringstream oss;
                caller.call("sync", {}, -1, GET, oss);
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
        REQUIRE_THROWS(caller.call("sync", {}, -1, GET));
        REQUIRE_THROWS(caller.call("sync", {"0"}, -1, PUT));
    }
}

} // namespace sls
