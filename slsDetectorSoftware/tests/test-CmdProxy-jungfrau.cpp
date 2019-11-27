#include "CmdProxy.h"
#include "Detector.h"
#include "catch.hpp"
#include "sls_detector_defs.h"
#include <sstream>

#include "tests/globals.h"
#include "versionAPI.h"

using sls::CmdProxy;
using sls::Detector;
using test::GET;
using test::PUT;

TEST_CASE("powerchip", "[.cmd][!mayfail]") {
    // TODO! this test currently fails with the
    // virtual detecto server
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::JUNGFRAU) {
        auto pc = det.getPowerChip();
        {
            std::ostringstream oss;
            proxy.Call("powerchip", {"1"}, -1, PUT, oss);
            REQUIRE(oss.str() == "powerchip 1\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("powerchip", {"0"}, -1, PUT, oss);
            REQUIRE(oss.str() == "powerchip 0\n");
        }
        {
            std::ostringstream oss;
            proxy.Call("powerchip", {}, -1, GET, oss);
            REQUIRE(oss.str() == "powerchip 0\n");
        }
        for (int i = 0; i != det.size(); ++i) {
            det.setPowerChip(pc[i], {i});
        }
    } else {
        REQUIRE_THROWS(proxy.Call("powerchip", {}, -1, GET));
    }
}

TEST_CASE("nframes", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU || det_type == defs::CHIPTESTBOARD) {
        auto nframes = det.getNumberOfFramesFromStart().squash();
        std::ostringstream oss;
        proxy.Call("nframes", {}, -1, GET, oss);
        REQUIRE(oss.str() == "nframes " + std::to_string(nframes) + "\n");

    } else {
        REQUIRE_THROWS(proxy.Call("nframes", {}, -1, GET));
    }
}

TEST_CASE("now", "[.cmd]") {
    // TODO! can we test this?
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU || det_type == defs::CHIPTESTBOARD) {
        std::ostringstream oss;
        proxy.Call("now", {}, -1, GET, oss);

        // Get only
        REQUIRE_THROWS(proxy.Call("now", {"2019"}, -1, PUT));
    } else {
        REQUIRE_THROWS(proxy.Call("now", {}, -1, GET));
    }
}

TEST_CASE("timestamp", "[.cmd]") {
    // TODO! can we test this?
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU || det_type == defs::CHIPTESTBOARD) {
        std::ostringstream oss;
        proxy.Call("timestamp", {}, -1, GET, oss);

        // Get only
        REQUIRE_THROWS(proxy.Call("timestamp", {"2019"}, -1, PUT));
    } else {
        REQUIRE_THROWS(proxy.Call("timestamp", {}, -1, GET));
    }
}

TEST_CASE("adcreg", "[.cmd]") {
    //TODO! what is a safe value to use? 
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU || det_type == defs::CHIPTESTBOARD ||
        det_type == defs::GOTTHARD) {
        std::ostringstream oss;
        proxy.Call("adcreg", {"0x0", "0"}, -1, PUT, oss);
        REQUIRE(oss.str() == "adcreg [0x0, 0]\n");
        // This is a put only command
        REQUIRE_THROWS(proxy.Call("adcreg", {}, -1, GET));
    } else {
        REQUIRE_THROWS(proxy.Call("adcreg", {"0x0", "0"}, -1, PUT));
        REQUIRE_THROWS(proxy.Call("adcreg", {}, -1, GET));
    }
}

TEST_CASE("bustest", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU || det_type == defs::CHIPTESTBOARD ||
        det_type == defs::GOTTHARD) {
        std::ostringstream oss;
        proxy.Call("bustest", {}, -1, PUT, oss);
        REQUIRE(oss.str() == "bustest successful\n");
        REQUIRE_THROWS(proxy.Call("bustest", {}, -1, GET));
    } else {
        REQUIRE_THROWS(proxy.Call("bustest", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("bustest", {}, -1, PUT));
    }
}

TEST_CASE("firmwaretest", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU || det_type == defs::CHIPTESTBOARD ||
        det_type == defs::GOTTHARD) {
        std::ostringstream oss;
        proxy.Call("firmwaretest", {}, -1, PUT, oss);
        REQUIRE(oss.str() == "firmwaretest successful\n");
        REQUIRE_THROWS(proxy.Call("firmwaretest", {}, -1, GET));
    } else {
        REQUIRE_THROWS(proxy.Call("firmwaretest", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("firmwaretest", {}, -1, PUT));
    }
}

TEST_CASE("resetfpga", "[.cmd]") {
    Detector det;
    CmdProxy proxy(&det);
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::JUNGFRAU || det_type == defs::CHIPTESTBOARD) {
        std::ostringstream oss;
        proxy.Call("resetfpga", {}, -1, PUT, oss);
        REQUIRE(oss.str() == "resetfpga successful\n");
        REQUIRE_THROWS(proxy.Call("resetfpga", {}, -1, GET));
    } else {
        REQUIRE_THROWS(proxy.Call("resetfpga", {}, -1, GET));
        REQUIRE_THROWS(proxy.Call("resetfpga", {}, -1, PUT));
    }
}



