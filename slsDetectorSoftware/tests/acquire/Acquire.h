// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package

#pragma once

#include "FileState.h"
#include "CTBState.h"
#include "sls/Detector.h"

#include <optional>

namespace sls::test::acquire {

    void wait_until_idle(const Detector &det);
    void acquire(Detector &det);
    void run(Detector &det, int64_t num_frames = 1, const FileState& file_state = default_file_state(), const std::optional<CTBState>& ctb_state = std::nullopt);

} // namespace sls::test::acquire