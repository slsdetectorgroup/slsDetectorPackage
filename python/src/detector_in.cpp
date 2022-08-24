// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include <pybind11/operators.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "sls/Detector.h"
#include "sls/ToString.h"
#include "sls/network_utils.h"
#include "sls/sls_detector_defs.h"
#include "typecaster.h"

#include "sls/TimeHelper.h"
#include <array>
#include <chrono>
namespace py = pybind11;
void init_det(py::module &m) {
    using sls::defs;
    using sls::Detector;
    using sls::ns;
    using sls::Positions;
    using sls::Result;

    py::class_<Detector> CppDetectorApi(m, "CppDetectorApi");
    CppDetectorApi.def(py::init<int>());

        [[FUNCTIONS]]
}
