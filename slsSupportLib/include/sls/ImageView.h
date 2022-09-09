// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package

#pragma once
#include <cstddef>
#include <cstdint>
namespace sls {
struct ImageView {
    // ImageView() {}
    // ImageView(int64_t frameNumber, char *data)
    //     : frameNumber(frameNumber), data(data) {}
    int64_t frameNumber{-1};
    int64_t framesInPack{1};
    char *data{nullptr};
};
} // namespace sls