// void test_dac(defs::dacIndex index, const std::string &dacname, int dacvalue)
// {
//     Detector det;
//     CmdProxy proxy(&det);
//     std::ostringstream oss_set, oss_get;
//     auto dacstr = std::to_string(dacvalue);
//     auto previous = det.getDAC(index, false);
//     proxy.Call(dacname, {dacstr}, -1, PUT, oss_set);
//     REQUIRE(oss_set.str() == dacname + " " + dacstr + "\n");
//     proxy.Call(dacname, {}, -1, GET, oss_get);
//     REQUIRE(oss_set.str() == dacname + " " + dacstr + "\n");
//     // Reset all dacs to previous value
//     for (int i = 0; i != det.size(); ++i) {
//         det.setDAC(index, previous[i], false, {i});
//     }
// }

// TEST_CASE("Setting and reading back EIGER dacs", "[.cmd]") {
//     // vsvp, vtr, vrf, vrs, vsvn, vtgstv, vcmp_ll, vcmp_lr, vcal, vcmp_rl,
//     // rxb_rb, rxb_lb, vcmp_rr, vcp, vcn, vis, vthreshold
//     Detector det;
//     CmdProxy proxy(&det);
//     auto det_type = det.getDetectorType().squash();
//     if (det_type == defs::EIGER) {
//         SECTION("vsvp") { test_dac(defs::SVP, "vsvp", 5); }
//         SECTION("vtr") { test_dac(defs::VRF, "vtr", 1200); }
//         SECTION("vrf") { test_dac(defs::VRF, "vrf", 1500); }
//         SECTION("vrs") { test_dac(defs::VRF, "vrs", 1510); }
//         SECTION("vsvn") { test_dac(defs::SVN, "vsvn", 3800); }
//         SECTION("vtgstv") { test_dac(defs::VTGSTV, "vtgstv", 2550); }
//         SECTION("vcmp_ll") { test_dac(defs::VCMP_LL, "vcmp_ll", 1400); }
//         SECTION("vcmp_lr") { test_dac(defs::VCMP_LR, "vcmp_lr", 1400); }
//         SECTION("vcal") { test_dac(defs::CAL, "vcal", 1400); }
//         SECTION("vcmp_rl") { test_dac(defs::VCMP_RL, "vcmp_rl", 1400); }
//         SECTION("rxb_rb") { test_dac(defs::RXB_RB, "rxb_rb", 1400); }
//         SECTION("rxb_lb") { test_dac(defs::RXB_LB, "rxb_lb", 1400); }
//         SECTION("vcmp_rr") { test_dac(defs::VCMP_RR, "vcmp_rr", 1400); }
//         SECTION("vcp") { test_dac(defs::VCP, "vcp", 1400); }
//         SECTION("vcn") { test_dac(defs::VCN, "vcn", 1400); }
//         SECTION("vis") { test_dac(defs::VIS, "vis", 1400); }
//         SECTION("iodelay") { test_dac(defs::IO_DELAY, "iodelay", 1400); }
//         SECTION("vthreshold") {
//             // Read out individual vcmp to be able to reset after
//             // the test is done
//             auto vcmp_ll = det.getDAC(defs::VCMP_LL, false);
//             auto vcmp_lr = det.getDAC(defs::VCMP_LR, false);
//             auto vcmp_rl = det.getDAC(defs::VCMP_RL, false);
//             auto vcmp_rr = det.getDAC(defs::VCMP_RR, false);
//             auto vcp = det.getDAC(defs::VCP, false);

//             {
//                 std::ostringstream oss;
//                 proxy.Call("vthreshold", {"1234"}, -1, PUT, oss);
//                 REQUIRE(oss.str() == "vthreshold 1234\n");
//             }
//             {
//                 std::ostringstream oss;
//                 proxy.Call("vthreshold", {}, -1, GET, oss);
//                 REQUIRE(oss.str() == "vthreshold 1234\n");
//             }

//             // Reset dacs after test
//             for (int i = 0; i != det.size(); ++i) {
//                 det.setDAC(defs::VCMP_LL, vcmp_ll[i], false, {i});
//                 det.setDAC(defs::VCMP_LR, vcmp_ll[i], false, {i});
//                 det.setDAC(defs::VCMP_RL, vcmp_ll[i], false, {i});
//                 det.setDAC(defs::VCMP_RR, vcmp_ll[i], false, {i});
//                 det.setDAC(defs::VCP, vcp[i], false, {i});
//             }
//         }
//     }
// }
