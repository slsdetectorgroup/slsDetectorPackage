#include "CmdProxy.h"
#include "Detector.h"
#include "catch.hpp"
#include "sls_detector_defs.h"
#include <sstream>

#include "tests/globals.h"

using sls::CmdProxy;
using sls::Detector;
using test::GET;
using test::PUT;

/* 
This file should contain receiver specific tests


*/

TEST_CASE("rx_missingpackets", "[.cmd]") {
    // TODO! This only tests for no crash how can we test
    // for correct values?
    Detector det;
    CmdProxy proxy(&det);
    proxy.Call("rx_missingpackets", {}, -1, GET);
}

// TEST_CASE("burstmode", "[.cmd][.gotthard2]") {
//     if (test::type == slsDetectorDefs::GOTTHARD2) {
//         REQUIRE_NOTHROW(multiSlsDetectorClient("burstmode 0", PUT));
//         REQUIRE_NOTHROW(multiSlsDetectorClient("burstmode 1", PUT));
//         REQUIRE_NOTHROW(multiSlsDetectorClient("burstmode", GET));
//     } else {
//         REQUIRE_THROWS(multiSlsDetectorClient("burstmod", GET));
//     }
// }

// TEST_CASE("vetoref", "[.cmd][.gotthard2]") {
//     if (test::type == slsDetectorDefs::GOTTHARD2) {
//         REQUIRE_THROWS(multiSlsDetectorClient("vetoref 3 0x3ff", PUT)); //
//         invalid chip index REQUIRE_THROWS(multiSlsDetectorClient("vetoref 0
//         0xFFFF", PUT)); // invalid value
//         REQUIRE_NOTHROW(multiSlsDetectorClient("vetoref 1 0x010", PUT));
//         REQUIRE_THROWS(multiSlsDetectorClient("vetoref", GET));
//     } else {
//         REQUIRE_THROWS(multiSlsDetectorClient("vetoref 3 0x0", PUT));
//     }
// }

// TEST_CASE("vetophoton", "[.cmd][.gotthard2]") {
//     if (test::type == slsDetectorDefs::GOTTHARD2) {
//         REQUIRE_THROWS(multiSlsDetectorClient("vetophoton 12 1 39950
//         examples/gotthard2_veto_photon.txt", PUT)); // invalid chip index
//         REQUIRE_THROWS(multiSlsDetectorClient("vetophoton -1 0 39950
//         examples/gotthard2_veto_photon.txt", PUT)); // invalid photon number
//         REQUIRE_NOTHROW(multiSlsDetectorClient("vetophoton -1 1 39950
//         examples/gotthard2_veto_photon.txt", PUT));
//         REQUIRE_THROWS(multiSlsDetectorClient("vetophoton", GET));
//         REQUIRE_NOTHROW(multiSlsDetectorClient("vetophoton -1", GET));
//     } else {
//         REQUIRE_THROWS(multiSlsDetectorClient("vetophoton -1", GET));
//     }
// }

// TEST_CASE("inj_ch", "[.cmd][.gotthard2]") {
//     if (test::type == slsDetectorDefs::GOTTHARD2) {
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("inj_ch 0 1", PUT,
//             nullptr, oss)); REQUIRE(oss.str() == "inj_ch [0, 1]\n");
//         }
//         {
//             std::ostringstream oss;
//             REQUIRE_NOTHROW(multiSlsDetectorClient("inj_ch", GET, nullptr,
//             oss)); REQUIRE(oss.str() == "inj_ch [0, 1]\n");
//         }
//         REQUIRE_THROWS(multiSlsDetectorClient("inj_ch -1 1", PUT));
//         REQUIRE_THROWS(multiSlsDetectorClient("inj_ch 0 0", PUT));
//     } else {
//         REQUIRE_THROWS(multiSlsDetectorClient("inj_ch", GET));
//     }
// }
