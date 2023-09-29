// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "py_headers.h"

#include "mythenFileIO.h"
#include "sls/Detector.h"
#include "sls/Result.h"
#include <chrono>
#include <vector>

using ds = std::chrono::duration<double>;

namespace py = pybind11;
void init_enums(py::module &);
void init_experimental(py::module &);
void init_det(py::module &);
void init_network(py::module &);
void init_pattern(py::module &);
void init_scan(py::module &);
void init_source(py::module &);
void init_duration(py::module &);
void init_pedestal(py::module &);

PYBIND11_MODULE(_slsdet, m) {
    m.doc() = R"pbdoc(
        C/C++ API
        -----------------------
        .. warning ::

            This is the compiled c extension. You probably want to look at the
            interface provided by sls instead.

    )pbdoc";

    init_enums(m);
    init_det(m);
    init_network(m);
    init_pattern(m);
    init_scan(m);
    init_source(m);
    init_duration(m);
    init_pedestal(m);
    //  init_experimental(m);

    py::module io = m.def_submodule("io", "Submodule for io");
    io.def("read_my302_file", &read_my302_file, "some");

#ifdef VERSION_INFO
    m.attr("__version__") = VERSION_INFO;
#else
    m.attr("__version__") = "dev";
#endif
}
