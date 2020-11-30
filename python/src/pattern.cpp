#include <pybind11/chrono.h>
#include <pybind11/numpy.h>
#include <pybind11/operators.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "sls/Pattern.h"
#include "sls/sls_detector_defs.h"
namespace py = pybind11;
void init_pattern(py::module &m) {

    using pat = sls::patternParameters;
    py::class_<pat> patternParameters(m, "patternParameters");

    PYBIND11_NUMPY_DTYPE(pat, word, ioctrl, limits, loop, nloop, wait,
                         waittime);

    patternParameters.def(py::init());
    patternParameters.def("numpy_view", [](py::object &obj) {
        pat &o = obj.cast<pat &>();
        return py::array_t<pat>(1, &o, obj);
    });
    //.def_readwrite("name", &Pet::name)
    // patternParameters.def_property(
    //     "some",
    //     [](py::object &obj) {
    //         pat &o = obj.cast<pat &>();
    //         return py::array_t<pat>(1, &o, obj);
    //     },
    //     [](py::object &obj) {
    //         pat &o = obj.cast<pat &>();
    //         return py::array_t<pat>(1, &o, obj);
    //     });

    // patternParameters.def_property_readonly(
    //     "loop",
    //     [](py::object &obj) {
    //         pat &o = obj.cast<pat &>();
    //         return py::array_t<uint32_t>(6, &o.loop[0], obj);
    //     });

    py::class_<sls::Pattern> Pattern(m, "Pattern");
    Pattern.def(py::init());
    Pattern.def("load", &sls::Pattern::load);
    Pattern.def("data", (pat * (sls::Pattern::*)()) & sls::Pattern::data,
                py::return_value_policy::reference);
}
