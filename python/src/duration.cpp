#include <chrono>
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

    m.def(
        "test_return_DurationWrapper",
        []() {
            DurationWrapper t(1.3);
            return t;
        },
        R"(
          Test function to return a DurationWrapper object. Ensures that the automatic conversion in typecaster.h works.
          )");

    m.def(
        "test_duration_to_ns",
        [](const std::chrono::nanoseconds t) {
            //Duration wrapper is used to be able to convert from time in python to chrono::nanoseconds
            //return count to have something to test 
            return t.count();
        },
        R"(
          Test function convert DurationWrapper or number to chrono::ns. Ensures that the automatic conversion in typecaster.h works.
          )"); // default value to test the default constructor
}
