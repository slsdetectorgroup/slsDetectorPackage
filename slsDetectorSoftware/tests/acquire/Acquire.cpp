// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package

#include "Acquire.h"
#include "CTBState.h"
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

void run(Detector &det, int64_t num_frames, const FileState &file_state) {
    auto ctb_state = default_ctb_state();
    run(det, num_frames, ctb_state, file_state);
}

void run(Detector &det, int64_t num_frames, const CTBState &ctb_state,
         const FileState &file_state) {
    FrameGuard frame_guard(det, num_frames);
    CTBStateGuard ctb_guard(det, ctb_state);

    {
        // binary
        auto binary_state = file_state;
        binary_state.file_format = defs::BINARY;
        FileStateGuard file_guard(det, binary_state);
        run_acquisition(det);
        auto frames_caught = det.getFramesCaught()[0][0];
        REQUIRE(frames_caught == num_frames);
    }

#ifdef HDF5C
    {
        // hdf5
        auto h5_state = file_state;
        h5_state.file_format = defs::HDF5;
        FileStateGuard file_guard(det, h5_state);
        run_acquisition(det);
        auto frames_caught = det.getFramesCaught()[0][0];
        REQUIRE(frames_caught == num_frames);
    }
#endif
}

} // namespace sls::test::acquire