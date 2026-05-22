// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "Caller.h"
#include "catch.hpp"
#include "sls/Detector.h"
#include "sls/sls_detector_defs.h"
#include "sls/versionAPI.h"
#include "test-Caller-global.h"
#include "tests/globals.h"

#include "acquire/Acquire.h"
#include "acquire/CTBState.h"
#include "acquire/FileState.h"

#include <filesystem>
#include <sstream>

namespace sls {

namespace acq = sls::test::acquire;

using test::GET;
using test::PUT;

// disable for jungfrau as it requires higher maximum receive buffer size
//  sysctl net.core.rmem_max=$((100*1024*1024))
//  sysctl net.core.rmem_default=$((100*1024*1024))
TEST_CASE("jungfrau_or_moench_acquire_check_file_size",
          "[.detectorintegration][.disable_check_data_file]") {

    Detector det;
    Caller caller(&det);
    auto det_type =
        det.getDetectorType().tsquash("Inconsistent detector types to test");

    if (det_type == defs::JUNGFRAU || det_type == defs::MOENCH) {

        auto num_udp_interfaces = det.getNumberofUDPInterfaces().tsquash(
            "inconsistent number of udp interfaces");

        int64_t num_frames_to_acquire = 2;
        auto f = acq::default_file_state();
        acq::run(det, num_frames_to_acquire, f);

        // check file size (assuming local pc)
        {
            detParameters par(det_type);
            int bytes_per_pixel = det.getDynamicRange().squash() / 8;
            // if 2 udp interfaces, data split into half
            size_t expected_image_size = (par.nChanX * par.nChanY * par.nChipX *
                                          par.nChipY * bytes_per_pixel) /
                                         num_udp_interfaces;
            REQUIRE_NOTHROW(test_acquire_binary_file_size(
                f, num_frames_to_acquire, expected_image_size));
        }
    }
}

TEST_CASE("eiger_acquire_check_file_size",
          "[.detectorintegration][.disable_check_data_file]") {
    Detector det;
    Caller caller(&det);
    auto det_type =
        det.getDetectorType().tsquash("Inconsistent detector types to test");

    if (det_type == defs::EIGER) {

        int dynamic_range = det.getDynamicRange().squash();
        if (dynamic_range != 16) {
            throw RuntimeError(
                "Eiger detector must have dynamic range 16 to test");
        }
        int64_t num_frames_to_acquire = 2;
        auto f = acq::default_file_state();
        acq::run(det, num_frames_to_acquire, f);

        // check file size (assuming local pc)
        {
            detParameters par(det_type);
            // data split into half due to 2 udp interfaces per half module
            int num_chips = (par.nChipX / 2);
            int bytes_per_pixel = (dynamic_range / 8);
            size_t expected_image_size =
                par.nChanX * par.nChanY * num_chips * bytes_per_pixel;
            REQUIRE_NOTHROW(test_acquire_binary_file_size(
                f, num_frames_to_acquire, expected_image_size));
        }
    }
}

TEST_CASE("mythen3_acquire_check_file_size",
          "[.detectorintegration][.disable_check_data_file]") {
    Detector det;
    Caller caller(&det);
    auto det_type =
        det.getDetectorType().tsquash("Inconsistent detector types to test");

    if (det_type == defs::MYTHEN3) {

        int dynamic_range = det.getDynamicRange().squash();
        int counter_mask = det.getCounterMask().squash();
        if (dynamic_range != 16 && counter_mask != 0x3) {
            throw RuntimeError("Mythen3 detector must have dynamic range 16 "
                               "and counter mask 0x3 to test");
        }
        int num_counters = __builtin_popcount(counter_mask);
        int64_t num_frames_to_acquire = 2;
        auto f = acq::default_file_state();
        acq::run(det, num_frames_to_acquire, f);

        // check file size (assuming local pc)
        {
            detParameters par(det_type);
            int bytes_per_pixel = dynamic_range / 8;
            int num_channels_per_counter = par.nChanX / 3;
            size_t expected_image_size = num_channels_per_counter *
                                         num_counters * par.nChipX *
                                         bytes_per_pixel;
            REQUIRE_NOTHROW(test_acquire_binary_file_size(
                f, num_frames_to_acquire, expected_image_size));
        }
    }
}

TEST_CASE("gotthard2_acquire_check_file_size",
          "[.detectorintegration][.disable_check_data_file]") {
    Detector det;
    Caller caller(&det);
    auto det_type =
        det.getDetectorType().tsquash("Inconsistent detector types to test");

    if (det_type == defs::GOTTHARD2) {

        int64_t num_frames_to_acquire = 2;
        auto f = acq::default_file_state();
        acq::run(det, num_frames_to_acquire, f);

        // check file size (assuming local pc)
        {
            detParameters par(det_type);
            int bytes_per_pixel = det.getDynamicRange().squash() / 8;
            size_t expected_image_size =
                par.nChanX * par.nChipX * bytes_per_pixel;
            REQUIRE_NOTHROW(test_acquire_binary_file_size(
                f, num_frames_to_acquire, expected_image_size));
        }
    }
}

void test_ctb_file_size_with_acquire(Detector &det, int64_t num_frames,
                                     const acq::CTBState &test_info,
                                     bool isXilinxCtb) {

    auto f = acq::default_file_state();
    acq::run(det, num_frames, test_info, f);

    // check file size (assuming local pc)
    uint64_t expected_image_size =
        calculate_ctb_image_size(test_info, isXilinxCtb).first;
    REQUIRE_NOTHROW(
        test_acquire_binary_file_size(f, num_frames, expected_image_size));
}

// disable for xilinx_ctb as it requires higher maximum receive buffer size
//  sysctl net.core.rmem_max=$((100*1024*1024))
//  sysctl net.core.rmem_default=$((100*1024*1024))
TEST_CASE("ctb_acquire_check_file_size",
          "[.detectorintegration][.disable_check_data_file]") {
    Detector det;
    Caller caller(&det);
    auto det_type =
        det.getDetectorType().tsquash("Inconsistent detector types to test");

    if (det_type == defs::CHIPTESTBOARD ||
        det_type == defs::XILINX_CHIPTESTBOARD) {
        bool isXilinxCtb = (det_type == defs::XILINX_CHIPTESTBOARD);
        bool isAltera = !isXilinxCtb;
        int64_t num_frames_to_acquire = 2;
        // all the test cases
        acq::CTBState ctb_state = acq::default_ctb_state(isAltera);
        ctb_state.readout_mode = defs::ANALOG_AND_DIGITAL;
        REQUIRE_NOTHROW(test_ctb_file_size_with_acquire(
            det, num_frames_to_acquire, ctb_state, isXilinxCtb));

        ctb_state = acq::default_ctb_state(isAltera);
        ctb_state.readout_mode = defs::ANALOG_AND_DIGITAL;
        ctb_state.dbit_offset = 16;
        REQUIRE_NOTHROW(test_ctb_file_size_with_acquire(
            det, num_frames_to_acquire, ctb_state, isXilinxCtb));

        ctb_state = acq::default_ctb_state(isAltera);
        ctb_state.readout_mode = defs::ANALOG_AND_DIGITAL;
        ctb_state.dbit_reorder = true;
        REQUIRE_NOTHROW(test_ctb_file_size_with_acquire(
            det, num_frames_to_acquire, ctb_state, isXilinxCtb));

        ctb_state = acq::default_ctb_state(isAltera);
        ctb_state.readout_mode = defs::ANALOG_AND_DIGITAL;
        ctb_state.dbit_offset = 16;
        ctb_state.dbit_reorder = true;
        REQUIRE_NOTHROW(test_ctb_file_size_with_acquire(
            det, num_frames_to_acquire, ctb_state, isXilinxCtb));

        ctb_state = acq::default_ctb_state(isAltera);
        ctb_state.readout_mode = defs::ANALOG_AND_DIGITAL;
        ctb_state.dbit_offset = 16;
        ctb_state.dbit_list.clear();
        REQUIRE_NOTHROW(test_ctb_file_size_with_acquire(
            det, num_frames_to_acquire, ctb_state, isXilinxCtb));

        ctb_state = acq::default_ctb_state(isAltera);
        ctb_state.readout_mode = defs::ANALOG_AND_DIGITAL;
        ctb_state.dbit_offset = 16;
        ctb_state.dbit_list.clear();
        ctb_state.dbit_reorder = true;
        REQUIRE_NOTHROW(test_ctb_file_size_with_acquire(
            det, num_frames_to_acquire, ctb_state, isXilinxCtb));

        ctb_state = acq::default_ctb_state(isAltera);
        ctb_state.readout_mode = defs::DIGITAL_AND_TRANSCEIVER;
        REQUIRE_NOTHROW(test_ctb_file_size_with_acquire(
            det, num_frames_to_acquire, ctb_state, isXilinxCtb));

        ctb_state = acq::default_ctb_state(isAltera);
        ctb_state.readout_mode = defs::DIGITAL_AND_TRANSCEIVER;
        ctb_state.dbit_offset = 16;
        REQUIRE_NOTHROW(test_ctb_file_size_with_acquire(
            det, num_frames_to_acquire, ctb_state, isXilinxCtb));

        ctb_state = acq::default_ctb_state(isAltera);
        ctb_state.readout_mode = defs::DIGITAL_AND_TRANSCEIVER;
        ctb_state.dbit_list.clear();
        REQUIRE_NOTHROW(test_ctb_file_size_with_acquire(
            det, num_frames_to_acquire, ctb_state, isXilinxCtb));

        ctb_state = acq::default_ctb_state(isAltera);
        ctb_state.readout_mode = defs::DIGITAL_AND_TRANSCEIVER;
        ctb_state.dbit_offset = 16;
        ctb_state.dbit_list.clear();
        REQUIRE_NOTHROW(test_ctb_file_size_with_acquire(
            det, num_frames_to_acquire, ctb_state, isXilinxCtb));

        ctb_state = acq::default_ctb_state(isAltera);
        ctb_state.readout_mode = defs::DIGITAL_AND_TRANSCEIVER;
        ctb_state.dbit_offset = 16;
        ctb_state.dbit_list.clear();
        ctb_state.dbit_reorder = true;
        REQUIRE_NOTHROW(test_ctb_file_size_with_acquire(
            det, num_frames_to_acquire, ctb_state, isXilinxCtb));

        ctb_state = acq::default_ctb_state(isAltera);
        ctb_state.readout_mode = defs::TRANSCEIVER_ONLY;
        ctb_state.dbit_offset = 16;
        ctb_state.dbit_list.clear();
        ctb_state.dbit_reorder = true;
        REQUIRE_NOTHROW(test_ctb_file_size_with_acquire(
            det, num_frames_to_acquire, ctb_state, isXilinxCtb));

        ctb_state = acq::default_ctb_state(isAltera);
        ctb_state.readout_mode = defs::ANALOG_ONLY;
        ctb_state.dbit_offset = 16;
        ctb_state.dbit_list.clear();
        ctb_state.dbit_reorder = true;
        REQUIRE_NOTHROW(test_ctb_file_size_with_acquire(
            det, num_frames_to_acquire, ctb_state, isXilinxCtb));
    }
}

} // namespace sls
