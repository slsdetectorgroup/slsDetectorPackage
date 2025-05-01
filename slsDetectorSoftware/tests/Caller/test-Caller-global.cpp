// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "test-Caller-global.h"
#include "Caller.h"
#include "catch.hpp"
#include "sls/Detector.h"
#include "sls/logger.h"
#include "tests/globals.h"

namespace sls {

using test::GET;
using test::PUT;
void test_valid_port_caller(const std::string &command,
                            const std::vector<std::string> &arguments,
                            int detector_id, int action) {
    Detector det;
    Caller caller(&det);

    std::vector<std::string> arg(arguments);
    if (arg.empty())
        arg.push_back("0");

    int test_values[3] = {77797, -1, 0};
    for (int i = 0; i != 3; ++i) {
        int port_number = test_values[i];
        arg[arg.size() - 1] = std::to_string(port_number);
        REQUIRE_THROWS(caller.call(command, arg, detector_id, action));
        /*REQUIRE_THROWS_WITH(proxy.Call(command, arguments, detector_id,
           action), "Invalid port range. Must be between 1 - 65535.");*/
    }
}

void test_dac_caller(defs::dacIndex index, const std::string &dacname,
                     int dacvalue) {
    Detector det;
    Caller caller(&det);
    std::ostringstream oss_set, oss_get;
    auto dacstr = std::to_string(dacvalue);
    auto previous = det.getDAC(index, false);
    // chip test board
    if (dacname == "dac") {
        auto dacIndexstr = std::to_string(static_cast<int>(index));
        caller.call(dacname, {dacIndexstr, dacstr}, -1, PUT, oss_set);
        REQUIRE(oss_set.str() ==
                dacname + " " + dacIndexstr + " " + dacstr + "\n");
        caller.call(dacname, {dacIndexstr}, -1, GET, oss_get);
        REQUIRE(oss_get.str() ==
                dacname + " " + dacIndexstr + " " + dacstr + "\n");
    }
    // other detectors
    else {
        caller.call("dac", {dacname, dacstr}, -1, PUT, oss_set);
        REQUIRE(oss_set.str() == "dac " + dacname + " " + dacstr + "\n");
        caller.call("dac", {dacname}, -1, GET, oss_get);
        REQUIRE(oss_get.str() == "dac " + dacname + " " + dacstr + "\n");
    }
    // Reset all dacs to previous value
    for (int i = 0; i != det.size(); ++i) {
        det.setDAC(index, previous[i], false, {i});
    }
}

void test_onchip_dac_caller(defs::dacIndex index, const std::string &dacname,
                            int dacvalue) {
    Detector det;
    Caller caller(&det);
    REQUIRE_THROWS(caller.call(dacname, {}, -1, GET));
    REQUIRE_THROWS(
        caller.call(dacname, {"10", "0x0"}, -1, PUT)); // chip index (-1 to 9)
    REQUIRE_THROWS(
        caller.call(dacname, {"-1", "0x400"}, -1, PUT)); // max val is 0x3ff

    int chipIndex = -1; // for now, it is -1 only
    auto prev_val = det.getOnChipDAC(index, chipIndex);
    auto dacValueStr = ToStringHex(dacvalue);
    auto chipIndexStr = std::to_string(chipIndex);
    std::ostringstream oss_set, oss_get;
    caller.call(dacname, {chipIndexStr, dacValueStr}, -1, PUT, oss_set);
    REQUIRE(oss_set.str() ==
            dacname + " " + chipIndexStr + " " + dacValueStr + "\n");
    caller.call(dacname, {chipIndexStr}, -1, GET, oss_get);
    REQUIRE(oss_get.str() ==
            dacname + " " + chipIndexStr + " " + dacValueStr + "\n");

    // Reset all dacs to previous value
    for (int i = 0; i != det.size(); ++i) {
        det.setOnChipDAC(index, chipIndex, prev_val[i], {i});
    }
}

testFileInfo get_file_state(const Detector &det) {
    return testFileInfo{
        det.getFilePath().tsquash("Inconsistent file path"),
        det.getFileNamePrefix().tsquash("Inconsistent file prefix"),
        det.getAcquisitionIndex().tsquash(
            "Inconsistent file acquisition index"),
        det.getFileWrite().tsquash("Inconsistent file write state"),
        det.getFileOverWrite().tsquash("Inconsistent file overwrite state"),
        det.getFileFormat().tsquash("Inconsistent file format")};
}

void set_file_state(Detector &det, const testFileInfo &file_info) {
    if (!file_info.file_path.empty())
        det.setFilePath(file_info.file_path);
    det.setFileNamePrefix(file_info.file_prefix);
    det.setAcquisitionIndex(file_info.file_acq_index);
    det.setFileWrite(file_info.file_write);
    det.setFileOverWrite(file_info.file_overwrite);
    det.setFileFormat(file_info.file_format);
}

void test_acquire_binary_file_size(const testFileInfo &file_info,
                                   uint64_t num_frames_to_acquire,
                                   uint64_t expected_image_size) {
    assert(file_info.file_format == defs::BINARY);
    std::string fname = file_info.file_path + "/" + file_info.file_prefix +
                        "_d0_f0_" + std::to_string(file_info.file_acq_index) +
                        ".raw";
    uint64_t expected_file_size =
        num_frames_to_acquire *
        (expected_image_size + sizeof(defs::sls_receiver_header));
    auto actual_file_size = std::filesystem::file_size(fname);
    REQUIRE(actual_file_size == expected_file_size);
}

void test_frames_caught(const Detector &det, int num_frames_to_acquire) {
    auto frames_caught = det.getFramesCaught().tsquash(
        "Inconsistent number of frames caught")[0];
    REQUIRE(frames_caught == num_frames_to_acquire);
}

void test_acquire_with_receiver(Caller &caller, std::chrono::seconds timeout) {
    REQUIRE_NOTHROW(caller.call("rx_start", {}, -1, PUT));
    REQUIRE_NOTHROW(caller.call("start", {}, -1, PUT));
    std::this_thread::sleep_for(timeout);
    REQUIRE_NOTHROW(caller.call("rx_stop", {}, -1, PUT));
}

testCommonDetAcquireInfo get_common_acquire_config_state(const Detector &det) {
    return testCommonDetAcquireInfo{
        det.getTimingMode().tsquash("Inconsistent timing mode"),
        det.getNumberOfFrames().tsquash("Inconsistent number of frames"),
        det.getNumberOfTriggers().tsquash("Inconsistent number of triggers"),
        det.getPeriod().tsquash("Inconsistent period")};
}

void set_common_acquire_config_state(
    Detector &det, const testCommonDetAcquireInfo &det_config_info) {
    det.setTimingMode(det_config_info.timing_mode);
    det.setNumberOfFrames(det_config_info.num_frames_to_acquire);
    det.setNumberOfTriggers(det_config_info.num_triggers);
    det.setPeriod(det_config_info.period);
}

testCtbAcquireInfo get_ctb_config_state(const Detector &det) {
    testCtbAcquireInfo ctb_config_info{
        det.getReadoutMode().tsquash("inconsistent readout mode to test"),
        true,
        det.getNumberOfAnalogSamples().tsquash(
            "inconsistent number of analog samples to test"),
        det.getNumberOfDigitalSamples().tsquash(
            "inconsistent number of digital samples to test"),
        det.getNumberOfTransceiverSamples().tsquash(
            "inconsistent number of transceiver samples to test"),
        0,
        det.getTenGigaADCEnableMask().tsquash(
            "inconsistent ten giga adc enable mask to test"),
        det.getRxDbitOffset().tsquash("inconsistent rx dbit offset to test"),
        det.getRxDbitList().tsquash("inconsistent rx dbit list to test"),
        det.getRxDbitReorder().tsquash("inconsistent rx dbit reorder to test"),
        det.getTransceiverEnableMask().tsquash(
            "inconsistent transceiver mask to test")};

    if (det.getDetectorType().tsquash("inconsistent detector type to test") ==
        slsDetectorDefs::CHIPTESTBOARD) {
        ctb_config_info.ten_giga =
            det.getTenGiga().tsquash("inconsistent ten giga enable to test");
        ctb_config_info.adc_enable_1g = det.getADCEnableMask().tsquash(
            "inconsistent adc enable mask to test");
    }
    return ctb_config_info;
}

void set_ctb_config_state(Detector &det,
                          const testCtbAcquireInfo &ctb_config_info) {
    det.setReadoutMode(ctb_config_info.readout_mode);
    if (det.getDetectorType().tsquash("inconsistent detector type to test") ==
        slsDetectorDefs::CHIPTESTBOARD) {
        det.setTenGiga(ctb_config_info.ten_giga);
        det.setADCEnableMask(ctb_config_info.adc_enable_1g);
    }
    det.setNumberOfAnalogSamples(ctb_config_info.num_adc_samples);
    det.setNumberOfDigitalSamples(ctb_config_info.num_dbit_samples);
    det.setNumberOfTransceiverSamples(ctb_config_info.num_trans_samples);
    det.setTenGigaADCEnableMask(ctb_config_info.adc_enable_10g);
    det.setRxDbitOffset(ctb_config_info.dbit_offset);
    det.setRxDbitList(ctb_config_info.dbit_list);
    det.setRxDbitReorder(ctb_config_info.dbit_reorder);
    det.setTransceiverEnableMask(ctb_config_info.transceiver_mask);
}

uint64_t calculate_ctb_image_size(const testCtbAcquireInfo &test_info) {
    uint64_t num_analog_bytes = 0, num_digital_bytes = 0,
             num_transceiver_bytes = 0;
    if (test_info.readout_mode == defs::ANALOG_ONLY ||
        test_info.readout_mode == defs::ANALOG_AND_DIGITAL) {
        uint32_t adc_enable_mask =
            (test_info.ten_giga ? test_info.adc_enable_10g
                                : test_info.adc_enable_1g);
        int num_analog_chans = __builtin_popcount(adc_enable_mask);
        const int num_bytes_per_sample = 2;
        num_analog_bytes =
            num_analog_chans * num_bytes_per_sample * test_info.num_adc_samples;
        LOG(logDEBUG1) << "[Analog Databytes: " << num_analog_bytes << ']';
    }

    // digital channels
    if (test_info.readout_mode == defs::DIGITAL_ONLY ||
        test_info.readout_mode == defs::ANALOG_AND_DIGITAL ||
        test_info.readout_mode == defs::DIGITAL_AND_TRANSCEIVER) {
        int num_digital_samples = test_info.num_dbit_samples;
        if (test_info.dbit_offset > 0) {
            uint64_t num_digital_bytes_reserved =
                num_digital_samples * sizeof(uint64_t);
            num_digital_bytes_reserved -= test_info.dbit_offset;
            num_digital_samples = num_digital_bytes_reserved / sizeof(uint64_t);
        }
        int num_digital_chans = test_info.dbit_list.size();
        if (num_digital_chans == 0) {
            num_digital_chans = 64;
        }
        if (!test_info.dbit_reorder) {
            uint32_t num_bits_per_sample = num_digital_chans;
            if (num_bits_per_sample % 8 != 0) {
                num_bits_per_sample += (8 - (num_bits_per_sample % 8));
            }
            num_digital_bytes = (num_bits_per_sample / 8) * num_digital_samples;
        } else {
            uint32_t num_bits_per_bit = num_digital_samples;
            if (num_bits_per_bit % 8 != 0) {
                num_bits_per_bit += (8 - (num_bits_per_bit % 8));
            }
            num_digital_bytes = num_digital_chans * (num_bits_per_bit / 8);
        }
        LOG(logDEBUG1) << "[Digital Databytes: " << num_digital_bytes << ']';
    }
    // transceiver channels
    if (test_info.readout_mode == defs::TRANSCEIVER_ONLY ||
        test_info.readout_mode == defs::DIGITAL_AND_TRANSCEIVER) {
        int num_transceiver_chans =
            __builtin_popcount(test_info.transceiver_mask);
        const int num_bytes_per_channel = 8;
        num_transceiver_bytes = num_transceiver_chans * num_bytes_per_channel *
                                test_info.num_trans_samples;
        LOG(logDEBUG1) << "[Transceiver Databytes: " << num_transceiver_bytes
                       << ']';
    }

    uint64_t image_size =
        num_analog_bytes + num_digital_bytes + num_transceiver_bytes;
    LOG(logDEBUG1) << "Expected image size: " << image_size;
    return image_size;
}

void test_ctb_acquire_with_receiver(const testCtbAcquireInfo &test_info,
                                    int64_t num_frames_to_acquire,
                                    Detector &det, Caller &caller) {

    // save previous state
    testFileInfo prev_file_info = get_file_state(det);
    testCommonDetAcquireInfo prev_det_config_info =
        // overwrite exptime if not using virtual ctb server
        get_common_acquire_config_state(det);
    testCtbAcquireInfo prev_ctb_config_info = get_ctb_config_state(det);

    // defaults
    testFileInfo test_file_info;
    set_file_state(det, test_file_info);
    testCommonDetAcquireInfo det_config;
    det_config.num_frames_to_acquire = num_frames_to_acquire;
    set_common_acquire_config_state(det, det_config);

    // set ctb config
    set_ctb_config_state(det, test_info);

    // acquire
    REQUIRE_NOTHROW(
        test_acquire_with_receiver(caller, std::chrono::seconds{2}));

    // check frames caught
    REQUIRE_NOTHROW(test_frames_caught(det, num_frames_to_acquire));

    // check file size (assuming local pc)
    uint64_t expected_image_size = calculate_ctb_image_size(test_info);
    REQUIRE_NOTHROW(test_acquire_binary_file_size(
        test_file_info, num_frames_to_acquire, expected_image_size));

    // restore previous state
    set_file_state(det, prev_file_info);
    set_common_acquire_config_state(det, prev_det_config_info);
    set_ctb_config_state(det, prev_ctb_config_info);
}

} // namespace sls
