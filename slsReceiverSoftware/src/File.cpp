// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "File.h"

#include <iostream>

namespace sls {

File::File(const slsDetectorDefs::fileFormat format) : format_(format) {}

File::~File() {}

slsDetectorDefs::fileFormat File::GetFileFormat() const { return format_; }

} // namespace sls
