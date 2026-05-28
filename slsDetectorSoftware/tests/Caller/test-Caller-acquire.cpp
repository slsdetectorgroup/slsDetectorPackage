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

void acquire_and_check_file_size(
    Detector &det, const acq::CTBState &ctb_state = acq::default_ctb_state()) {
    auto acq_state = acq::default_acquisition_state();
    acq_state.num_frames = 2;
    auto file_state = acq::default_file_state();
    acq::run(det, acq_state, file_state, ctb_state);
    auto image_size = acq::get_expected_image_size(det, ctb_state);
    REQUIRE_NOTHROW(
        checks::check_binary_file_size(image_size, acq_state.num_frames));
}

void test_ctb_binary_file_size(Detector &det) {
    auto det_type =
        det.getDetectorType().tsquash("Inconsistent detector types to test");

    acq::CTBState ctb_state =
        acq::default_ctb_state(det_type == defs::CHIPTESTBOARD);
    // readout mode = defs::ANALOG_AND_DIGITAL
    // analog samples = 5000
    // digital samples = 6000
    // trans samples = 288
    // ten_giga = isAltera ? false : true
    // adc enable 1g = 0xFFFFFF00
    // adc enable 1g = 0xFF00FFFF
    // dbit offset = 0
    // dbit list = {0, 12, 2, 43}
    // dbit reorder = false
    // trans mask = 0x3
    {
        acq::CTBStateGuard ctb_guard(det, ctb_state);
        REQUIRE_NOTHROW(acquire_and_check_file_size(det, ctb_state));
    }
    {
        acq::CTBStateGuard ctb_guard(det, ctb_state);
        ctb_state.dbit_reorder = true;
        REQUIRE_NOTHROW(acquire_and_check_file_size(det, ctb_state));
    }
    {
        acq::CTBStateGuard ctb_guard(det, ctb_state);
        ctb_state.dbit_offset = 16;
        REQUIRE_NOTHROW(acquire_and_check_file_size(det, ctb_state));
    }
    {
        acq::CTBStateGuard ctb_guard(det, ctb_state);
        ctb_state.dbit_offset = 16;
        ctb_state.dbit_reorder = true;
        REQUIRE_NOTHROW(acquire_and_check_file_size(det, ctb_state));
    }
    {
        acq::CTBStateGuard ctb_guard(det, ctb_state);
        ctb_state.dbit_list.clear();
        REQUIRE_NOTHROW(acquire_and_check_file_size(det, ctb_state));
    }
    {
        acq::CTBStateGuard ctb_guard(det, ctb_state);
        ctb_state.dbit_offset = 16;
        ctb_state.dbit_list.clear();
        REQUIRE_NOTHROW(acquire_and_check_file_size(det, ctb_state));
    }
    {
        acq::CTBStateGuard ctb_guard(det, ctb_state);
        ctb_state.dbit_reorder = true;
        ctb_state.dbit_list.clear();
        REQUIRE_NOTHROW(acquire_and_check_file_size(det, ctb_state));
    }
    {
        acq::CTBStateGuard ctb_guard(det, ctb_state);
        ctb_state.dbit_offset = 16;
        ctb_state.dbit_reorder = true;
        ctb_state.dbit_list.clear();
        REQUIRE_NOTHROW(acquire_and_check_file_size(det, ctb_state));
    }
    {
        acq::CTBStateGuard ctb_guard(det, ctb_state);
        ctb_state.readout_mode = defs::DIGITAL_AND_TRANSCEIVER;
        REQUIRE_NOTHROW(acquire_and_check_file_size(det, ctb_state));
    }
    {
        acq::CTBStateGuard ctb_guard(det, ctb_state);
        ctb_state.readout_mode = defs::DIGITAL_AND_TRANSCEIVER;
        ctb_state.dbit_offset = 16;
        REQUIRE_NOTHROW(acquire_and_check_file_size(det, ctb_state));
    }
    {
        acq::CTBStateGuard ctb_guard(det, ctb_state);
        ctb_state.readout_mode = defs::DIGITAL_AND_TRANSCEIVER;
        ctb_state.dbit_reorder = true;
        REQUIRE_NOTHROW(acquire_and_check_file_size(det, ctb_state));
    }
    {
        acq::CTBStateGuard ctb_guard(det, ctb_state);
        ctb_state.readout_mode = defs::DIGITAL_AND_TRANSCEIVER;
        ctb_state.dbit_offset = 16;
        ctb_state.dbit_reorder = true;
        REQUIRE_NOTHROW(acquire_and_check_file_size(det, ctb_state));
    }
    {
        acq::CTBStateGuard ctb_guard(det, ctb_state);
        ctb_state.readout_mode = defs::DIGITAL_AND_TRANSCEIVER;
        ctb_state.dbit_list.clear();
        REQUIRE_NOTHROW(acquire_and_check_file_size(det, ctb_state));
    }
    {
        acq::CTBStateGuard ctb_guard(det, ctb_state);
        ctb_state.readout_mode = defs::DIGITAL_AND_TRANSCEIVER;
        ctb_state.dbit_offset = 16;
        ctb_state.dbit_list.clear();
        REQUIRE_NOTHROW(acquire_and_check_file_size(det, ctb_state));
    }
    {
        acq::CTBStateGuard ctb_guard(det, ctb_state);
        ctb_state.readout_mode = defs::DIGITAL_AND_TRANSCEIVER;
        ctb_state.dbit_reorder = true;
        ctb_state.dbit_list.clear();
        REQUIRE_NOTHROW(acquire_and_check_file_size(det, ctb_state));
    }
    {
        acq::CTBStateGuard ctb_guard(det, ctb_state);
        ctb_state.readout_mode = defs::DIGITAL_AND_TRANSCEIVER;
        ctb_state.dbit_offset = 16;
        ctb_state.dbit_reorder = true;
        ctb_state.dbit_list.clear();
        REQUIRE_NOTHROW(acquire_and_check_file_size(det, ctb_state));
    }
    {
        acq::CTBStateGuard ctb_guard(det, ctb_state);
        ctb_state.readout_mode = defs::TRANSCEIVER_ONLY;
        REQUIRE_NOTHROW(acquire_and_check_file_size(det, ctb_state));
    }
    {
        acq::CTBStateGuard ctb_guard(det, ctb_state);
        ctb_state.readout_mode = defs::TRANSCEIVER_ONLY;
        ctb_state.dbit_reorder = true;
        REQUIRE_NOTHROW(acquire_and_check_file_size(det, ctb_state));
    }
    {
        acq::CTBStateGuard ctb_guard(det, ctb_state);
        ctb_state.readout_mode = defs::TRANSCEIVER_ONLY;
        ctb_state.dbit_offset = 16;
        REQUIRE_NOTHROW(acquire_and_check_file_size(det, ctb_state));
    }
    {
        acq::CTBStateGuard ctb_guard(det, ctb_state);
        ctb_state.readout_mode = defs::TRANSCEIVER_ONLY;
        ctb_state.dbit_offset = 16;
        ctb_state.dbit_reorder = true;
        REQUIRE_NOTHROW(acquire_and_check_file_size(det, ctb_state));
    }
    {
        acq::CTBStateGuard ctb_guard(det, ctb_state);
        ctb_state.readout_mode = defs::TRANSCEIVER_ONLY;
        ctb_state.dbit_list.clear();
        REQUIRE_NOTHROW(acquire_and_check_file_size(det, ctb_state));
    }
    {
        acq::CTBStateGuard ctb_guard(det, ctb_state);
        ctb_state.readout_mode = defs::TRANSCEIVER_ONLY;
        ctb_state.dbit_offset = 16;
        ctb_state.dbit_list.clear();
        REQUIRE_NOTHROW(acquire_and_check_file_size(det, ctb_state));
    }
    {
        acq::CTBStateGuard ctb_guard(det, ctb_state);
        ctb_state.readout_mode = defs::TRANSCEIVER_ONLY;
        ctb_state.dbit_reorder = true;
        ctb_state.dbit_list.clear();
        REQUIRE_NOTHROW(acquire_and_check_file_size(det, ctb_state));
    }
    {
        acq::CTBStateGuard ctb_guard(det, ctb_state);
        ctb_state.readout_mode = defs::TRANSCEIVER_ONLY;
        ctb_state.dbit_offset = 16;
        ctb_state.dbit_reorder = true;
        ctb_state.dbit_list.clear();
        REQUIRE_NOTHROW(acquire_and_check_file_size(det, ctb_state));
    }
    {
        acq::CTBStateGuard ctb_guard(det, ctb_state);
        ctb_state.readout_mode = defs::ANALOG_ONLY;
        ctb_state.dbit_offset = 16;
        ctb_state.dbit_reorder = true;
        REQUIRE_NOTHROW(acquire_and_check_file_size(det, ctb_state));
    }
}

// disable for jungfrau as it requires higher maximum receive buffer size
//  sysctl net.core.rmem_max=$((100*1024*1024))
//  sysctl net.core.rmem_default=$((100*1024*1024))
TEST_CASE("acquire_check_binary_file_size",
          "[.detectorintegration][.disable_check_data_file]") {

    Detector det;
    auto det_type =
        det.getDetectorType().tsquash("Inconsistent detector types to test");
    if (det_type == defs::CHIPTESTBOARD ||
        det_type == defs::XILINX_CHIPTESTBOARD)
        test_ctb_binary_file_size(det);
    else
        REQUIRE_NOTHROW(acquire_and_check_file_size(det));
}

} // namespace sls
