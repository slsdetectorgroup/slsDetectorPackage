// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package

#pragma once

#include "Acquire.h"
#include "CTBState.h"
#include "FileState.h"

#include <variant>

namespace sls::test::acquire {

struct CommonExpectedState {
    defs::detectorType det_type{};
    defs::timingMode timing_mode{};
    defs::xy geometry{};
    int image_size{};
    defs::xy port_shape{};
    int max_frames_per_file{};
    defs::frameDiscardPolicy frame_discard_policy{};
    bool partial_frames_padding{};
    defs::scanParameters scan_parameters{};
    uint64_t total_frames{};
    uint64_t frames_in_file{};
    std::map<std::string, std::string> additional_json_header{};
};

struct JungfrauExpectedState {
    std::vector<defs::ROI> rois{};
    ns exptime{};
    ns period{};
    int num_udp_interfaces{};
    int read_n_rows{};
    defs::speedLevel readout_speed{};
};

struct MoenchExpectedState {
    std::vector<defs::ROI> rois{};
    ns exptime{};
    ns period{};
    int num_udp_interfaces{};
    int read_n_rows{};
    defs::speedLevel readout_speed{};
};

struct EigerExpectedState {
    std::vector<defs::ROI> rois{};
    int dynamic_range{};
    bool ten_giga{};
    ns exptime{};
    ns period{};
    int threshold_energy{};
    ns sub_exptime{};
    ns sub_period{};
    bool quad{};
    int read_n_rows{};
    std::vector<int64_t> rate_corrections{};
    defs::speedLevel readout_speed{};
};

struct Mythen3ExpectedState {
    std::vector<defs::ROI> rois{};
    int dynamic_range{};
    bool ten_giga{};
    ns period{};
    int counter_mask{};
    std::array<ns, 3> exp_times{};
    std::array<ns, 3> gate_delays{};
    int num_gates{};
    std::array<int, 3> threshold_energies{};
    defs::speedLevel readout_speed{};
};

struct Gotthard2ExpectedState {
    std::vector<defs::ROI> rois{};
    ns exptime{};
    ns period{};
    defs::burstMode burst_mode{};
    defs::speedLevel readout_speed{};
};

struct CTBExpectedState {
    ns exptime{};
    ns period{};
    CTBState ctb_acq_state{};
};

using DetectorSpecificState =
    std::variant<JungfrauExpectedState, MoenchExpectedState, EigerExpectedState,
                 Mythen3ExpectedState, Gotthard2ExpectedState,
                 CTBExpectedState>;

struct ExpectedState {
    FileState file_state{};
    AcquisitionState acquisition_state{};
    CommonExpectedState common_state{};
    DetectorSpecificState detector_specific_state{};
};

template <typename T>
const T &get_detector_specific_state(const ExpectedState &expected_state) {
    return std::get<T>(expected_state.detector_specific_state);
}

template <typename T> bool is_type(const ExpectedState &e) {
    return std::holds_alternative<T>(e.detector_specific_state);
}

ExpectedState build_expected_state(
    const Detector &det,
    const AcquisitionState &acq_state = default_acquisition_state(),
    const FileState &file_state = default_file_state(),
    std::optional<CTBState> ctb_state = std::nullopt);

int get_expected_image_size(const Detector &det,
                            std::optional<CTBState> ctb_state = std::nullopt);

} // namespace sls::test::acquire