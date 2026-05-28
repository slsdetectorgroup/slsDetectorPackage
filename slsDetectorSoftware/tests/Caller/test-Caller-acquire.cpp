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
#include "acquire/ExpectedState.h"
#include "acquire/FileState.h"
#include "checks/MasterFileChecks.h"

#include <filesystem>
#include <sstream>

namespace sls {

namespace acq = sls::test::acquire;
namespace checks = sls::test::checks;

using test::GET;
using test::PUT;

// disable for jungfrau as it requires higher maximum receive buffer size
//  sysctl net.core.rmem_max=$((100*1024*1024))
//  sysctl net.core.rmem_default=$((100*1024*1024))
TEST_CASE("acquire_check_binary_file_size",
          "[.detectorintegration][.disable_check_data_file]") {

    Detector det;
    auto det_type =
        det.getDetectorType().tsquash("Inconsistent detector types to test");
    if (det_type == defs::CHIPTESTBOARD ||
        det_type == defs::XILINX_CHIPTESTBOARD) {
        // skip for CTB as it has different tests for file size
        return;
    }
    auto acq_state = acq::default_acquisition_state();
    acq_state.num_frames = 2;
    acq::run(det, acq_state);
    auto image_size = acq::get_expected_image_size(det);
    checks::check_binary_file_size(image_size, acq_state.num_frames);
}

void test_ctb_file_size_with_acquire(Detector &det, int64_t num_frames,
                                     const acq::CTBState &test_info,
                                     bool isXilinxCtb) {
    auto acq_state = acq::default_acquisition_state();
    acq_state.num_frames = num_frames;
    acq::run(det, acq_state);
    auto image_size = acq::get_expected_image_size(det, test_info);
    checks::check_binary_file_size(image_size, acq_state.num_frames);
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
