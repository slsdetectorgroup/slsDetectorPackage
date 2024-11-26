// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "Caller.h"
#include "catch.hpp"
#include "sls/Detector.h"
#include "sls/sls_detector_defs.h"
#include <sstream>

#include "sls/Result.h"
#include "sls/ToString.h"
#include "sls/versionAPI.h"
#include "test-Caller-global.h"
#include "tests/globals.h"

namespace sls {

using test::GET;
using test::PUT;

/* dacs */

TEST_CASE("dacname", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::CHIPTESTBOARD ||
        det_type == defs::XILINX_CHIPTESTBOARD) {
        defs::dacIndex ind = static_cast<defs::dacIndex>(2);
        std::string str_dac_index = "2";
        auto prev = det.getDacName(ind);

        // 1 arg throw
        REQUIRE_THROWS(caller.call("dacname", {"2", "3", "bname"}, -1, PUT));
        // invalid index
        REQUIRE_THROWS(caller.call("dacname", {"18", "bname"}, -1, PUT));
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(
                caller.call("dacname", {str_dac_index, "bname"}, -1, PUT, oss));
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(
                caller.call("dacname", {str_dac_index}, -1, GET, oss));
            REQUIRE(oss.str() ==
                    std::string("dacname ") + str_dac_index + " bname\n");
        }
        det.setDacName(ind, prev);

    } else {
        REQUIRE_THROWS(caller.call("dacname", {"2", "b"}, -1, PUT));
        REQUIRE_THROWS(caller.call("dacname", {"2"}, -1, GET));
    }
}

TEST_CASE("dacindex", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::CHIPTESTBOARD ||
        det_type == defs::XILINX_CHIPTESTBOARD) {
        defs::dacIndex ind = static_cast<defs::dacIndex>(2);
        std::string str_dac_index = "2";

        // 1 arg throw
        REQUIRE_THROWS(caller.call("dacindex", {"2", "2"}, -1, PUT));
        // invalid index
        REQUIRE_THROWS(caller.call("dacindex", {"18"}, -1, PUT));
        auto dacname = det.getDacName(ind);
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(caller.call("dacindex", {dacname}, -1, GET, oss));
            REQUIRE(oss.str() ==
                    std::string("dacindex ") + str_dac_index + '\n');
        }
    } else {
        REQUIRE_THROWS(caller.call("dacindex", {"2"}, -1, GET));
    }
}

TEST_CASE("adclist", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::CHIPTESTBOARD ||
        det_type == defs::XILINX_CHIPTESTBOARD) {
        auto prev = det.getAdcNames();

        REQUIRE_THROWS(caller.call("adclist", {"a", "s", "d"}, -1, PUT));

        std::vector<std::string> names;
        for (int iarg = 0; iarg != 32; ++iarg) {
            names.push_back("a");
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(caller.call("adclist", names, -1, PUT, oss));
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(caller.call("adclist", {}, -1, GET, oss));
            REQUIRE(oss.str() ==
                    std::string("adclist ") + ToString(names) + '\n');
        }
        det.setAdcNames(prev);

    } else {
        REQUIRE_THROWS(caller.call("adclist", {"a", "b"}, -1, PUT));
        REQUIRE_THROWS(caller.call("adclist", {}, -1, GET));
    }
}

TEST_CASE("adcname", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::CHIPTESTBOARD ||
        det_type == defs::XILINX_CHIPTESTBOARD) {
        int ind = 2;
        std::string str_adc_index = "2";
        auto prev = det.getAdcName(ind);

        // 1 arg throw
        REQUIRE_THROWS(caller.call("adcname", {"2", "3", "bname"}, -1, PUT));
        // invalid index
        REQUIRE_THROWS(caller.call("adcname", {"32", "bname"}, -1, PUT));
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(
                caller.call("adcname", {str_adc_index, "bname"}, -1, PUT, oss));
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(
                caller.call("adcname", {str_adc_index}, -1, GET, oss));
            REQUIRE(oss.str() ==
                    std::string("adcname ") + str_adc_index + " bname\n");
        }
        det.setAdcName(ind, prev);

    } else {
        REQUIRE_THROWS(caller.call("adcname", {"2", "b"}, -1, PUT));
        REQUIRE_THROWS(caller.call("adcname", {"2"}, -1, GET));
    }
}

TEST_CASE("adcindex", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::CHIPTESTBOARD ||
        det_type == defs::XILINX_CHIPTESTBOARD) {
        int ind = 2;
        std::string str_adc_index = "2";

        // 1 arg throw
        REQUIRE_THROWS(caller.call("adcindex", {"2", "2"}, -1, PUT));
        // invalid index
        REQUIRE_THROWS(caller.call("adcindex", {"32"}, -1, PUT));
        auto adcname = det.getAdcName(ind);
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(caller.call("adcindex", {adcname}, -1, GET, oss));
            REQUIRE(oss.str() ==
                    std::string("adcindex ") + str_adc_index + '\n');
        }
    } else {
        REQUIRE_THROWS(caller.call("adcindex", {"2"}, -1, GET));
    }
}

