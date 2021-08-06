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

using sls::CmdProxy;
using sls::Detector;
using test::GET;
using test::PUT;

// time specific measurements for gotthard2
TEST_CASE("timegotthard2", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::GOTTHARD2) {
        // exptime
        auto prev_val = det.getExptime();
        {
            std::ostringstream oss;
            proxy.Call("exptime", {"220ns"}, -1, PUT, oss);
            REQUIRE(oss.str() == "exptime 220ns\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("exptime", {}, -1, GET, oss);
            REQUIRE(oss.str() == "exptime 222ns\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setExptime(prev_val[i], {i});
        }
        // burst period
        prev_val = det.getBurstPeriod();
        {
            std::ostringstream oss;
            proxy.Call("burstperiod", {"220ns"}, -1, PUT, oss);
            REQUIRE(oss.str() == "burstperiod 220ns\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("burstperiod", {}, -1, GET, oss);
            REQUIRE(oss.str() == "burstperiod 222ns\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setBurstPeriod(prev_val[i], {i});
        }
        // delay after trigger
        prev_val = det.getDelayAfterTrigger();
        {
            std::ostringstream oss;
            proxy.Call("delay", {"220ns"}, -1, PUT, oss);
            REQUIRE(oss.str() == "delay 220ns\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("delay", {}, -1, GET, oss);
            REQUIRE(oss.str() == "delay 222ns\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setDelayAfterTrigger(prev_val[i], {i});
        }
        // period in burst mode
        auto burst_prev_val = det.getBurstMode();
        det.setBurstMode(defs::BURST_INTERNAL, {});
        prev_val = det.getPeriod();
        {
            std::ostringstream oss;
            proxy.Call("period", {"220ns"}, -1, PUT, oss);
            REQUIRE(oss.str() == "period 220ns\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("period", {}, -1, GET, oss);
            REQUIRE(oss.str() == "period 222ns\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setPeriod(prev_val[i], {i});
        }
        // period in continuous mode
        det.setBurstMode(defs::CONTINUOUS_INTERNAL, {});
        prev_val = det.getPeriod();
        {
            std::ostringstream oss;
            proxy.Call("period", {"220ns"}, -1, PUT, oss);
            REQUIRE(oss.str() == "period 220ns\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("period", {}, -1, GET, oss);
            REQUIRE(oss.str() == "period 222ns\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setPeriod(prev_val[i], {i});
            det.setBurstMode(burst_prev_val[i], {i});
        }
    }
}
/* dacs */

TEST_CASE("Setting and reading back GOTTHARD2 dacs", "[.cmd][.dacs]") {
    // vref_h_adc,   vb_comp_fe, vb_comp_adc,  vcom_cds,
    // vref_restore, vb_opa_1st, vref_comp_fe, vcom_adc1,
    // vref_prech,   vref_l_adc, vref_cds,     vb_cs,
    // vb_opa_fd,    vcom_adc2

    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::GOTTHARD2) {
        SECTION("vref_h_adc") {
            test_dac(defs::VREF_H_ADC, "vref_h_adc", 2099);
        }
        SECTION("vb_comp_fe") { test_dac(defs::VB_COMP_FE, "vb_comp_fe", 0); }
        SECTION("vb_comp_adc") {
            test_dac(defs::VB_COMP_ADC, "vb_comp_adc", 0);
        }
        SECTION("vcom_cds") { test_dac(defs::VCOM_CDS, "vcom_cds", 1400); }
        SECTION("vref_rstore") {
            test_dac(defs::VREF_RSTORE, "vref_rstore", 640);
        }
        SECTION("vb_opa_1st") { test_dac(defs::VB_OPA_1ST, "vb_opa_1st", 0); }
        SECTION("vref_comp_fe") {
            test_dac(defs::VREF_COMP_FE, "vref_comp_fe", 0);
        }
        SECTION("vcom_adc1") { test_dac(defs::VCOM_ADC1, "vcom_adc1", 1400); }
        SECTION("vref_prech") {
            test_dac(defs::VREF_PRECH, "vref_prech", 1720);
        }
        SECTION("vref_l_adc") { test_dac(defs::VREF_L_ADC, "vref_l_adc", 700); }
        SECTION("vref_cds") { test_dac(defs::VREF_CDS, "vref_cds", 1200); }
        SECTION("vb_cs") { test_dac(defs::VB_CS, "vb_cs", 2799); }
        SECTION("vb_opa_fd") { test_dac(defs::VB_OPA_FD, "vb_opa_fd", 0); }
        SECTION("vcom_adc2") { test_dac(defs::VCOM_ADC2, "vcom_adc2", 1400); }
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
    }
}

/* on chip dacs */

TEST_CASE("vchip_comp_fe", "[.cmd][.onchipdacs]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::GOTTHARD2) {
        SECTION("vchip_comp_fe") {
            test_onchip_dac(defs::VB_COMP_FE, "vchip_comp_fe", 0x137);
        }
    } else {
        REQUIRE_THROWS(proxy.Call("vchip_comp_fe", {}, -1, GET));
    }
}

TEST_CASE("vchip_opa_1st", "[.cmd][.onchipdacs]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::GOTTHARD2) {
        SECTION("vchip_opa_1st") {
            test_onchip_dac(defs::VB_OPA_1ST, "vchip_opa_1st", 0x000);
        }
    } else {
        REQUIRE_THROWS(proxy.Call("vchip_opa_1st", {}, -1, GET));
    }
}

TEST_CASE("vchip_opa_fd", "[.cmd][.onchipdacs]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::GOTTHARD2) {
        SECTION("vchip_opa_fd") {
            test_onchip_dac(defs::VB_OPA_FD, "vchip_opa_fd", 0x134);
        }
    } else {
        REQUIRE_THROWS(proxy.Call("vchip_opa_fd", {}, -1, GET));
    }
}

TEST_CASE("vchip_comp_adc", "[.cmd][.onchipdacs]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::GOTTHARD2) {
        SECTION("vchip_comp_adc") {
            test_onchip_dac(defs::VB_COMP_ADC, "vchip_comp_adc", 0x3FF);
        }
    } else {
        REQUIRE_THROWS(proxy.Call("vchip_comp_adc", {}, -1, GET));
    }
}

TEST_CASE("vchip_ref_comp_fe", "[.cmd][.onchipdacs]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::GOTTHARD2) {
        SECTION("vchip_ref_comp_fe") {
            test_onchip_dac(defs::VREF_COMP_FE, "vchip_ref_comp_fe", 0x100);
        }
    } else {
        REQUIRE_THROWS(proxy.Call("vchip_ref_comp_fe", {}, -1, GET));
    }
}

TEST_CASE("vchip_cs", "[.cmd][.onchipdacs]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::GOTTHARD2) {
        SECTION("vchip_cs") { test_onchip_dac(defs::VB_CS, "vchip_cs", 0x0D0); }
    } else {
        REQUIRE_THROWS(proxy.Call("vchip_cs", {}, -1, GET));
    }
}

/* Gotthard2 Specific */

TEST_CASE("bursts", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::GOTTHARD2) {
        auto prev_burst =
            det.getNumberOfBursts().tsquash("#bursts should be same to test");
        auto prev_trigger =
            det.getNumberOfFrames().tsquash("#frames should be same to test");
        auto prev_frames = det.getNumberOfTriggers().tsquash(
            "#triggers should be same to test");
        auto prev_timingMode = det.getTimingMode();
        auto prev_burstMode = det.getBurstMode();
        // changing continuous mode frames and bursts
        det.setBurstMode(defs::BURST_INTERNAL);
        det.setTimingMode(defs::AUTO_TIMING);
        {
            std::ostringstream oss;
            proxy.Call("bursts", {"3"}, -1, PUT, oss);
            REQUIRE(oss.str() == "bursts 3\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("bursts", {}, -1, GET, oss);
            REQUIRE(oss.str() == "bursts 3\n");
        }
        REQUIRE_THROWS(proxy.Call("bursts", {"0"}, -1, PUT));
        // trigger mode: reg set to 1, but bursts must be same
        det.setTimingMode(defs::TRIGGER_EXPOSURE);
        {
            std::ostringstream oss;
            proxy.Call("bursts", {}, -1, GET, oss);
            REQUIRE(oss.str() == "bursts 3\n");
        }
        det.setTimingMode(defs::AUTO_TIMING);
        {
            std::ostringstream oss;
            proxy.Call("bursts", {}, -1, GET, oss);
            REQUIRE(oss.str() == "bursts 3\n");
        }
        // continuous mode: reg set to #frames,
        // but bursts should return same value
        det.setBurstMode(defs::CONTINUOUS_INTERNAL);
        det.setNumberOfFrames(2);
        {
            std::ostringstream oss;
            proxy.Call("bursts", {}, -1, GET, oss);
            REQUIRE(oss.str() == "bursts 3\n");
        }
        det.setTimingMode(defs::TRIGGER_EXPOSURE);
        {
            std::ostringstream oss;
            proxy.Call("bursts", {}, -1, GET, oss);
            REQUIRE(oss.str() == "bursts 3\n");
        }
        det.setBurstMode(defs::BURST_INTERNAL);
        {
            std::ostringstream oss;
            proxy.Call("bursts", {}, -1, GET, oss);
            REQUIRE(oss.str() == "bursts 3\n");
        }
        // set to previous values
        det.setNumberOfBursts(prev_burst);
        det.setNumberOfFrames(prev_frames);
        det.setNumberOfTriggers(prev_trigger);
        for (int i = 0; i != det.size(); ++i) {
            det.setTimingMode(prev_timingMode[i], {i});
            det.setBurstMode(prev_burstMode[i], {i});
        }
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
        // Reset to previous value
        for (int i = 0; i != det.size(); ++i) {
            det.setBurstPeriod(previous[i], {i});
        }
    } else {
        REQUIRE_THROWS(proxy.Call("burstperiod", {}, -1, GET));
    }
}

TEST_CASE("burstsl", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::GOTTHARD2) {
        REQUIRE_NOTHROW(proxy.Call("burstsl", {}, -1, GET));
    } else {
        REQUIRE_THROWS(proxy.Call("burstsl", {}, -1, GET));
    }
}

TEST_CASE("inj_ch", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::GOTTHARD2) {
        REQUIRE_THROWS(
            proxy.Call("inj_ch", {"-1", "1"}, -1, PUT)); // invalid offset
        REQUIRE_THROWS(
            proxy.Call("inj_ch", {"0", "0"}, -1, PUT)); // invalid increment
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
    } else {
        REQUIRE_THROWS(proxy.Call("inj_ch", {}, -1, GET));
    }
}

