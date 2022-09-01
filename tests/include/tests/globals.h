// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once
#include "sls/sls_detector_defs.h"

#define TEST_FILE_NAME_BAD_CHANNELS ("test-file_utils-channels.txt")

namespace sls {

using dt = slsDetectorDefs::detectorType;
using di = slsDetectorDefs::dacIndex;
using defs = slsDetectorDefs;

namespace test {
extern std::string hostname;
extern std::string detector_type;
extern dt type;
extern std::string my_ip;
extern decltype(defs::GET_ACTION) GET;
extern decltype(defs::PUT_ACTION) PUT;

} // namespace test

} // namespace sls
