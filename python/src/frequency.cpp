// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
/*
This file contains Python bindings for the Hz and for conversion to other units from and to string.
*/
#include "py_headers.h"
#include <cmath>

#include "sls/ToString.h"
#include "sls/sls_detector_defs.h"

namespace py = pybind11;

constexpr double kHz = 1e3;
constexpr double MHz = 1e6;

void init_freq(py::module &m) {

    py::class_<slsDetectorDefs::Hz> Hz(m, "Hz");
    Hz.def(py::init<int>());
    Hz.def(py::init([](double v) {
        return slsDetectorDefs::Hz(static_cast<int>(std::round(v)));
    }));
    Hz.def_readwrite("value", &slsDetectorDefs::Hz::value);
    Hz.def("__repr__", [](const slsDetectorDefs::Hz &f) { 
        return sls::ToString(f); 
    });
    Hz.def("__str__", [](const slsDetectorDefs::Hz &f) { 
        return sls::ToString(f); 
    });
    Hz.def(py::self == py::self);
    m.def("kHz", [](double v) {
        return slsDetectorDefs::Hz(static_cast<int>(std::round(v * kHz)));
    });

    m.def("MHz", [](double v) {
        return slsDetectorDefs::Hz(static_cast<int>(std::round(v * MHz)));
    });
    py::implicitly_convertible<int, slsDetectorDefs::Hz>();
    py::implicitly_convertible<double, slsDetectorDefs::Hz>();
}