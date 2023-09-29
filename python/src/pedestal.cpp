// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package

#include "py_headers.h"

#include "sls/ToString.h"
#include "sls/sls_detector_defs.h"

namespace py = pybind11;
void init_pedestal(py::module &m) {

    using src = slsDetectorDefs::pedestalParameters;
    py::class_<src> pedestalParameters(m, "pedestalParameters");

    pedestalParameters.def(py::init());
    pedestalParameters.def_readwrite("enable", &src::enable);
    pedestalParameters.def_readwrite("frames", &src::frames);
    pedestalParameters.def_readwrite("loops", &src::loops);
    pedestalParameters.def(pybind11::self == pybind11::self);

    pedestalParameters.def("__repr__",
                           [](const src &a) { return sls::ToString(a); });
}
