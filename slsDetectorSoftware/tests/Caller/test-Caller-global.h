// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once
#include "sls/sls_detector_defs.h"

namespace sls {
void test_valid_port(const std::string &command,
                     const std::vector<std::string> &arguments, int detector_id,
                     int action);

void test_dac(slsDetectorDefs::dacIndex index, const std::string &dacname,
              int dacvalue);
void test_onchip_dac(slsDetectorDefs::dacIndex index,
                     const std::string &dacname, int dacvalue);

} // namespace sls
