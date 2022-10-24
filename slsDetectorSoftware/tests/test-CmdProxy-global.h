// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once
#include "sls/sls_detector_defs.h"

namespace sls {

void test_dac(slsDetectorDefs::dacIndex index, const std::string &dacname,
              int dacvalue);
void test_onchip_dac(slsDetectorDefs::dacIndex index,
                     const std::string &dacname, int dacvalue);

} // namespace sls
