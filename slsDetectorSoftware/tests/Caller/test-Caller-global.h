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
};

struct testCtbAcquireInfo {
    defs::readoutMode readout_mode{defs::ANALOG_AND_DIGITAL};
    bool ten_giga{false};
    int num_adc_samples{5000};
    int num_dbit_samples{6000};
    int num_trans_samples{288};
    uint32_t adc_enable_1g{0xFFFFFF00};
    uint32_t adc_enable_10g{0xFF00FFFF};
    int dbit_offset{0};
    std::vector<int> dbit_list{0, 12, 2, 43};
    bool dbit_reorder{false};
    uint32_t transceiver_mask{0x3};
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

testCtbAcquireInfo get_ctb_config_state(const Detector &det);
void set_ctb_config_state(Detector &det,
                          const testCtbAcquireInfo &ctb_config_info);
uint64_t calculate_ctb_image_size(const testCtbAcquireInfo &test_info);
void test_ctb_acquire_with_receiver(const testCtbAcquireInfo &test_info,
                                    int64_t num_frames_to_acquire,
                                    Detector &det, Caller &caller);

} // namespace sls
