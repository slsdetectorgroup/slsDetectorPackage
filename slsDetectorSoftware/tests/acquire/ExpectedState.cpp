// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package

#include "ExpectedState.h"
#include "Caller/test-Caller-global.h"
#include "receiver_defs.h"

// unnamed namespace for internal linkage
namespace {
using sls::defs;
using sls::Detector;
using ns = std::chrono::nanoseconds;
namespace acq = sls::test::acquire;

defs::detectorType get_detector_type(const Detector &det) {
    return det.getDetectorType().tsquash("Inconsistent detector type");
}

defs::xy get_geometry(const Detector &det) {
    auto modGeometry = det.getModuleGeometry();
    auto portperModGeometry = det.getPortPerModuleGeometry();
    return defs::xy{modGeometry.x * portperModGeometry.x,
                    modGeometry.y * portperModGeometry.y};
}

int get_dynamic_range(const Detector &det) {
    return det.getDynamicRange().tsquash("Inconsistent dynamic range");
}

defs::xy get_port_shape(const Detector &det,
                        std::optional<acq::CTBState> &ctb_state) {
    auto det_type = get_detector_type(det);
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
        if (!ctb_state.has_value()) {
            throw sls::RuntimeError(
                "CTB state must be provided to calculate expected port shape");
        }
        portSize.x =
            sls::calculate_ctb_image_size(
                ctb_state.value(), det_type == defs::XILINX_CHIPTESTBOARD)
                .second;
        portSize.y = 1;
    }
    return portSize;
}

uint64_t get_total_frames(const Detector &det) {
    uint64_t repeats =
        det.getNumberOfTriggers().tsquash("Inconsistent number of triggers");
    uint64_t numFrames =
        det.getNumberOfFrames().tsquash("Inconsistent number of frames");
    int numAdditionalStorageCells = 0;
    auto det_type = get_detector_type(det);
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
    return total_frames;
}

std::vector<defs::ROI> get_rois(const Detector &det) {
    auto rois = det.getRxROI();
    auto detsize = det.getDetectorSize();
    auto det_type = get_detector_type(det);
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
    return rois;
}

ns get_exptime(const Detector &det) {
    return ns(det.getExptime().tsquash("Inconsistent exposure time"));
}

ns get_period(const Detector &det) {
    return ns(det.getPeriod().tsquash("Inconsistent exposure time"));
}

int get_num_udp_interfaces(const Detector &det) {
    return det.getNumberofUDPInterfaces().tsquash(
        "Inconsistent number of UDP interfaces");
}

int get_read_n_rows(const Detector &det) {
    return det.getReadNRows().tsquash("Inconsistent number of read rows");
}

defs::speedLevel get_readout_speed(const Detector &det) {
    return det.getReadoutSpeed().tsquash("Inconsistent readout speed");
}

bool get_ten_giga(const Detector &det) {
    return det.getTenGiga().tsquash("Inconsistent 10Giga setting");
}

std::pair<ns, ns> get_sub_exptime_and_sub_period(const Detector &det) {
    auto exptime = det.getSubExptime().tsquash("Inconsistent sub exptime");
    auto deadtime = det.getSubDeadTime().tsquash("Inconsistent sub deadtime");
    auto sub_period = exptime + deadtime;
    return std::make_pair(ns(exptime), ns(sub_period));
}

acq::CommonExpectedState
build_common_state(const Detector &det,
                   std::optional<acq::CTBState> ctb_state) {
    acq::CommonExpectedState e;
    e.det_type = get_detector_type(det);
    e.timing_mode = det.getTimingMode().tsquash("Inconsistent timing mode");
    e.geometry = get_geometry(det);
    e.image_size = acq::get_expected_image_size(det, ctb_state);
    e.port_shape = get_port_shape(det, ctb_state);
    e.max_frames_per_file =
        det.getFramesPerFile().tsquash("Inconsistent frames per file");
    e.frame_discard_policy = det.getRxFrameDiscardPolicy().tsquash(
        "Inconsistent frame discard policy");
    e.partial_frames_padding = static_cast<int>(
        det.getPartialFramesPadding().tsquash("Inconsistent frame padding"));
    e.scan_parameters = det.getScan().tsquash("Inconsistent scan parameters");
    e.total_frames = get_total_frames(det);
    e.frames_in_file = det.getFramesCaught()[0][0];
    e.additional_json_header =
        det.getAdditionalJsonHeader().tsquash("Inconsistent JSON header");
    return e;
}

acq::JungfrauExpectedState build_jungfrau_specific_state(const Detector &det) {
    acq::JungfrauExpectedState e;
    e.rois = get_rois(det);
    e.exptime = get_exptime(det);
    e.period = get_period(det);
    e.num_udp_interfaces = get_num_udp_interfaces(det);
    e.read_n_rows = get_read_n_rows(det);
    e.readout_speed = get_readout_speed(det);
    return e;
}

