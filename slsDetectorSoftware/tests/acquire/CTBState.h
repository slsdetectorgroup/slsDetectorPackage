// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once

#include "sls/Detector.h"

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

inline CTBState default_ctb_state() {
    return {
        defs::ANALOG_AND_DIGITAL,
        false,
        5000,
        6000,
        288,
        0xFFFFFF00,
        0xFF00FFFF,
        0,
        {0, 12, 2, 43},
        false,
        0x3
    };
}

inline CTBState get_ctb_state(const Detector& det) {
    return CTBState{
        det.getReadoutMode().tsquash("Inconsistent readout mode"),
        true, // always true for xilinx, det.getTenGiga() for ctb
        det.getNumberOfAnalogSamples().tsquash("Inconsistent number of analog samples"),
        det.getNumberOfDigitalSamples().tsquash("Inconsistent number of digital samples"),
        det.getNumberOfTransceiverSamples().tsquash("Inconsistent number of transceiver samples"),
        0, // always 0 for xilinx, det.getTenGigaADCEnableMask() for ctb
        det.getTenGigaADCEnableMask().tsquash("Inconsistent ten giga adc enable mask"),
        det.getRxDbitOffset().tsquash("Inconsistent rx dbit offset"),
        det.getRxDbitList().tsquash("Inconsistent rx dbit list"),
        det.getRxDbitReorder().tsquash("Inconsistent rx dbit reorder"),
        det.getTransceiverEnableMask().tsquash("Inconsistent transceiver mask")
    };
}

inline void set_ctb_state(Detector& det, const CTBState& s) {
    det.setReadoutMode(s.readout_mode);
    if (det.getDetectorType().tsquash("inconsistent detector type") ==
        slsDetectorDefs::CHIPTESTBOARD) {
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

class CTBStateGuard {
public:
    explicit CTBStateGuard(Detector& det) : det(det), saved_(get_ctb_state(det)) {}
    ~CTBStateGuard() {
        set_ctb_state(det, saved_);
    }

private:
    Detector& det;
    CTBState saved_;

    static bool is_ctb(const Detector& det) {
        auto type = det.getDetectorType().tsquash("");
        return type == defs::CHIPTESTBOARD ||
               type == defs::XILINX_CHIPTESTBOARD;
    }
};


} // namespace sls::test::acquire