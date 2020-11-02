#include <pybind11/chrono.h>
#include <pybind11/operators.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "sls/sls_detector_defs.h"
namespace py = pybind11;
void init_enums(py::module &m) {
    py::class_<slsDetectorDefs> Defs(m, "slsDetectorDefs");
    py::class_<slsDetectorDefs::xy> xy(m, "xy");
    xy.def(py::init());
	xy.def(py::init<int,int>());
    xy.def_readwrite("x", &slsDetectorDefs::xy::x);
    xy.def_readwrite("y", &slsDetectorDefs::xy::y);

[[ENUMS]]

}
