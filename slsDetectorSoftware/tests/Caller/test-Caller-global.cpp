// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "test-Caller-global.h"
#include "Caller.h"
#include "catch.hpp"
#include "sls/Detector.h"
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
    std::cout << "exepected file size: " << expected_file_size
              << " receiver header size :" << sizeof(defs::sls_receiver_header)
              << " num frames:" << num_frames_to_acquire << std::endl;
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
    testCommonDetAcquireInfo det_config_info{
        det.getTimingMode().tsquash("Inconsistent timing mode"),
        det.getNumberOfFrames().tsquash("Inconsistent number of frames"),
        det.getNumberOfTriggers().tsquash("Inconsistent number of triggers"),
        det.getPeriod().tsquash("Inconsistent period"),
        {}};
    auto det_type =
        det.getDetectorType().tsquash("Inconsistent detector types to test");
    if (det_type != defs::MYTHEN3) {
        det_config_info.exptime[0] =
            det.getExptime().tsquash("inconsistent exptime to test");
    } else {
        det_config_info.exptime =
            det.getExptimeForAllGates().tsquash("inconsistent exptime to test");
    }
    return det_config_info;
}

void set_common_acquire_config_state(
    Detector &det, const testCommonDetAcquireInfo &det_config_info) {
    det.setTimingMode(det_config_info.timing_mode);
    det.setNumberOfFrames(det_config_info.num_frames_to_acquire);
    det.setNumberOfTriggers(det_config_info.num_triggers);
    det.setPeriod(det_config_info.period);
    auto det_type =
        det.getDetectorType().tsquash("Inconsistent detector types to test");
    if (det_type != defs::MYTHEN3) {
        det.setExptime(det_config_info.exptime[0]);
    } else {
        for (int iGate = 0; iGate < 3; ++iGate) {
            det.setExptime(iGate, det_config_info.exptime[iGate]);
        }
    }
}

} // namespace sls
