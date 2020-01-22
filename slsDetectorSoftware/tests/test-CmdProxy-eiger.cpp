#include "CmdProxy.h"
#include "Detector.h"
#include "catch.hpp"
#include "sls_detector_defs.h"
#include <array>
#include <sstream>

#include "test-CmdProxy-global.h"
#include "tests/globals.h"
#include "versionAPI.h"

using sls::CmdProxy;
using sls::Detector;
using test::GET;
using test::PUT;

TEST_CASE("Eiger transmission delay", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    auto frame = det.getTransmissionDelayFrame();
    auto left = det.getTransmissionDelayLeft();
    auto right = det.getTransmissionDelayRight();
    if (det_type == defs::EIGER) {
        SECTION("txndelay_frame") {
            std::ostringstream oss1, oss2;
            proxy.Call("txndelay_frame", {"5000"}, -1, PUT, oss1);
            REQUIRE(oss1.str() == "txndelay_frame 5000\n");
            proxy.Call("txndelay_frame", {}, -1, GET, oss2);
            REQUIRE(oss2.str() == "txndelay_frame 5000\n");
        }
        SECTION("txndelay_left") {
            std::ostringstream oss1, oss2;
            proxy.Call("txndelay_left", {"5000"}, -1, PUT, oss1);
            REQUIRE(oss1.str() == "txndelay_left 5000\n");
            proxy.Call("txndelay_left", {}, -1, GET, oss2);
            REQUIRE(oss2.str() == "txndelay_left 5000\n");
        }
        SECTION("txndelay_right") {
            std::ostringstream oss1, oss2;
            proxy.Call("txndelay_right", {"5000"}, -1, PUT, oss1);
            REQUIRE(oss1.str() == "txndelay_right 5000\n");
            proxy.Call("txndelay_right", {}, -1, GET, oss2);
            REQUIRE(oss2.str() == "txndelay_right 5000\n");
        }

        // Reset to previous values
        for (int i = 0; i != det.size(); ++i) {
            det.setTransmissionDelayFrame(frame[i]);
            det.setTransmissionDelayLeft(left[i]);
            det.setTransmissionDelayRight(right[i]);
        }
    }
}

TEST_CASE("dr", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::EIGER) {
        // The only detector currently supporting setting dr
        // is EIGER?
        auto dr = det.getDynamicRange().squash();
        std::array<int, 4> vals{4, 8, 16, 32};
        for (const auto val : vals) {
            std::ostringstream oss1, oss2;
            proxy.Call("dr", {std::to_string(val)}, -1, PUT, oss1);
            REQUIRE(oss1.str() == "dr " + std::to_string(val) + '\n');
            proxy.Call("dr", {}, -1, GET, oss2);
            REQUIRE(oss2.str() == "dr " + std::to_string(val) + '\n');
        }
        det.setDynamicRange(dr);
    } else {
        // For the other detectors we should get an error message
        // except for dr 16
        REQUIRE_THROWS(proxy.Call("dr", {"4"}, -1, PUT));
        REQUIRE_THROWS(proxy.Call("dr", {"8"}, -1, PUT));
        REQUIRE_THROWS(proxy.Call("dr", {"32"}, -1, PUT));

        std::ostringstream oss1, oss2;
        proxy.Call("dr", {"16"}, -1, PUT, oss1);
        REQUIRE(oss1.str() == "dr 16\n");
        proxy.Call("dr", {"16"}, -1, PUT, oss2);
        REQUIRE(oss2.str() == "dr 16\n");
    }
}

TEST_CASE("interruptsubframe", "[.cmd][!mayfail]") {
    // TODO! Fix this for virtual server
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::EIGER) {
        auto previous = det.getInterruptSubframe();

        std::ostringstream oss1, oss2, oss3;
        proxy.Call("interruptsubframe", {"1"}, -1, PUT, oss1);
        REQUIRE(oss1.str() == "interruptsubframe 1\n");
        proxy.Call("interruptsubframe", {}, -1, GET, oss2);
        REQUIRE(oss2.str() == "interruptsubframe 1\n");
        proxy.Call("interruptsubframe", {"0"}, -1, PUT, oss3);
        REQUIRE(oss3.str() == "interruptsubframe 0\n");
        for (int i = 0; i != det.size(); ++i) {
            det.setInterruptSubframe(previous[i], {i});
        }

    } else {
        REQUIRE_THROWS(proxy.Call("interruptsubframe", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("interruptsubframe", {"1"}, -1, PUT));
    }
}

