// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package

#include "CTBState.h"
#include "GeneralData.h"

namespace sls::test::acquire {

std::pair<uint64_t, int> calculate_ctb_image_size(const CTBState &test_info,
                                                  bool isXilinxCtb) {
    LOG(logDEBUG1) << test_info;
    CtbImageInputs inputs{};
    inputs.mode = test_info.readout_mode;
    inputs.nAnalogSamples = test_info.num_adc_samples;
    inputs.adcMask = test_info.adc_enable_10g;
    if (!isXilinxCtb && !test_info.ten_giga) {
        inputs.adcMask = test_info.adc_enable_1g;
    }
    inputs.nTransceiverSamples = test_info.num_trans_samples;
    inputs.transceiverMask = test_info.transceiver_mask;
    inputs.nDigitalSamples = test_info.num_dbit_samples;
    inputs.dbitOffset = test_info.dbit_offset;
    inputs.dbitReorder = test_info.dbit_reorder;
    inputs.dbitList = test_info.dbit_list;

    auto out = computeCtbImageSize(inputs);
    uint64_t image_size =
        out.nAnalogBytes + out.nDigitalBytes + out.nTransceiverBytes;
    LOG(logDEBUG1) << "Expected image size: " << image_size;
    int npixelx = out.nPixelsX;
    LOG(logDEBUG1) << "Expected number of pixels in x: " << npixelx;
    return std::make_pair(image_size, npixelx);
}

} // namespace sls::test::acquire