TEST_CASE("signallist", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::CHIPTESTBOARD ||
        det_type == defs::XILINX_CHIPTESTBOARD) {
        auto prev = det.getSignalNames();

        REQUIRE_THROWS(caller.call("signallist", {"a", "s", "d"}, -1, PUT));

        std::vector<std::string> names;
        for (int iarg = 0; iarg != 64; ++iarg) {
            names.push_back("a");
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(caller.call("signallist", names, -1, PUT, oss));
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(caller.call("signallist", {}, -1, GET, oss));
            REQUIRE(oss.str() ==
                    std::string("signallist ") + ToString(names) + '\n');
        }
        det.setSignalNames(prev);

    } else {
        REQUIRE_THROWS(caller.call("signallist", {"a", "b"}, -1, PUT));
        REQUIRE_THROWS(caller.call("signallist", {}, -1, GET));
    }
}

TEST_CASE("signalname", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::CHIPTESTBOARD ||
        det_type == defs::XILINX_CHIPTESTBOARD) {
        int ind = 2;
        std::string str_signal_index = "2";
        auto prev = det.getSignalName(ind);

        // 1 arg throw
        REQUIRE_THROWS(caller.call("signalname", {"2", "3", "bname"}, -1, PUT));
        // invalid index
        REQUIRE_THROWS(caller.call("signalname", {"64", "bname"}, -1, PUT));
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(caller.call(
                "signalname", {str_signal_index, "bname"}, -1, PUT, oss));
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(
                caller.call("signalname", {str_signal_index}, -1, GET, oss));
            REQUIRE(oss.str() ==
                    std::string("signalname ") + str_signal_index + " bname\n");
        }
        det.setSignalName(ind, prev);

    } else {
        REQUIRE_THROWS(caller.call("signalname", {"2", "b"}, -1, PUT));
        REQUIRE_THROWS(caller.call("signalname", {"2"}, -1, GET));
    }
}

TEST_CASE("signalindex", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::CHIPTESTBOARD ||
        det_type == defs::XILINX_CHIPTESTBOARD) {
        int ind = 2;
        std::string str_signal_index = "2";

        // 1 arg throw
        REQUIRE_THROWS(caller.call("signalindex", {"2", "2"}, -1, PUT));
        // invalid index
        REQUIRE_THROWS(caller.call("signalindex", {"64"}, -1, PUT));
        auto signalname = det.getSignalName(ind);
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(
                caller.call("signalindex", {signalname}, -1, GET, oss));
            REQUIRE(oss.str() ==
                    std::string("signalindex ") + str_signal_index + '\n');
        }
    } else {
        REQUIRE_THROWS(caller.call("signalindex", {"2"}, -1, GET));
    }
}

TEST_CASE("powerlist", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::CHIPTESTBOARD ||
        det_type == defs::XILINX_CHIPTESTBOARD) {
        auto prev = det.getPowerNames();

        REQUIRE_THROWS(caller.call("powerlist", {"a", "s", "d"}, -1, PUT));

        std::vector<std::string> names;
        for (int iarg = 0; iarg != 5; ++iarg) {
            names.push_back("a");
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(caller.call("powerlist", names, -1, PUT, oss));
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(caller.call("powerlist", {}, -1, GET, oss));
            REQUIRE(oss.str() ==
                    std::string("powerlist ") + ToString(names) + '\n');
        }
        det.setPowerNames(prev);

    } else {
        REQUIRE_THROWS(caller.call("powerlist", {"a", "b"}, -1, PUT));
        REQUIRE_THROWS(caller.call("powerlist", {}, -1, GET));
    }
}

TEST_CASE("powername", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::CHIPTESTBOARD ||
        det_type == defs::XILINX_CHIPTESTBOARD) {
        defs::dacIndex ind = static_cast<defs::dacIndex>(2 + defs::V_POWER_A);
        std::string str_power_index = "2";
        auto prev = det.getPowerName(ind);

        // 1 arg throw
        REQUIRE_THROWS(caller.call("powername", {"2", "3", "bname"}, -1, PUT));
        // invalid index
        REQUIRE_THROWS(caller.call("powername", {"5", "bname"}, -1, PUT));
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(caller.call("powername", {str_power_index, "bname"},
                                        -1, PUT, oss));
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(
                caller.call("powername", {str_power_index}, -1, GET, oss));
            REQUIRE(oss.str() ==
                    std::string("powername ") + str_power_index + " bname\n");
        }
        det.setPowerName(ind, prev);

    } else {
        REQUIRE_THROWS(caller.call("powername", {"2", "b"}, -1, PUT));
        REQUIRE_THROWS(caller.call("powername", {"2"}, -1, GET));
    }
}

TEST_CASE("powerindex", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::CHIPTESTBOARD ||
        det_type == defs::XILINX_CHIPTESTBOARD) {
        defs::dacIndex ind = static_cast<defs::dacIndex>(2 + defs::V_POWER_A);
        std::string str_power_index = "2";

        // 1 arg throw
        REQUIRE_THROWS(caller.call("powerindex", {"2", "2"}, -1, PUT));
        // invalid index
        REQUIRE_THROWS(caller.call("powerindex", {"5"}, -1, PUT));
        auto powername = det.getPowerName(ind);
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(
                caller.call("powerindex", {powername}, -1, GET, oss));
            REQUIRE(oss.str() ==
                    std::string("powerindex ") + str_power_index + '\n');
        }
    } else {
        REQUIRE_THROWS(caller.call("powerindex", {"2"}, -1, GET));
    }
}

