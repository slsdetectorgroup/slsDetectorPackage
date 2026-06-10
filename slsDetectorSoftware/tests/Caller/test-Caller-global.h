// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once

#include "acquire/CTBState.h"
#include "sls/sls_detector_defs.h"

#include <chrono>
#include <filesystem>
#include <optional>
#include <thread>

namespace sls {

namespace acq = sls::test::acquire;

void test_valid_port_caller(const std::string &command,
                            const std::vector<std::string> &arguments,
                            int detector_id, int action);

void test_dac_caller(slsDetectorDefs::dacIndex index,
                     const std::string &dacname, int dacvalue);
void test_onchip_dac_caller(slsDetectorDefs::dacIndex index,
                            const std::string &dacname, int dacvalue);

std::pair<uint64_t, int>
calculate_ctb_image_size(const acq::CTBState &test_info, bool isXilinxCtb);

} // namespace sls
