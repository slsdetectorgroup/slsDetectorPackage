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

TEST_CASE("udp_datastream with master file",
          "[.detectorintegration][.disable_check_data_file]") {
    Detector det;
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::EIGER) {
        auto prev_val_left = det.getUDPDataStream(defs::LEFT);
        auto prev_val_right = det.getUDPDataStream(defs::RIGHT);

        det.setUDPDataStream(defs::LEFT, false);
        // check master file
        {
            // expected
            std::vector<defs::portPosition> expected_ports =
                det.getPortPositionList();
            std::vector<int> expected_disabled_ports =
                det.getRxDisabledUDPPortIndices();
            REQUIRE(expected_disabled_ports.size() > 0);

            test_run_with_master_file_checker(
                det, [&](auto &det, auto &acq_state, auto &file_state,
                         auto &checker) {
                    checks::check_udp_ports_type(checker, expected_ports);
                    checks::check_udp_ports_disabled(checker,
                                                     expected_disabled_ports);
                });
        }

        for (int i = 0; i != det.size(); ++i) {
            det.setUDPDataStream(defs::LEFT, prev_val_left[i], {i});
            det.setUDPDataStream(defs::RIGHT, prev_val_right[i], {i});
        }
    } else if ((det_type == defs::JUNGFRAU || det_type == defs::MOENCH) &&
               (det.getNumberofUDPInterfaces().squash(0) == 2)) {
        auto prev_val_top = det.getUDPDataStream(defs::TOP);
        auto prev_val_bottom = det.getUDPDataStream(defs::BOTTOM);

        det.setUDPDataStream(defs::TOP, false);
        // check master file
        {
            // expected
            std::vector<defs::portPosition> expected_ports =
                det.getPortPositionList();
            std::vector<int> expected_disabled_ports =
                det.getRxDisabledUDPPortIndices();
            REQUIRE(expected_disabled_ports.size() > 0);

            test_run_with_master_file_checker(
                det, [&](auto &det, auto &acq_state, auto &file_state,
                         auto &checker) {
                    checks::check_udp_ports_type(checker, expected_ports);
                    checks::check_udp_ports_disabled(checker,
                                                     expected_disabled_ports);
                });
        }

        for (int i = 0; i != det.size(); ++i) {
            det.setUDPDataStream(defs::TOP, prev_val_top[i], {i});
            det.setUDPDataStream(defs::BOTTOM, prev_val_bottom[i], {i});
        }
    }
}

} // namespace sls
