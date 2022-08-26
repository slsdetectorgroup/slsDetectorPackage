// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "py_headers.h"

#include "sls/Pattern.h"
#include "sls/sls_detector_defs.h"
namespace py = pybind11;
void init_pattern(py::module &m) {

    using pat = sls::patternParameters;
    py::class_<pat> patternParameters(m, "patternParameters");

    PYBIND11_NUMPY_DTYPE(pat, word, ioctrl, limits, startloop, stoploop, nloop, wait,
                         waittime);

    patternParameters.def(py::init());
    patternParameters.def("numpy_view", [](py::object &obj) {
        pat &o = obj.cast<pat &>();
        return py::array_t<pat>(1, &o, obj);
    });

    py::class_<sls::Pattern> Pattern(m, "Pattern");
    Pattern.def(py::init());
    Pattern.def("load", &sls::Pattern::load);
    Pattern.def("data", (pat * (sls::Pattern::*)()) & sls::Pattern::data,
                py::return_value_policy::reference);
}
