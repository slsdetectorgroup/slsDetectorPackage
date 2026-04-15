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

// not implemented at the moment
TEST_CASE("configtransceiver", "[.detectorintegration]") {
    Detector det;
    Caller caller(&det);
    REQUIRE_THROWS(caller.call("configtransceiver", {}, -1, PUT));
    REQUIRE_THROWS(caller.call("configtransceiver", {}, -1, GET));
}
} // namespace sls
