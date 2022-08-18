#include <pybind11/chrono.h>
#include <pybind11/operators.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "sls/Duration.h"
namespace py = pybind11;
using sls::Duration;

void init_duration(py::module &m) {

    py::class_<Duration>(m, "Duration")
        .def(py::init())
        .def(py::init<double>())
        .def("total_seconds", &Duration::total_seconds)
        .def("count", &Duration::count);

}
