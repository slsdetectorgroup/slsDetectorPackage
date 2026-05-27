// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once

#include "acquire/ExpectedState.h"
#include "master_file/Checker.h"

#include "MasterAttributes.h"

namespace sls::test::checks {

namespace acq = sls::test::acquire;
namespace mf = sls::test::master_file;

// different values based on file format
template <typename CheckerT>
void check_version(CheckerT &checker) {
#ifdef HDF5C
    if constexpr(std::is_same_v<CheckerT, mf::Checker<mf::H5Context>>) {
        checker.template check<double>(MasterAttributes::N_VERSION.data(), HDF5_WRITER_VERSION, mf::AccessType::Attribute);
    } else
#endif
    {
        checker.template check<double>(MasterAttributes::N_VERSION.data(), BINARY_WRITER_VERSION);
    }
}

template <typename CheckerT>
void check_detector_type(CheckerT &checker, const acq::ExpectedState& expected) {
    checker.template check<std::string>(MasterAttributes::N_DETECTOR_TYPE.data(), ToString(expected.common_state.det_type));
}

template <typename CheckerT>
void check_timing_mode(CheckerT &checker, const acq::ExpectedState& expected) {
    checker.template check<std::string>(MasterAttributes::N_TIMING_MODE.data(), ToString(expected.common_state.timing_mode));
}

template <typename CheckerT>
void check_geometry(CheckerT &checker, const acq::ExpectedState& expected) {
    checker.template check<defs::xy>(MasterAttributes::N_GEOMETRY.data(), expected.common_state.geometry);
}

template <typename CheckerT>
void check_image_size(CheckerT &checker, const acq::ExpectedState& expected) {
    checker.template check<int>(MasterAttributes::N_IMAGE_SIZE.data(), expected.common_state.image_size);
}

template <typename CheckerT>
void check_port_shape(CheckerT &checker, const acq::ExpectedState& expected) {
    checker.template check<defs::xy>(MasterAttributes::N_PIXELS.data(), expected.common_state.port_shape);
}

template <typename CheckerT>
void check_frames_per_file(CheckerT &checker, const acq::ExpectedState& expected) {
    checker.template check<int>(MasterAttributes::N_MAX_FRAMES_PER_FILE.data(), expected.common_state.max_frames_per_file);
}

template <typename CheckerT>
void check_frame_discard_policy(CheckerT &checker, const acq::ExpectedState& expected) {
    checker.template check<std::string>(MasterAttributes::N_FRAME_DISCARD_POLICY.data(), ToString(expected.common_state.frame_discard_policy));
}

template <typename CheckerT>
void check_partial_frames_padding(CheckerT &checker, const acq::ExpectedState& expected) {
    checker.template check<int>(MasterAttributes::N_FRAME_PADDING.data(), expected.common_state.partial_frames_padding);
}   

template <typename CheckerT>
void check_scan_parameters(CheckerT &checker, const acq::ExpectedState& expected) {
    checker.template check<defs::scanParameters>(MasterAttributes::N_SCAN_PARAMETERS.data(), expected.common_state.scan_parameters);
}

template <typename CheckerT>
void check_total_frames(CheckerT &checker, const acq::ExpectedState& expected) {
    checker.template check<uint64_t>(MasterAttributes::N_TOTAL_FRAMES.data(), expected.common_state.total_frames);
}

template <typename CheckerT>
void check_frames_in_file(CheckerT &checker, const acq::ExpectedState& expected) {
    checker.template check<uint64_t>(MasterAttributes::N_FRAMES_IN_FILE.data(), expected.common_state.frames_in_file);
}

template <typename CheckerT>
void check_additional_json_header(CheckerT &checker, const acq::ExpectedState& expected) {
    checker.template check<std::map<std::string, std::string>>(MasterAttributes::N_ADDITIONAL_JSON_HEADER.data(), expected.common_state.additional_json_header);
}

template <typename CheckerT>
void check_rois(CheckerT &checker, const acq::ExpectedState& expected) {
    checker.template check<std::vector<defs::ROI>>(MasterAttributes::N_RECEIVER_ROIS.data(), expected.detector_specific_state.rois);
}

template <typename CheckerT>
void check_exptime(CheckerT &checker, const acq::ExpectedState& expected) {
    checker.template check<std::string>(MasterAttributes::N_EXPOSURE_TIME.data(), ToString(expected.detector_specific_state.exptime));
}

template <typename CheckerT>
void check_period(CheckerT &checker, const acq::ExpectedState& expected) {
    checker.template check<std::string>(MasterAttributes::N_ACQUISITION_PERIOD.data(), ToString(expected.detector_specific_state.period));
}

template <typename CheckerT>
void check_num_udp_interfaces(CheckerT &checker, const acq::ExpectedState& expected) {
    checker.template check<int>(MasterAttributes::N_NUM_UDP_INTERFACES.data(), expected.detector_specific_state.num_udp_interfaces);
}

template <typename CheckerT>
void check_read_n_rows(CheckerT &checker, const acq::ExpectedState& expected) {
    checker.template check<int>(MasterAttributes::N_NUMBER_OF_ROWS.data(), expected.detector_specific_state.read_n_rows);
}

template <typename CheckerT>
void check_readout_speed(CheckerT &checker, const acq::ExpectedState& expected) {
    checker.template check<std::string>(MasterAttributes::N_READOUT_SPEED.data(), ToString(expected.detector_specific_state.readout_speed));
}

template <typename CheckerT>
void check_dynamic_range(CheckerT &checker, const acq::ExpectedState& expected) {
    checker.template check<int>(MasterAttributes::N_DYNAMIC_RANGE.data(), expected.detector_specific_state.dynamic_range);
}

template <typename CheckerT>
void check_ten_giga(CheckerT &checker, const acq::ExpectedState& expected) {
    checker.template check<int>(MasterAttributes::N_TEN_GIGA.data(), static_cast<int>(expected.detector_specific_state.ten_giga));
}

template <typename CheckerT>
void check_threshold_energy(CheckerT &checker, const acq::ExpectedState& expected) {
    checker.template check<int>(MasterAttributes::N_THRESHOLD_ENERGY.data(), expected.detector_specific_state.threshold_energy);
}

template <typename CheckerT>
void check_sub_exptime(CheckerT &checker, const acq::ExpectedState& expected) {
    checker.template check<std::string>(MasterAttributes::N_SUB_EXPOSURE_TIME.data(), ToString(expected.detector_specific_state.sub_exptime));
}

template <typename CheckerT>
void check_sub_period(CheckerT &checker, const acq::ExpectedState& expected) {
    checker.template check<std::string>(MasterAttributes::N_SUB_ACQUISITION_PERIOD.data(), ToString(expected.detector_specific_state.sub_period));
}

template <typename CheckerT>
void check_quad(CheckerT &checker, const acq::ExpectedState& expected) {
    checker.template check<int>(MasterAttributes::N_QUAD.data(), static_cast<int>(expected.detector_specific_state.quad));
}

template <typename CheckerT>
void check_rate_corrections(CheckerT &checker, const acq::ExpectedState& expected) {
    checker.template check<std::vector<int64_t>>(MasterAttributes::N_RATE_CORRECTIONS.data(), expected.detector_specific_state.rate_corrections);
}

template <typename CheckerT>
void check_counter_mask(CheckerT &checker, const acq::ExpectedState& expected) {
    checker.template check<int>(MasterAttributes::N_COUNTER_MASK.data(), expected.detector_specific_state.counter_mask);
}

template <typename CheckerT>
void check_exptime_array(CheckerT &checker, const acq::ExpectedState& expected) {
    checker.template check<std::array<sls::ns, 3UL>>(MasterAttributes::N_EXPOSURE_TIMES.data(), expected.detector_specific_state.exp_times);
}

template <typename CheckerT>
void check_gate_delay_array(CheckerT &checker, const acq::ExpectedState& expected) {
    checker.template check<std::array<sls::ns, 3UL>>(MasterAttributes::N_GATE_DELAYS.data(), expected.detector_specific_state.gate_delays);
}

template <typename CheckerT>
void check_gates(CheckerT &checker, const acq::ExpectedState& expected) {
    checker.template check<int>(MasterAttributes::N_GATES.data(), expected.detector_specific_state.num_gates);
}

template <typename CheckerT>
void check_threshold_energies(CheckerT &checker, const acq::ExpectedState& expected) {
    checker.template check<std::array<int, 3UL>>(MasterAttributes::N_THRESHOLD_ENERGIES.data(), expected.detector_specific_state.thresholdAllEnergyeV);
}

template <typename CheckerT>
void check_burst_mode(CheckerT &checker, const acq::ExpectedState& expected) {
    checker.template check<std::string>(MasterAttributes::N_BURST_MODE.data(), ToString(expected.detector_specific_state.burst_mode));
}
/*

template <typename CheckerT>
void test_master_file_adc_mask(const Detector &det, CheckerT &checker) {
    acq::CTBState test_ctb_config = acq::default_ctb_state();
    auto adc_mask = test_ctb_config.adc_enable_10g;
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::CHIPTESTBOARD) {
        auto tengiga = test_ctb_config.ten_giga;
        if (!tengiga)
            adc_mask = test_ctb_config.adc_enable_1g;
    }

    REQUIRE_NOTHROW(checker.template check<uint32_t>(
        MasterAttributes::N_ADC_MASK.data(), adc_mask));
}

template <typename CheckerT>
void test_master_file_analog_flag(const Detector &det, CheckerT &checker) {
    acq::CTBState test_info = acq::default_ctb_state();
    auto romode = test_info.readout_mode;
    auto analog = static_cast<int>(
        (romode == defs::ANALOG_ONLY || romode == defs::ANALOG_AND_DIGITAL));

    REQUIRE_NOTHROW(
        checker.template check<int>(MasterAttributes::N_ANALOG.data(), analog));
}

template <typename CheckerT>
void test_master_file_analog_samples(const Detector &det, CheckerT &checker) {
    acq::CTBState test_info = acq::default_ctb_state();
    auto analog_samples = test_info.num_adc_samples;

    REQUIRE_NOTHROW(checker.template check<int>(
        MasterAttributes::N_ANALOG_SAMPLES.data(), analog_samples));
}

template <typename CheckerT>
void test_master_file_digital_flag(const Detector &det, CheckerT &checker) {
    acq::CTBState test_info = acq::default_ctb_state();
    auto romode = test_info.readout_mode;
    auto digital = static_cast<int>(romode == defs::DIGITAL_ONLY ||
                                    romode == defs::ANALOG_AND_DIGITAL ||
                                    romode == defs::DIGITAL_AND_TRANSCEIVER);

    REQUIRE_NOTHROW(checker.template check<int>(
        MasterAttributes::N_DIGITAL.data(), digital));
}

template <typename CheckerT>
void test_master_file_digital_samples(const Detector &det, CheckerT &checker) {
    acq::CTBState test_info = acq::default_ctb_state();
    auto digital_samples = test_info.num_dbit_samples;

    REQUIRE_NOTHROW(checker.template check<int>(
        MasterAttributes::N_DIGITAL_SAMPLES.data(), digital_samples));
}

template <typename CheckerT>
void test_master_file_dbit_offset(const Detector &det, CheckerT &checker) {
    acq::CTBState test_info = acq::default_ctb_state();
    auto dbit_offset = test_info.dbit_offset;

    REQUIRE_NOTHROW(checker.template check<int>(
        MasterAttributes::N_DBIT_OFFSET.data(), dbit_offset));
}

template <typename CheckerT>
void test_master_file_dbit_reorder(const Detector &det, CheckerT &checker) {
    acq::CTBState test_info 

template <typename CheckerT>
void test_master_file_timing_mode(const Detector &det, CheckerT &checker) {
    auto timing_mode = det.getTimingMode().tsquash("Inconsistent timing mode");
    REQUIRE_NOTHROW(checker.template check<std::string>(
        MasterAttributes::N_TIMING_MODE.data(), ToString(timing_mode)));
}

template <typename CheckerT>
void test_master_file_geometry(const Detector &det, CheckerT &checker) {
    auto modGeometry = det.getModuleGeometry();
    auto portperModGeometry = det.getPortPerModuleGeometry();
    auto geometry = defs::xy{modGeometry.x * portperModGeometry.x,
                             modGeometry.y * portperModGeometry.y};
    REQUIRE_NOTHROW(checker.template check<defs::xy>(
        MasterAttributes::N_GEOMETRY.data(), geometry));
}

template <typename CheckerT>
void test_master_file_image_size(const Detector &det, CheckerT &checker) {

    auto det_type =
        det.getDetectorType().tsquash("Inconsistent detector types to test");
    int bytes_per_pixel = det.getDynamicRange().squash() / 8;
    detParameters par(det_type);

    int image_size = 0;
    switch (det_type) {

    case defs::EIGER: {
        int num_chips = (par.nChipX / 2);
        image_size = par.nChanX * par.nChanY * num_chips * bytes_per_pixel;
    } break;

    case defs::JUNGFRAU:
    case defs::MOENCH: {
        auto num_udp_interfaces = det.getNumberofUDPInterfaces().tsquash(
            "inconsistent number of udp interfaces");
        image_size = (par.nChanX * par.nChanY * par.nChipX * par.nChipY *
                      bytes_per_pixel) /
                     num_udp_interfaces;
    } break;

    case defs::MYTHEN3: {
        int counter_mask = det.getCounterMask().squash();
        int num_counters = __builtin_popcount(counter_mask);
        int num_channels_per_counter = par.nChanX / MAX_NUM_COUNTERS;
        image_size = num_channels_per_counter * num_counters * par.nChipX *
                     bytes_per_pixel;
    } break;

    case defs::GOTTHARD2: {
        image_size = par.nChanX * par.nChipX * bytes_per_pixel;
    } break;

    case defs::CHIPTESTBOARD:
    case defs::XILINX_CHIPTESTBOARD: {
        acq::CTBState test_info = acq::default_ctb_state();
        image_size = calculate_ctb_image_size(
                         test_info, (det_type == defs::XILINX_CHIPTESTBOARD))
                         .first;
    } break;

    default:
        throw sls::RuntimeError("Unsupported detector type for this test");
    }

    REQUIRE_NOTHROW(checker.template check<int>(
        MasterAttributes::N_IMAGE_SIZE.data(), image_size));
}

template <typename CheckerT>
void test_master_file_det_size(const Detector &det, CheckerT &checker) {

    auto det_type =
        det.getDetectorType().tsquash("Inconsistent detector types to test");
    auto portSize = det.getPortSize()[0];

    // m3 assumes all counters enabled when getting num channels from client
    // TODO: in future, remove assumption
    if (det_type == defs::MYTHEN3) {
        int nchan = portSize.x / MAX_NUM_COUNTERS;
        auto counter_mask = det.getCounterMask().tsquash(
            "Inconsistent counter mask for Mythen3 detector");
        int num_counters = __builtin_popcount(counter_mask);
        portSize.x = nchan * num_counters;
    } else if (det_type == defs::CHIPTESTBOARD ||
               det_type == defs::XILINX_CHIPTESTBOARD) {
        acq::CTBState test_info = acq::default_ctb_state();
        portSize.x = calculate_ctb_image_size(
                         test_info, det_type == defs::XILINX_CHIPTESTBOARD)
                         .second;
        portSize.y = 1;
    }

    REQUIRE_NOTHROW(checker.template check<defs::xy>(
        MasterAttributes::N_PIXELS.data(), portSize));
}

template <typename CheckerT>
void test_master_file_max_frames_per_file(const Detector &det,
                                          CheckerT &checker) {
    auto max_frames_per_file =
        det.getFramesPerFile().tsquash("Inconsistent max frames per file");

    REQUIRE_NOTHROW(checker.template check<int>(
        MasterAttributes::N_MAX_FRAMES_PER_FILE.data(), max_frames_per_file));
}

template <typename CheckerT>
void test_master_file_frame_discard_policy(const Detector &det,
                                           CheckerT &checker) {
    auto policy = det.getRxFrameDiscardPolicy().tsquash(
        "Inconsistent frame discard policy");

    REQUIRE_NOTHROW(checker.template check<std::string>(
        MasterAttributes::N_FRAME_DISCARD_POLICY.data(), ToString(policy)));
}

template <typename CheckerT>
void test_master_file_frame_padding(const Detector &det, CheckerT &checker) {
    auto padding = static_cast<int>(
        det.getPartialFramesPadding().tsquash("Inconsistent frame padding"));

    REQUIRE_NOTHROW(checker.template check<int>(
        MasterAttributes::N_FRAME_PADDING.data(), padding));
}

template <typename CheckerT>
void test_master_file_scan_parameters(const Detector &det, CheckerT &checker) {
    auto scan_params = det.getScan().tsquash("Inconsistent scan parameters");

    REQUIRE_NOTHROW(checker.template check<defs::scanParameters>(
        MasterAttributes::N_SCAN_PARAMETERS.data(), scan_params));
}

template <typename CheckerT>
void test_master_file_total_frames(const Detector &det, CheckerT &checker) {
    uint64_t repeats =
        det.getNumberOfTriggers().tsquash("Inconsistent number of triggers");
    uint64_t numFrames =
        det.getNumberOfFrames().tsquash("Inconsistent number of frames");
    int numAdditionalStorageCells = 0;
    auto det_type =
        det.getDetectorType().tsquash("Inconsistent detector types");
    if (det_type == defs::GOTTHARD2) {
        auto timing_mode =
            det.getTimingMode().tsquash("Inconsistent timing mode");
        auto burst_mode = det.getBurstMode().tsquash("Inconsistent burst mode");
        auto numBursts =
            det.getNumberOfBursts().tsquash("Inconsistent number of bursts");
        if (timing_mode == defs::AUTO_TIMING) {
            // burst mode, repeats = #bursts
            if (burst_mode == defs::BURST_INTERNAL ||
                burst_mode == defs::BURST_EXTERNAL) {
                repeats = numBursts;
            }
            // continuous, repeats = 1 (no trigger as well)
            else {
                repeats = 1;
            }
        } else {
            // trigger
            // continuous, numFrames is limited
            if (burst_mode == defs::CONTINUOUS_INTERNAL ||
                burst_mode == defs::CONTINUOUS_EXTERNAL) {
                numFrames = 1;
            }
        }
    } else if (det_type == defs::JUNGFRAU) {
        numAdditionalStorageCells =
            det.getNumberOfAdditionalStorageCells().tsquash(
                "Inconsistent number of additional storage cells");
    }
    uint64_t total_frames =
        numFrames * repeats * (int64_t)(numAdditionalStorageCells + 1);

    REQUIRE_NOTHROW(checker.template check<uint64_t>(
        MasterAttributes::N_TOTAL_FRAMES.data(), total_frames));
}

template <typename CheckerT>
void test_master_file_rois(const Detector &det, CheckerT &checker) {
    auto rois = det.getRxROI();
    auto detsize = det.getDetectorSize();
    auto det_type =
        det.getDetectorType().tsquash("Inconsistent detector types to test");
    // compensate for m3 channel size and counter mask mess
    if (det_type == defs::MYTHEN3) {
        int nchan = detsize.x / MAX_NUM_COUNTERS;
        auto counter_mask = det.getCounterMask().tsquash(
            "Inconsistent counter mask for Mythen3 detector");
        int num_counters = __builtin_popcount(counter_mask);
        detsize.x = nchan * num_counters;
    }
    // replace -1 for complete ROI
    bool is2D = (detsize.y > 1);
    for (auto &roi : rois) {
        if (roi.completeRoi()) {
            roi.xmin = 0;
            roi.xmax = detsize.x - 1;
            if (is2D) {
                roi.ymin = 0;
                roi.ymax = detsize.y - 1;
            }
        }
    }

    REQUIRE_NOTHROW(checker.template check<std::vector<defs::ROI>>(
        MasterAttributes::N_RECEIVER_ROIS.data(), rois));
}

template <typename CheckerT>
void test_master_file_exptime(const Detector &det, CheckerT &checker) {
    auto exptime = det.getExptime().tsquash("Inconsistent exposure time");

    REQUIRE_NOTHROW(checker.template check<std::string>(
        MasterAttributes::N_EXPOSURE_TIME.data(), ToString(exptime)));
}

template <typename CheckerT>
void test_master_file_period(const Detector &det, CheckerT &checker) {
    auto period = det.getPeriod().tsquash("Inconsistent period");

    REQUIRE_NOTHROW(checker.template check<std::string>(
        MasterAttributes::N_ACQUISITION_PERIOD.data(), ToString(period)));
}

template <typename CheckerT>
void test_master_file_num_udp_interfaces(const Detector &det,
                                         CheckerT &checker) {
    auto num_udp_interfaces = det.getNumberofUDPInterfaces().tsquash(
        "Inconsistent number of UDP interfaces");

    REQUIRE_NOTHROW(checker.template check<int>(
        MasterAttributes::N_NUM_UDP_INTERFACES.data(), num_udp_interfaces));
}

template <typename CheckerT>
void test_master_file_read_n_rows(const Detector &det, CheckerT &checker) {
    auto readnrows = det.getReadNRows().tsquash("Inconsistent number of rows");

    REQUIRE_NOTHROW(checker.template check<int>(
        MasterAttributes::N_NUMBER_OF_ROWS.data(), readnrows));
}

template <typename CheckerT>
void test_master_file_readout_speed(const Detector &det, CheckerT &checker) {
    auto readout_speed =
        det.getReadoutSpeed().tsquash("Inconsistent readout speed");

    REQUIRE_NOTHROW(checker.template check<std::string>(
        MasterAttributes::N_READOUT_SPEED.data(), ToString(readout_speed)));
}

template <typename CheckerT>
void test_master_file_frames_in_file(CheckerT &checker,
                                     const int frames_in_file) {
    REQUIRE_NOTHROW(checker.template check<int>(
        MasterAttributes::N_FRAMES_IN_FILE.data(), frames_in_file));
}

template <typename CheckerT>
void test_master_file_json_header(const Detector &det, CheckerT &checker) {
    auto json_header =
        det.getAdditionalJsonHeader().tsquash("Inconsistent JSON header");

    REQUIRE_NOTHROW(checker.template check<std::map<std::string, std::string>>(
        MasterAttributes::N_ADDITIONAL_JSON_HEADER.data(), json_header));
}

template <typename CheckerT>
void test_master_file_dynamic_range(const Detector &det, CheckerT &checker) {
    auto dr = det.getDynamicRange().tsquash("Inconsistent dynamic range");

    REQUIRE_NOTHROW(checker.template check<int>(
        MasterAttributes::N_DYNAMIC_RANGE.data(), dr));
}

template <typename CheckerT>
void test_master_file_ten_giga(const Detector &det, CheckerT &checker) {
    auto ten_giga =
        static_cast<int>(det.getTenGiga().tsquash("Inconsistent ten giga"));

    REQUIRE_NOTHROW(checker.template check<int>(
        MasterAttributes::N_TEN_GIGA.data(), ten_giga));
}

template <typename CheckerT>
void test_master_file_threshold_energy(const Detector &det, CheckerT &checker) {
    auto threshold =
        det.getThresholdEnergy().tsquash("Inconsistent threshold energy");

    REQUIRE_NOTHROW(checker.template check<int>(
        MasterAttributes::N_THRESHOLD_ENERGY.data(), threshold));
}

template <typename CheckerT>
void test_master_file_sub_exptime(const Detector &det, CheckerT &checker) {
    auto sub_exptime =
        det.getSubExptime().tsquash("Inconsistent sub exposure time");

    REQUIRE_NOTHROW(checker.template check<std::string>(
        MasterAttributes::N_SUB_EXPOSURE_TIME.data(), ToString(sub_exptime)));
}

template <typename CheckerT>
void test_master_file_sub_period(const Detector &det, CheckerT &checker) {
    auto exptime = det.getSubExptime().tsquash("Inconsistent sub exptime");
    auto deadtime = det.getSubDeadTime().tsquash("Inconsistent sub deadtime");
    auto sub_period = exptime + deadtime;

    REQUIRE_NOTHROW(checker.template check<std::string>(
        MasterAttributes::N_SUB_ACQUISITION_PERIOD.data(),
        ToString(sub_period)));
}

template <typename CheckerT>
void test_master_file_quad(const Detector &det, CheckerT &checker) {
    auto quad = static_cast<int>(det.getQuad().tsquash("Inconsistent quad"));

    REQUIRE_NOTHROW(
        checker.template check<int>(MasterAttributes::N_QUAD.data(), quad));
}

template <typename CheckerT>
void test_master_file_rate_corrections(const Detector &det, CheckerT &checker) {
    std::vector<int64_t> dead_times;
    for (auto item : det.getRateCorrection())
        dead_times.push_back(item.count());

    REQUIRE_NOTHROW(checker.template check<std::vector<int64_t>>(
        MasterAttributes::N_RATE_CORRECTIONS.data(), dead_times));
}

template <typename CheckerT>
void test_master_file_counter_mask(const Detector &det, CheckerT &checker) {
    auto counter_mask = static_cast<int>(
        det.getCounterMask().tsquash("Inconsistent counter mask"));

    REQUIRE_NOTHROW(checker.template check<int>(
        MasterAttributes::N_COUNTER_MASK.data(), counter_mask));
}

template <typename CheckerT>
void test_master_file_exptimes(const Detector &det, CheckerT &checker) {
    auto exptimes =
        det.getExptimeForAllGates().tsquash("Inconsistent exposure times");

    REQUIRE_NOTHROW(checker.template check<std::array<sls::ns, 3UL>>(
        MasterAttributes::N_EXPOSURE_TIMES.data(), exptimes));
}

template <typename CheckerT>
void test_master_file_gate_delays(const Detector &det, CheckerT &checker) {
    auto gate_delays =
        det.getGateDelayForAllGates().tsquash("Inconsistent GateDelay");

    REQUIRE_NOTHROW(checker.template check<std::array<sls::ns, 3UL>>(
        MasterAttributes::N_GATE_DELAYS.data(), gate_delays));
}

template <typename CheckerT>
void test_master_file_gates(const Detector &det, CheckerT &checker) {
    auto gates = det.getNumberOfGates().tsquash("Inconsistent number of gates");

    REQUIRE_NOTHROW(
        checker.template check<int>(MasterAttributes::N_GATES.data(), gates));
}

template <typename CheckerT>
void test_master_file_threadhold_energies(const Detector &det,
                                          CheckerT &checker) {
    auto threshold_energies =
        det.getAllThresholdEnergy().tsquash("Inconsistent threshold energies");

    REQUIRE_NOTHROW(checker.template check<std::array<int, 3UL>>(
        MasterAttributes::N_THRESHOLD_ENERGIES.data(), threshold_energies));
}

template <typename CheckerT>
void test_master_file_burst_mode(const Detector &det, CheckerT &checker) {
    auto burst_mode = det.getBurstMode().tsquash("Inconsistent burst mode");

    REQUIRE_NOTHROW(checker.template check<std::string>(
        MasterAttributes::N_BURST_MODE.data(), ToString(burst_mode)));
}

template <typename CheckerT>
void test_master_file_adc_mask(const Detector &det, CheckerT &checker) {
    acq::CTBState test_ctb_config = acq::default_ctb_state();
    auto adc_mask = test_ctb_config.adc_enable_10g;
    auto det_type = det.getDetectorType().squash();
    if (det_type == defs::CHIPTESTBOARD) {
        auto tengiga = test_ctb_config.ten_giga;
        if (!tengiga)
            adc_mask = test_ctb_config.adc_enable_1g;
    }

    REQUIRE_NOTHROW(checker.template check<uint32_t>(
        MasterAttributes::N_ADC_MASK.data(), adc_mask));
}

template <typename CheckerT>
void test_master_file_analog_flag(const Detector &det, CheckerT &checker) {
    acq::CTBState test_info = acq::default_ctb_state();
    auto romode = test_info.readout_mode;
    auto analog = static_cast<int>(
        (romode == defs::ANALOG_ONLY || romode == defs::ANALOG_AND_DIGITAL));

    REQUIRE_NOTHROW(
        checker.template check<int>(MasterAttributes::N_ANALOG.data(), analog));
}

template <typename CheckerT>
void test_master_file_analog_samples(const Detector &det, CheckerT &checker) {
    acq::CTBState test_info = acq::default_ctb_state();
    auto analog_samples = test_info.num_adc_samples;

    REQUIRE_NOTHROW(checker.template check<int>(
        MasterAttributes::N_ANALOG_SAMPLES.data(), analog_samples));
}

template <typename CheckerT>
void test_master_file_digital_flag(const Detector &det, CheckerT &checker) {
    acq::CTBState test_info = acq::default_ctb_state();
    auto romode = test_info.readout_mode;
    auto digital = static_cast<int>(romode == defs::DIGITAL_ONLY ||
                                    romode == defs::ANALOG_AND_DIGITAL ||
                                    romode == defs::DIGITAL_AND_TRANSCEIVER);

    REQUIRE_NOTHROW(checker.template check<int>(
        MasterAttributes::N_DIGITAL.data(), digital));
}

template <typename CheckerT>
void test_master_file_digital_samples(const Detector &det, CheckerT &checker) {
    acq::CTBState test_info = acq::default_ctb_state();
    auto digital_samples = test_info.num_dbit_samples;

    REQUIRE_NOTHROW(checker.template check<int>(
        MasterAttributes::N_DIGITAL_SAMPLES.data(), digital_samples));
}

template <typename CheckerT>
void test_master_file_dbit_offset(const Detector &det, CheckerT &checker) {
    acq::CTBState test_info = acq::default_ctb_state();
    auto dbit_offset = test_info.dbit_offset;

    REQUIRE_NOTHROW(checker.template check<int>(
        MasterAttributes::N_DBIT_OFFSET.data(), dbit_offset));
}

template <typename CheckerT>
void test_master_file_dbit_reorder(const Detector &det, CheckerT &checker) {
    acq::CTBState test_info = acq::default_ctb_state();
    auto dbit_reorder = test_info.dbit_reorder;

    REQUIRE_NOTHROW(checker.template check<int>(
        MasterAttributes::N_DBIT_REORDER.data(), dbit_reorder));
}

template <typename CheckerT>
void test_master_file_dbit_bitset(const Detector &det, CheckerT &checker) {
    acq::CTBState test_info = acq::default_ctb_state();
    uint64_t dbit_bitset = 0;
    for (auto &i : test_info.dbit_list) {
        dbit_bitset |= (static_cast<uint64_t>(1) << i);
    }

    REQUIRE_NOTHROW(checker.template check<uint64_t>(
        MasterAttributes::N_DBIT_BITSET.data(), dbit_bitset));
}

template <typename CheckerT>
void test_master_file_transceiver_mask(const Detector &det, CheckerT &checker) {
    acq::CTBState test_info = acq::default_ctb_state();
    auto trans_mask = test_info.transceiver_mask;

    REQUIRE_NOTHROW(checker.template check<int>(
        MasterAttributes::N_TRANSCEIVER_MASK.data(), trans_mask));
}

template <typename CheckerT>
void test_master_file_transceiver_flag(const Detector &det, CheckerT &checker) {
    acq::CTBState test_info = acq::default_ctb_state();
    auto romode = test_info.readout_mode;
    auto trans = static_cast<int>(romode == defs::DIGITAL_AND_TRANSCEIVER ||
                                  romode == defs::TRANSCEIVER_ONLY);

    REQUIRE_NOTHROW(checker.template check<int>(
        MasterAttributes::N_TRANSCEIVER.data(), trans));
}

template <typename CheckerT>
void test_master_file_transceiver_samples(const Detector &det,
                                          CheckerT &checker) {
    acq::CTBState test_info = acq::default_ctb_state();
    auto trans_samples = test_info.num_trans_samples;
    REQUIRE_NOTHROW(checker.template check<int>(
        MasterAttributes::N_TRANSCEIVER_SAMPLES.data(), trans_samples));
}

*/

template <typename CheckerT>
void check_metadata(CheckerT &checker, const acq::ExpectedState& expected);

template <typename CheckerT>
void check_common_metadata(CheckerT &checker, const acq::ExpectedState& expected) {
    check_version(checker, expected);
    check_detector_type(checker, expected);
    check_geometry(checker, expected);
    check_image_size(checker, expected);
    check_port_shape(checker, expected);
    check_frames_per_file(checker, expected);
    check_frame_discard_policy(checker, expected);
    check_partial_frames_padding(checker, expected);
    check_scan_parameters(checker, expected);
    check_total_frames(checker, expected);
    check_frames_in_file(checker, expected);
    check_additional_json_header(checker, expected);
}

template <typename CheckerT>
void check_jungfrau_metadata(CheckerT &checker, const acq::ExpectedState& expected) {
    check_rois(checker, expected);
    check_exptime(checker, expected);
    check_period(checker, expected);
    check_num_udp_interfaces(checker, expected);
    check_read_n_rows(checker, expected);
    check_readout_speed(checker, expected);
}

template <typename CheckerT>
void check_moench_metadata(CheckerT &checker, const acq::ExpectedState& expected) {
    check_rois(checker, expected);
    check_exptime(checker, expected);
    check_period(checker, expected);
    check_num_udp_interfaces(checker, expected);
    check_read_n_rows(checker, expected);
    check_readout_speed(checker, expected);
}

template <typename CheckerT>
void check_eiger_metadata(CheckerT &checker, const acq::ExpectedState& expected) {
    check_rois(checker, expected);
    check_dynamic_range(checker, expected);
    check_ten_giga(checker, expected);
    check_exptime(checker, expected);
    check_period(checker, expected);
    check_threshold_energy(checker, expected);
    check_sub_exptime(checker, expected);
    check_sub_period(checker, expected);
    check_quad(checker, expected);
    check_read_n_rows(checker, expected);
    check_rate_corrections(checker, expected);
    check_readout_speed(checker, expected);
}

template <typename CheckerT>
void check_mythen3_metadata(CheckerT &checker, const acq::ExpectedState& expected) {
    check_rois(checker, expected);
    check_dynamic_range(checker, expected);
    check_ten_giga(checker, expected);
    check_period(checker, expected);
    check_counter_mask(checker, expected);
    check_exptime_array(checker, expected);
    check_gate_delay_array(checker, expected);
    check_gates(checker, expected);
    check_threshold_energies(checker, expected);
    check_readout_speed(checker, expected);
}

template <typename CheckerT>
void check_gotthard2_metadata(CheckerT &checker, const acq::ExpectedState& expected) {
    check_rois(checker, expected);
    check_exptime(checker, expected);
    check_period(checker, expected);
    check_burst_mode(checker, expected);
    check_readout_speed(checker, expected);
}

template <typename CheckerT>
void check_ctb_metadata(CheckerT &checker, const acq::ExpectedState& expected);

}