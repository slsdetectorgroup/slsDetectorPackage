// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
/*
This file contains Python bindings for the Hz and for conversion to other units from and to string.
*/
#include "py_headers.h"

#include "sls/ToString.h"
#include "sls/sls_detector_defs.h"

namespace py = pybind11;
void init_freq(py::module &m) {

    py::class_<slsDetectorDefs::Hz> Hz(m, "Hz");
    Hz.def(py::init<int v) {
        return slsDetectorDefs::Hz(v * 1000000);
    }));
    Hz.def(py::init([](const std::string &s) { 
        return sls::StringTo<slsDetectorDefs::Hz>(s); 
    }));
    Hz.def_readwrite("value", &slsDetectorDefs::Hz::value);
    Hz.def("__repr__", [](const slsDetectorDefs::Hz &f) { return sls::ToString(f); });
    Hz.def("__str__", [](const slsDetectorDefs::Hz &f) { return sls::ToString(f); });
    Hz.def(py::self == py::self);
    py::implicitly_convertible<std::string, slsDetectorDefs::Hz>();
}