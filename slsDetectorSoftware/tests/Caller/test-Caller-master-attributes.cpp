// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "master_file/Context.h"
#include "master_file/ReadersJson.h"
#ifdef HDF5C
#include "master_file/ReadersH5.h"
#endif
#include "master_file/Checker.h"

#include "Caller.h"
#include "MasterAttributes.h"
#include "catch.hpp"
#include "receiver_defs.h"
#include "sls/Detector.h"
#include "sls/ToString.h"
#include "sls/logger.h"
#include "sls/sls_detector_defs.h"
#include "test-Caller-global.h"
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

inline bool operator==(sls::ns lhs, sls::ns rhs) {
    return lhs.count() == rhs.count();
}

// different values for json and hdf5 => need to specialize separately
template <typename CheckerT>
void test_master_file_version(const Detector &det, CheckerT &checker);

template <>
void test_master_file_version<mf::Checker<mf::JsonContext>>(
    const Detector &det, mf::Checker<mf::JsonContext> &checker) {
    REQUIRE_NOTHROW(checker.template check<double>(
        MasterAttributes::N_VERSION.data(), BINARY_WRITER_VERSION));
}
#ifdef HDF5C
template <>
void test_master_file_version<mf::Checker<mf::H5Context>>(
    const Detector &det, mf::Checker<mf::H5Context> &checker) {
    REQUIRE_NOTHROW(checker.template check<double>(
        MasterAttributes::N_VERSION.data(), HDF5_WRITER_VERSION,
        mf::AccessType::Attribute));
}
#endif

template <typename CheckerT>
void test_master_file_type(const Detector &det, CheckerT &checker) {
    auto det_type = det.getDetectorType().tsquash("Inconsistent detector type");
    REQUIRE_NOTHROW(checker.template check<std::string>(
        MasterAttributes::N_DETECTOR_TYPE.data(), ToString(det_type)));
}

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

template <typename CheckerT>
void test_master_file_common_metadata(const Detector &det, CheckerT &checker) {
    test_master_file_version(det, checker);
    test_master_file_type(det, checker);
    test_master_file_timing_mode(det, checker);
    test_master_file_geometry(det, checker);
    test_master_file_image_size(det, checker);
    test_master_file_det_size(det, checker);
    test_master_file_max_frames_per_file(det, checker);
    test_master_file_frame_discard_policy(det, checker);
    test_master_file_frame_padding(det, checker);
    test_master_file_scan_parameters(det, checker);
    test_master_file_total_frames(det, checker);
    test_master_file_json_header(det, checker);
    // TODO: test frame header format?
}

template <typename CheckerT>
void test_master_file_jungfrau_metadata(const Detector &det,
                                        CheckerT &checker) {
    REQUIRE_NOTHROW(test_master_file_common_metadata(det, checker));
    // Jungfrau specific metadata
    REQUIRE_NOTHROW(test_master_file_rois(det, checker));
    REQUIRE_NOTHROW(test_master_file_exptime(det, checker));
    REQUIRE_NOTHROW(test_master_file_period(det, checker));
    REQUIRE_NOTHROW(test_master_file_num_udp_interfaces(det, checker));
    REQUIRE_NOTHROW(test_master_file_read_n_rows(det, checker));
    REQUIRE_NOTHROW(test_master_file_readout_speed(det, checker));
}

template <typename CheckerT>
void test_master_file_eiger_metadata(const Detector &det, CheckerT &checker) {
    REQUIRE_NOTHROW(test_master_file_common_metadata(det, checker));
    // Eiger specific metadata
    REQUIRE_NOTHROW(test_master_file_rois(det, checker));
    REQUIRE_NOTHROW(test_master_file_dynamic_range(det, checker));
    REQUIRE_NOTHROW(test_master_file_ten_giga(det, checker));
    REQUIRE_NOTHROW(test_master_file_exptime(det, checker));
    REQUIRE_NOTHROW(test_master_file_period(det, checker));
    REQUIRE_NOTHROW(test_master_file_threshold_energy(det, checker));
    REQUIRE_NOTHROW(test_master_file_sub_exptime(det, checker));
    REQUIRE_NOTHROW(test_master_file_sub_period(det, checker));
    REQUIRE_NOTHROW(test_master_file_quad(det, checker));
    REQUIRE_NOTHROW(test_master_file_read_n_rows(det, checker));
    REQUIRE_NOTHROW(test_master_file_rate_corrections(det, checker));
    REQUIRE_NOTHROW(test_master_file_readout_speed(det, checker));
}

template <typename CheckerT>
void test_master_file_moench_metadata(const Detector &det, CheckerT &checker) {
    REQUIRE_NOTHROW(test_master_file_common_metadata(det, checker));
    // Moench specific metadata
    REQUIRE_NOTHROW(test_master_file_rois(det, checker));
    REQUIRE_NOTHROW(test_master_file_exptime(det, checker));
    REQUIRE_NOTHROW(test_master_file_period(det, checker));
    REQUIRE_NOTHROW(test_master_file_num_udp_interfaces(det, checker));
    REQUIRE_NOTHROW(test_master_file_read_n_rows(det, checker));
    REQUIRE_NOTHROW(test_master_file_readout_speed(det, checker));
}

