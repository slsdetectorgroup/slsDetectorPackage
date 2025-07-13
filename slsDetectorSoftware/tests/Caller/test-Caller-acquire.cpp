// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "Caller.h"
#include "catch.hpp"
#include "sls/Detector.h"
#include "sls/sls_detector_defs.h"
#include "test-Caller-global.h"
#include "sls/versionAPI.h"
#include "tests/globals.h"

#include <filesystem>
#include <sstream>

namespace sls {

using test::GET;
using test::PUT;


TEST_CASE("jungfrau_acquire_check_file_size", "[.cmdcall][.cmdacquire]") {
    Detector det;
    Caller caller(&det);
    auto det_type =
        det.getDetectorType().tsquash("Inconsistent detector types to test");

    if (det_type == defs::JUNGFRAU) {

        // save previous state
        testFileInfo prev_file_info = get_file_state(det);
        testCommonDetAcquireInfo prev_det_config_info =
            get_common_acquire_config_state(det);

        // save previous specific det type config
        auto exptime = det.getExptime().tsquash("inconsistent exptime to test");
        auto num_udp_interfaces = det.getNumberofUDPInterfaces().tsquash(
            "inconsistent number of udp interfaces");
        auto n_rows =
            det.getReadNRows().tsquash("inconsistent number of rows to test");

        // defaults
        int num_frames_to_acquire = 2;
        testFileInfo test_file_info;
        set_file_state(det, test_file_info);
        testCommonDetAcquireInfo det_config;
        det_config.num_frames_to_acquire = num_frames_to_acquire;
        set_common_acquire_config_state(det, det_config);

        // set default specific det type config
        det.setExptime(std::chrono::microseconds{200});
        det.setReadNRows(512);

        // acquire
        test_acquire_with_receiver(caller, det);

        // check frames caught
        test_frames_caught(det, num_frames_to_acquire);

        // check file size (assuming local pc)
        {
            detParameters par(det_type);
            int bytes_per_pixel = det.getDynamicRange().squash() / 8;
            // if 2 udp interfaces, data split into half
            size_t expected_image_size = (par.nChanX * par.nChanY * par.nChipX *
                                          par.nChipY * bytes_per_pixel) /
                                         num_udp_interfaces;
            test_acquire_binary_file_size(test_file_info, num_frames_to_acquire,
                                          expected_image_size);
        }
        // restore previous state
        set_file_state(det, prev_file_info);
        set_common_acquire_config_state(det, prev_det_config_info);

        // restore previous specific det type config
        det.setExptime(exptime);
        det.setReadNRows(n_rows);
    }
}

TEST_CASE("eiger_acquire_check_file_size", "[.cmdcall][.cmdacquire]") {
    Detector det;
    Caller caller(&det);
    auto det_type =
        det.getDetectorType().tsquash("Inconsistent detector types to test");

    if (det_type == defs::EIGER) {

        // save previous state
        testFileInfo prev_file_info = get_file_state(det);
        testCommonDetAcquireInfo prev_det_config_info =
            get_common_acquire_config_state(det);

        // save previous specific det type config
        auto exptime = det.getExptime().tsquash("inconsistent exptime to test");
        auto n_rows =
            det.getReadNRows().tsquash("inconsistent number of rows to test");
        auto dynamic_range =
            det.getDynamicRange().tsquash("inconsistent dynamic range to test");
        REQUIRE(false ==
                det.getTenGiga().tsquash("inconsistent 10Giga to test"));

        // defaults
        int num_frames_to_acquire = 2;
        testFileInfo test_file_info;
        set_file_state(det, test_file_info);
        testCommonDetAcquireInfo det_config;
        det_config.num_frames_to_acquire = num_frames_to_acquire;
        set_common_acquire_config_state(det, det_config);

        // set default specific det type config
        det.setExptime(std::chrono::microseconds{200});
        det.setReadNRows(256);
        det.setDynamicRange(16);

        // acquire
        test_acquire_with_receiver(caller, det);

        // check frames caught
        test_frames_caught(det, num_frames_to_acquire);

        // check file size (assuming local pc)
        {
            detParameters par(det_type);
            // data split into half due to 2 udp interfaces per half module
            int num_chips = (par.nChipX / 2);
            int bytes_per_pixel = (dynamic_range / 8);
            size_t expected_image_size =
                par.nChanX * par.nChanY * num_chips * bytes_per_pixel;
            test_acquire_binary_file_size(test_file_info, num_frames_to_acquire,
                                          expected_image_size);
        }

        // restore previous state
        set_file_state(det, prev_file_info);
        set_common_acquire_config_state(det, prev_det_config_info);

        // restore previous specific det type config
        det.setExptime(exptime);
        det.setReadNRows(n_rows);
        det.setDynamicRange(dynamic_range);
    }
}

TEST_CASE("moench_acquire_check_file_size", "[.cmdcall][.cmdacquire]") {
    Detector det;
    Caller caller(&det);
    auto det_type =
        det.getDetectorType().tsquash("Inconsistent detector types to test");

    if (det_type == defs::MOENCH) {

        // save previous state
        testFileInfo prev_file_info = get_file_state(det);
        testCommonDetAcquireInfo prev_det_config_info =
            get_common_acquire_config_state(det);

        // save previous specific det type config
        auto exptime = det.getExptime().tsquash("inconsistent exptime to test");
        auto num_udp_interfaces = det.getNumberofUDPInterfaces().tsquash(
            "inconsistent number of udp interfaces");
        auto n_rows =
            det.getReadNRows().tsquash("inconsistent number of rows to test");

        // defaults
        int num_frames_to_acquire = 2;
        testFileInfo test_file_info;
        set_file_state(det, test_file_info);
        testCommonDetAcquireInfo det_config;
        det_config.num_frames_to_acquire = num_frames_to_acquire;
        set_common_acquire_config_state(det, det_config);

        // set default specific det type config
        det.setExptime(std::chrono::microseconds{200});
        det.setReadNRows(400);

        // acquire
        test_acquire_with_receiver(caller, det);

        // check frames caught
        test_frames_caught(det, num_frames_to_acquire);

        // check file size (assuming local pc)
        {
            detParameters par(det_type);
            int bytes_per_pixel = det.getDynamicRange().squash() / 8;
            // if 2 udp interfaces, data split into half
            size_t expected_image_size = (par.nChanX * par.nChanY * par.nChipX *
                                          par.nChipY * bytes_per_pixel) /
                                         num_udp_interfaces;
            test_acquire_binary_file_size(test_file_info, num_frames_to_acquire,
                                          expected_image_size);
        }

        // restore previous state
        set_file_state(det, prev_file_info);
        set_common_acquire_config_state(det, prev_det_config_info);

        // restore previous specific det type config
        det.setExptime(exptime);
        det.setReadNRows(n_rows);
    }
}

TEST_CASE("mythen3_acquire_check_file_size", "[.cmdcall][.cmdacquire]") {
    Detector det;
    Caller caller(&det);
    auto det_type =
        det.getDetectorType().tsquash("Inconsistent detector types to test");

    if (det_type == defs::MYTHEN3) {

        // save previous state
        testFileInfo prev_file_info = get_file_state(det);
        testCommonDetAcquireInfo prev_det_config_info =
            get_common_acquire_config_state(det);

        // save previous specific det type config
        auto exptime =
            det.getExptimeForAllGates().tsquash("inconsistent exptime to test");
        auto dynamic_range =
            det.getDynamicRange().tsquash("inconsistent dynamic range to test");
        uint32_t counter_mask =
            det.getCounterMask().tsquash("inconsistent counter mask to test");

        // defaults
        int num_frames_to_acquire = 2;
        testFileInfo test_file_info;
        set_file_state(det, test_file_info);
        testCommonDetAcquireInfo det_config;
        det_config.num_frames_to_acquire = num_frames_to_acquire;
        set_common_acquire_config_state(det, det_config);

        // set default specific det type config
        det.setExptime(-1, std::chrono::microseconds{200});
        int test_dynamic_range = 16;
        det.setDynamicRange(test_dynamic_range);
        int test_counter_mask = 0x3;
        int num_counters = __builtin_popcount(test_counter_mask);
        det.setCounterMask(test_counter_mask);

        // acquire
        test_acquire_with_receiver(caller, det);

        // check frames caught
        test_frames_caught(det, num_frames_to_acquire);

        // check file size (assuming local pc)
        {
            detParameters par(det_type);
            int bytes_per_pixel = test_dynamic_range / 8;
            int num_channels_per_counter = par.nChanX / 3;
            size_t expected_image_size = num_channels_per_counter *
                                         num_counters * par.nChipX *
                                         bytes_per_pixel;
            test_acquire_binary_file_size(test_file_info, num_frames_to_acquire,
                                          expected_image_size);
        }

        // restore previous state
        set_file_state(det, prev_file_info);
        set_common_acquire_config_state(det, prev_det_config_info);

        // restore previous specific det type config
        for (int iGate = 0; iGate < 3; ++iGate) {
            det.setExptime(iGate, exptime[iGate]);
        }
        det.setDynamicRange(dynamic_range);
        det.setCounterMask(counter_mask);
    }
}


TEST_CASE("gotthard2_acquire_check_file_size", "[.cmdcall][.cmdacquire]") {
    Detector det;
    Caller caller(&det);
    auto det_type =
        det.getDetectorType().tsquash("Inconsistent detector types to test");

    if (det_type == defs::GOTTHARD2) {

        // save previous state
        testFileInfo prev_file_info = get_file_state(det);
        testCommonDetAcquireInfo prev_det_config_info =
            get_common_acquire_config_state(det);

        // save previous specific det type config
        auto exptime = det.getExptime().tsquash("inconsistent exptime to test");
        auto burst_mode =
            det.getBurstMode().tsquash("inconsistent burst mode to test");
        auto number_of_bursts = det.getNumberOfBursts().tsquash(
            "inconsistent number of bursts to test");
        auto burst_period =
            det.getBurstPeriod().tsquash("inconsistent burst period to test");

        // defaults
        int num_frames_to_acquire = 2;
        testFileInfo test_file_info;
        set_file_state(det, test_file_info);
        testCommonDetAcquireInfo det_config;
        det_config.num_frames_to_acquire = num_frames_to_acquire;
        set_common_acquire_config_state(det, det_config);

        // set default specific det type config
        det.setExptime(std::chrono::microseconds{200});
        det.setBurstMode(defs::CONTINUOUS_EXTERNAL);
        det.setNumberOfBursts(1);
        det.setBurstPeriod(std::chrono::milliseconds{0});

        // acquire
        test_acquire_with_receiver(caller, det);

        // check frames caught
        test_frames_caught(det, num_frames_to_acquire);

        // check file size (assuming local pc)
        {
            detParameters par(det_type);
            int bytes_per_pixel = det.getDynamicRange().squash() / 8;
            size_t expected_image_size =
                par.nChanX * par.nChipX * bytes_per_pixel;
            test_acquire_binary_file_size(test_file_info, num_frames_to_acquire,
                                          expected_image_size);
        }
        // restore previous state
        set_file_state(det, prev_file_info);
        set_common_acquire_config_state(det, prev_det_config_info);

        // restore previous specific det type config
        det.setExptime(exptime);
        det.setBurstMode(burst_mode);
        det.setNumberOfBursts(number_of_bursts);
        det.setBurstPeriod(burst_period);
    }
}

TEST_CASE("ctb_acquire_check_file_size", "[.cmdcall][.cmdacquire]") {
    Detector det;
    Caller caller(&det);
    auto det_type =
        det.getDetectorType().tsquash("Inconsistent detector types to test");

    if (det_type == defs::CHIPTESTBOARD ||
        det_type == defs::XILINX_CHIPTESTBOARD) {
        int num_frames_to_acquire = 2;
        // all the test cases
        {
            testCtbAcquireInfo test_ctb_config;
            test_ctb_config.readout_mode = defs::ANALOG_AND_DIGITAL;
            REQUIRE_NOTHROW(test_ctb_acquire_with_receiver(
                test_ctb_config, num_frames_to_acquire, det, caller));
        }
        {
            testCtbAcquireInfo test_ctb_config;
            test_ctb_config.readout_mode = defs::ANALOG_AND_DIGITAL;
            test_ctb_config.dbit_offset = 16;
            REQUIRE_NOTHROW(test_ctb_acquire_with_receiver(
                test_ctb_config, num_frames_to_acquire, det, caller));
        }
        {
            testCtbAcquireInfo test_ctb_config;
            test_ctb_config.readout_mode = defs::ANALOG_AND_DIGITAL;
            test_ctb_config.dbit_reorder = true;
            REQUIRE_NOTHROW(test_ctb_acquire_with_receiver(
                test_ctb_config, num_frames_to_acquire, det, caller));
        }
        {
            testCtbAcquireInfo test_ctb_config;
            test_ctb_config.readout_mode = defs::ANALOG_AND_DIGITAL;
            test_ctb_config.dbit_offset = 16;
            test_ctb_config.dbit_reorder = true;
            REQUIRE_NOTHROW(test_ctb_acquire_with_receiver(
                test_ctb_config, num_frames_to_acquire, det, caller));
        }
        {
            testCtbAcquireInfo test_ctb_config;
            test_ctb_config.readout_mode = defs::ANALOG_AND_DIGITAL;
            test_ctb_config.dbit_offset = 16;
            test_ctb_config.dbit_list.clear();
            REQUIRE_NOTHROW(test_ctb_acquire_with_receiver(
                test_ctb_config, num_frames_to_acquire, det, caller));
        }
        {
            testCtbAcquireInfo test_ctb_config;
            test_ctb_config.readout_mode = defs::ANALOG_AND_DIGITAL;
            test_ctb_config.dbit_offset = 16;
            test_ctb_config.dbit_list.clear();
            test_ctb_config.dbit_reorder = true;
            REQUIRE_NOTHROW(test_ctb_acquire_with_receiver(
                test_ctb_config, num_frames_to_acquire, det, caller));
        }
        {
            testCtbAcquireInfo test_ctb_config;
            test_ctb_config.readout_mode = defs::DIGITAL_AND_TRANSCEIVER;
            REQUIRE_NOTHROW(test_ctb_acquire_with_receiver(
                test_ctb_config, num_frames_to_acquire, det, caller));
        }
        {
            testCtbAcquireInfo test_ctb_config;
            test_ctb_config.readout_mode = defs::DIGITAL_AND_TRANSCEIVER;
            test_ctb_config.dbit_offset = 16;
            REQUIRE_NOTHROW(test_ctb_acquire_with_receiver(
                test_ctb_config, num_frames_to_acquire, det, caller));
        }
        {
            testCtbAcquireInfo test_ctb_config;
            test_ctb_config.readout_mode = defs::DIGITAL_AND_TRANSCEIVER;
            test_ctb_config.dbit_list.clear();
            REQUIRE_NOTHROW(test_ctb_acquire_with_receiver(
                test_ctb_config, num_frames_to_acquire, det, caller));
        }
        {
            testCtbAcquireInfo test_ctb_config;
            test_ctb_config.readout_mode = defs::DIGITAL_AND_TRANSCEIVER;
            test_ctb_config.dbit_offset = 16;
            test_ctb_config.dbit_list.clear();
            REQUIRE_NOTHROW(test_ctb_acquire_with_receiver(
                test_ctb_config, num_frames_to_acquire, det, caller));
        }
        {
            testCtbAcquireInfo test_ctb_config;
            test_ctb_config.readout_mode = defs::DIGITAL_AND_TRANSCEIVER;
            test_ctb_config.dbit_offset = 16;
            test_ctb_config.dbit_list.clear();
            test_ctb_config.dbit_reorder = true;
            REQUIRE_NOTHROW(test_ctb_acquire_with_receiver(
                test_ctb_config, num_frames_to_acquire, det, caller));
        }
        {
            testCtbAcquireInfo test_ctb_config;
            test_ctb_config.readout_mode = defs::TRANSCEIVER_ONLY;
            test_ctb_config.dbit_offset = 16;
            test_ctb_config.dbit_list.clear();
            test_ctb_config.dbit_reorder = true;
            REQUIRE_NOTHROW(test_ctb_acquire_with_receiver(
                test_ctb_config, num_frames_to_acquire, det, caller));
        }
        {
            testCtbAcquireInfo test_ctb_config;
            test_ctb_config.readout_mode = defs::ANALOG_ONLY;
            test_ctb_config.dbit_offset = 16;
            test_ctb_config.dbit_list.clear();
            test_ctb_config.dbit_reorder = true;
            REQUIRE_NOTHROW(test_ctb_acquire_with_receiver(
                test_ctb_config, num_frames_to_acquire, det, caller));
        }
    }
}

} // namespace sls