TEST_CASE("overflow", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::EIGER) {
        auto previous = det.getOverFlowMode();

        std::ostringstream oss1, oss2, oss3;
        proxy.Call("overflow", {"1"}, -1, PUT, oss1);
        REQUIRE(oss1.str() == "overflow 1\n");
        proxy.Call("overflow", {}, -1, GET, oss2);
        REQUIRE(oss2.str() == "overflow 1\n");
        proxy.Call("overflow", {"0"}, -1, PUT, oss3);
        REQUIRE(oss3.str() == "overflow 0\n");
        for (int i = 0; i != det.size(); ++i) {
            det.setOverFlowMode(previous[i], {i});
        }

    } else {
        REQUIRE_THROWS(proxy.Call("overflow", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("overflow", {"1"}, -1, PUT));
    }
}

TEST_CASE("trimen", "[.cmd][.this]") {
    // TODO! Also Mythen?
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::EIGER) {

        auto previous = det.getTrimEnergies();
        std::ostringstream oss1, oss2;
        proxy.Call("trimen", {"4500", "5400", "6400"}, -1, PUT, oss1);
        REQUIRE(oss1.str() == "trimen [4500, 5400, 6400]\n");
        proxy.Call("trimen", {}, -1, GET, oss2);
        REQUIRE(oss2.str() == "trimen [4500, 5400, 6400]\n");

        for (int i = 0; i != det.size(); ++i) {
            det.setTrimEnergies(previous[i], {i});
        }
    } else {
        REQUIRE_THROWS(proxy.Call("trimen", {"4500", "5400", "6400"}, -1, PUT));
        REQUIRE_THROWS(proxy.Call("trimen", {}, -1, GET));
    }
}

// TEST_CASE("threshold"{

// })