TEST_CASE("powervalues", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::CHIPTESTBOARD ||
        det_type == defs::XILINX_CHIPTESTBOARD) {
        REQUIRE_NOTHROW(caller.call("powervalues", {}, -1, GET));
        REQUIRE_THROWS(caller.call("powervalues", {}, -1, PUT));
    } else {
        REQUIRE_THROWS(caller.call("powervalues", {}, -1, GET));
    }
}

TEST_CASE("slowadcvalues", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::CHIPTESTBOARD ||
        det_type == defs::XILINX_CHIPTESTBOARD) {
        REQUIRE_NOTHROW(caller.call("slowadcvalues", {}, -1, GET));
        REQUIRE_THROWS(caller.call("slowadcvalues", {}, -1, PUT));
    } else {
        REQUIRE_THROWS(caller.call("slowadcvalues", {}, -1, GET));
    }
}

TEST_CASE("slowadclist", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::CHIPTESTBOARD ||
        det_type == defs::XILINX_CHIPTESTBOARD) {
        auto prev = det.getSlowADCNames();

        REQUIRE_THROWS(caller.call("slowadclist", {"a", "s", "d"}, -1, PUT));

        std::vector<std::string> names;
        for (int iarg = 0; iarg != 8; ++iarg) {
            names.push_back("a");
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(caller.call("slowadclist", names, -1, PUT, oss));
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(caller.call("slowadclist", {}, -1, GET, oss));
            REQUIRE(oss.str() ==
                    std::string("slowadclist ") + ToString(names) + '\n');
        }
        det.setSlowADCNames(prev);

    } else {
        REQUIRE_THROWS(caller.call("slowadclist", {"a", "b"}, -1, PUT));
        REQUIRE_THROWS(caller.call("slowadclist", {}, -1, GET));
    }
}

TEST_CASE("slowadcname", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::CHIPTESTBOARD ||
        det_type == defs::XILINX_CHIPTESTBOARD) {
        defs::dacIndex ind = static_cast<defs::dacIndex>(2 + defs::SLOW_ADC0);
        std::string str_slowadc_index = "2";
        auto prev = det.getSlowADCName(ind);

        // 1 arg throw
        REQUIRE_THROWS(
            caller.call("slowadcname", {"2", "3", "bname"}, -1, PUT));
        // invalid index
        REQUIRE_THROWS(caller.call("slowadcname", {"8", "bname"}, -1, PUT));
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(caller.call(
                "slowadcname", {str_slowadc_index, "bname"}, -1, PUT, oss));
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(
                caller.call("slowadcname", {str_slowadc_index}, -1, GET, oss));
            REQUIRE(oss.str() == std::string("slowadcname ") +
                                     str_slowadc_index + " bname\n");
        }
        det.setSlowADCName(ind, prev);

    } else {
        REQUIRE_THROWS(caller.call("slowadcname", {"2", "b"}, -1, PUT));
        REQUIRE_THROWS(caller.call("slowadcname", {"2"}, -1, GET));
    }
}

TEST_CASE("slowadcindex", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::CHIPTESTBOARD ||
        det_type == defs::XILINX_CHIPTESTBOARD) {
        defs::dacIndex ind = static_cast<defs::dacIndex>(2 + defs::SLOW_ADC0);
        std::string str_slowadc_index = "2";

        // 1 arg throw
        REQUIRE_THROWS(caller.call("slowadcindex", {"2", "2"}, -1, PUT));
        // invalid index
        REQUIRE_THROWS(caller.call("slowadcindex", {"8"}, -1, PUT));
        auto slowadcname = det.getSlowADCName(ind);
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(
                caller.call("slowadcindex", {slowadcname}, -1, GET, oss));
            REQUIRE(oss.str() ==
                    std::string("slowadcindex ") + str_slowadc_index + '\n');
        }
    } else {
        REQUIRE_THROWS(caller.call("slowadcindex", {"2"}, -1, GET));
    }
}

/* dacs */

