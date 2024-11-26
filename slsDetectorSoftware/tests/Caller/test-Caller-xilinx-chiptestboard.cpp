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

TEST_CASE("configtransceiver", "[.cmdcall]") {
    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::XILINX_CHIPTESTBOARD) {
        REQUIRE_THROWS(caller.call("configtransceiver", {}, -1, GET));
        REQUIRE_NOTHROW(caller.call("configtransceiver", {}, -1, PUT));
    } else {
        REQUIRE_THROWS(caller.call("configtransceiver", {}, -1, PUT));
        REQUIRE_THROWS(caller.call("configtransceiver", {}, -1, GET));
    }
}
} // namespace sls