TEST_CASE("vetophoton", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::GOTTHARD2) {
        REQUIRE_THROWS(proxy.Call("vetophoton", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vetophoton", {"-1"}, -1, GET));
        REQUIRE_NOTHROW(
            proxy.Call("vetophoton", {"-1", "/tmp/bla.txt"}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vetophoton", {"12", "1", "39950"}, -1,
                                  PUT)); // invalid chip index
        REQUIRE_THROWS(proxy.Call("vetophoton", {"-1", "0"}, -1,
                                  PUT)); // invalid photon number
        REQUIRE_THROWS(proxy.Call("vetophoton", {"-1", "1", "39950"}, -1,
                                  PUT)); // invald file
    } else {
        REQUIRE_THROWS(
            proxy.Call("vetophoton", {"-1", "/tmp/bla.txt"}, -1, GET));
    }
}

TEST_CASE("vetoref", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::GOTTHARD2) {
        REQUIRE_THROWS(proxy.Call("vetoref", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vetoref", {"3", "0x3ff"}, -1,
                                  PUT)); // invalid chip index
        REQUIRE_NOTHROW(proxy.Call("vetoref", {"1", "0x010"}, -1, PUT));
    } else {
        REQUIRE_THROWS(proxy.Call("vetoref", {"3", "0x0"}, -1, PUT));
    }
}

