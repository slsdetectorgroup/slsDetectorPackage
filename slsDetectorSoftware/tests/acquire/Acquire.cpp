// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package

#include "Acquire.h"
#include "FileState.h"

#include "catch.hpp"
#include <chrono>
#include <thread>

namespace sls::test::acquire {

void wait_until_idle(const Detector &det) {
    bool idle = false;
    while (!idle) {
        auto statusList = det.getDetectorStatus();
        if (statusList.any(defs::ERROR)) {
            throw RuntimeError("error status during acquisition");
        }
        if (statusList.contains_only(defs::IDLE, defs::STOPPED)) {
            idle = true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void run_acquisition(Detector &det) {
    det.startReceiver();
    det.startDetector();
    wait_until_idle(det);
    det.stopReceiver();
}

void run(Detector &det, const AcquisitionState &acq_state,
         const FileState &file_state) {

    INFO(ToString(det.getDetectorType().squash(defs::GENERIC))
         << " acquiring with num_frames = " << acq_state.num_frames);

    FileStateGuard file_guard(det, file_state);
    AcquisitionStateGuard acq_guard(det, acq_state);
    run_acquisition(det);
    auto frames_caught = det.getFramesCaught()[0][0];
    REQUIRE(frames_caught == acq_state.num_frames);
}

} // namespace sls::test::acquire