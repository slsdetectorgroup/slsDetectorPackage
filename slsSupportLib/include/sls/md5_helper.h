// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once

#include <string>

namespace sls {
std::string md5_calculate_checksum(char *buffer, ssize_t bytes);
} // namespace sls