// TEST_CASE("activate", "[.cmd][.eiger]") {
//     if (test::type == slsDetectorDefs::EIGER) {
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("0:activate 1", PUT,
//             nullptr, oss)); REQUIRE(oss.str() == "activate 1\n");
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("0:activate 1 nopadding",
//             PUT, nullptr, oss)); REQUIRE(oss.str() == "activate 1
//             nopadding\n");
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("0:activate 0 padding",
//             PUT, nullptr, oss)); REQUIRE(oss.str() == "activate 0
//             padding\n");
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("0:activate 0 nopadding",
//             PUT, nullptr, oss)); REQUIRE(oss.str() == "activate 0
//             nopadding\n");
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("0:activate 1 padding",
//             PUT, nullptr, oss)); REQUIRE(oss.str() == "activate 1
//             padding\n");
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("0:activate", GET,
//             nullptr, oss)); REQUIRE(oss.str() == "activate 1 padding\n");
//         }
//     } else {
//         REQUIRE_THROWS(multiSlsDetectorClient("activate", GET));
//     }
// }

TEST_CASE("subexptime", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);

    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::EIGER) {
        auto time = det.getSubExptime();
        std::ostringstream oss1, oss2;
        proxy.Call("subexptime", {"2.5us"}, -1, PUT, oss1);
        REQUIRE(oss1.str() == "subexptime 2.5us\n");
        proxy.Call("subexptime", {}, -1, GET, oss2);
        REQUIRE(oss2.str() == "subexptime 2.5us\n");
        for (int i = 0; i != det.size(); ++i) {
            det.setSubExptime(time[i], {i});
        }
    } else {
        REQUIRE_THROWS(proxy.Call("subexptime", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("subexptime", {"2.13"}, -1, PUT));
    }
}

TEST_CASE("subdeadtime", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);

    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::EIGER) {
        auto time = det.getSubDeadTime();
        std::ostringstream oss1, oss2;
        proxy.Call("subdeadtime", {"500us"}, -1, PUT, oss1);
        REQUIRE(oss1.str() == "subdeadtime 500us\n");
        proxy.Call("subdeadtime", {}, -1, GET, oss2);
        REQUIRE(oss2.str() == "subdeadtime 500us\n");
        for (int i = 0; i != det.size(); ++i) {
            det.setSubDeadTime(time[i], {i});
        }
    } else {
        REQUIRE_THROWS(proxy.Call("subdeadtime", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("subdeadtime", {"2.13"}, -1, PUT));
    }
}

TEST_CASE("tengiga", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);

    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::EIGER || det_type == defs::CHIPTESTBOARD) {
        auto tengiga = det.getTenGiga();
        det.setTenGiga(false);

        std::ostringstream oss1, oss2;
        proxy.Call("tengiga", {"1"}, -1, PUT, oss1);
        REQUIRE(oss1.str() == "tengiga 1\n");
        proxy.Call("tengiga", {}, -1, GET, oss2);
        REQUIRE(oss2.str() == "tengiga 1\n");

        for (int i = 0; i != det.size(); ++i) {
            det.setTenGiga(tengiga[i], {i});
        }
    }
}

TEST_CASE("quad", "[.cmd]") {
    // TODO! set and get once available in virtual detector
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::EIGER) {
        // Quad only works with a single half module EIGER
        std::ostringstream oss;
        proxy.Call("quad", {}, -1, GET, oss);
        REQUIRE(oss.str() == "quad 0\n");
    } else {
        REQUIRE_THROWS(proxy.Call("quad", {}, -1, GET));
    }
}

TEST_CASE("Setting and reading back EIGER dacs", "[.cmd]") {
    // vsvp, vtr, vrf, vrs, vsvn, vtgstv, vcmp_ll, vcmp_lr, vcal, vcmp_rl,
    // rxb_rb, rxb_lb, vcmp_rr, vcp, vcn, vis, vthreshold
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::EIGER) {
        SECTION("vsvp") { test_dac(defs::SVP, "vsvp", 5); }
        SECTION("vtr") { test_dac(defs::VRF, "vtr", 1200); }
        SECTION("vrf") { test_dac(defs::VRF, "vrf", 1500); }
        SECTION("vrs") { test_dac(defs::VRF, "vrs", 1510); }
        SECTION("vsvn") { test_dac(defs::SVN, "vsvn", 3800); }
        SECTION("vtgstv") { test_dac(defs::VTGSTV, "vtgstv", 2550); }
        SECTION("vcmp_ll") { test_dac(defs::VCMP_LL, "vcmp_ll", 1400); }
        SECTION("vcmp_lr") { test_dac(defs::VCMP_LR, "vcmp_lr", 1400); }
        SECTION("vcal") { test_dac(defs::CAL, "vcal", 1400); }
        SECTION("vcmp_rl") { test_dac(defs::VCMP_RL, "vcmp_rl", 1400); }
        SECTION("rxb_rb") { test_dac(defs::RXB_RB, "rxb_rb", 1400); }
        SECTION("rxb_lb") { test_dac(defs::RXB_LB, "rxb_lb", 1400); }
        SECTION("vcmp_rr") { test_dac(defs::VCMP_RR, "vcmp_rr", 1400); }
        SECTION("vcp") { test_dac(defs::VCP, "vcp", 1400); }
        SECTION("vcn") { test_dac(defs::VCN, "vcn", 1400); }
        SECTION("vis") { test_dac(defs::VIS, "vis", 1400); }
        SECTION("iodelay") { test_dac(defs::IO_DELAY, "iodelay", 1400); }
        SECTION("vthreshold") {
            // Read out individual vcmp to be able to reset after
            // the test is done
            auto vcmp_ll = det.getDAC(defs::VCMP_LL, false);
            auto vcmp_lr = det.getDAC(defs::VCMP_LR, false);
            auto vcmp_rl = det.getDAC(defs::VCMP_RL, false);
            auto vcmp_rr = det.getDAC(defs::VCMP_RR, false);
            auto vcp = det.getDAC(defs::VCP, false);

            {
                std::ostringstream oss;
                proxy.Call("vthreshold", {"1234"}, -1, PUT, oss);
                REQUIRE(oss.str() == "vthreshold 1234\n");
            }
            {
                std::ostringstream oss;
                proxy.Call("vthreshold", {}, -1, GET, oss);
                REQUIRE(oss.str() == "vthreshold 1234\n");
            }

            // Reset dacs after test
            for (int i = 0; i != det.size(); ++i) {
                det.setDAC(defs::VCMP_LL, vcmp_ll[i], false, {i});
                det.setDAC(defs::VCMP_LR, vcmp_ll[i], false, {i});
                det.setDAC(defs::VCMP_RL, vcmp_ll[i], false, {i});
                det.setDAC(defs::VCMP_RR, vcmp_ll[i], false, {i});
                det.setDAC(defs::VCP, vcp[i], false, {i});
            }
        }
        REQUIRE_THROWS(proxy.Call("vref_ds", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vcascn_pb", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vcascp_pb", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vout_cm", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vcasc_out", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vin_cm", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vref_comp", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("ib_test_c", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vpreamp", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vshaper", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vshaperneg", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vipre", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("viinsh", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vdcsh", {}, -1, GET));
        // REQUIRE_THROWS(proxy.Call("vth1", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vth2", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vth3", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vpl", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vph", {}, -1, GET));
        // REQUIRE_THROWS(proxy.Call("vtrim", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vcassh", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vcas", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vicin", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vipre_out", {}, -1, GET));
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
        REQUIRE_THROWS(proxy.Call("vb_ds", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vb_comp", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vb_pixbuf", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vin_com", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("vdd_prot", {}, -1, GET));
    }
}

// TEST_CASE("trigger", "[.cmd]") {
//     Detector det;
//     CmdProxy proxy(&det);
//     auto det_type = det.getDetectorType().squash();
//     if (det_type != defs::EIGER) {
//         proxy.Call("trigger", {}, -1, PUT);
//     } else {

//         {
//             std::ostringstream oss;
//             proxy.Call("timing", {"trigger"}, -1, PUT, oss);
//             REQUIRE(oss.str() == "timing trigger\n");
//         }
//         auto startingfnum = det.getStartingFrameNumber().tsquash(
//             "inconsistent frame nr in test");
//         det.startDetector();

//         {
//             std::ostringstream oss;
//             proxy.Call("trigger", {}, -1, PUT, oss);
//             REQUIRE(oss.str() == "trigger successful\n");
//         }

//         auto currentfnum = det.getStartingFrameNumber().tsquash(
//             "inconsistent frame nr in test");

//         REQUIRE(startingfnum +1 == currentfnum);
//         det.stopDetector();
//         {
//             std::ostringstream oss;
//             proxy.Call("timing", {"auto"}, -1, PUT, oss);
//             REQUIRE(oss.str() == "timing auto\n");
//         }
//     }
//     if(test::type != slsDetectorDefs::EIGER) {
//         REQUIRE_THROWS(multiSlsDetectorClient("trigger", PUT));
//     } else {
//         // trigger
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("timing trigger", PUT,
//             nullptr, oss)); REQUIRE(oss.str() == "timing trigger\n");
//         }
//         int startingfnum = 0;
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("startingfnum", GET,
//             nullptr, oss)); std::string s = (oss.str()).erase (0,
//             strlen("startingfnum ")); startingfnum = std::stoi(s);
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("start", PUT, nullptr,
//             oss)); REQUIRE(oss.str() == "start successful\n");
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("status", GET,
//             nullptr, oss)); REQUIRE(oss.str() != "status idle\n");
//             REQUIRE(oss.str()
//             != "status stopped\n");
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("trigger", PUT,
//             nullptr, oss)); REQUIRE(oss.str() == "trigger successful\n");
//         }
//         REQUIRE_NOTHROW(multiSlsDetectorClient("stop", PUT));
//         int currentfnum = 0;
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("startingfnum", GET,
//             nullptr, oss)); std::string s = (oss.str()).erase (0,
//             strlen("startingfnum ")); currentfnum = std::stoi(s);
//         }
//         REQUIRE((startingfnum + 1) == currentfnum);

//         REQUIRE_NOTHROW(multiSlsDetectorClient("timing auto", PUT));
//     }
// }