TEST_CASE("dac", "[.cmdcall][.dacs]") {
    // dac 0 to dac 17

    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::CHIPTESTBOARD ||
        det_type == defs::XILINX_CHIPTESTBOARD) {
        for (int i = 0; i < 18; ++i) {
            SECTION("dac " + std::to_string(i)) {
                test_dac_caller(static_cast<defs::dacIndex>(i), "dac", 0);
            }
        }

        // eiger
        // REQUIRE_THROWS(caller.call("dac", {"vthreshold"}, -1, GET));
        // REQUIRE_THROWS(caller.call("dac", {"vsvp"}, -1, GET));
        // REQUIRE_THROWS(caller.call("dac", {"vsvn"}, -1, GET));
        // REQUIRE_THROWS(caller.call("dac", {"vtrim"}, -1, GET));
        // REQUIRE_THROWS(caller.call("dac", {"vrpreamp"}, -1, GET));
        // REQUIRE_THROWS(caller.call("dac", {"vrshaper"}, -1, GET));
        // REQUIRE_THROWS(caller.call("dac", {"vtgstv"}, -1, GET));
        // REQUIRE_THROWS(caller.call("dac", {"vcmp_ll"}, -1, GET));
        // REQUIRE_THROWS(caller.call("dac", {"vcmp_lr"}, -1, GET));
        // REQUIRE_THROWS(caller.call("dac", {"vcal"}, -1, GET));
        // REQUIRE_THROWS(caller.call("dac", {"vcmp_rl"}, -1, GET));
        // REQUIRE_THROWS(caller.call("dac", {"vcmp_rr"}, -1, GET));
        // REQUIRE_THROWS(caller.call("dac", {"rxb_rb"}, -1, GET));
        // REQUIRE_THROWS(caller.call("dac", {"rxb_lb"}, -1, GET));
        // REQUIRE_THROWS(caller.call("dac", {"vcp"}, -1, GET));
        // REQUIRE_THROWS(caller.call("dac", {"vcn"}, -1, GET));
        // REQUIRE_THROWS(caller.call("dac", {"vishaper"}, -1, GET));
        // REQUIRE_THROWS(caller.call("dac", {"iodelay"}, -1, GET));
        // jungfrau
        REQUIRE_THROWS(caller.call("dac", {"vb_comp"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vdd_prot"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vin_com"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vref_prech"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vb_pixbuf"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vb_ds"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vref_ds"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vref_comp"}, -1, GET));
        // gotthard
        REQUIRE_THROWS(caller.call("dac", {"vref_ds"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vcascn_pb"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vcascp_pb"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vout_cm"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vcasc_out"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vin_cm"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vref_comp"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"ib_test_c"}, -1, GET));
        // mythen3
        REQUIRE_THROWS(caller.call("dac", {"vrpreamp"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vrshaper"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vrshaper_n"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vipre"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vishaper"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vdcsh"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vth1"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vth2"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vth3"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vcal_n"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vcal_p"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vtrim"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vcassh"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vcas"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vicin"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vipre_out"}, -1, GET));
        // gotthard2
        REQUIRE_THROWS(caller.call("dac", {"vref_h_Signal"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vb_comp_fe"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vb_comp_adc"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vcom_cds"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vref_rstore"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vb_opa_1st"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vref_comp_fe"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vcom_adc1"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vref_l_adc"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vref_cds"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vb_cs"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vb_opa_fd"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"vcom_adc2"}, -1, GET));
    }
}

TEST_CASE("adcvpp", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::CHIPTESTBOARD) {
        auto prev_val = det.getADCVpp(false);
        {
            std::ostringstream oss;
            caller.call("adcvpp", {"1"}, -1, PUT, oss);
            REQUIRE(oss.str() == "adcvpp 1\n");
        }
        {
            std::ostringstream oss;
            caller.call("adcvpp", {"1140", "mv"}, -1, PUT, oss);
            REQUIRE(oss.str() == "adcvpp 1140 mV\n");
        }
        {
            std::ostringstream oss;
            caller.call("adcvpp", {"mv"}, -1, GET, oss);
            REQUIRE(oss.str() == "adcvpp 1140 mV\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setADCVpp(prev_val[i], false, {i});
        }
    } else {
        REQUIRE_THROWS(caller.call("adcvpp", {}, -1, GET));
    }
}

/* CTB Specific */

TEST_CASE("samples", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::CHIPTESTBOARD ||
        det_type == defs::XILINX_CHIPTESTBOARD) {
        auto prev_asamples = det.getNumberOfAnalogSamples();
        auto prev_dsamples = det.getNumberOfDigitalSamples();
        {
            std::ostringstream oss;
            caller.call("samples", {"25"}, -1, PUT, oss);
            REQUIRE(oss.str() == "samples 25\n");
        }
        {
            std::ostringstream oss;
            caller.call("samples", {"450"}, -1, PUT, oss);
            REQUIRE(oss.str() == "samples 450\n");
        }
        {
            std::ostringstream oss;
            caller.call("samples", {}, -1, GET, oss);
            REQUIRE(oss.str() == "samples 450\n");
        }
        {
            std::ostringstream oss;
            caller.call("asamples", {}, -1, GET, oss);
            REQUIRE(oss.str() == "asamples 450\n");
        }
        if (det_type == defs::CHIPTESTBOARD ||
            det_type == defs::XILINX_CHIPTESTBOARD) {
            std::ostringstream oss;
            caller.call("dsamples", {}, -1, GET, oss);
            REQUIRE(oss.str() == "dsamples 450\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setNumberOfAnalogSamples(prev_asamples[i], {i});
            det.setNumberOfDigitalSamples(prev_dsamples[i], {i});
        }
    } else {
        REQUIRE_THROWS(caller.call("samples", {}, -1, GET));
    }
}

TEST_CASE("asamples", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::CHIPTESTBOARD ||
        det_type == defs::XILINX_CHIPTESTBOARD) {
        auto prev_val = det.getNumberOfAnalogSamples();
        {
            std::ostringstream oss;
            caller.call("asamples", {"25"}, -1, PUT, oss);
            REQUIRE(oss.str() == "asamples 25\n");
        }
        {
            std::ostringstream oss;
            caller.call("asamples", {"450"}, -1, PUT, oss);
            REQUIRE(oss.str() == "asamples 450\n");
        }
        {
            std::ostringstream oss;
            caller.call("asamples", {}, -1, GET, oss);
            REQUIRE(oss.str() == "asamples 450\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setNumberOfAnalogSamples(prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(caller.call("asamples", {}, -1, GET));
    }
}

TEST_CASE("adcclk", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::CHIPTESTBOARD) {
        auto prev_val = det.getADCClock();
        {
            std::ostringstream oss;
            caller.call("adcclk", {"20"}, -1, PUT, oss);
            REQUIRE(oss.str() == "adcclk 20\n");
        }
        {
            std::ostringstream oss;
            caller.call("adcclk", {"10"}, -1, PUT, oss);
            REQUIRE(oss.str() == "adcclk 10\n");
        }
        {
            std::ostringstream oss;
            caller.call("adcclk", {}, -1, GET, oss);
            REQUIRE(oss.str() == "adcclk 10\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setADCClock(prev_val[i], {i});
        }
    } else {
        // clock index might work
        // REQUIRE_THROWS(caller.call("adcclk", {}, -1, GET));
    }
}

TEST_CASE("runclk", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::CHIPTESTBOARD) {
        auto prev_val = det.getRUNClock();
        {
            std::ostringstream oss;
            caller.call("runclk", {"20"}, -1, PUT, oss);
            REQUIRE(oss.str() == "runclk 20\n");
        }
        {
            std::ostringstream oss;
            caller.call("runclk", {"10"}, -1, PUT, oss);
            REQUIRE(oss.str() == "runclk 10\n");
        }
        {
            std::ostringstream oss;
            caller.call("runclk", {}, -1, GET, oss);
            REQUIRE(oss.str() == "runclk 10\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setRUNClock(prev_val[i], {i});
        }
    } else {
        // clock index might work
        // REQUIRE_THROWS(caller.call("runclk", {}, -1, GET));
    }
}

TEST_CASE("syncclk", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::CHIPTESTBOARD) {
        REQUIRE_NOTHROW(caller.call("syncclk", {}, -1, GET));
    } else {
        // clock index might work
        // REQUIRE_THROWS(caller.call("syncclk", {}, -1, GET));
    }
}

TEST_CASE("v_limit", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::CHIPTESTBOARD ||
        det_type == defs::XILINX_CHIPTESTBOARD) {
        auto prev_val = det.getPower(defs::V_LIMIT);
        {
            std::ostringstream oss;
            caller.call("v_limit", {"1500"}, -1, PUT, oss);
            REQUIRE(oss.str() == "v_limit 1500\n");
        }
        {
            std::ostringstream oss;
            caller.call("v_limit", {"0"}, -1, PUT, oss);
            REQUIRE(oss.str() == "v_limit 0\n");
        }
        {
            std::ostringstream oss;
            caller.call("v_limit", {"0"}, -1, PUT, oss);
            REQUIRE(oss.str() == "v_limit 0\n");
        }
        {
            std::ostringstream oss;
            caller.call("v_limit", {}, -1, GET, oss);
            REQUIRE(oss.str() == "v_limit 0\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            if (prev_val[i] == -100) {
                prev_val[i] = 0;
            }
            det.setPower(defs::V_LIMIT, prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(caller.call("v_limit", {}, -1, GET));
    }
}

TEST_CASE("adcenable", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::CHIPTESTBOARD) {
        auto prev_val = det.getADCEnableMask();
        {
            std::ostringstream oss;
            caller.call("adcenable", {"0x8d0aa0d8"}, -1, PUT, oss);
            REQUIRE(oss.str() == "adcenable 0x8d0aa0d8\n");
        }
        {
            std::ostringstream oss;
            caller.call("adcenable", {"0xffffffff"}, -1, PUT, oss);
            REQUIRE(oss.str() == "adcenable 0xffffffff\n");
        }
        {
            std::ostringstream oss;
            caller.call("adcenable", {}, -1, GET, oss);
            REQUIRE(oss.str() == "adcenable 0xffffffff\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setADCEnableMask(prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(caller.call("adcenable", {}, -1, GET));
    }
}

TEST_CASE("adcenable10g", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::CHIPTESTBOARD ||
        det_type == defs::XILINX_CHIPTESTBOARD) {
        auto prev_val = det.getTenGigaADCEnableMask();
        {
            std::ostringstream oss;
            caller.call("adcenable10g", {"0xff0000ff"}, -1, PUT, oss);
            REQUIRE(oss.str() == "adcenable10g 0xff0000ff\n");
        }
        {
            std::ostringstream oss;
            caller.call("adcenable10g", {"0xffffffff"}, -1, PUT, oss);
            REQUIRE(oss.str() == "adcenable10g 0xffffffff\n");
        }
        {
            std::ostringstream oss;
            caller.call("adcenable10g", {}, -1, GET, oss);
            REQUIRE(oss.str() == "adcenable10g 0xffffffff\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setTenGigaADCEnableMask(prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(caller.call("adcenable10g", {}, -1, GET));
    }
}

TEST_CASE("transceiverenable", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::CHIPTESTBOARD ||
        det_type == defs::XILINX_CHIPTESTBOARD) {
        auto prev_val = det.getTransceiverEnableMask();
        {
            std::ostringstream oss;
            caller.call("transceiverenable", {"0x3"}, -1, PUT, oss);
            REQUIRE(oss.str() == "transceiverenable 0x3\n");
        }
        {
            std::ostringstream oss;
            caller.call("transceiverenable", {"0xf"}, -1, PUT, oss);
            REQUIRE(oss.str() == "transceiverenable 0xf\n");
        }
        {
            std::ostringstream oss;
            caller.call("transceiverenable", {}, -1, GET, oss);
            REQUIRE(oss.str() == "transceiverenable 0xf\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setTransceiverEnableMask(prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(caller.call("transceiverenable", {}, -1, GET));
    }
}

/* CTB Specific */

TEST_CASE("dsamples", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::CHIPTESTBOARD ||
        det_type == defs::XILINX_CHIPTESTBOARD) {
        auto prev_val = det.getNumberOfDigitalSamples();
        {
            std::ostringstream oss;
            caller.call("dsamples", {"1"}, -1, PUT, oss);
            REQUIRE(oss.str() == "dsamples 1\n");
        }
        {
            std::ostringstream oss;
            caller.call("dsamples", {"450"}, -1, PUT, oss);
            REQUIRE(oss.str() == "dsamples 450\n");
        }
        {
            std::ostringstream oss;
            caller.call("dsamples", {}, -1, GET, oss);
            REQUIRE(oss.str() == "dsamples 450\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setNumberOfDigitalSamples(prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(caller.call("dsamples", {}, -1, GET));
    }
}

TEST_CASE("tsamples", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::CHIPTESTBOARD ||
        det_type == defs::XILINX_CHIPTESTBOARD) {
        auto prev_val = det.getNumberOfTransceiverSamples();
        {
            std::ostringstream oss;
            caller.call("tsamples", {"1"}, -1, PUT, oss);
            REQUIRE(oss.str() == "tsamples 1\n");
        }
        {
            std::ostringstream oss;
            caller.call("tsamples", {"450"}, -1, PUT, oss);
            REQUIRE(oss.str() == "tsamples 450\n");
        }
        {
            std::ostringstream oss;
            caller.call("tsamples", {}, -1, GET, oss);
            REQUIRE(oss.str() == "tsamples 450\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setNumberOfTransceiverSamples(prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(caller.call("tsamples", {}, -1, GET));
    }
}

TEST_CASE("romode", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::CHIPTESTBOARD ||
        det_type == defs::XILINX_CHIPTESTBOARD) {
        auto prev_romode = det.getReadoutMode();
        auto prev_asamples = det.getNumberOfAnalogSamples();
        auto prev_dsamples = det.getNumberOfDigitalSamples();
        auto prev_tsamples = det.getNumberOfTransceiverSamples();
        det.setNumberOfAnalogSamples(5000);
        det.setNumberOfDigitalSamples(5000);
        det.setNumberOfTransceiverSamples(5000);
        {
            std::ostringstream oss;
            caller.call("romode", {"digital"}, -1, PUT, oss);
            REQUIRE(oss.str() == "romode digital\n");
        }
        {
            std::ostringstream oss;
            caller.call("romode", {"analog_digital"}, -1, PUT, oss);
            REQUIRE(oss.str() == "romode analog_digital\n");
        }
        {
            std::ostringstream oss;
            caller.call("romode", {"analog"}, -1, PUT, oss);
            REQUIRE(oss.str() == "romode analog\n");
        }
        {
            std::ostringstream oss;
            caller.call("romode", {}, -1, GET, oss);
            REQUIRE(oss.str() == "romode analog\n");
        }
        {
            std::ostringstream oss;
            caller.call("romode", {"transceiver"}, -1, PUT, oss);
            REQUIRE(oss.str() == "romode transceiver\n");
        }
        {
            std::ostringstream oss;
            caller.call("romode", {"digital_transceiver"}, -1, PUT, oss);
            REQUIRE(oss.str() == "romode digital_transceiver\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setReadoutMode(prev_romode[i], {i});
            det.setNumberOfAnalogSamples(prev_asamples[i], {i});
            det.setNumberOfDigitalSamples(prev_dsamples[i], {i});
            det.setNumberOfTransceiverSamples(prev_tsamples[i], {i});
        }
    } else {
        REQUIRE_THROWS(caller.call("romode", {}, -1, GET));
    }
}

TEST_CASE("dbitclk", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::CHIPTESTBOARD) {
        auto prev_val = det.getRUNClock();
        {
            std::ostringstream oss;
            caller.call("dbitclk", {"20"}, -1, PUT, oss);
            REQUIRE(oss.str() == "dbitclk 20\n");
        }
        {
            std::ostringstream oss;
            caller.call("dbitclk", {"10"}, -1, PUT, oss);
            REQUIRE(oss.str() == "dbitclk 10\n");
        }
        {
            std::ostringstream oss;
            caller.call("dbitclk", {}, -1, GET, oss);
            REQUIRE(oss.str() == "dbitclk 10\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setRUNClock(prev_val[i], {i});
        }
    } else {
        // clock index might work
        // REQUIRE_THROWS(caller.call("dbitclk", {}, -1, GET));
    }
}

TEST_CASE("v_a", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::CHIPTESTBOARD ||
        det_type == defs::XILINX_CHIPTESTBOARD) {
        auto prev_val = det.getPower(defs::V_POWER_A);
        {
            std::ostringstream oss1, oss2;
            caller.call("v_a", {"1200"}, -1, PUT, oss1);
            REQUIRE(oss1.str() == "v_a 1200\n");
            caller.call("v_a", {}, -1, GET, oss2);
            REQUIRE(oss2.str() == "v_a 1200\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setPower(defs::V_POWER_A, prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(caller.call("v_a", {}, -1, GET));
    }
}

TEST_CASE("v_b", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::CHIPTESTBOARD ||
        det_type == defs::XILINX_CHIPTESTBOARD) {
        auto prev_val = det.getPower(defs::V_POWER_B);
        {
            std::ostringstream oss1, oss2;
            caller.call("v_b", {"1200"}, -1, PUT, oss1);
            REQUIRE(oss1.str() == "v_b 1200\n");
            caller.call("v_b", {}, -1, GET, oss2);
            REQUIRE(oss2.str() == "v_b 1200\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setPower(defs::V_POWER_B, prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(caller.call("v_b", {}, -1, GET));
    }
}

TEST_CASE("v_c", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::CHIPTESTBOARD ||
        det_type == defs::XILINX_CHIPTESTBOARD) {
        auto prev_val = det.getPower(defs::V_POWER_C);
        {
            std::ostringstream oss1, oss2;
            caller.call("v_c", {"1200"}, -1, PUT, oss1);
            REQUIRE(oss1.str() == "v_c 1200\n");
            caller.call("v_c", {}, -1, GET, oss2);
            REQUIRE(oss2.str() == "v_c 1200\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setPower(defs::V_POWER_C, prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(caller.call("v_c", {}, -1, GET));
    }
}

TEST_CASE("v_d", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::CHIPTESTBOARD ||
        det_type == defs::XILINX_CHIPTESTBOARD) {
        auto prev_val = det.getPower(defs::V_POWER_D);
        {
            std::ostringstream oss1, oss2;
            caller.call("v_d", {"1200"}, -1, PUT, oss1);
            REQUIRE(oss1.str() == "v_d 1200\n");
            caller.call("v_d", {}, -1, GET, oss2);
            REQUIRE(oss2.str() == "v_d 1200\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setPower(defs::V_POWER_D, prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(caller.call("v_d", {}, -1, GET));
    }
}

TEST_CASE("v_io", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::CHIPTESTBOARD ||
        det_type == defs::XILINX_CHIPTESTBOARD) {
        // better not to play with setting it
        REQUIRE_NOTHROW(caller.call("v_io", {}, -1, GET));
    } else {
        REQUIRE_THROWS(caller.call("v_io", {}, -1, GET));
    }
}

TEST_CASE("v_chip", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::CHIPTESTBOARD) {
        // better not to play with setting it
        REQUIRE_NOTHROW(caller.call("v_chip", {}, -1, GET));
    } else {
        REQUIRE_THROWS(caller.call("v_chip", {}, -1, GET));
    }
}

TEST_CASE("vm_a", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::CHIPTESTBOARD) {
        REQUIRE_NOTHROW(caller.call("vm_a", {}, -1, GET));
    } else {
        REQUIRE_THROWS(caller.call("vm_a", {}, -1, GET));
    }
}

TEST_CASE("vm_b", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::CHIPTESTBOARD) {
        REQUIRE_NOTHROW(caller.call("vm_b", {}, -1, GET));
    } else {
        REQUIRE_THROWS(caller.call("vm_b", {}, -1, GET));
    }
}

TEST_CASE("vm_c", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::CHIPTESTBOARD) {
        REQUIRE_NOTHROW(caller.call("vm_c", {}, -1, GET));
    } else {
        REQUIRE_THROWS(caller.call("vm_c", {}, -1, GET));
    }
}

TEST_CASE("vm_d", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::CHIPTESTBOARD) {
        REQUIRE_NOTHROW(caller.call("vm_d", {}, -1, GET));
    } else {
        REQUIRE_THROWS(caller.call("vm_d", {}, -1, GET));
    }
}

TEST_CASE("vm_io", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::CHIPTESTBOARD) {
        REQUIRE_NOTHROW(caller.call("vm_io", {}, -1, GET));
    } else {
        REQUIRE_THROWS(caller.call("vm_io", {}, -1, GET));
    }
}

TEST_CASE("im_a", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::CHIPTESTBOARD) {
        REQUIRE_NOTHROW(caller.call("im_a", {}, -1, GET));
    } else {
        REQUIRE_THROWS(caller.call("im_a", {}, -1, GET));
    }
}

TEST_CASE("im_b", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::CHIPTESTBOARD) {
        REQUIRE_NOTHROW(caller.call("im_b", {}, -1, GET));
    } else {
        REQUIRE_THROWS(caller.call("im_b", {}, -1, GET));
    }
}

TEST_CASE("im_c", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::CHIPTESTBOARD) {
        REQUIRE_NOTHROW(caller.call("im_c", {}, -1, GET));
    } else {
        REQUIRE_THROWS(caller.call("im_c", {}, -1, GET));
    }
}

TEST_CASE("im_d", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::CHIPTESTBOARD) {
        REQUIRE_NOTHROW(caller.call("im_d", {}, -1, GET));
    } else {
        REQUIRE_THROWS(caller.call("im_d", {}, -1, GET));
    }
}

TEST_CASE("im_io", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::CHIPTESTBOARD) {
        REQUIRE_NOTHROW(caller.call("im_io", {}, -1, GET));
    } else {
        REQUIRE_THROWS(caller.call("im_io", {}, -1, GET));
    }
}

TEST_CASE("slowadc", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::CHIPTESTBOARD ||
        det_type == defs::XILINX_CHIPTESTBOARD) {
        for (int i = 0; i <= 7; ++i) {
            REQUIRE_NOTHROW(
                caller.call("slowadc", {std::to_string(i)}, -1, GET));
            REQUIRE_THROWS(caller.call("slowadc", {"0"}, -1, PUT));
        }
    } else {
        REQUIRE_THROWS(caller.call("slowadc", {"0"}, -1, GET));
    }
}

TEST_CASE("extsampling", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::CHIPTESTBOARD) {
        auto prev_val = det.getExternalSampling();
        {
            std::ostringstream oss;
            caller.call("extsampling", {"1"}, -1, PUT, oss);
            REQUIRE(oss.str() == "extsampling 1\n");
        }
        {
            std::ostringstream oss;
            caller.call("extsampling", {"0"}, -1, PUT, oss);
            REQUIRE(oss.str() == "extsampling 0\n");
        }
        {
            std::ostringstream oss;
            caller.call("extsampling", {}, -1, GET, oss);
            REQUIRE(oss.str() == "extsampling 0\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setExternalSampling(prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(caller.call("extsampling", {}, -1, GET));
    }
}

TEST_CASE("extsamplingsrc", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::CHIPTESTBOARD) {
        auto prev_val = det.getExternalSamplingSource();
        {
            std::ostringstream oss;
            caller.call("extsamplingsrc", {"63"}, -1, PUT, oss);
            REQUIRE(oss.str() == "extsamplingsrc 63\n");
        }
        {
            std::ostringstream oss;
            caller.call("extsamplingsrc", {"0"}, -1, PUT, oss);
            REQUIRE(oss.str() == "extsamplingsrc 0\n");
        }
        {
            std::ostringstream oss;
            caller.call("extsamplingsrc", {}, -1, GET, oss);
            REQUIRE(oss.str() == "extsamplingsrc 0\n");
        }
        REQUIRE_THROWS(caller.call("extsamplingsrc", {"64"}, -1, PUT));
        for (int i = 0; i != det.size(); ++i) {
            det.setExternalSamplingSource(prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(caller.call("extsamplingsrc", {}, -1, GET));
    }
}

TEST_CASE("diodelay", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::CHIPTESTBOARD) {
        {
            std::ostringstream oss;
            caller.call("diodelay", {"0x01010", "0"}, -1, PUT, oss);
            REQUIRE(oss.str() == "diodelay [0x01010, 0]\n");
        }
        {
            std::ostringstream oss;
            caller.call("diodelay", {"0x01010", "775"}, -1, PUT, oss);
            REQUIRE(oss.str() == "diodelay [0x01010, 775]\n");
        }
        REQUIRE_THROWS(caller.call("diodelay", {}, -1, GET));
        REQUIRE_THROWS(caller.call("diodelay", {"0x01010", "776"}, -1, GET));
    } else {
        REQUIRE_THROWS(caller.call("diodelay", {"0x01010", "775"}, -1, PUT));
    }
}

TEST_CASE("led", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::CHIPTESTBOARD) {
        auto prev_val = det.getLEDEnable();
        {
            std::ostringstream oss;
            caller.call("led", {"1"}, -1, PUT, oss);
            REQUIRE(oss.str() == "led 1\n");
        }
        {
            std::ostringstream oss;
            caller.call("led", {"0"}, -1, PUT, oss);
            REQUIRE(oss.str() == "led 0\n");
        }
        {
            std::ostringstream oss;
            caller.call("led", {}, -1, GET, oss);
            REQUIRE(oss.str() == "led 0\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setLEDEnable(prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(caller.call("led", {}, -1, GET));
    }
}

} // namespace sls
