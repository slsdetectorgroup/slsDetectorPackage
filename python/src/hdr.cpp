// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package

#include "py_headers.h"

#include "sls/ToString.h"
#include "sls/sls_detector_defs.h"

namespace py = pybind11;
void init_hdr(py::module &m) {

    using hdr = slsDetectorDefs::hdrParameters;
    py::class_<hdr> hdrParameters(m, "hdrParameters");

    hdrParameters.def(py::init());
    hdrParameters.def(py::init<slsDetectorDefs::operationMode>());
    hdrParameters.def_readwrite("enable", &hdr::enable);
    hdrParameters.def_readwrite("opMode", &hdr::opMode);
    hdrParameters.def(pybind11::self == pybind11::self);

    hdrParameters.def("__repr__", [](const hdr &a) { return sls::ToString(a); });
}