template <typename CheckerT>
void test_master_file_mythen3_metadata(const Detector &det, CheckerT &checker) {
    REQUIRE_NOTHROW(test_master_file_common_metadata(det, checker));
    // Mythen3 specific metadata
    REQUIRE_NOTHROW(test_master_file_rois(det, checker));
    REQUIRE_NOTHROW(test_master_file_dynamic_range(det, checker));
    REQUIRE_NOTHROW(test_master_file_ten_giga(det, checker));
    REQUIRE_NOTHROW(test_master_file_period(det, checker));
    REQUIRE_NOTHROW(test_master_file_counter_mask(det, checker));
    REQUIRE_NOTHROW(test_master_file_exptimes(det, checker));
    REQUIRE_NOTHROW(test_master_file_gate_delays(det, checker));
    REQUIRE_NOTHROW(test_master_file_gates(det, checker));
    REQUIRE_NOTHROW(test_master_file_threadhold_energies(det, checker));
    REQUIRE_NOTHROW(test_master_file_readout_speed(det, checker));
}

template <typename CheckerT>
void test_master_file_gotthard2_metadata(const Detector &det,
                                         CheckerT &checker) {
    REQUIRE_NOTHROW(test_master_file_common_metadata(det, checker));
    // Gotthard2 specific metadata
    REQUIRE_NOTHROW(test_master_file_exptime(det, checker));
    REQUIRE_NOTHROW(test_master_file_period(det, checker));
    REQUIRE_NOTHROW(test_master_file_burst_mode(det, checker));
    REQUIRE_NOTHROW(test_master_file_readout_speed(det, checker));
}

template <typename CheckerT>
void test_master_file_ctb_metadata(const Detector &det, CheckerT &checker) {
    auto det_type = det.getDetectorType().squash();
    REQUIRE_NOTHROW(test_master_file_common_metadata(det, checker));
    // Ctb specific metadata
    REQUIRE_NOTHROW(test_master_file_exptime(det, checker));
    REQUIRE_NOTHROW(test_master_file_period(det, checker));
    if (det_type == defs::CHIPTESTBOARD)
        REQUIRE_NOTHROW(test_master_file_ten_giga(det, checker));
    REQUIRE_NOTHROW(test_master_file_adc_mask(det, checker));
    REQUIRE_NOTHROW(test_master_file_analog_flag(det, checker));
    REQUIRE_NOTHROW(test_master_file_analog_samples(det, checker));
    REQUIRE_NOTHROW(test_master_file_digital_flag(det, checker));
    REQUIRE_NOTHROW(test_master_file_digital_samples(det, checker));
    REQUIRE_NOTHROW(test_master_file_dbit_offset(det, checker));
    REQUIRE_NOTHROW(test_master_file_dbit_reorder(det, checker));
    REQUIRE_NOTHROW(test_master_file_dbit_bitset(det, checker));
    REQUIRE_NOTHROW(test_master_file_transceiver_mask(det, checker));
    REQUIRE_NOTHROW(test_master_file_transceiver_flag(det, checker));
    REQUIRE_NOTHROW(test_master_file_transceiver_samples(det, checker));
}

template <typename CheckerT>
void test_master_file_metadata(const Detector &det, CheckerT &checker) {
    auto det_type =
        det.getDetectorType().tsquash("Inconsistent detector types");
    switch (det_type) {
    case defs::JUNGFRAU:
        test_master_file_jungfrau_metadata(det, checker);
        break;
    case defs::EIGER:
        test_master_file_eiger_metadata(det, checker);
        break;
    case defs::MOENCH:
        test_master_file_moench_metadata(det, checker);
        break;
    case defs::MYTHEN3:
        test_master_file_mythen3_metadata(det, checker);
        break;
    case defs::GOTTHARD2:
        test_master_file_gotthard2_metadata(det, checker);
        break;
    case defs::CHIPTESTBOARD:
    case defs::XILINX_CHIPTESTBOARD:
        test_master_file_ctb_metadata(det, checker);
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
    int64_t num_frames = 1;

    auto f = acq::default_file_state();
    acq::run(det, num_frames, f);
    std::string master_file_prefix = acq::get_master_file_name_prefix(f);

    // binary (/tmp/sls_test_master_0.json)
    std::string fname = master_file_prefix + ".json";
    auto doc = parse_binary_master_attributes(fname);
    mf::Checker<mf::JsonContext> checker(mf::JsonContext{doc});
    test_master_file_metadata(det, checker);
    test_master_file_frames_in_file(checker, num_frames);

    // hdf5 (/tmp/sls_test_master_0.h5)
#ifdef HDF5C
    fname = master_file_prefix + ".h5";
    try {
        mf::Checker<mf::H5Context> checker(mf::H5Context{fname});
        test_master_file_metadata(det, checker);
        test_master_file_frames_in_file(checker, num_frames);
    } catch (H5::Exception &e) {
        LOG(logERROR) << "HDF5 error: " << e.getDetailMsg();
        throw;
    }
#endif
}

} // namespace sls
