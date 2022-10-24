// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once
#include "sls/sls_detector_defs.h"
#include <string>

namespace sls {

struct SingleDetectorConfig {
    slsDetectorDefs::detectorType type_enum =
        slsDetectorDefs::detectorType::EIGER;
    const std::string hostname = "beb031+beb032+";
    const std::string type_string = "Eiger";
    const std::string my_ip = "129.129.205.171";
};

} // namespace sls
