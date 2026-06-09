// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once

#include "sls/Detector.h"
#include "sls/logger.h"

#include <cstdint>
#include <optional>
#include <vector>

namespace sls::test::acquire {

struct CTBState {
    defs::readoutMode readout_mode;
    int num_adc_samples;
    int num_dbit_samples;
    int num_trans_samples;
    bool ten_giga;
    uint32_t adc_enable_1g;
    uint32_t adc_enable_10g;
    int dbit_offset;
    std::vector<int> dbit_list;
    bool dbit_reorder;
    uint32_t transceiver_mask;
};

/** an example of CTB config */
inline CTBState
default_ctb_state(const defs::detectorType &detType = defs::GENERIC) {
    if (detType == defs::GENERIC) {
        throw sls::RuntimeError(
            "Cannot determine default CTB state for generic detector type.");
    }
    auto isAltera = (detType == defs::CHIPTESTBOARD);
    return {defs::ANALOG_AND_DIGITAL, 5000,       6000,       288,
            isAltera ? false : true,  0xFFFFFF00, 0xFF00FFFF, 0,
            {0, 12, 2, 43},           false,      0x3};
}

inline CTBState get_ctb_state(const Detector &det) {
    auto isAltera =
        (det.getDetectorType().squash(defs::GENERIC) == defs::CHIPTESTBOARD);
    return CTBState{
        det.getReadoutMode().tsquash("Inconsistent readout mode"),
        det.getNumberOfAnalogSamples().tsquash(
            "Inconsistent number of analog samples"),
        det.getNumberOfDigitalSamples().tsquash(
            "Inconsistent number of digital samples"),
        det.getNumberOfTransceiverSamples().tsquash(
            "Inconsistent number of transceiver samples"),
        isAltera ? det.getTenGiga().tsquash("Inconsisten ten giga enable")
                 : true,
        isAltera
            ? det.getADCEnableMask().tsquash("Inconsistent adc enable mask")
            : 0,
        det.getTenGigaADCEnableMask().tsquash(
            "Inconsistent ten giga adc enable mask"),
        det.getRxDbitOffset().tsquash("Inconsistent rx dbit offset"),
        det.getRxDbitList().tsquash("Inconsistent rx dbit list"),
        det.getRxDbitReorder().tsquash("Inconsistent rx dbit reorder"),
        det.getTransceiverEnableMask().tsquash(
            "Inconsistent transceiver mask")};
}

inline void set_ctb_state(Detector &det, const CTBState &s) {
    auto isAltera =
        (det.getDetectorType().squash(defs::GENERIC) == defs::CHIPTESTBOARD);
    det.setReadoutMode(s.readout_mode);
    if (isAltera) {
        det.setTenGiga(s.ten_giga);
        det.setADCEnableMask(s.adc_enable_1g);
    }
    det.setNumberOfAnalogSamples(s.num_adc_samples);
    det.setNumberOfDigitalSamples(s.num_dbit_samples);
    det.setNumberOfTransceiverSamples(s.num_trans_samples);
    det.setTenGigaADCEnableMask(s.adc_enable_10g);
    det.setRxDbitOffset(s.dbit_offset);
    det.setRxDbitList(s.dbit_list);
    det.setRxDbitReorder(s.dbit_reorder);
    det.setTransceiverEnableMask(s.transceiver_mask);
}

inline std::ostream &operator<<(std::ostream &os, const CTBState &s) {
    os << "CTB State:"
       << "\n  Readout Mode: " << ToString(s.readout_mode)
       << "\n  Num ADC Samples: " << s.num_adc_samples
       << "\n  Num DBIT Samples: " << s.num_dbit_samples
       << "\n  Num Trans Samples: " << s.num_trans_samples
       << "\n  Ten Giga: " << s.ten_giga
       << "\n  ADC Enable 1G: " << ToStringHex(s.adc_enable_1g)
       << "\n  ADC Enable 10G: " << ToStringHex(s.adc_enable_10g)
       << "\n  DBIT Offset: " << s.dbit_offset
       << "\n  DBIT List: " << ToString(s.dbit_list)
       << "\n  DBIT Reorder: " << s.dbit_reorder
       << "\n  Transceiver Mask: " << ToStringHex(s.transceiver_mask);
    return os;
}

inline void print_current_ctb_state(const Detector &det) {
    auto ctb_state = get_ctb_state(det);
    LOG(logINFO) << ctb_state;
}

/**
 * @brief RAII guard for restoring the existing ctb configuration after a test.
 *
 * The constructor saves the current ctb config and sets a new config, while the
 * destructor restores the original config.
 */
class CTBStateGuard {
  public:
    explicit CTBStateGuard(Detector &det,
                           std::optional<CTBState> new_state = std::nullopt)
        : det(det) {
        auto type = det.getDetectorType().squash(defs::GENERIC);
        if (type == defs::CHIPTESTBOARD || type == defs::XILINX_CHIPTESTBOARD) {
            active = true;
            if (!new_state.has_value()) {
                throw sls::RuntimeError(
                    "New CTB state must be provided for CTBStateGuard.");
            }
            saved_ = get_ctb_state(det);
            set_ctb_state(det, new_state.value());
        }
    }
    ~CTBStateGuard() {
        if (active)
            set_ctb_state(det, saved_);
    }

  private:
    Detector &det;
    bool active{false};
    CTBState saved_;
};

} // namespace sls::test::acquire