// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once

#include "checks/MasterFileChecks.h"
#include "sls/sls_detector_defs.h"

#include <chrono>
#include <filesystem>
#include <optional>
#include <string>
#include <thread>

namespace sls {

int chipVersionToX10(const std::string &chipVersion);

namespace acq = sls::test::acquire;
namespace mf = sls::test::master_file;
namespace checks = sls::test::checks;

void test_valid_port_caller(const std::string &command,
                            const std::vector<std::string> &arguments,
                            int detector_id, int action);

void test_dac_caller(slsDetectorDefs::dacIndex index,
                     const std::string &dacname, int dacvalue, bool mV = false);
void test_onchip_dac_caller(slsDetectorDefs::dacIndex index,
                            const std::string &dacname, int dacvalue);

/**
 * Helper function to run an acquisition and check the master file (both binary
 * and hdf5) for expected values. The function takes in a lambda that is called
 * with the master filechecker object to perform checks on the master file. The
 * acquisition is run with default acquisition and file states, but these can be
 * modified within the lambda if needed. This version has the master file
 * checker object created within the function instead of using a helper
 * function, to allow for more flexibility in handling exceptions (especially
 * HDF5 exceptions) and logging.
 */
template <typename F>
void test_run_with_master_file_checker(Detector &det, F f) {

    auto acq_state = acq::default_acquisition_state();
    auto file_state = acq::default_file_state();
    std::array<defs::fileFormat, 2> formats = {defs::BINARY, defs::HDF5};

    for (const auto &format : formats) {
        file_state.file_format = format;
        acq::run(det, acq_state, file_state);
        std::string fname = acq::get_master_file_name(file_state);
        if (format == defs::HDF5) {
#ifdef HDF5C
            try {
                mf::Checker<mf::H5Context> checker(fname);
                f(det, acq_state, file_state, checker);
            } catch (H5::Exception &e) {
                LOG(logERROR) << "HDF5 error: " << e.getDetailMsg();
                throw;
            }
#endif
        } else {
            mf::Checker<mf::JsonContext> checker(fname);
            f(det, acq_state, file_state, checker);
        }
    }
}

} // namespace sls
