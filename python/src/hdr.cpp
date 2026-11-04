// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package

#include "py_headers.h"

#include "sls/ToString.h"
#include "sls/sls_detector_defs.h"

namespace py = pybind11;
void init_hdr(py::module &m) {

    using hdr = slsDetectorDefs::hdrParameters;
    py::class_<hdr> hdrParameters(m, "hdrParameters") {
        .def(py::init())
            .def(py::init<slsDetectorDefs::operationMode>())
            .def_readwrite("enable", &hdr::enable)
            .def_readwrite("opMode", &hdr::opMode)
            .def(pybind11::self == pybind11::self)
            .def("__repr__", [](const hdr &a) { return sls::ToString(a); })
    }
}