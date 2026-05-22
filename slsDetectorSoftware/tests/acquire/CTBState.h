// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once

#include "sls/Detector.h"
#include "sls/logger.h"

#include <cstdint>
#include <vector>

namespace sls::test::acquire {

struct CTBState {
    defs::readoutMode readout_mode;
    bool ten_giga;
    int num_adc_samples;
    int num_dbit_samples;
    int num_trans_samples;
    uint32_t adc_enable_1g;
    uint32_t adc_enable_10g;
    int dbit_offset;
    std::vector<int> dbit_list;
    bool dbit_reorder;
    uint32_t transceiver_mask;
};

inline CTBState default_ctb_state(bool isAltera = false) {
    return {defs::ANALOG_AND_DIGITAL,
            isAltera ? false : true,
            5000,
            6000,
            288,
            0xFFFFFF00,
            0xFF00FFFF,
            0,
            {0, 12, 2, 43},
            false,
            0x3};
}

inline CTBState get_ctb_state(const Detector &det, bool isAltera) {
    return CTBState{
        det.getReadoutMode().tsquash("Inconsistent readout mode"),
        isAltera ? det.getTenGiga().tsquash("Inconsisten ten giga enable")
                 : true,
        det.getNumberOfAnalogSamples().tsquash(
            "Inconsistent number of analog samples"),
        det.getNumberOfDigitalSamples().tsquash(
            "Inconsistent number of digital samples"),
        det.getNumberOfTransceiverSamples().tsquash(
            "Inconsistent number of transceiver samples"),
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

inline void set_ctb_state(Detector &det, const CTBState &s, bool isAltera) {
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

inline void print_ctb_state(const CTBState &s) {
    LOG(logINFO) << "CTB State:"
                 << "\n  Readout Mode: " << ToString(s.readout_mode)
                 << "\n  Ten Giga: " << s.ten_giga
                 << "\n  Num ADC Samples: " << s.num_adc_samples
                 << "\n  Num DBIT Samples: " << s.num_dbit_samples
                 << "\n  Num Trans Samples: " << s.num_trans_samples
                 << "\n  ADC Enable 1G: " << ToStringHex(s.adc_enable_1g)
                 << "\n  ADC Enable 10G: " << ToStringHex(s.adc_enable_10g)
                 << "\n  DBIT Offset: " << s.dbit_offset
                 << "\n  DBIT List: " << ToString(s.dbit_list)
                 << "\n  DBIT Reorder: " << s.dbit_reorder
                 << "\n  Transceiver Mask: " << ToStringHex(s.transceiver_mask);
}

class CTBStateGuard {
  public:
    explicit CTBStateGuard(Detector &det, const CTBState &new_state)
        : det(det) {
        auto type = det.getDetectorType().squash(defs::GENERIC);
        isAltera = (type == defs::CHIPTESTBOARD);
        active =
            (type == defs::CHIPTESTBOARD || type == defs::XILINX_CHIPTESTBOARD);
        if (active) {
            saved_ = get_ctb_state(det, isAltera);
            set_ctb_state(det, new_state, isAltera);
        }
    }
    ~CTBStateGuard() {
        if (active)
            set_ctb_state(det, saved_, isAltera);
    }

  private:
    Detector &det;
    bool active{false};
    bool isAltera{false};
    CTBState saved_;
};

} // namespace sls::test::acquire