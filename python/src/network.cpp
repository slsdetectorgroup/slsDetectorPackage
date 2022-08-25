// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
/*
This file contains Python bindings for the IpAddr and MacAddr
classes.
*/
#include "py_headers.h"

#include "sls/network_utils.h"
namespace py = pybind11;
using sls::IpAddr;
using sls::MacAddr;
void init_network(py::module &m) {

    py::class_<IpAddr>(m, "IpAddr")
        .def(py::init())
        .def(py::init<const std::string &>())
        .def(py::init<uint32_t>())
        .def(py::init<const IpAddr &>())
        .def("hex", &IpAddr::hex)
        .def("uint32", &IpAddr::uint32)
        .def(py::self == py::self)
        .def("__repr__", &IpAddr::str)
        .def("str", &IpAddr::str);

    py::class_<MacAddr>(m, "MacAddr")
        .def(py::init())
        .def(py::init<const std::string &>())
        .def(py::init<uint64_t>())
        .def(py::init<const MacAddr &>())
        .def("hex", &MacAddr::hex)
        .def(py::self == py::self)
        .def("uint64", &MacAddr::uint64)
        .def("__repr__", &MacAddr::str)
        .def("str", &MacAddr::str);
}
