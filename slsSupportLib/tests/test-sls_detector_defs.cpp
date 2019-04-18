#include "catch.hpp"
#include "slsDetector.h"
#include "sls_detector_defs.h"

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

TEST_CASE("sls_detector_module from type", "[support]") {
    sls_detector_module m(dt::EIGER);
    CHECK(m.serialnumber == 0);
    CHECK(m.nchan == 256 * 256 * 4);
    CHECK(m.nchip == 4);
    CHECK(m.ndac == 16);
    CHECK(m.reg == 0);
    CHECK(m.iodelay == 0);
    CHECK(m.tau == 0);
    CHECK(m.eV == 0);
    CHECK(m.dacs != nullptr);
    CHECK(m.chanregs != nullptr);
}

TEST_CASE("assign module", "[support]") {
    sls_detector_module m0;
    sls_detector_module m1(dt::EIGER);
    m1.serialnumber = 14;
    m1.reg = 500;
    m1.iodelay = 750;

    m0 = m1; // Assignment operator
    CHECK(m0.serialnumber == 14);
    CHECK(m0.reg == 500);
    CHECK(m0.iodelay == 750);
    CHECK(m0.nchan == 256 * 256 * 4);

    auto m3 = m1; // Copy constructor
    CHECK(m3.serialnumber == 14);
    CHECK(m3.reg == 500);
    CHECK(m3.iodelay == 750);
    CHECK(m3.nchan == 256 * 256 * 4);
}