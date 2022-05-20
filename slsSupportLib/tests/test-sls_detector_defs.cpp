// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "catch.hpp"
#include "sls/sls_detector_defs.h"

namespace sls {

using dt = slsDetectorDefs::detectorType;

TEST_CASE("sls_detector_module default construction", "[support][new]") {
    sls_detector_module m;
    CHECK(m.serialnumber == 0);
    CHECK(m.nchan == 0);
    CHECK(m.nchip == 0);
    CHECK(m.ndac == 0);
    CHECK(m.reg == -1);
    CHECK(m.iodelay == 0);
    CHECK(m.tau == 0);
    CHECK(m.eV[0] == -1);
    CHECK(m.eV[1] == -1);
    CHECK(m.eV[2] == -1);
    CHECK(m.dacs == nullptr);
    CHECK(m.chanregs == nullptr);
}

TEST_CASE("sls_detector_module from type", "[support]") {
    sls_detector_module m(dt::EIGER);
    CHECK(m.serialnumber == 0);
    CHECK(m.nchan == 256 * 256 * 4);
    CHECK(m.nchip == 4);
    CHECK(m.ndac == 16);
    CHECK(m.reg == -1);
    CHECK(m.iodelay == 0);
    CHECK(m.tau == 0);
    CHECK(m.eV[0] == -1);
    CHECK(m.eV[1] == -1);
    CHECK(m.eV[2] == -1);
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

TEST_CASE("default construct scanParameters") {
    slsDetectorDefs::scanParameters p;
    CHECK(p.dacSettleTime_ns == 0);
    CHECK(p.dacInd == slsDetectorDefs::DAC_0);
    CHECK(p.enable == 0);
    CHECK(p.startOffset == 0);
    CHECK(p.stopOffset == 0);
    CHECK(p.stepSize == 0);
}

TEST_CASE("compare two scanParameters") {
    slsDetectorDefs::scanParameters p0;
    slsDetectorDefs::scanParameters p1;

    CHECK(p0 == p1);

    p0.enable = 1;
    CHECK_FALSE(p0 == p1);
}

} // namespace sls
