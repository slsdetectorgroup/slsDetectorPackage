// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once

class Caller;
#include "sls/Detector.h"
#include "sls/sls_detector_defs.h"

#include <chrono>
#include <filesystem>
#include <thread>

namespace sls {
struct testFileInfo {
    std::string file_path{"/tmp"};
    std::string file_prefix{"sls_test"};
    int64_t file_acq_index{0};
    bool file_write{true};
    bool file_overwrite{true};
    slsDetectorDefs::fileFormat file_format{slsDetectorDefs::BINARY};
};

struct testCommonDetAcquireInfo {
    slsDetectorDefs::timingMode timing_mode{slsDetectorDefs::AUTO_TIMING};
    int64_t num_frames_to_acquire{2};
    int64_t num_triggers{1};
    std::chrono::nanoseconds period{std::chrono::milliseconds{2}};
    std::array<std::chrono::nanoseconds, 3> exptime{
        std::chrono::microseconds{200}, std::chrono::nanoseconds{0},
        std::chrono::nanoseconds{0}};
};

void test_valid_port_caller(const std::string &command,
                            const std::vector<std::string> &arguments,
                            int detector_id, int action);

void test_dac_caller(slsDetectorDefs::dacIndex index,
                     const std::string &dacname, int dacvalue);
void test_onchip_dac_caller(slsDetectorDefs::dacIndex index,
                            const std::string &dacname, int dacvalue);

testFileInfo get_file_state(const Detector &det);
void set_file_state(Detector &det, const testFileInfo &file_info);
void test_acquire_binary_file_size(const testFileInfo &file_info,
                                   uint64_t num_frames_to_acquire,
                                   uint64_t expected_image_size);

void test_frames_caught(const Detector &det, int num_frames_to_acquire);

void test_acquire_with_receiver(Caller &caller, std::chrono::seconds timeout);

testCommonDetAcquireInfo get_common_acquire_config_state(const Detector &det);
void set_common_acquire_config_state(
    Detector &det, const testCommonDetAcquireInfo &det_config_info);

} // namespace sls
