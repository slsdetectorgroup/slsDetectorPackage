#include "catch.hpp"
#include "sls_detector_defs.h"
#include "slsDetector.h"

using dt = slsDetectorDefs::detectorType;

TEST_CASE("sls_detector_module default construction", "[support][new]") {
    sls_detector_module m;
    CHECK(m.serialnumber == 0);
    CHECK(m.nchan == 0);
    CHECK(m.nchip == 0);
    CHECK(m.ndac == 0);
    CHECK(m.reg == 0);
    CHECK(m.iodelay == 0);
    CHECK(m.tau == 0);
    CHECK(m.eV == 0);
    CHECK(m.dacs == nullptr);
    CHECK(m.chanregs == nullptr);
}

TEST_CASE("sls_detector_module from type", "[support][new]") {
    sls_detector_module m(dt::EIGER);
    CHECK(m.serialnumber == 0);
    CHECK(m.nchan == 256*256*4);
    CHECK(m.nchip == 4);
    CHECK(m.ndac == 16);
    CHECK(m.reg == 0);
    CHECK(m.iodelay == 0);
    CHECK(m.tau == 0);
    CHECK(m.eV == 0);
    CHECK(m.dacs != nullptr);
    CHECK(m.chanregs != nullptr);
}