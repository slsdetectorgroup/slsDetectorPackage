#include <pybind11/chrono.h>
#include <pybind11/numpy.h>
#include <pybind11/operators.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

// #include "sls/Pattern.h"
#include "sls/ToString.h"
#include "sls/sls_detector_defs.h"

namespace py = pybind11;
void init_source(py::module &m) {

    using src = slsDetectorDefs::currentSrcParameters;
    py::class_<src> currentSrcParameters(m, "currentSrcParameters");

    currentSrcParameters.def(py::init());
    currentSrcParameters.def_readwrite("enable_", &src::enable_);
    currentSrcParameters.def_readwrite("fix_", &src::fix_);
    currentSrcParameters.def_readwrite("normal_", &src::normal_);
    currentSrcParameters.def_readwrite("select_", &src::select_);
    currentSrcParameters.def(pybind11::self == pybind11::self);

    currentSrcParameters.def("__repr__",
                             [](const src &a) { return sls::ToString(a); });
}
