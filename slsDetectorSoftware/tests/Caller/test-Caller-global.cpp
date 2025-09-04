// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "test-Caller-global.h"
#include "Caller.h"
#include "GeneralData.h"
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

void test_acquire_with_receiver(Caller &caller, const Detector &det) {
    REQUIRE_NOTHROW(caller.call("rx_start", {}, -1, PUT));
    REQUIRE_NOTHROW(caller.call("start", {}, -1, PUT));
    bool idle = false;
    while (!idle) {
        std::ostringstream oss;
        REQUIRE_NOTHROW(caller.call("status", {}, -1, GET));
        auto statusList = det.getDetectorStatus();
        if (statusList.any(defs::ERROR)) {
            throw std::runtime_error("error status while acquiring");
        }
        if (statusList.contains_only(defs::IDLE, defs::STOPPED)) {
            idle = true;
        }
    }
    REQUIRE_NOTHROW(caller.call("rx_stop", {}, -1, PUT));
}

void create_files_for_acquire(
    Detector &det, Caller &caller, int64_t num_frames,
    const std::optional<testCtbAcquireInfo> &test_info) {

    // save previous state
    testFileInfo prev_file_info = get_file_state(det);
    auto prev_num_frames = det.getNumberOfFrames().tsquash(
        "Inconsistent number of frames to acquire");
    std::optional<testCtbAcquireInfo> prev_ctb_config_info{};
    if (test_info) {
        prev_ctb_config_info = get_ctb_config_state(det);
    }

    // set state for acquire
    testFileInfo test_file_info;
    set_file_state(det, test_file_info);
    det.setNumberOfFrames(num_frames);
    if (test_info) {
        set_ctb_config_state(det, *test_info);
    }

    // acquire and get num frames caught
    REQUIRE_NOTHROW(test_acquire_with_receiver(caller, det));
    auto frames_caught = det.getFramesCaught().tsquash(
        "Inconsistent number of frames caught")[0];
    REQUIRE(frames_caught == num_frames);

    // hdf5
#ifdef HDF5C
    test_file_info.file_format = defs::HDF5;
    test_file_info.file_acq_index = 0;
    set_file_state(det, test_file_info);

    // acquire and get num frames caught
    test_acquire_with_receiver(caller, det);
    frames_caught = det.getFramesCaught().tsquash(
        "Inconsistent number of frames caught")[0];
    REQUIRE(frames_caught == num_frames);
#endif

    // restore previous state
    // file
    set_file_state(det, prev_file_info);
    det.setNumberOfFrames(prev_num_frames);
    if (test_info) {
        set_ctb_config_state(det, *prev_ctb_config_info);
    }
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

std::pair<uint64_t, int>
calculate_ctb_image_size(const testCtbAcquireInfo &test_info,
                         bool isXilinxCtb) {

    // test_info.print(); // for debugging
    sls::CtbImageInputs inputs{};
    inputs.mode = test_info.readout_mode;
    inputs.nAnalogSamples = test_info.num_adc_samples;
    inputs.adcMask = test_info.adc_enable_10g;
    if (!isXilinxCtb && !test_info.ten_giga) {
        inputs.adcMask = test_info.adc_enable_1g;
    }
    inputs.nTransceiverSamples = test_info.num_trans_samples;
    inputs.transceiverMask = test_info.transceiver_mask;
    inputs.nDigitalSamples = test_info.num_dbit_samples;
    inputs.dbitOffset = test_info.dbit_offset;
    inputs.dbitReorder = test_info.dbit_reorder;
    inputs.dbitList = test_info.dbit_list;

    auto out = computeCtbImageSize(inputs);
    uint64_t image_size =
        out.nAnalogBytes + out.nDigitalBytes + out.nTransceiverBytes;
    LOG(logDEBUG1) << "Expected image size: " << image_size;
    int npixelx = out.nPixelsX;
    LOG(logDEBUG1) << "Expected number of pixels in x: " << npixelx;
    return std::make_pair(image_size, npixelx);
}

} // namespace sls
