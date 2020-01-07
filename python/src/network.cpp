/*
This file contains Python bindings for the IpAddr and MacAddr
classes. 
*/


#include <pybind11/chrono.h>
#include <pybind11/operators.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "network_utils.h"
namespace py = pybind11;
using sls::IpAddr;
using sls::MacAddr;
void init_network(py::module &m) {

    py::class_ <IpAddr> IpAddr(m, "IpAddr");
    IpAddr.def(py::init())
    .def(py::init<const std::string&>())
    .def(py::init<uint32_t>())
    .def("hex", &IpAddr::hex)
    .def("uint32", &IpAddr::uint32)
    .def(py::self == py::self)
    .def("__repr__", &IpAddr::str)
    .def("str", &IpAddr::str);


    py::class_ <MacAddr> MacAddr(m, "MacAddr");
    MacAddr.def(py::init())
    .def(py::init<const std::string&>())
    .def(py::init<uint64_t>())
    .def("hex", &MacAddr::hex)
    .def(py::self == py::self)
    .def("uint64", &MacAddr::uint64)
    .def("__repr__", &MacAddr::str)
    .def("str", &MacAddr::str);

}
