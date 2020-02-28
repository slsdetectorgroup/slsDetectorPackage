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

TEST_CASE("Setting and reading back GOTTHARD2 dacs", "[.cmd][.dacs]") {
    // vref_h_adc,   vb_comp_fe, vb_comp_adc,  vcom_cds,
    // vref_restore, vb_opa_1st, vref_comp_fe, vcom_adc1,
    // vref_prech,   vref_l_adc, vref_cds,     vb_cs,
    // vb_opa_fd,    vcom_adc2

    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::GOTTHARD2) {
        SECTION("vref_h_adc") { test_dac(defs::VREF_H_ADC, "vref_h_adc", 2099); }
        SECTION("vb_comp_fe") { test_dac(defs::VB_COMP_FE, "vb_comp_fe", 0); }
        SECTION("vb_comp_adc") { test_dac(defs::VB_COMP_ADC, "vb_comp_adc", 0); }
        SECTION("vcom_cds") { test_dac(defs::VCOM_CDS, "vcom_cds", 1400); }
        SECTION("vref_rstore") { test_dac(defs::VREF_RSTORE, "vref_rstore", 640); }
        SECTION("vb_opa_1st") { test_dac(defs::VB_OPA_1ST, "vb_opa_1st", 0); }
        SECTION("vref_comp_fe") { test_dac(defs::VREF_COMP_FE, "vref_comp_fe", 0); }
        SECTION("vcom_adc1") { test_dac(defs::VCOM_ADC1, "vcom_adc1", 1400); }
        SECTION("vref_prech") { test_dac(defs::VREF_PRECH, "vref_prech", 1720); }
        SECTION("vref_l_adc") { test_dac(defs::VREF_L_ADC, "vref_l_adc", 700); }
        SECTION("vref_cds") { test_dac(defs::VREF_CDS, "vref_cds", 1200); }
        SECTION("vb_cs") { test_dac(defs::VB_CS, "vb_cs", 2799); }
        SECTION("vb_opa_fd") { test_dac(defs::VB_OPA_FD, "vb_opa_fd", 0); }
        SECTION("vcom_adc2") { test_dac(defs::VCOM_ADC2, "vcom_adc2", 1400); }
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
        REQUIRE_THROWS(proxy.Call("vref_ds", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vcascn_pb", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vcascp_pb", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vout_cm", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vcasc_out", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vin_cm", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vref_comp", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("ib_test_c", {}, -1, GET));
        // jungfrau
        REQUIRE_THROWS(proxy.Call("vb_comp", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vdd_prot", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vin_com", {}, -1, GET));
        // REQUIRE_THROWS(proxy.Call("vref_prech", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vb_pixbuf", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vb_ds", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vref_ds", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vref_comp", {}, -1, GET));
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
    }
}

