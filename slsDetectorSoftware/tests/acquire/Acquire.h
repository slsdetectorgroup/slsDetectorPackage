// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package

#pragma once

class FileState;
class CTBState;
class Detector;

namespace sls::test::acquire {

void wait_until_idle(const Detector &det);
void run_acquisition(Detector &det);
void run(Detector &det, int64_t num_frames = 1,
         const CTBState &ctb_state = default_ctb_state(),
         const FileState &file_state = default_file_state());

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