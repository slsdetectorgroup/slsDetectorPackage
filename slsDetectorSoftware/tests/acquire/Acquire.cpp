// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package

#include "Acquire.h"

#include "catch.hpp"
#include <thread>

namespace sls::test::acquire {

    void wait_until_idle(const Detector &det) {
        bool idle = false;
        while (!idle) {
            auto statusList = det.getDetectorStatus();
            if (statusList.any(defs::ERROR)) {
                throw std::runtime_error("error status during acquisition");
            }
            if (statusList.contains_only(defs::IDLE, defs::STOPPED)) {
                idle = true;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    void acquire(Detector &det) {
        det.startReceiver();
        det.startDetector();
        wait_until_idle(det);
        det.stopReceiver();
    }

    void run(Detector &det, int64_t num_frames, const FileState& file_state, const std::optional<CTBState>& ctb_state) {
        FileStateGuard file_guard(det);
        CTBStateGuard ctb_guard(det);
        if (ctb_state) {
            set_ctb_state(det, ctb_state.value());
        }
        
        // frames
        auto prev_frames = det.getNumberOfFrames().tsquash(
            "Inconsistent number of frames to acquire");
        det.setNumberOfFrames(num_frames);

        // binary
        auto bin_state = file_state;
        bin_state.file_format = defs::BINARY;
        set_file_state(det, bin_state);
        acquire(det);
        {
            auto frames_caught = det.getFramesCaught()[0][0];
            REQUIRE(frames_caught == num_frames);
        }

        // hdf5
#ifdef HDF5C
        auto h5_state = file_state;
        h5_state.file_format = defs::HDF5;
        set_file_state(det, h5_state);
        acquire(det);    
        {
            auto frames_caught = det.getFramesCaught()[0][0];
            REQUIRE(frames_caught == num_frames);
        }
#endif
        // restore previous state
        det.setNumberOfFrames(prev_frames);
    }

} // namespace sls::test::acquire