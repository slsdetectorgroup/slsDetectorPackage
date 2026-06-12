// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "sls/Detector.h"
#include "sls/ToString.h"
#include "test-Caller-global.h"

#include "catch.hpp"
#include <string>

namespace sls {

namespace acq = sls::test::acquire;

TEST_CASE("check_master_file_attributes",
          "[.detectorintegration][.disable_check_data_file]") {

    Detector det;
    auto detType = det.getDetectorType().squash(defs::GENERIC);
    INFO("Testing master file attributes with " << ToString(detType));

    // if ctb, set to default and restore after test
    std::optional<acq::CTBState> ctb_state = std::nullopt;
    if (detType == defs::CHIPTESTBOARD ||
        detType == defs::XILINX_CHIPTESTBOARD) {
        ctb_state = std::make_optional(acq::default_ctb_state(detType));
    }
    acq::CTBStateGuard ctb_guard(det, ctb_state);

    test_run_with_master_file_checker(
        det, [&](auto &det, auto &acq_state, auto &file_state, auto &checker) {
            // get expected state of parameters and check against master file
            auto expected_state = acq::build_expected_state(
                det, acq_state, file_state, ctb_state);
            checks::check_metadata(checker, expected_state);
        });
}

} // namespace sls
