// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once

#include "Caller.h"
#include "sls/Detector.h"
#include "sls/ToString.h"
#include "sls/logger.h"
#include "sls/sls_detector_defs.h"

#include <chrono>
#include <filesystem>
#include <optional>
#include <thread>

namespace sls {
struct testFileInfo {
    std::string file_path{"/tmp"};
    std::string file_prefix{"sls_test"};
    int64_t file_acq_index{0};
    bool file_write{true};
    bool file_overwrite{true};
    slsDetectorDefs::fileFormat file_format{slsDetectorDefs::BINARY};
    std::string getMasterFileNamePrefix() const {
        return file_path + "/" + file_prefix + "_master_" +
               std::to_string(file_acq_index);
    }
    std::string getVirtualFileName() const {
        return file_path + "/" + file_prefix + "_virtual_" +
               std::to_string(file_acq_index) + ".h5";
    }
    inline void print() const {
        LOG(logINFO) << "File Info: "
                     << "\n\tFile Path: " << file_path
                     << "\n\tFile Prefix: " << file_prefix
                     << "\n\tFile Acquisition Index: " << file_acq_index
                     << "\n\tFile Write: " << file_write
                     << "\n\tFile Overwrite: " << file_overwrite
                     << "\n\tFile Format: " << ToString(file_format)
                     << "\n\tMaster Filename: " << getMasterFileNamePrefix()
                     << "\n\tVirtual Filename: " << getVirtualFileName();
    }
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

    inline void print() const {
        LOG(logINFO) << "CTB Acquire Info: "
                     << "\n\tReadout Mode: " << ToString(readout_mode)
                     << "\n\tTen Giga: " << ten_giga
                     << "\n\tADC Enable 1G: " << std::hex << adc_enable_1g
                     << std::dec << "\n\tADC Enable 10G: " << std::hex
                     << adc_enable_10g << std::dec
                     << "\n\tNumber of Analog Samples: " << num_adc_samples
                     << "\n\tNumber of Digital Samples: " << num_dbit_samples
                     << "\n\tNumber of Transceiver Samples: "
                     << num_trans_samples << "\n\tDBIT Offset: " << dbit_offset
                     << "\n\tDBIT Reorder: " << dbit_reorder
                     << "\n\tDBIT List: " << ToString(dbit_list)
                     << "\n\tTransceiver Mask: " << std::hex << transceiver_mask
                     << std::dec << std::endl;
    }
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

void test_acquire_with_receiver(Caller &caller, const Detector &det);

void create_files_for_acquire(
    Detector &det, Caller &caller, int64_t num_frames = 1,
    const std::optional<testCtbAcquireInfo> &test_info = std::nullopt);

testCtbAcquireInfo get_ctb_config_state(const Detector &det);
void set_ctb_config_state(Detector &det,
                          const testCtbAcquireInfo &ctb_config_info);
std::pair<uint64_t, int>
calculate_ctb_image_size(const testCtbAcquireInfo &test_info, bool isXilinxCtb);

} // namespace sls
