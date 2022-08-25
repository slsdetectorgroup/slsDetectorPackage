// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package

#include "py_headers.h"

#include "sls/ToString.h"
#include "sls/sls_detector_defs.h"

namespace py = pybind11;
void init_source(py::module &m) {

    using src = slsDetectorDefs::currentSrcParameters;
    py::class_<src> currentSrcParameters(m, "currentSrcParameters");

    currentSrcParameters.def(py::init());
    currentSrcParameters.def_readwrite("enable", &src::enable);
    currentSrcParameters.def_readwrite("fix", &src::fix);
    currentSrcParameters.def_readwrite("normal", &src::normal);
    currentSrcParameters.def_readwrite("select", &src::select);
    currentSrcParameters.def(pybind11::self == pybind11::self);

    currentSrcParameters.def("__repr__",
                             [](const src &a) { return sls::ToString(a); });
}
