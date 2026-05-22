// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once

#include "Caller.h"
#include "sls/Detector.h"
#include "sls/ToString.h"
#include "sls/logger.h"
#include "sls/sls_detector_defs.h"

#include "acquire/Acquire.h"

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
                     const std::string &dacname, int dacvalue, bool mV = false);
void test_onchip_dac_caller(slsDetectorDefs::dacIndex index,
                            const std::string &dacname, int dacvalue);

void test_acquire_binary_file_size(const acq::FileState &file_info,
                                   uint64_t num_frames_to_acquire,
                                   uint64_t expected_image_size);

std::pair<uint64_t, int>
calculate_ctb_image_size(const acq::CTBState &test_info, bool isXilinxCtb);

} // namespace sls
