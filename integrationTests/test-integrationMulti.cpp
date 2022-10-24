// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "DetectorImpl.h"
#include "catch.hpp"
#include "sls/string_utils.h"
#include "tests/globals.h"
#include <iostream>

namespace sls {

using namespace Catch::literals;

TEST_CASE("Initialize a multi detector", "[.integration][.multi]") {
    auto hostnames = split(test::hostname, '+');

    DetectorImpl d(0, true, true);
    d.setHostname(test::hostname.c_str());

    CHECK(d.getHostname() == test::hostname);
    for (size_t i = 0; i != hostnames.size(); ++i) {
        CHECK(d.getHostname(i) == hostnames[i]);
    }

    CHECK(d.getDetectorTypeAsEnum() == test::type);
    CHECK(d.getDetectorTypeAsString() == test::detector_type);

    CHECK(d.getNumberOfDetectors() == hostnames.size());
    d.freeSharedMemory();
}

TEST_CASE("Set and read timers", "[.integration][.multi]") {

    DetectorImpl d(0, true, true);
    d.setHostname(test::hostname.c_str());

    // FRAME_NUMBER
    int n_frames = 3;
    d.setNumberOfFrames(n_frames);
    CHECK(d.setNumberOfFrames() == n_frames);

    // ACQUISITION_TIME
    double exptime = 0.3;
    d.setExposureTime(exptime, true);
    CHECK(d.setExposureTime(-1, true) == Approx(exptime));
    CHECK(d.setExposureTime(-1) == Approx(exptime * 1E9));

    // FRAME_PERIOD,
    double period = 0.5;
    d.setExposurePeriod(period, true);
    CHECK(d.setExposurePeriod(-1, true) == Approx(period));
    CHECK(d.setExposurePeriod(-1) == Approx(period * 1E9));

    // DELAY_AFTER_TRIGGER,
    // GATES_NUMBER,
    // TRIGGER_NUMBER,
    // ACTUAL_TIME
    // MEASUREMENT_TIME

    // PROGRESS, /**< fraction of measurement elapsed - only get! */
    // MEASUREMENTS_NUMBER,

    // FRAMES_FROM_START,
    // FRAMES_FROM_START_PG,
    // SAMPLES,

    // SUBFRAME_ACQUISITION_TIME, /**< subframe exposure time */
    double subframe_exposure = 2000000; // ns
    if (test::type == dt::EIGER) {
        d.setSubFrameExposureTime(subframe_exposure);
        CHECK(d.setSubFrameExposureTime(-1) == Approx(subframe_exposure));
    }

    // STORAGE_CELL_NUMBER, /**<number of storage cells */

    // SUBFRAME_DEADTIME, /**< subframe deadtime */
    double subframe_deadtime = 4000; // ns
    if (test::type == dt::EIGER) {
        d.setSubFrameExposureDeadTime(subframe_deadtime);
        CHECK(d.setSubFrameExposureDeadTime(-1) == Approx(subframe_deadtime));
    }

    if (test::type == dt::EIGER) {
        // 32bit is needed for subframe exposure
        d.setDynamicRange(32);
        CHECK(d.setDynamicRange(-1) == 32);
        d.setReadOutFlags(ro::PARALLEL);

        // Needed to have measured values
        d.acquire();

        // MEASURED_PERIOD,	/**< measured period */
        for (int i = 0; i != d.getNumberOfDetectors(); ++i) {
            CHECK(d.getMeasuredPeriod(true, i) == Approx(period));
        }

        // MEASURED_SUBPERIOD,	/**< measured subperiod */
        for (int i = 0; i != d.getNumberOfDetectors(); ++i) {
            CHECK(d.getMeasuredSubFramePeriod(false, i) ==
                  Approx(subframe_deadtime + subframe_exposure));
        }
    }

    // MAX_TIMERS

    d.freeSharedMemory();
}

} // namespace sls
