// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package

#pragma once

#include "sls/Detector.h"
#include <cstdint>

namespace sls::test::acquire {

class FileState;
class CTBState;

void wait_until_idle(const Detector &det);
void run_acquisition(Detector &det);
void run(Detector &det, int64_t num_frames, const CTBState &ctb_state,
         const FileState &file_state);
void run(Detector &det, int64_t num_frames, const FileState &file_state);

class FrameGuard {
  public:
    FrameGuard(Detector &det, int64_t new_frames)
        : det(det), prev(det.getNumberOfFrames().tsquash(
                        "Inconsistent number of frames")) {
        det.setNumberOfFrames(new_frames);
    }

    ~FrameGuard() { det.setNumberOfFrames(prev); }

  private:
    Detector &det;
    int64_t prev;
};

} // namespace sls::test::acquire