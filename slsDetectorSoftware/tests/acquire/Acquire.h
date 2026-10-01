// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package

#pragma once

#include "sls/Detector.h"
#include "sls/logger.h"

namespace sls::test::acquire {

class FileState;

// at the moment, only number of frames
struct AcquisitionState {
    int64_t num_frames;
};

inline AcquisitionState default_acquisition_state() { return {1}; }

inline AcquisitionState get_acquisition_state(const Detector &det) {
    return AcquisitionState{
        det.getNumberOfFrames().tsquash("Inconsistent number of frames")};
}

inline void set_acquisition_state(Detector &det, const AcquisitionState &s) {
    det.setNumberOfFrames(s.num_frames);
}

inline std::ostream &operator<<(std::ostream &os, const AcquisitionState &s) {
    os << "Acquisition State:"
       << "\n  Number of Frames: " << s.num_frames;
    return os;
}

/**
 * @brief RAII guard for restoring the acquisition state of a detector.
 * The constructor saves the current acquisition state and sets a new state,
 * while the destructor restores the original state.
 *
 */
class AcquisitionStateGuard {
  public:
    explicit AcquisitionStateGuard(Detector &det,
                                   const AcquisitionState &new_state)
        : det(det), saved_(get_acquisition_state(det)) {
        set_acquisition_state(det, new_state);
    }
    ~AcquisitionStateGuard() { set_acquisition_state(det, saved_); }

  private:
    Detector &det;
    AcquisitionState saved_;
};

void wait_until_idle(const Detector &det);
void run_acquisition(Detector &det);
void run(Detector &det, const AcquisitionState &acq_state,
         const FileState &file_state);

} // namespace sls::test::acquire