TEST_CASE("vchip", "[.cmd][.onchipdacs]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::GOTTHARD2) {
        std::vector<std::string> on_chip_dac_names = {"vchip_comp_fe", "vchip_opa_1st", "vchip_opa_fd", "vchip_comp_adc", "vchip_ref_comp_fe", "vchip_cs"};
        std::vector<defs::dacIndex> on_chip_dac_indices = {defs::VB_COMP_FE, defs::VB_OPA_1ST, defs::VB_OPA_FD, defs::VB_COMP_ADC, defs::VREF_COMP_FE, defs::VB_CS};
        std::vector<int> values = {0x137, 0x000, 0x134, 0x3FF, 0x100, 0x0D0};

        for (size_t i = 0; i < on_chip_dac_names.size(); ++i) {
            REQUIRE_THROWS(proxy.Call(on_chip_dac_names[i], {}, -1, GET));
            REQUIRE_THROWS(proxy.Call(on_chip_dac_names[i], {"10", "0x0"}, -1, GET)); // chip index (-1 to 9)
            REQUIRE_THROWS(proxy.Call(on_chip_dac_names[i], {"-1", "0x400"}, -1, GET)); // max val is 0x3ff
        
            auto previous = det.getOnChipDAC(on_chip_dac_indices[i], -1);
            auto dacstr = sls::ToStringHex(values[i]);
            int chip_index = -1;
            auto chip_index_str = std::to_string(chip_index);

            std::ostringstream oss_set, oss_get;
            proxy.Call(on_chip_dac_names[i], {chip_index_str, dacstr}, -1, PUT, oss_set);
            REQUIRE(oss_set.str() == on_chip_dac_names[i] + " " + chip_index_str + " " + dacstr + "\n");
            proxy.Call(on_chip_dac_names[i], {chip_index_str}, -1, GET, oss_get);
            REQUIRE(oss_get.str() == on_chip_dac_names[i] + " " + chip_index_str + " "  + dacstr + "\n");
            // Reset all dacs to previous value
            for (int i = 0; i != det.size(); ++i) {
                det.setOnChipDAC(on_chip_dac_indices[i], chip_index, previous[i], {i});
            }
        }
    } else {
        REQUIRE_THROWS(proxy.Call("vchip_comp_fe", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vchip_opa_1st", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vchip_opa_fd", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vchip_comp_adc", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vchip_ref_comp_fe", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vchip_cs", {}, -1, GET));        
    }
}


TEST_CASE("burstmode", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::GOTTHARD2) {
        auto burst = det.getBurstMode();
        auto burststr = sls::ToString(burst);
        {
            std::ostringstream oss;
            proxy.Call("burstmode", {"internal"}, -1, PUT, oss);
            REQUIRE(oss.str() == "burstmode internal\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("burstmode", {"off"}, -1, PUT, oss);
            REQUIRE(oss.str() == "burstmode off\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("burstmode", {}, -1, GET, oss);
            REQUIRE(oss.str() == "burstmode off\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setBurstMode(burst[i], {i});
        }
    } else {
        REQUIRE_THROWS(proxy.Call("burstmode", {}, -1, GET));
    }
}


TEST_CASE("vetoref", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::GOTTHARD2) {
        REQUIRE_THROWS(proxy.Call("vetoref", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vetoref", {"3", "0x3ff"}, -1, PUT)); // invalid chip index
        REQUIRE_NOTHROW(proxy.Call("vetoref", {"1", "0x010"}, -1, PUT));
    } else {
        REQUIRE_THROWS(proxy.Call("vetoref", {"3", "0x0"}, -1, PUT));
    }
}


TEST_CASE("vetophoton", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::GOTTHARD2) {
        REQUIRE_THROWS(proxy.Call("vetophoton", {}, -1, GET));
        REQUIRE_NOTHROW(proxy.Call("vetophoton", {"-1"}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vetophoton", {"12", "1", "39950"}, -1, PUT));// invalid chip index
        REQUIRE_THROWS(proxy.Call("vetophoton", {"-1", "0"}, -1, PUT)); // invalid photon number
        REQUIRE_THROWS(proxy.Call("vetophoton", {"-1", "1", "39950"}, -1, PUT)); // invald file
    } else {
        REQUIRE_THROWS(proxy.Call("vetophoton", {"-1"}, -1, GET));
    }
}



TEST_CASE("inj_ch", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::GOTTHARD2) {
        auto inj = det.getInjectChannel();
        REQUIRE_THROWS(proxy.Call("inj_ch", {"-1", "1"}, -1, PUT));// invalid offset
        REQUIRE_THROWS(proxy.Call("inj_ch", {"0", "0"}, -1, PUT));// invalid increment
        {
            std::ostringstream oss;
            proxy.Call("inj_ch", {"0", "1"}, -1, PUT, oss);
            REQUIRE(oss.str() == "inj_ch [0, 1]\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("inj_ch", {}, -1, GET, oss);
            REQUIRE(oss.str() == "inj_ch [0, 1]\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setInjectChannel(inj[i][0], inj[i][1], {i});
        }
    } else {
        REQUIRE_THROWS(proxy.Call("inj_ch", {}, -1, GET));
    }
}


TEST_CASE("bursts", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::GOTTHARD2) {
        auto previous = det.getNumberOfBursts().squash(1);
        auto previousTrigger = det.getNumberOfTriggers().squash(1);

        std::ostringstream oss_set, oss_get;
        proxy.Call("bursts", {"3"}, -1, PUT, oss_set);
        REQUIRE(oss_set.str() == "bursts 3\n");

        // change to trigger and back (bursts should still be same)
        proxy.Call("timing", {"trigger"}, -1, PUT);
        proxy.Call("triggers", {"2"}, -1, PUT);
        proxy.Call("timing", {"auto"}, -1, PUT);


        proxy.Call("bursts", {}, -1, GET, oss_get);
        REQUIRE(oss_get.str() == "bursts 3\n");


        det.setNumberOfBursts(previous);
        det.setNumberOfTriggers(previousTrigger);
    } else {
        REQUIRE_THROWS(proxy.Call("bursts", {}, -1, GET));
    }
}

TEST_CASE("burstperiod", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::GOTTHARD2) {
        auto previous = det.getBurstPeriod();

        std::ostringstream oss_set, oss_get;
        proxy.Call("burstperiod", {"30ms"}, -1, PUT, oss_set);
        REQUIRE(oss_set.str() == "burstperiod 30ms\n");
        proxy.Call("burstperiod", {}, -1, GET, oss_get);
        REQUIRE(oss_get.str() == "burstperiod 30ms\n");
        // Reset all dacs to previous value
        for (int i = 0; i != det.size(); ++i) {
            det.setBurstPeriod(previous[i], {i});
        }
    } else {
        REQUIRE_THROWS(proxy.Call("burstperiod", {}, -1, GET));
    }
}


TEST_CASE("currentsource", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::GOTTHARD2) {
        auto prev_val = det.getCurrentSource();
        {
            std::ostringstream oss;
            proxy.Call("currentsource", {"1"}, -1, PUT, oss);
            REQUIRE(oss.str() == "currentsource 1\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("currentsource", {"0"}, -1, PUT, oss);
            REQUIRE(oss.str() == "currentsource 0\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("currentsource", {}, -1, GET, oss);
            REQUIRE(oss.str() == "currentsource 0\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setCurrentSource(prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(proxy.Call("currentsource", {}, -1, GET));
    }
}

TEST_CASE("timingsource", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::GOTTHARD2) {
        auto prev_val = det.getTimingSource();
       /* { until its activated in fpga
            std::ostringstream oss;
            proxy.Call("timingsource", {"external"}, -1, PUT, oss);
            REQUIRE(oss.str() == "timingsource external\n");
        }*/
        { 
            std::ostringstream oss;
            proxy.Call("timingsource", {"internal"}, -1, PUT, oss);
            REQUIRE(oss.str() == "timingsource internal\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("timingsource", {}, -1, GET, oss);
            REQUIRE(oss.str() == "timingsource internal\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setTimingSource(prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(proxy.Call("timingsource", {}, -1, GET));
    }
}
