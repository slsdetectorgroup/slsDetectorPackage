// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "master_file/Context.h"
#include "master_file/ReadersJson.h"
#ifdef HDF5C
#include "master_file/ReadersH5.h"
#endif
#include "acquire/ExpectedState.h"
#include "checks/MasterFileChecks.h"
#include "test-Caller-global.h"

#include "Caller.h"
#include "catch.hpp"
#include "receiver_defs.h"
#include "sls/Detector.h"
#include "sls/ToString.h"
#include "sls/logger.h"
#include "sls/sls_detector_defs.h"
#include "tests/globals.h"

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

/*
inline bool operator==(sls::ns lhs, sls::ns rhs) {
    return lhs.count() == rhs.count();
}
*/

template <typename CheckerT>
void test_master_file_common_metadata(
    const Detector &det, CheckerT &checker,
    const acq::ExpectedState &expected_state) {
    test_master_file_version(det, checker, expected_state);
    test_master_file_type(det, checker, expected_state);
    test_master_file_timing_mode(det, checker, expected_state);
    test_master_file_geometry(det, checker, expected_state);
    test_master_file_image_size(det, checker, expected_state);
    test_master_file_port_shape(det, checker, expected_state);
    test_master_file_frames_per_file(det, checker, expected_state);
    test_master_file_frame_discard_policy(det, checker, expected_state);
    test_master_file_frame_padding(det, checker, expected_state);
    test_master_file_scan_parameters(det, checker, expected_state);
    test_master_file_total_frames(det, checker, expected_state);
    test_master_file_json_header(det, checker, expected_state);
    // TODO: test frame header format?
}

template <typename CheckerT>
void test_master_file_jungfrau_metadata(
    const Detector &det, CheckerT &checker,
    const acq::ExpectedState &expected_state) {
    REQUIRE_NOTHROW(test_master_file_rois(det, checker, expected_state));
    REQUIRE_NOTHROW(test_master_file_exptime(det, checker, expected_state));
    REQUIRE_NOTHROW(test_master_file_period(det, checker, expected_state));
    REQUIRE_NOTHROW(
        test_master_file_num_udp_interfaces(det, checker, expected_state));
    REQUIRE_NOTHROW(test_master_file_read_n_rows(det, checker, expected_state));
    REQUIRE_NOTHROW(
        test_master_file_readout_speed(det, checker, expected_state));
}

template <typename CheckerT>
void test_master_file_moench_metadata(
    const Detector &det, CheckerT &checker,
    const acq::ExpectedState &expected_state) {
    REQUIRE_NOTHROW(test_master_file_rois(det, checker, expected_state));
    REQUIRE_NOTHROW(test_master_file_exptime(det, checker, expected_state));
    REQUIRE_NOTHROW(test_master_file_period(det, checker, expected_state));
    REQUIRE_NOTHROW(
        test_master_file_num_udp_interfaces(det, checker, expected_state));
    REQUIRE_NOTHROW(test_master_file_read_n_rows(det, checker, expected_state));
    REQUIRE_NOTHROW(
        test_master_file_readout_speed(det, checker, expected_state));
}

template <typename CheckerT>
void test_master_file_eiger_metadata(const Detector &det, CheckerT &checker,
                                     const acq::ExpectedState &expected_state) {
    REQUIRE_NOTHROW(test_master_file_rois(det, checker, expected_state));
    REQUIRE_NOTHROW(
        test_master_file_dynamic_range(det, checker, expected_state));
    REQUIRE_NOTHROW(test_master_file_ten_giga(det, checker, expected_state));
    REQUIRE_NOTHROW(test_master_file_exptime(det, checker, expected_state));
    REQUIRE_NOTHROW(test_master_file_period(det, checker, expected_state));
    REQUIRE_NOTHROW(
        test_master_file_threshold_energy(det, checker, expected_state));
    REQUIRE_NOTHROW(test_master_file_sub_exptime(det, checker, expected_state));
    REQUIRE_NOTHROW(test_master_file_sub_period(det, checker, expected_state));
    REQUIRE_NOTHROW(test_master_file_quad(det, checker, expected_state));
    REQUIRE_NOTHROW(test_master_file_read_n_rows(det, checker, expected_state));
    REQUIRE_NOTHROW(
        test_master_file_rate_corrections(det, checker, expected_state));
    REQUIRE_NOTHROW(
        test_master_file_readout_speed(det, checker, expected_state));
}

template <typename CheckerT>
void test_master_file_mythen3_metadata(
    const Detector &det, CheckerT &checker,
    const acq::ExpectedState &expected_state) {
    REQUIRE_NOTHROW(test_master_file_rois(det, checker, expected_state));
    REQUIRE_NOTHROW(
        test_master_file_dynamic_range(det, checker, expected_state));
    REQUIRE_NOTHROW(test_master_file_ten_giga(det, checker, expected_state));
    REQUIRE_NOTHROW(test_master_file_period(det, checker, expected_state));
    REQUIRE_NOTHROW(
        test_master_file_counter_mask(det, checker, expected_state));
    REQUIRE_NOTHROW(test_master_file_exptimes(det, checker, expected_state));
    REQUIRE_NOTHROW(test_master_file_gate_delays(det, checker, expected_state));
    REQUIRE_NOTHROW(test_master_file_gates(det, checker, expected_state));
    REQUIRE_NOTHROW(
        test_master_file_threadhold_energies(det, checker, expected_state));
    REQUIRE_NOTHROW(
        test_master_file_readout_speed(det, checker, expected_state));
}

