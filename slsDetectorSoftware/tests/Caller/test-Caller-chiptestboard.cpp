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

TEST_CASE("dacname", "[.detectorintegration]") {
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
        REQUIRE_THROWS(caller.call("dacname", {str_dac_index, "v_a"}, -1, PUT));
        REQUIRE_THROWS(caller.call("dacname", {str_dac_index, "v_b"}, -1, PUT));
        REQUIRE_THROWS(caller.call("dacname", {str_dac_index, "v_c"}, -1, PUT));
        REQUIRE_THROWS(caller.call("dacname", {str_dac_index, "v_d"}, -1, PUT));
        REQUIRE_THROWS(
            caller.call("dacname", {str_dac_index, "v_io"}, -1, PUT));
        det.setDacName(ind, prev);

    } else {
        REQUIRE_THROWS(caller.call("dacname", {"2", "b"}, -1, PUT));
        REQUIRE_THROWS(caller.call("dacname", {"2"}, -1, GET));
    }
}

TEST_CASE("dacindex", "[.detectorintegration]") {
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

TEST_CASE("adclist", "[.detectorintegration]") {
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

TEST_CASE("adcname", "[.detectorintegration]") {
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

TEST_CASE("adcindex", "[.detectorintegration]") {
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

TEST_CASE("signallist", "[.detectorintegration]") {
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

TEST_CASE("signalname", "[.detectorintegration]") {
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

TEST_CASE("signalindex", "[.detectorintegration]") {
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

TEST_CASE("powerlist", "[.detectorintegration]") {
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

TEST_CASE("powername", "[.detectorintegration]") {
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

TEST_CASE("powerindex", "[.detectorintegration]") {
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

TEST_CASE("powervalues", "[.detectorintegration]") {
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

TEST_CASE("slowadcvalues", "[.detectorintegration]") {
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

TEST_CASE("slowadclist", "[.detectorintegration]") {
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

TEST_CASE("slowadcname", "[.detectorintegration]") {
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

TEST_CASE("slowadcindex", "[.detectorintegration]") {
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

TEST_CASE("dac", "[.detectorintegration][dacs]") {
    // dac 0 to dac 17

    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::CHIPTESTBOARD ||
        det_type == defs::XILINX_CHIPTESTBOARD) {

        // normal dacs
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

        // ctb and xilinx
        REQUIRE_THROWS(caller.call("dac", {"18"}, -1, GET));
        REQUIRE_THROWS(caller.call("dac", {"5", "4096"}, -1, PUT));
        if (det_type == defs::CHIPTESTBOARD)
            REQUIRE_THROWS(caller.call("dac", {"5", "2501", "mV"}, -1, PUT));
        else
            REQUIRE_THROWS(caller.call("dac", {"5", "2049", "mV"}, -1, PUT));

        for (int idac = 0; idac < 18; ++idac) {
            SECTION("dac " + std::to_string(idac)) {
                test_dac_caller(static_cast<defs::dacIndex>(idac), "dac", 0);
                test_dac_caller(static_cast<defs::dacIndex>(idac), "dac", 1200);
                test_dac_caller(static_cast<defs::dacIndex>(idac), "dac", 1200,
                                true);
                test_dac_caller(static_cast<defs::dacIndex>(idac), "dac", -100);
                det.setDacName(static_cast<defs::dacIndex>(idac),
                               "dacname" + std::to_string(idac));

                test_dac_caller(defs::DAC_0, "dacname" + std::to_string(idac),
                                -100);
            }
            REQUIRE_THROWS(
                caller.call("dac", {std::to_string(idac), "-2"}, -1, PUT));
            REQUIRE_THROWS(
                caller.call("dac", {std::to_string(idac), "-1"}, -1, PUT));
        }

        // power dacs
        if (det.isVirtualDetectorServer().tsquash(
                "Inconsistent virtual servers")) {
            std::vector<std::string> names{"v_a", "v_b", "v_c", "v_d", "v_io"};
            std::vector<defs::dacIndex> indices{
                defs::V_POWER_A, defs::V_POWER_B, defs::V_POWER_C,
                defs::V_POWER_D, defs::V_POWER_IO};
            for (size_t iPower = 0; iPower < names.size(); ++iPower) {
                auto prev_val = det.getDAC(indices[iPower], true);
                auto prev_val_power = det.isPowerEnabled(indices[iPower]);

                // turn them off to be able to set the power dacs without issue
                det.setPowerEnabled(std::vector{indices[iPower]}, false);

                // this is the first command touching power dacs, should not be
                // -100
                if (det_type == defs::XILINX_CHIPTESTBOARD ||
                    det_type == defs::CHIPTESTBOARD) {
                    REQUIRE(prev_val.any(-100) == false);
                    REQUIRE(prev_val.any(-1) == false);
                    REQUIRE(prev_val.any(0) == false);
                }

                REQUIRE_THROWS(
                    caller.call("dac", {names[iPower], "-2"}, -1, PUT));
                REQUIRE_THROWS(
                    caller.call("dac", {names[iPower], "-100"}, -1, PUT));
                REQUIRE_THROWS(
                    caller.call("dac", {names[iPower], "-1"}, -1, PUT));
                REQUIRE_THROWS(
                    caller.call("dac", {names[iPower], "0"}, -1, PUT));
                REQUIRE_THROWS(
                    caller.call("dac", {names[iPower], "4096"}, -1, PUT));
                // not mV
                REQUIRE_THROWS(
                    caller.call("dac", {names[iPower], "1200"}, -1, PUT));
                REQUIRE_THROWS(caller.call("dac", {names[iPower]}, -1, GET));

                // min
                if (names[iPower] == "v_io")
                    REQUIRE_THROWS(caller.call(
                        "dac", {names[iPower], "1199", "mV"}, -1, PUT));
                else {
                    if (det_type == defs::XILINX_CHIPTESTBOARD) {
                        REQUIRE_THROWS(caller.call(
                            "dac", {names[iPower], "1040", "mV"}, -1, PUT));
                    } else {
                        REQUIRE_THROWS(caller.call(
                            "dac", {names[iPower], "635", "mV"}, -1, PUT));
                    }
                }
                // max
                if (det_type == defs::XILINX_CHIPTESTBOARD) {
                    REQUIRE_THROWS(caller.call(
                        "dac", {names[iPower], "2662", "mV"}, -1, PUT));
                } else {
                    REQUIRE_THROWS(caller.call(
                        "dac", {names[iPower], "2469", "mV"}, -1, PUT));
                }
                {
                    std::ostringstream oss1, oss2;
                    caller.call("dac", {names[iPower], "1200", "mV"}, -1, PUT,
                                oss1);
                    REQUIRE(oss1.str() ==
                            "dac " + names[iPower] + " 1200 mV\n");
                    caller.call("dac", {names[iPower], "mV"}, -1, GET, oss2);
                    REQUIRE(oss2.str() ==
                            "dac " + names[iPower] + " 1200 mV\n");
                }
                // throw if trying to set dac when power is on
                {
                    det.setPowerEnabled(std::vector{indices[iPower]}, true);
                    REQUIRE_THROWS(caller.call(
                        "dac", {names[iPower], "1200", "mV"}, -1, PUT));
                }
                // Reset all dacs to previous value
                for (int imod = 0; imod != det.size(); ++imod) {
                    det.setPowerEnabled(std::vector{indices[iPower]}, false,
                                        {imod});
                    det.setDAC(indices[iPower], prev_val[imod], true, {imod});
                    det.setPowerEnabled(std::vector{indices[iPower]},
                                        prev_val_power[imod], {imod});
                }
            }

            // test get of vchip
            if (det_type == defs::CHIPTESTBOARD) {
                REQUIRE_THROWS(
                    caller.call("dac", {"v_chip", "1700", "mV"}, -1, PUT));
                REQUIRE_THROWS(caller.call("dac", {"v_chip"}, -1, GET));
                REQUIRE_NOTHROW(caller.call("dac", {"v_chip", "mV"}, -1, GET));
            } else {
                REQUIRE_THROWS(caller.call("dac", {"v_chip", "mV"}, -1, GET));
            }
        }
    }
}

TEST_CASE("adcvpp", "[.detectorintegration]") {
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

TEST_CASE("samples", "[.detectorintegration]") {
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

TEST_CASE("asamples", "[.detectorintegration]") {
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

TEST_CASE("adcclk", "[.detectorintegration]") {
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

TEST_CASE("runclk", "[.detectorintegration]") {
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

TEST_CASE("syncclk", "[.detectorintegration]") {
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

TEST_CASE("v_limit", "[.detectorintegration]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::CHIPTESTBOARD ||
        det_type == defs::XILINX_CHIPTESTBOARD) {
        auto prev_val = det.getDAC(defs::V_LIMIT, true);
        auto prev_dac_val = det.getDAC(defs::DAC_0, false);

        REQUIRE_THROWS(caller.call("v_limit", {"1200", "mV"}, -1, PUT));
        REQUIRE_THROWS(caller.call("v_limit", {"-100"}, -1, PUT));
        REQUIRE_THROWS(caller.call("v_limit", {"-100", "mV"}, -1, PUT));
        REQUIRE_THROWS(caller.call("v_limit", {"0", "mV"}, -1, PUT));
        {
            std::ostringstream oss;
            caller.call("v_limit", {"0"}, -1, PUT, oss);
            REQUIRE(oss.str() == "v_limit 0 mV\n");
        }
        {
            std::ostringstream oss;
            caller.call("v_limit", {}, -1, GET, oss);
            REQUIRE(oss.str() == "v_limit 0 mV\n");
        }
        {
            std::ostringstream oss;
            caller.call("v_limit", {"1500"}, -1, PUT, oss);
            REQUIRE(oss.str() == "v_limit 1500 mV\n");
            REQUIRE_THROWS(caller.call("dac", {"0", "1501", "mV"}, -1, PUT));
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setDAC(defs::V_LIMIT, prev_val[i], true, {i});
            det.setDAC(defs::DAC_0, prev_dac_val[i], false, {i});
        }
    } else {
        REQUIRE_THROWS(caller.call("v_limit", {}, -1, GET));
    }
}

TEST_CASE("adcenable", "[.detectorintegration]") {
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

TEST_CASE("adcenable10g", "[.detectorintegration]") {
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

TEST_CASE("transceiverenable", "[.detectorintegration]") {
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

TEST_CASE("dsamples", "[.detectorintegration]") {
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

TEST_CASE("tsamples", "[.detectorintegration]") {
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
        if (det_type == defs::XILINX_CHIPTESTBOARD) {
            std::ostringstream oss;
            caller.call("tsamples", {"10000"}, -1, PUT, oss);
            REQUIRE(oss.str() == "tsamples 10000\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setNumberOfTransceiverSamples(prev_val[i], {i});
        }
    } else {
        REQUIRE_THROWS(caller.call("tsamples", {}, -1, GET));
    }
}

TEST_CASE("romode", "[.detectorintegration]") {
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

TEST_CASE("dbitclk", "[.detectorintegration]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::CHIPTESTBOARD) {
        auto prev_val = det.getDBITClock();
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
            det.setDBITClock(prev_val[i], {i});
        }
    } else {
        // clock index might work
        // REQUIRE_THROWS(caller.call("dbitclk", {}, -1, GET));
    }
}

TEST_CASE("v_abcd", "[.detectorintegration]") {
    Detector det;
    Caller caller(&det);

    // removed in favor of "dac" and "power" commands
    std::vector<std::string> cmds{"v_a", "v_b", "v_c", "v_d", "v_io", "v_chip"};
    std::vector<defs::dacIndex> indices{defs::V_POWER_A,  defs::V_POWER_B,
                                        defs::V_POWER_C,  defs::V_POWER_D,
                                        defs::V_POWER_IO, defs::V_POWER_CHIP};

    for (size_t i = 0; i < cmds.size(); ++i) {
        try {
            caller.call(cmds[i], {}, -1, GET);
        } catch (const std::exception &e) {
            REQUIRE(std::string(e.what()).find(
                        "removed and is no longer available") !=
                    std::string::npos);
        }
    }
}

TEST_CASE("power", "[.detectorintegration]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::CHIPTESTBOARD ||
        det_type == defs::XILINX_CHIPTESTBOARD) {

        std::vector<std::string> cmds{"v_a", "v_b", "v_c", "v_d", "v_io"};
        std::vector<defs::dacIndex> indices{defs::V_POWER_A, defs::V_POWER_B,
                                            defs::V_POWER_C, defs::V_POWER_D,
                                            defs::V_POWER_IO};

        for (size_t iPower = 0; iPower < cmds.size(); ++iPower) {
            auto prev_val = det.isPowerEnabled(indices[iPower]);

            REQUIRE_THROWS(caller.call("power", {"vrandom"}, -1, GET));
            REQUIRE_THROWS(caller.call("power", {"v_chip"}, -1, GET));
            REQUIRE_THROWS(caller.call("power", {"on", "v_a"}, -1, PUT));
            {
                std::ostringstream oss;
                caller.call("power", {"v_a", "on"}, -1, PUT, oss);
                REQUIRE(oss.str() == "power [v_a] on\n");
            }
            {
                std::ostringstream oss;
                caller.call("power", {"v_a"}, -1, GET, oss);
                REQUIRE(oss.str() == "power v_a on\n");
            }
            {
                std::ostringstream oss;
                caller.call("power", {"v_a", "v_c", "on"}, -1, PUT, oss);
                REQUIRE(oss.str() == "power [v_a, v_c] on\n");
            }
            {
                std::ostringstream oss1, oss2, oss3;
                caller.call("power", {"v_a", "v_b", "off"}, -1, PUT);
                caller.call("power", {"v_a"}, -1, GET, oss1);
                caller.call("power", {"v_b"}, -1, GET, oss2);
                caller.call("power", {"v_c"}, -1, GET, oss3);
                REQUIRE(oss1.str() == "power v_a off\n");
                REQUIRE(oss2.str() == "power v_b off\n");
                REQUIRE(oss3.str() == "power v_c on\n");
            }
            { // power chip
                caller.call("powerchip", {"1"}, -1, PUT);
                std::ostringstream oss1, oss2, oss3, oss4, oss5;
                caller.call("power", {"v_a"}, -1, GET, oss1);
                caller.call("power", {"v_b"}, -1, GET, oss2);
                caller.call("power", {"v_c"}, -1, GET, oss3);
                caller.call("power", {"v_d"}, -1, GET, oss4);
                caller.call("power", {"v_io"}, -1, GET, oss5);
                REQUIRE(oss1.str() == "power v_a on\n");
                REQUIRE(oss2.str() == "power v_b on\n");
                REQUIRE(oss3.str() == "power v_c on\n");
                REQUIRE(oss4.str() == "power v_d on\n");
                REQUIRE(oss5.str() == "power v_io on\n");
            }
            for (int imod = 0; imod != det.size(); ++imod) {
                det.setPowerEnabled(std::vector{indices[iPower]},
                                    prev_val[imod], {imod});
            }
        }
    } else {
        REQUIRE_THROWS(caller.call("power", {"v_a"}, -1, GET));
    }
}

TEST_CASE("vm_a", "[.detectorintegration]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::CHIPTESTBOARD) {
        REQUIRE_NOTHROW(caller.call("vm_a", {}, -1, GET));
    } else {
        REQUIRE_THROWS(caller.call("vm_a", {}, -1, GET));
    }
}

TEST_CASE("vm_b", "[.detectorintegration]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::CHIPTESTBOARD) {
        REQUIRE_NOTHROW(caller.call("vm_b", {}, -1, GET));
    } else {
        REQUIRE_THROWS(caller.call("vm_b", {}, -1, GET));
    }
}

TEST_CASE("vm_c", "[.detectorintegration]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::CHIPTESTBOARD) {
        REQUIRE_NOTHROW(caller.call("vm_c", {}, -1, GET));
    } else {
        REQUIRE_THROWS(caller.call("vm_c", {}, -1, GET));
    }
}

TEST_CASE("vm_d", "[.detectorintegration]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::CHIPTESTBOARD) {
        REQUIRE_NOTHROW(caller.call("vm_d", {}, -1, GET));
    } else {
        REQUIRE_THROWS(caller.call("vm_d", {}, -1, GET));
    }
}

TEST_CASE("vm_io", "[.detectorintegration]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::CHIPTESTBOARD) {
        REQUIRE_NOTHROW(caller.call("vm_io", {}, -1, GET));
    } else {
        REQUIRE_THROWS(caller.call("vm_io", {}, -1, GET));
    }
}

TEST_CASE("im_a", "[.detectorintegration]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::CHIPTESTBOARD) {
        REQUIRE_NOTHROW(caller.call("im_a", {}, -1, GET));
    } else {
        REQUIRE_THROWS(caller.call("im_a", {}, -1, GET));
    }
}

TEST_CASE("im_b", "[.detectorintegration]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::CHIPTESTBOARD) {
        REQUIRE_NOTHROW(caller.call("im_b", {}, -1, GET));
    } else {
        REQUIRE_THROWS(caller.call("im_b", {}, -1, GET));
    }
}

TEST_CASE("im_c", "[.detectorintegration]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::CHIPTESTBOARD) {
        REQUIRE_NOTHROW(caller.call("im_c", {}, -1, GET));
    } else {
        REQUIRE_THROWS(caller.call("im_c", {}, -1, GET));
    }
}

TEST_CASE("im_d", "[.detectorintegration]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::CHIPTESTBOARD) {
        REQUIRE_NOTHROW(caller.call("im_d", {}, -1, GET));
    } else {
        REQUIRE_THROWS(caller.call("im_d", {}, -1, GET));
    }
}

TEST_CASE("im_io", "[.detectorintegration]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::CHIPTESTBOARD) {
        REQUIRE_NOTHROW(caller.call("im_io", {}, -1, GET));
    } else {
        REQUIRE_THROWS(caller.call("im_io", {}, -1, GET));
    }
}

TEST_CASE("slowadc", "[.detectorintegration]") {
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

TEST_CASE("extsampling", "[.detectorintegration]") {
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

TEST_CASE("extsamplingsrc", "[.detectorintegration]") {
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

TEST_CASE("diodelay", "[.detectorintegration]") {
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

TEST_CASE("led", "[.detectorintegration]") {
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

TEST_CASE("define_reg", "[.detectorintegration][reg]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::CHIPTESTBOARD ||
        det_type == defs::XILINX_CHIPTESTBOARD) {

        auto prev_reg_defines = det.getRegisterDefinitions();
        auto prev_bit_defines = det.getBitDefinitions();
        det.clearRegisterDefinitions();
        det.clearBitDefinitions();

        {
            // invalid puts
            // missing arg
            REQUIRE_THROWS(caller.call("define_reg", {}, -1, GET));
            // missing arg
            REQUIRE_THROWS(caller.call("define_reg", {"TEST_REG"}, -1, PUT));
            // invalid module id
            REQUIRE_THROWS(
                caller.call("define_reg", {"TEST_REG", "0x201"}, 0, PUT));

            // valid put
            REQUIRE_NOTHROW(
                caller.call("define_reg", {"TEST_REG", "0x200"}, -1, PUT));
            // modify reg
            REQUIRE_NOTHROW(
                caller.call("define_reg", {"TEST_REG", "0x201"}, -1, PUT));
            REQUIRE_NOTHROW(
                caller.call("define_reg", {"TEST_REG2", "0x202"}, -1, PUT));

            // invalid puts
            // existing reg addr
            REQUIRE_THROWS(
                caller.call("define_reg", {"TEST_REG3", "0x201"}, -1, PUT));

            // valid gets
            {
                // get by name
                std::ostringstream oss;
                REQUIRE_NOTHROW(
                    caller.call("define_reg", {"TEST_REG"}, -1, GET, oss));
                REQUIRE(oss.str() == "define_reg 0x201\n");
            }
            {
                // get by addr
                std::ostringstream oss;
                REQUIRE_NOTHROW(
                    caller.call("define_reg", {"0x201"}, -1, GET, oss));
                REQUIRE(oss.str() == "define_reg TEST_REG\n");
            }

            // invalid gets
            // doesnt exist
            REQUIRE_THROWS(caller.call("define_reg", {"TEST_REG3"}, -1, GET));
            REQUIRE_THROWS(caller.call("define_reg", {"0x203"}, -1, GET));
            // ensure correct exception message
            try {
                caller.call("define_reg", {"0x203"}, -1, GET);
            } catch (const std::exception &e) {
                REQUIRE(std::string(e.what()).find(
                            "No entry found for value") != std::string::npos);
            }
        }
        det.clearRegisterDefinitions();
        det.clearBitDefinitions();

        det.setRegisterDefinitions(prev_reg_defines);
        det.setBitDefinitions(prev_bit_defines);

    } else {
        REQUIRE_THROWS(
            caller.call("define_reg", {"TEST_REG", "0x200"}, -1, PUT));
        REQUIRE_THROWS(caller.call("define_reg", {"TEST_REG"}, -1, GET));
    }
}

TEST_CASE("define_bit", "[.detectorintegration][reg]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::CHIPTESTBOARD ||
        det_type == defs::XILINX_CHIPTESTBOARD) {

        auto prev_reg_defines = det.getRegisterDefinitions();
        auto prev_bit_defines = det.getBitDefinitions();

        det.clearRegisterDefinitions();
        det.clearBitDefinitions();
        det.setRegisterDefinition("TEST_REG", RegisterAddress(0x201));
        det.setRegisterDefinition("TEST_REG2", RegisterAddress(0x202));

        {
            // invalid puts
            // skipped register
            REQUIRE_THROWS(
                caller.call("define_bit", {"TEST_BIT", "1"}, -1, PUT));
            // named register doesnt exist
            REQUIRE_THROWS(caller.call(
                "define_bit", {"TEST_BIT", "RANDOM_REG", "1"}, -1, PUT));
            // invalid bit position
            REQUIRE_THROWS(
                caller.call("define", {"TEST_BIT", "TEST_REG", "32"}, -1, PUT));

            // valid puts
            REQUIRE_NOTHROW(caller.call(
                "define_bit", {"TEST_BIT", "TEST_REG2", "1"}, -1, PUT));
            // modify reg
            REQUIRE_NOTHROW(caller.call(
                "define_bit", {"TEST_BIT", "TEST_REG", "1"}, -1, PUT));

            // modify position
            REQUIRE_NOTHROW(caller.call(
                "define_bit", {"TEST_BIT", "TEST_REG", "2"}, -1, PUT));
            // another bit to same reg
            REQUIRE_NOTHROW(caller.call(
                "define_bit", {"TEST_BIT2", "TEST_REG", "4"}, -1, PUT));
            // bit to a different reg
            REQUIRE_NOTHROW(caller.call(
                "define_bit", {"TEST_BIT3", "TEST_REG2", "3"}, -1, PUT));

            // valid gets
            {
                // get by name
                std::ostringstream oss;
                REQUIRE_NOTHROW(
                    caller.call("define_bit", {"TEST_BIT"}, -1, GET, oss));
                REQUIRE(oss.str() == "define_bit [TEST_REG, 2]\n");
            }
            {
                // get by addr+pos name
                std::ostringstream oss;
                REQUIRE_NOTHROW(
                    caller.call("define_bit", {"TEST_REG", "2"}, -1, GET, oss));
                REQUIRE(oss.str() == "define_bit TEST_BIT\n");
            }
            {
                // get by addr val + pos
                std::ostringstream oss;
                REQUIRE_NOTHROW(
                    caller.call("define_bit", {"0x201", "2"}, -1, GET, oss));
                REQUIRE(oss.str() == "define_bit TEST_BIT\n");
            }

            // invalid gets
            // bit doesnt exist
            REQUIRE_THROWS(
                caller.call("define_bit", {"TEST_REG", "3"}, -1, GET));
            // addr doesnt exist
            REQUIRE_THROWS(
                caller.call("define_bit", {"TEST_REG3", "2"}, -1, GET));
            // ensure correct exception message
            try {
                caller.call("define_bit", {"TEST_REG", "3"}, -1, GET);
            } catch (const std::exception &e) {
                REQUIRE(std::string(e.what()).find(
                            "No entry found for value") != std::string::npos);
            }
        }
        det.clearRegisterDefinitions();
        det.clearBitDefinitions();

        det.setRegisterDefinitions(prev_reg_defines);
        det.setBitDefinitions(prev_bit_defines);

    } else {
        REQUIRE_THROWS(
            caller.call("define_bit", {"TEST_BIT", "0x200", "2"}, -1, PUT));
        REQUIRE_THROWS(caller.call("define_bit", {"0x200", "2"}, -1, GET));
    }
}

TEST_CASE("using define for reg, setbit, getbit and clearbit",
          "[.detectorintegration][reg]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::CHIPTESTBOARD ||
        det_type == defs::XILINX_CHIPTESTBOARD) {

        if (det.isVirtualDetectorServer().tsquash(
                "inconsistent virtual values")) {

            auto prev_reg_defines = det.getRegisterDefinitions();
            auto prev_bit_defines = det.getBitDefinitions();

            det.clearRegisterDefinitions();
            det.clearBitDefinitions();
            det.setRegisterDefinition("TEST_REG", RegisterAddress(0x201));
            det.setRegisterDefinition("TEST_REG2", RegisterAddress(0x202));
            det.setBitDefinition("TEST_BIT",
                                 BitAddress(RegisterAddress(0x201), 2));
            det.setBitDefinition("TEST_BIT2",
                                 BitAddress(RegisterAddress(0x201), 4));
            det.setBitDefinition("TEST_BIT3",
                                 BitAddress(RegisterAddress(0x202), 3));

            auto prev_val_addr = det.readRegister(RegisterAddress(0x201));
            auto prev_val_addr2 = det.readRegister(RegisterAddress(0x202));

            // invalid puts
            // doesnt exist addr
            REQUIRE_THROWS(
                caller.call("reg", {"RANDOM_REG", "0xf00"}, -1, PUT));
            REQUIRE_THROWS(
                caller.call("clearbit", {"RANDOM_REG", "TEST_BIT"}, -1, PUT));
            REQUIRE_THROWS(
                caller.call("setbit", {"RANDOM_REG", "TEST_BIT"}, -1, PUT));
            REQUIRE_THROWS(
                caller.call("getbit", {"RANDOM_REG", "TEST_BIT"}, -1, GET));
            // using bit name for reg (only hardcoded values allowed)
            REQUIRE_THROWS(
                caller.call("reg", {"TEST_REG", "TEST_BIT"}, -1, PUT));
            // using bit name and reg (only bit names or both reg and bit
            // hardcoded allowed)
            REQUIRE_THROWS(
                caller.call("clearbit", {"TEST_REG", "TEST_BIT"}, -1, PUT));
            REQUIRE_THROWS(
                caller.call("setbit", {"TEST_REG", "TEST_BIT"}, -1, PUT));
            REQUIRE_THROWS(
                caller.call("getbit", {"TEST_REG", "TEST_BIT"}, -1, GET));

            // valid puts and gets
            {
                // reg hard coded value of 0
                std::ostringstream oss;
                REQUIRE_NOTHROW(
                    caller.call("reg", {"TEST_REG", "0x0"}, -1, PUT));
                REQUIRE_NOTHROW(caller.call("reg", {"TEST_REG"}, -1, GET, oss));
                REQUIRE(oss.str() == "reg 0x0\n");
            }
            {
                // reg hard coded value
                std::ostringstream oss;
                REQUIRE_NOTHROW(
                    caller.call("reg", {"TEST_REG", "0x10"}, -1, PUT));
                REQUIRE_NOTHROW(caller.call("reg", {"TEST_REG"}, -1, GET, oss));
                REQUIRE(oss.str() == "reg 0x10\n");
            }
            {
                // set bit
                std::ostringstream oss;
                REQUIRE_NOTHROW(caller.call("setbit", {"TEST_BIT"}, -1, PUT));
                REQUIRE_NOTHROW(
                    caller.call("setbit", {"TEST_REG", "2"}, -1, PUT));
                REQUIRE_NOTHROW(caller.call("reg", {"TEST_REG"}, -1, GET, oss));
                REQUIRE(oss.str() == "reg 0x14\n");
            }
            {
                // get bit
                std::ostringstream oss, oss2;
                REQUIRE_NOTHROW(
                    caller.call("getbit", {"TEST_REG", "2"}, -1, GET, oss));
                REQUIRE(oss.str() == "getbit 1\n");
                REQUIRE_NOTHROW(
                    caller.call("getbit", {"TEST_BIT"}, -1, GET, oss2));
                REQUIRE(oss2.str() == "getbit 1\n");
            }
            {
                // clear bit
                std::ostringstream oss;
                REQUIRE_NOTHROW(caller.call("clearbit", {"TEST_BIT"}, -1, PUT));
                REQUIRE_NOTHROW(
                    caller.call("clearbit", {"TEST_REG", "2"}, -1, PUT));
                REQUIRE_NOTHROW(caller.call("reg", {"TEST_REG"}, -1, GET, oss));
                REQUIRE(oss.str() == "reg 0x10\n");
            }
            for (int i = 0; i != det.size(); ++i) {
                det.writeRegister(RegisterAddress(0x201),
                                  RegisterValue(prev_val_addr[i]), false, {i});
                det.writeRegister(RegisterAddress(0x202),
                                  RegisterValue(prev_val_addr2[i]), false, {i});
            }
            det.clearRegisterDefinitions();
            det.clearBitDefinitions();

            det.setRegisterDefinitions(prev_reg_defines);
            det.setBitDefinitions(prev_bit_defines);
        }

    } else {
        REQUIRE_THROWS(caller.call("reg", {"TEST_REG", "0x200"}, -1, PUT));
        REQUIRE_THROWS(caller.call("reg", {"TEST_REG"}, -1, GET));
    }
}

TEST_CASE("definelist_reg", "[.detectorintegration][reg]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::CHIPTESTBOARD ||
        det_type == defs::XILINX_CHIPTESTBOARD) {

        auto prev_reg_defines = det.getRegisterDefinitions();

        det.clearRegisterDefinitions();
        det.clearBitDefinitions();
        det.setRegisterDefinition("TEST_REG", RegisterAddress(0x201));
        det.setRegisterDefinition("TEST_REG2", RegisterAddress(0x202));

        // invalid
        // cannot put
        REQUIRE_THROWS(
            caller.call("definelist_reg", {"TEST_REG", "0x201"}, -1, PUT));
        // too many args
        REQUIRE_THROWS(caller.call("definelist_reg", {"TEST_MACRO"}, -1, GET));

        // valid
        REQUIRE_NOTHROW(caller.call("definelist_reg", {}, -1, GET));
        det.clearRegisterDefinitions();
        det.clearBitDefinitions();

        det.setRegisterDefinitions(prev_reg_defines);
    } else {
        REQUIRE_THROWS(caller.call("definelist_reg", {}, -1, GET));
    }
}

TEST_CASE("definelist_bit", "[.detectorintegration][reg]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::CHIPTESTBOARD ||
        det_type == defs::XILINX_CHIPTESTBOARD) {

        auto prev_reg_defines = det.getRegisterDefinitions();
        auto prev_bit_defines = det.getBitDefinitions();

        det.clearRegisterDefinitions();
        det.clearBitDefinitions();
        det.setRegisterDefinition("TEST_REG", RegisterAddress(0x201));
        det.setRegisterDefinition("TEST_REG2", RegisterAddress(0x202));
        det.setBitDefinition("TEST_BIT", BitAddress(RegisterAddress(0x201), 2));
        det.setBitDefinition("TEST_BIT2",
                             BitAddress(RegisterAddress(0x201), 4));
        det.setBitDefinition("TEST_BIT3",
                             BitAddress(RegisterAddress(0x202), 3));

        // invalid
        // cannot put
        REQUIRE_THROWS(
            caller.call("definelist_bit", {"TEST_BIT", "0x201", "2"}, -1, PUT));
        // too many args
        REQUIRE_THROWS(caller.call("definelist_bit", {"TEST_BIT"}, -1, GET));

        // valid
        REQUIRE_NOTHROW(caller.call("definelist_bit", {}, -1, GET));
        det.clearRegisterDefinitions();
        det.clearBitDefinitions();

        det.setRegisterDefinitions(prev_reg_defines);
        det.setBitDefinitions(prev_bit_defines);
    } else {
        REQUIRE_THROWS(caller.call("definelist_bit", {}, -1, GET));
    }
}

} // namespace sls
