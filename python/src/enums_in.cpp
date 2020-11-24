#include <pybind11/chrono.h>
#include <pybind11/numpy.h>
#include <pybind11/operators.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "sls/sls_detector_defs.h"
namespace py = pybind11;
void init_enums(py::module &m) {
    py::class_<slsDetectorDefs> Defs(m, "slsDetectorDefs");
    py::class_<slsDetectorDefs::xy> xy(m, "xy");
    xy.def(py::init());
    xy.def(py::init<int, int>());
    xy.def_readwrite("x", &slsDetectorDefs::xy::x);
    xy.def_readwrite("y", &slsDetectorDefs::xy::y);

    py::class_<slsDetectorDefs::patternParameters> patternParameters(
        m, "patternParameters");

    using pat = slsDetectorDefs::patternParameters;
    PYBIND11_NUMPY_DTYPE(pat, word, patioctrl, patlimits, patloop, patnloop,
                         patwait, patwaittime);

    patternParameters.def(py::init());
    patternParameters.def("numpy_view", [](py::object &obj) { 
        pat& o = obj.cast<pat&>();
        return py::array_t<pat>(1, &o, obj); });
    patternParameters.def("load", &pat::load);

    [[ENUMS]]
}
