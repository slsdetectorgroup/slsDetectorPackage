// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once

#include "acquire/ExpectedState.h"
#include "master_file/Checker.h"

#include "MasterAttributes.h"

namespace sls::test::checks {

namespace acq = sls::test::acquire;
namespace mf = sls::test::master_file;

inline bool operator==(sls::ns lhs, sls::ns rhs) {
    return lhs.count() == rhs.count();
}

// different values based on file format
template <typename CheckerT> void check_version(CheckerT &checker) {
#ifdef HDF5C
    if constexpr (std::is_same_v<CheckerT, mf::Checker<mf::H5Context>>) {
        checker.template check<double>(MasterAttributes::N_VERSION.data(),
                                       HDF5_WRITER_VERSION,
                                       mf::AccessType::Attribute);
    } else
#endif
    {
        checker.template check<double>(MasterAttributes::N_VERSION.data(),
                                       BINARY_WRITER_VERSION);
    }
}

template <typename CheckerT>
void check_detector_type(CheckerT &checker, const defs::detectorType &value) {
    checker.template check<std::string>(
        MasterAttributes::N_DETECTOR_TYPE.data(), ToString(value));
}

template <typename CheckerT>
void check_timing_mode(CheckerT &checker, const defs::timingMode &value) {
    checker.template check<std::string>(MasterAttributes::N_TIMING_MODE.data(),
                                        ToString(value));
}

template <typename CheckerT>
void check_geometry(CheckerT &checker, const defs::xy &value) {
    checker.template check<defs::xy>(MasterAttributes::N_GEOMETRY.data(),
                                     value);
}

template <typename CheckerT>
void check_image_size(CheckerT &checker, const int &value) {
    checker.template check<int>(MasterAttributes::N_IMAGE_SIZE.data(), value);
}

template <typename CheckerT>
void check_port_shape(CheckerT &checker, const defs::xy &value) {
    checker.template check<defs::xy>(MasterAttributes::N_PIXELS.data(), value);
}

template <typename CheckerT>
void check_frames_per_file(CheckerT &checker, const int &value) {
    checker.template check<int>(MasterAttributes::N_MAX_FRAMES_PER_FILE.data(),
                                value);
}

template <typename CheckerT>
void check_frame_discard_policy(CheckerT &checker,
                                const defs::frameDiscardPolicy &value) {
    checker.template check<std::string>(
        MasterAttributes::N_FRAME_DISCARD_POLICY.data(), ToString(value));
}

template <typename CheckerT>
void check_partial_frames_padding(CheckerT &checker, const bool &value) {
    checker.template check<int>(MasterAttributes::N_FRAME_PADDING.data(),
                                static_cast<int>(value));
}

template <typename CheckerT>
void check_scan_parameters(CheckerT &checker,
                           const defs::scanParameters &value) {
    checker.template check<defs::scanParameters>(
        MasterAttributes::N_SCAN_PARAMETERS.data(), value);
}

template <typename CheckerT>
void check_total_frames(CheckerT &checker, const uint64_t &value) {
    checker.template check<uint64_t>(MasterAttributes::N_TOTAL_FRAMES.data(),
                                     value);
}

template <typename CheckerT>
void check_frames_in_file(CheckerT &checker, const uint64_t &value) {
    checker.template check<uint64_t>(MasterAttributes::N_FRAMES_IN_FILE.data(),
                                     value);
}

template <typename CheckerT>
void check_additional_json_header(
    CheckerT &checker, const std::map<std::string, std::string> &value) {
    checker.template check<std::map<std::string, std::string>>(
        MasterAttributes::N_ADDITIONAL_JSON_HEADER.data(), value);
}

template <typename CheckerT>
void check_rois(CheckerT &checker, const std::vector<defs::ROI> &value) {
    checker.template check<std::vector<defs::ROI>>(
        MasterAttributes::N_RECEIVER_ROIS.data(), value);
}

template <typename CheckerT>
void check_exptime(CheckerT &checker, const ns &value) {
    checker.template check<std::string>(
        MasterAttributes::N_EXPOSURE_TIME.data(), ToString(value));
}

template <typename CheckerT>
void check_period(CheckerT &checker, const ns &value) {
    checker.template check<std::string>(
        MasterAttributes::N_ACQUISITION_PERIOD.data(), ToString(value));
}

template <typename CheckerT>
void check_num_udp_interfaces(CheckerT &checker, const int &value) {
    checker.template check<int>(MasterAttributes::N_NUM_UDP_INTERFACES.data(),
                                value);
}

template <typename CheckerT>
void check_read_n_rows(CheckerT &checker, const int &value) {
    checker.template check<int>(MasterAttributes::N_NUMBER_OF_ROWS.data(),
                                value);
}

template <typename CheckerT>
void check_readout_speed(CheckerT &checker, const defs::speedLevel &value) {
    checker.template check<std::string>(
        MasterAttributes::N_READOUT_SPEED.data(), ToString(value));
}

template <typename CheckerT>
void check_dynamic_range(CheckerT &checker, const int &value) {
    checker.template check<int>(MasterAttributes::N_DYNAMIC_RANGE.data(),
                                value);
}

template <typename CheckerT>
void check_ten_giga(CheckerT &checker, const bool &value) {
    checker.template check<int>(MasterAttributes::N_TEN_GIGA.data(),
                                static_cast<int>(value));
}

template <typename CheckerT>
void check_threshold_energy(CheckerT &checker, const int &value) {
    checker.template check<int>(MasterAttributes::N_THRESHOLD_ENERGY.data(),
                                value);
}

template <typename CheckerT>
void check_sub_exptime(CheckerT &checker, const ns &value) {
    checker.template check<std::string>(
        MasterAttributes::N_SUB_EXPOSURE_TIME.data(), ToString(value));
}

template <typename CheckerT>
void check_sub_period(CheckerT &checker, const ns &value) {
    checker.template check<std::string>(
        MasterAttributes::N_SUB_ACQUISITION_PERIOD.data(), ToString(value));
}

template <typename CheckerT>
void check_quad(CheckerT &checker, const bool &value) {
    checker.template check<int>(MasterAttributes::N_QUAD.data(),
                                static_cast<int>(value));
}

template <typename CheckerT>
void check_rate_corrections(CheckerT &checker,
                            const std::vector<int64_t> &value) {
    checker.template check<std::vector<int64_t>>(
        MasterAttributes::N_RATE_CORRECTIONS.data(), value);
}

template <typename CheckerT>
void check_counter_mask(CheckerT &checker, const int &value) {
    checker.template check<int>(MasterAttributes::N_COUNTER_MASK.data(), value);
}

template <typename CheckerT>
void check_exptime_array(CheckerT &checker,
                         const std::array<sls::ns, 3UL> &value) {
    checker.template check<std::array<sls::ns, 3UL>>(
        MasterAttributes::N_EXPOSURE_TIMES.data(), value);
}

template <typename CheckerT>
void check_gate_delay_array(CheckerT &checker,
                            const std::array<sls::ns, 3UL> &value) {
    checker.template check<std::array<sls::ns, 3UL>>(
        MasterAttributes::N_GATE_DELAYS.data(), value);
}

template <typename CheckerT>
void check_gates(CheckerT &checker, const int &value) {
    checker.template check<int>(MasterAttributes::N_GATES.data(), value);
}

template <typename CheckerT>
void check_threshold_energies(CheckerT &checker,
                              const std::array<int, 3UL> &value) {
    checker.template check<std::array<int, 3UL>>(
        MasterAttributes::N_THRESHOLD_ENERGIES.data(), value);
}

template <typename CheckerT>
void check_burst_mode(CheckerT &checker, const defs::burstMode &value) {
    checker.template check<std::string>(MasterAttributes::N_BURST_MODE.data(),
                                        ToString(value));
}

template <typename CheckerT>
void check_readout_mode(CheckerT &checker, const defs::readoutMode &value) {
    auto analog = static_cast<int>(
        (value == defs::ANALOG_ONLY || value == defs::ANALOG_AND_DIGITAL));
    auto digital = static_cast<int>(value == defs::DIGITAL_ONLY ||
                                    value == defs::ANALOG_AND_DIGITAL ||
                                    value == defs::DIGITAL_AND_TRANSCEIVER);
    auto trans = static_cast<int>(value == defs::DIGITAL_AND_TRANSCEIVER ||
                                  value == defs::TRANSCEIVER_ONLY);

    if (analog)
        checker.template check<int>(MasterAttributes::N_ANALOG.data(),
                                    static_cast<int>(analog));
    if (digital)
        checker.template check<int>(MasterAttributes::N_DIGITAL.data(),
                                    static_cast<int>(digital));
    if (trans)
        checker.template check<int>(MasterAttributes::N_TRANSCEIVER.data(),
                                    static_cast<int>(trans));
}

template <typename CheckerT>
void check_analog_samples(CheckerT &checker, const int &value) {
    checker.template check<int>(MasterAttributes::N_ANALOG_SAMPLES.data(),
                                value);
}

template <typename CheckerT>
void check_digital_samples(CheckerT &checker, const int &value) {
    checker.template check<int>(MasterAttributes::N_DIGITAL_SAMPLES.data(),
                                value);
}

template <typename CheckerT>
void check_transceiver_samples(CheckerT &checker, const int &value) {
    checker.template check<int>(MasterAttributes::N_TRANSCEIVER_SAMPLES.data(),
                                value);
}

template <typename CheckerT>
void check_adc_mask(CheckerT &checker, const uint32_t &value) {
    checker.template check<uint32_t>(MasterAttributes::N_ADC_MASK.data(),
                                     value);
}

template <typename CheckerT>
void check_dbit_offset(CheckerT &checker, const int &value) {
    checker.template check<int>(MasterAttributes::N_DBIT_OFFSET.data(), value);
}

template <typename CheckerT>
void check_dbit_list(CheckerT &checker, const std::vector<int> &value) {
    uint64_t dbit_bitset = 0;
    for (auto &i : value) {
        dbit_bitset |= (static_cast<uint64_t>(1) << i);
    }
    checker.template check<uint64_t>(MasterAttributes::N_DBIT_BITSET.data(),
                                     dbit_bitset);
}

template <typename CheckerT>
void check_dbit_reorder(CheckerT &checker, const int &value) {
    checker.template check<int>(MasterAttributes::N_DBIT_REORDER.data(), value);
}

template <typename CheckerT>
void check_transceiver_mask(CheckerT &checker, const uint32_t &value) {
    checker.template check<uint32_t>(
        MasterAttributes::N_TRANSCEIVER_MASK.data(), value);
}

template <typename CheckerT>
void check_common_metadata(CheckerT &checker,
                           const acq::ExpectedState &expected) {
    check_version(checker);
    auto st = expected.common_state;
    check_detector_type(checker, st.det_type);
    check_geometry(checker, st.geometry);
    check_image_size(checker, st.image_size);
    check_port_shape(checker, st.port_shape);
    check_frames_per_file(checker, st.max_frames_per_file);
    check_frame_discard_policy(checker, st.frame_discard_policy);
    check_partial_frames_padding(checker, st.partial_frames_padding);
    check_scan_parameters(checker, st.scan_parameters);
    check_total_frames(checker, st.total_frames);
    check_frames_in_file(checker, st.frames_in_file);
    check_additional_json_header(checker, st.additional_json_header);
}

template <typename CheckerT>
void check_jungfrau_metadata(CheckerT &checker,
                             const acq::ExpectedState &expected) {
    const auto &st =
        std::get<acq::JungfrauExpectedState>(expected.detector_specific_state);
    check_rois(checker, st.rois);
    check_exptime(checker, st.exptime);
    check_period(checker, st.period);
    check_num_udp_interfaces(checker, st.num_udp_interfaces);
    check_read_n_rows(checker, st.read_n_rows);
    check_readout_speed(checker, st.readout_speed);
}

template <typename CheckerT>
void check_moench_metadata(CheckerT &checker,
                           const acq::ExpectedState &expected) {
    const auto &st =
        std::get<acq::MoenchExpectedState>(expected.detector_specific_state);
    check_rois(checker, st.rois);
    check_exptime(checker, st.exptime);
    check_period(checker, st.period);
    check_num_udp_interfaces(checker, st.num_udp_interfaces);
    check_read_n_rows(checker, st.read_n_rows);
    check_readout_speed(checker, st.readout_speed);
}

template <typename CheckerT>
void check_eiger_metadata(CheckerT &checker,
                          const acq::ExpectedState &expected) {
    const auto &st =
        std::get<acq::EigerExpectedState>(expected.detector_specific_state);
    check_rois(checker, st.rois);
    check_dynamic_range(checker, st.dynamic_range);
    check_ten_giga(checker, st.ten_giga);
    check_exptime(checker, st.exptime);
    check_period(checker, st.period);
    check_threshold_energy(checker, st.threshold_energy);
    check_sub_exptime(checker, st.sub_exptime);
    check_sub_period(checker, st.sub_period);
    check_quad(checker, st.quad);
    check_read_n_rows(checker, st.read_n_rows);
    check_rate_corrections(checker, st.rate_corrections);
    check_readout_speed(checker, st.readout_speed);
}

template <typename CheckerT>
void check_mythen3_metadata(CheckerT &checker,
                            const acq::ExpectedState &expected) {
    const auto &st =
        std::get<acq::Mythen3ExpectedState>(expected.detector_specific_state);
    check_rois(checker, st.rois);
    check_dynamic_range(checker, st.dynamic_range);
    check_ten_giga(checker, st.ten_giga);
    check_period(checker, st.period);
    check_counter_mask(checker, st.counter_mask);
    check_exptime_array(checker, st.exp_times);
    check_gate_delay_array(checker, st.gate_delays);
    check_gates(checker, st.num_gates);
    check_threshold_energies(checker, st.threshold_energies);
    check_readout_speed(checker, st.readout_speed);
}

template <typename CheckerT>
void check_gotthard2_metadata(CheckerT &checker,
                              const acq::ExpectedState &expected) {
    const auto &st =
        std::get<acq::Gotthard2ExpectedState>(expected.detector_specific_state);
    check_rois(checker, st.rois);
    check_exptime(checker, st.exptime);
    check_period(checker, st.period);
    check_burst_mode(checker, st.burst_mode);
    check_readout_speed(checker, st.readout_speed);
}

template <typename CheckerT>
void check_ctb_metadata(CheckerT &checker, const acq::ExpectedState &expected) {
    const auto &st =
        std::get<acq::CTBExpectedState>(expected.detector_specific_state);
    check_exptime(checker, st.exptime);
    check_period(checker, st.period);

    // 10g for xilinx, 1g/10g for altera ctb
    bool ten_giga = true;
    if (expected.common_state.det_type == defs::CHIPTESTBOARD) {
        check_ten_giga(checker, st.ctb_acq_state.ten_giga);
        if (!st.ctb_acq_state.ten_giga) {
            ten_giga = false;
        }
    }
    if (ten_giga)
        check_adc_mask(checker, st.ctb_acq_state.adc_enable_10g);
    else
        check_adc_mask(checker, st.ctb_acq_state.adc_enable_1g);

    check_readout_mode(checker, st.ctb_acq_state.readout_mode);
    check_analog_samples(checker, st.ctb_acq_state.num_adc_samples);
    check_digital_samples(checker, st.ctb_acq_state.num_dbit_samples);
    check_transceiver_samples(checker, st.ctb_acq_state.num_trans_samples);
    check_dbit_offset(checker, st.ctb_acq_state.dbit_offset);
    check_dbit_list(checker, st.ctb_acq_state.dbit_list);
    check_dbit_reorder(checker, st.ctb_acq_state.dbit_reorder);
    check_transceiver_mask(checker, st.ctb_acq_state.transceiver_mask);
}

template <typename CheckerT>
void check_metadata(CheckerT &checker, const acq::ExpectedState &expected) {
    check_common_metadata(checker, expected);
    switch (expected.common_state.det_type) {
    case defs::JUNGFRAU:
        check_jungfrau_metadata(checker, expected);
        break;
    case defs::MOENCH:
        check_moench_metadata(checker, expected);
        break;
    case defs::EIGER:
        check_eiger_metadata(checker, expected);
        break;
    case defs::MYTHEN3:
        check_mythen3_metadata(checker, expected);
        break;
    case defs::GOTTHARD2:
        check_gotthard2_metadata(checker, expected);
        break;
    case defs::CHIPTESTBOARD:
    case defs::XILINX_CHIPTESTBOARD:
        check_ctb_metadata(checker, expected);
        break;
    default:
        throw std::runtime_error(
            "Unsupported detector type for metadata checks");
    }
}

inline void
check_binary_file_size(const int expected_image_size, const int64_t num_frames,
                       const acq::FileState &st = acq::default_file_state()) {
    assert(st.file_format == defs::BINARY);
    auto fname = acq::get_first_port_first_file_name(st);
    auto expected_file_size =
        num_frames * (expected_image_size + sizeof(defs::sls_receiver_header));
    auto actual_file_size = std::filesystem::file_size(fname);
    REQUIRE(actual_file_size == expected_file_size);
}

} // namespace sls::test::checks