TEST_CASE("vetofile", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::GOTTHARD2) {
        REQUIRE_THROWS(proxy.Call("vetofile", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vetofile", {"12", "/tmp/bla.txt"}, -1,
                                  PUT)); // invalid chip index
    } else {
        REQUIRE_THROWS(proxy.Call("vetofile", {"-1"}, -1, GET));
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
            proxy.Call("burstmode", {"burst_internal"}, -1, PUT, oss);
            REQUIRE(oss.str() == "burstmode burst_internal\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("burstmode", {"cw_internal"}, -1, PUT, oss);
            REQUIRE(oss.str() == "burstmode cw_internal\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("burstmode", {}, -1, GET, oss);
            REQUIRE(oss.str() == "burstmode cw_internal\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setBurstMode(burst[i], {i});
        }
    } else {
        REQUIRE_THROWS(proxy.Call("burstmode", {}, -1, GET));
    }
}

TEST_CASE("cdsgain", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::GOTTHARD2) {
        auto prev_val = det.getCDSGain();
        {
            std::ostringstream oss;
            proxy.Call("cdsgain", {"1"}, -1, PUT, oss);
            REQUIRE(oss.str() == "cdsgain 1\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("cdsgain", {"0"}, -1, PUT, oss);
            REQUIRE(oss.str() == "cdsgain 0\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("cdsgain", {}, -1, GET, oss);
            REQUIRE(oss.str() == "cdsgain 0\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setCDSGain(prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(proxy.Call("cdsgain", {}, -1, GET));
    }
}

TEST_CASE("filter", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::GOTTHARD2) {
        auto prev_val = det.getFilter();
        {
            std::ostringstream oss;
            proxy.Call("filter", {"1"}, -1, PUT, oss);
            REQUIRE(oss.str() == "filter 1\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("filter", {"0"}, -1, PUT, oss);
            REQUIRE(oss.str() == "filter 0\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("filter", {}, -1, GET, oss);
            REQUIRE(oss.str() == "filter 0\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setFilter(prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(proxy.Call("filter", {}, -1, GET));
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

TEST_CASE("veto", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::GOTTHARD2) {
        auto prev_val = det.getVeto();
        {
            std::ostringstream oss;
            proxy.Call("veto", {"1"}, -1, PUT, oss);
            REQUIRE(oss.str() == "veto 1\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("veto", {"0"}, -1, PUT, oss);
            REQUIRE(oss.str() == "veto 0\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("veto", {}, -1, GET, oss);
            REQUIRE(oss.str() == "veto 0\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setVeto(prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(proxy.Call("veto", {}, -1, GET));
    }
}

TEST_CASE("vetostream", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::GOTTHARD2) {
        auto prev_val = det.getVetoStream();
        {
            std::ostringstream oss;
            proxy.Call("vetostream", {"none"}, -1, PUT, oss);
            REQUIRE(oss.str() == "vetostream none\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("vetostream", {}, -1, GET, oss);
            REQUIRE(oss.str() == "vetostream none\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("vetostream", {"lll"}, -1, PUT, oss);
            REQUIRE(oss.str() == "vetostream lll\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("vetostream", {}, -1, GET, oss);
            REQUIRE(oss.str() == "vetostream lll\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("vetostream", {"lll", "10gbe"}, -1, PUT, oss);
            REQUIRE(oss.str() == "vetostream lll, 10gbe\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("vetostream", {}, -1, GET, oss);
            REQUIRE(oss.str() == "vetostream lll, 10gbe\n");
        }
        REQUIRE_THROWS(proxy.Call("vetostream", {"lll", "none"}, -1, PUT));
        for (int i = 0; i != det.size(); ++i) {
            det.setVetoStream(prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(proxy.Call("vetostream", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vetostream", {"none"}, -1, PUT));
    }
    REQUIRE_THROWS(proxy.Call("vetostream", {"dfgd"}, -1, GET));
}

TEST_CASE("vetoalg", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::GOTTHARD2) {
        auto prev_val_3g = det.getVetoAlgorithm(defs::streamingInterface::LOW_LATENCY_LINK);
        auto prev_val_10g =
            det.getVetoAlgorithm(defs::streamingInterface::ETHERNET_10GB);
        {
            std::ostringstream oss;
            proxy.Call("vetoalg", {"default", "lll"}, -1, PUT, oss);
            REQUIRE(oss.str() == "vetoalg default lll\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("vetoalg", {"lll"}, -1, GET, oss);
            REQUIRE(oss.str() == "vetoalg default lll\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("vetoalg", {"default", "10gbe"}, -1, PUT, oss);
            REQUIRE(oss.str() == "vetoalg default 10gbe\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("vetoalg", {"10gbe"}, -1, GET, oss);
            REQUIRE(oss.str() == "vetoalg default 10gbe\n");
        }
        REQUIRE_THROWS(
            proxy.Call("vetoalg", {"default", "lll", "10gbe"}, -1, PUT));
        REQUIRE_THROWS(proxy.Call("vetoalg", {"default", "none"}, -1, PUT));
        for (int i = 0; i != det.size(); ++i) {
            det.setVetoAlgorithm(prev_val_3g[i], defs::streamingInterface::LOW_LATENCY_LINK,
                                 {i});
            det.setVetoAlgorithm(prev_val_10g[i],
                                 defs::streamingInterface::ETHERNET_10GB, {i});
        }
    } else {
        REQUIRE_THROWS(proxy.Call("vetoalg", {"lll"}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vetoalg", {"none"}, -1, PUT));
    }
    REQUIRE_THROWS(proxy.Call("vetoalg", {"dfgd"}, -1, GET));
}

TEST_CASE("confadc", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::GOTTHARD2) {
        int ndet = det.size();
        int nchip = 10;
        int nadc = 32;
        int prev_val[ndet][nchip][nadc];
        for (int i = 0; i != ndet; ++i) {
            for (int j = 0; j != nchip; ++j) {
                for (int k = 0; k != nadc; ++k) {
                    prev_val[i][j][k] =
                        det.getADCConfiguration(j, k, {i}).squash();
                }
            }
        }

        REQUIRE_THROWS(proxy.Call("confadc", {"11", "2", "0x3ff"}, -1,
                                  PUT)); // invalid chip index
        REQUIRE_THROWS(proxy.Call("confadc", {"-1", "10", "0x3ff"}, -1,
                                  PUT)); // invalid adc index
        REQUIRE_THROWS(proxy.Call("confadc", {"-1", "10", "0x1fff"}, -1,
                                  PUT)); // invalid value
        {
            std::ostringstream oss;
            proxy.Call("confadc", {"-1", "-1", "0x11"}, -1, PUT, oss);
            REQUIRE(oss.str() == "confadc [-1, -1, 0x11]\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("confadc", {"2", "3"}, -1, GET, oss);
            REQUIRE(oss.str() == "confadc 0x11\n");
        }
        for (int i = 0; i != ndet; ++i) {
            for (int j = 0; j != nchip; ++j) {
                for (int k = 0; k != nadc; ++k) {
                    det.setADCConfiguration(j, k, prev_val[i][j][k], {i});
                }
            }
        }
    } else {
        REQUIRE_THROWS(proxy.Call("confadc", {}, -1, GET));
    }
}

TEST_CASE("badchannels", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::GOTTHARD2) {
        REQUIRE_THROWS(proxy.Call("badchannels", {}, -1, GET));
        REQUIRE_NOTHROW(proxy.Call("badchannels", {"/tmp/bla.txt"}, -1, GET));
        REQUIRE_NOTHROW(proxy.Call("badchannels", {"/tmp/bla.txt"}, -1, PUT));
    } else {
        REQUIRE_THROWS(proxy.Call("badchannels", {}, -1, GET));
    }
}