acq::MoenchExpectedState build_moench_specific_state(const Detector &det) {
    acq::MoenchExpectedState e;
    e.rois = get_rois(det);
    e.exptime = get_exptime(det);
    e.period = get_period(det);
    e.num_udp_interfaces = get_num_udp_interfaces(det);
    e.read_n_rows = get_read_n_rows(det);
    e.readout_speed = get_readout_speed(det);
    return e;
}

acq::EigerExpectedState build_eiger_specific_state(const Detector &det) {
    acq::EigerExpectedState e;
    e.rois = get_rois(det);
    e.dynamic_range = get_dynamic_range(det);
    e.ten_giga = get_ten_giga(det);
    e.exptime = get_exptime(det);
    e.period = get_period(det);
    e.threshold_energy =
        det.getThresholdEnergy().tsquash("Inconsistent threshold energy");
    auto [sub_exptime, sub_period] = get_sub_exptime_and_sub_period(det);
    e.sub_exptime = sub_exptime;
    e.sub_period = sub_period;
    e.quad = det.getQuad().tsquash("Inconsistent quad setting");
    e.read_n_rows = get_read_n_rows(det);
    {
        for (auto item : det.getRateCorrection())
            e.rate_corrections.push_back(item.count());
    }
    e.readout_speed = get_readout_speed(det);
    return e;
}

acq::Mythen3ExpectedState build_mythen3_specific_state(const Detector &det) {
    acq::Mythen3ExpectedState e;
    e.rois = get_rois(det);
    e.dynamic_range = get_dynamic_range(det);
    e.ten_giga = get_ten_giga(det);
    e.period = get_period(det);
    e.counter_mask = det.getCounterMask().tsquash(
        "Inconsistent counter mask for Mythen3 detector");
    e.exp_times = det.getExptimeForAllGates().tsquash(
        "Inconsistent exposure times for all gates");
    e.gate_delays =
        det.getGateDelayForAllGates().tsquash("Inconsistent gate delays");
    e.num_gates = det.getNumberOfGates().tsquash(
        "Inconsistent number of gates for Mythen3 detector");
    e.threshold_energies =
        det.getAllThresholdEnergy().tsquash("Inconsistent threshold energies");
    e.readout_speed = get_readout_speed(det);
    return e;
}

acq::Gotthard2ExpectedState
build_gotthard2_specific_state(const Detector &det) {
    acq::Gotthard2ExpectedState e;
    e.rois = get_rois(det);
    e.exptime = get_exptime(det);
    e.period = get_period(det);
    e.burst_mode = det.getBurstMode().tsquash("Inconsistent burst mode");
    e.readout_speed = get_readout_speed(det);
    return e;
}

acq::CTBExpectedState build_ctb_specific_state(const Detector &det,
                                               const acq::CTBState &ctb_state) {
    acq::CTBExpectedState e;
    e.exptime = get_exptime(det);
    e.period = get_period(det);
    e.ctb_acq_state = ctb_state;
    return e;
}

acq::DetectorSpecificState
build_detector_specific_state(const Detector &det,
                              std::optional<acq::CTBState> ctb_state) {
    switch (det.getDetectorType().tsquash("bad type")) {
    case defs::JUNGFRAU:
        return build_jungfrau_specific_state(det);
    case defs::MOENCH:
        return build_moench_specific_state(det);
    case defs::EIGER:
        return build_eiger_specific_state(det);
    case defs::MYTHEN3:
        return build_mythen3_specific_state(det);
    case defs::GOTTHARD2:
        return build_gotthard2_specific_state(det);
    case defs::CHIPTESTBOARD:
    case defs::XILINX_CHIPTESTBOARD:
        if (!ctb_state.has_value()) {
            throw sls::RuntimeError(
                "CTB state must be provided to build expected state");
        }
        return build_ctb_specific_state(det, ctb_state.value());
    default:
        throw sls::RuntimeError("Unsupported detector type");
    }
}
} // anonymous namespace

namespace sls::test::acquire {

ExpectedState build_expected_state(const Detector &det,
                                   const AcquisitionState &acq_state,
                                   const FileState &file_state,
                                   std::optional<CTBState> ctb_state) {
    ExpectedState e;
    e.common_state = build_common_state(det, ctb_state);
    e.file_state = file_state;
    e.acquisition_state = acq_state;
    e.detector_specific_state = build_detector_specific_state(det, ctb_state);
    return e;
}

int get_expected_image_size(const Detector &det,
                            std::optional<CTBState> ctb_state) {
    auto det_type = get_detector_type(det);
    auto dynamic_range = get_dynamic_range(det);
    int bytes_per_pixel = dynamic_range / 8;
    int image_size = 0;
    detParameters par(det_type);

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
    case defs::XILINX_CHIPTESTBOARD:
        if (!ctb_state.has_value()) {
            throw sls::RuntimeError(
                "CTB state must be provided to calculate expected image size");
        }
        LOG(logINFORED) << ctb_state.value();
        image_size =
            sls::calculate_ctb_image_size(
                ctb_state.value(), (det_type == defs::XILINX_CHIPTESTBOARD))
                .first;
        break;
    default:
        throw sls::RuntimeError("Unsupported detector type for this test");
    }
    return image_size;
}

} // namespace sls::test::acquire