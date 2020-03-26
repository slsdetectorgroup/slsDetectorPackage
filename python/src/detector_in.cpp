#include <pybind11/chrono.h>
#include <pybind11/operators.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "Detector.h"
#include "ToString.h"
#include "network_utils.h"
#include "sls_detector_defs.h"
#include "typecaster.h"
namespace py = pybind11;
void init_det(py::module &m) {
    using sls::Detector;
    using sls::Positions;
    using sls::Result;
    using sls::defs;
    
    py::class_<Detector> CppDetectorApi(m, "CppDetectorApi");
    CppDetectorApi
        .def(py::init<int>())

    [[FUNCTIONS]]
}