template <typename CheckerT>
void test_master_file_gotthard2_metadata(
    const Detector &det, CheckerT &checker,
    const acq::ExpectedState &expected_state) {
    REQUIRE_NOTHROW(test_master_file_rois(det, checker, expected_state));
    REQUIRE_NOTHROW(test_master_file_exptime(det, checker, expected_state));
    REQUIRE_NOTHROW(test_master_file_period(det, checker, expected_state));
    REQUIRE_NOTHROW(test_master_file_burst_mode(det, checker, expected_state));
    REQUIRE_NOTHROW(
        test_master_file_readout_speed(det, checker, expected_state));
}

template <typename CheckerT>
void test_master_file_ctb_metadata(const Detector &det, CheckerT &checker,
                                   const acq::ExpectedState &expected_state) {
    REQUIRE_NOTHROW(test_master_file_exptime(det, checker, expected_state));
    REQUIRE_NOTHROW(test_master_file_period(det, checker, expected_state));
    REQUIRE_NOTHROW(
        test_master_file_readout_mode(det, checker, expected_state));
    if (expected_state.common_state.det_type == defs::CHIPTESTBOARD) {
        REQUIRE_NOTHROW(
            test_master_file_ten_giga(det, checker, expected_state));
        REQUIRE_NOTHROW(
            test_master_file_adc_mask_1g(det, checker, expected_state));
    }
    REQUIRE_NOTHROW(
        test_master_file_analog_samples(det, checker, expected_state));
    REQUIRE_NOTHROW(
        test_master_file_digital_samples(det, checker, expected_state));
    REQUIRE_NOTHROW(
        test_master_file_transceiver_samples(det, checker, expected_state));
    REQUIRE_NOTHROW(
        test_master_file_adc_mask_10g(det, checker, expected_state));
    REQUIRE_NOTHROW(test_master_file_dbit_offset(det, checker, expected_state));
    REQUIRE_NOTHROW(test_master_file_dbit_bitset(det, checker, expected_state));
    REQUIRE_NOTHROW(
        test_master_file_dbit_reorder(det, checker, expected_state));
    REQUIRE_NOTHROW(
        test_master_file_transceiver_mask(det, checker, expected_state));
    REQUIRE_NOTHROW(
        test_master_file_transceiver_flag(det, checker, expected_state));
}

template <typename CheckerT>
void test_master_file_metadata(const Detector &det, CheckerT &checker,
                               const acq::ExpectedState &expected_state) {
    // test_master_file_frames_in_file(checker, num_frames);

    test_master_file_common_metadata(det, checker, expected_state);
    switch (expected_state.common_state.det_type) {
    case defs::JUNGFRAU:
        test_master_file_jungfrau_metadata(det, checker, expected_state);
        break;
    case defs::EIGER:
        test_master_file_eiger_metadata(det, checker, expected_state);
        break;
    case defs::MOENCH:
        test_master_file_moench_metadata(det, checker, expected_state);
        break;
    case defs::MYTHEN3:
        test_master_file_mythen3_metadata(det, checker, expected_state);
        break;
    case defs::GOTTHARD2:
        test_master_file_gotthard2_metadata(det, checker, expected_state);
        break;
    case defs::CHIPTESTBOARD:
    case defs::XILINX_CHIPTESTBOARD:
        test_master_file_ctb_metadata(det, checker, expected_state);
        break;
    default:
        break;
    }
}

rapidjson::Document parse_binary_master_attributes(std::string file_path) {
    REQUIRE(std::filesystem::exists(file_path) == true);
    std::ifstream file(file_path);
    REQUIRE(file.is_open());
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string json_str = buffer.str();

    rapidjson::Document doc;
    rapidjson::ParseResult result = doc.Parse(json_str.c_str());
    if (!result) {
        std::cout << "JSON parse error: " << GetParseError_En(result.Code())
                  << " (at offset " << result.Offset() << ")" << std::endl;

        // Optional: Show problematic snippet
        size_t offset = result.Offset();
        std::string context =
            json_str.substr(std::max(0, (int)offset - 20), 40);
        std::cout << "Context around error: \"" << context << "\"" << std::endl;
    }
    REQUIRE(result);
    return doc;
}

TEST_CASE("check_master_file_attributes",
          "[.detectorintegration][.disable_check_data_file]") {

    Detector det;
    // currently num frame = 1 (default)
    auto acq_state = acq::default_acquisition_state();
    auto file_state = acq::default_file_state();

    // binary => /tmp/sls_test_master_0.json
    file_state.file_format = defs::BINARY;
    acq::run(det, acq_state, file_state);
    // get json context
    std::string fname = acq::get_master_file_name(file_state);
    auto doc = parse_binary_master_attributes(fname);
    mf::Checker<mf::JsonContext> checker(mf::JsonContext{doc});
    // get expected state of parameters and check against master file
    acq::ExpectedState expected_state =
        acq::build_expected_state(det, acq_state, file_state);
    checks::check_metadata(checker, expected_state);

#ifdef HDF5C
    try {
        // hdf5 => /tmp/sls_test_master_0.h5
        file_state.file_format = defs::HDF5;
        acq::run(det, acq_state, file_state);
        // get h5 context
        std::string fname = acq::get_master_file_name(file_state);
        mf::Checker<mf::H5Context> checker(mf::H5Context{fname});
        acq::ExpectedState expected_state =
            acq::build_expected_state(det, acq_state, file_state);
        checks::check_metadata(checker, expected_state);
    } catch (H5::Exception &e) {
        LOG(logERROR) << "HDF5 error: " << e.getDetailMsg();
        throw;
    }
#endif
}

} // namespace sls
