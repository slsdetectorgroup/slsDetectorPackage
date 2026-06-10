// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "acquire/ExpectedState.h"
#include "checks/MasterFileChecks.h"

#include "sls/Detector.h"
#include "sls/ToString.h"
#include "sls/logger.h"

#include "catch.hpp"
#include <filesystem>
#include <fstream>
#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <sstream>
#include <string>

#ifdef HDF5C
#include "H5Cpp.h"
#endif

namespace sls {

namespace mf = sls::test::master_file;
namespace acq = sls::test::acquire;
namespace checks = sls::test::checks;

TEST_CASE("check_master_file_attributes",
          "[.detectorintegration][.disable_check_data_file]") {

    Detector det;
    auto detType = det.getDetectorType().squash(defs::GENERIC);
    INFO("Testing master file attributes with " << ToString(detType));

    // currently num frame = 1 (default)
    auto acq_state = acq::default_acquisition_state();
    auto file_state = acq::default_file_state();

    // if ctb, set to default and restore after test
    std::optional<acq::CTBState> ctb_state = std::nullopt;
    if (detType == defs::CHIPTESTBOARD ||
        detType == defs::XILINX_CHIPTESTBOARD) {
        ctb_state = std::make_optional(acq::default_ctb_state(detType));
    }
    acq::CTBStateGuard ctb_guard(det, ctb_state);

    // binary => /tmp/sls_test_master_0.json
    file_state.file_format = defs::BINARY;
    acq::run(det, acq_state, file_state);

    std::string fname = acq::get_master_file_name(file_state);
    mf::Checker<mf::JsonContext> checker(fname);

    // get expected state of parameters and check against master file
    acq::ExpectedState expected_state =
        acq::build_expected_state(det, acq_state, file_state, ctb_state);
    checks::check_metadata(checker, expected_state);

#ifdef HDF5C
    try {
        // hdf5 => /tmp/sls_test_master_0.h5
        file_state.file_format = defs::HDF5;
        acq::run(det, acq_state, file_state);

        std::string fname = acq::get_master_file_name(file_state);
        mf::Checker<mf::H5Context> checker(fname);

        // get expected state of parameters and check against master file
        acq::ExpectedState expected_state =
            acq::build_expected_state(det, acq_state, file_state, ctb_state);
        checks::check_metadata(checker, expected_state);
    } catch (H5::Exception &e) {
        LOG(logERROR) << "HDF5 error: " << e.getDetailMsg();
        throw;
    }
#endif
}

} // namespace sls
