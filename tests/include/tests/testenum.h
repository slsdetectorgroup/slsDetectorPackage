// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once
#include <cstdint>

namespace sls {

enum class func_id { read_data, read_int, read_half_data, combined };
constexpr size_t MB = 1048576;
constexpr size_t DATA_SIZE = 50 * MB;

} // namespace sls
