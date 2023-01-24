#include "py_headers.h"

#include "DurationWrapper.h"
#include <sstream>
namespace py = pybind11;
using sls::DurationWrapper;

void init_duration(py::module &m) {
    py::class_<DurationWrapper>(m, "DurationWrapper")
        .def(py::init())
        .def(py::init<double>())
        .def("total_seconds", &DurationWrapper::total_seconds)
        .def("count", &DurationWrapper::count)
        .def("set_count", &DurationWrapper::set_count)
        .def("__eq__", &DurationWrapper::operator==)
        .def("__repr__", [](const DurationWrapper &self) {
            std::stringstream ss;
            ss << "sls::DurationWrapper(total_seconds: " << self.total_seconds()
               << " count: " << self.count() << ")";
            return ss.str();
        });
}
