// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package

#pragma once
#include "sls/sls_detector_defs.h"

#include <cstddef>
namespace sls {
struct ImageView {
    size_t size;
    size_t firstIndex;
    slsDetectorDefs::sls_receiver_header header;
    char data[];
};
} // namespace sls