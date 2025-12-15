// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
/*
This file contains Python bindings for the RegisterAddr, BitAddress and
RegisterValue classes.
*/
#include "py_headers.h"
#include "sls/bit_utils.h"

namespace py = pybind11;

using sls::BitAddress;
using sls::RegisterAddress;
using sls::RegisterValue;

void init_bit(py::module &m) {

    py::class_<RegisterAddress>(m, "RegisterAddress")
        .def(py::init())
        .def(py::init<const std::string &>())
        .def(py::init<uint32_t>())
        .def(py::init<const RegisterAddress &>())
        .def("__repr__", &RegisterAddress::str)
        .def("str", &RegisterAddress::str)
        .def("uint32", [](const RegisterAddress &v) { return static_cast<uint32_t>(v); })
        .def(py::self == py::self)
        .def(py::self != py::self);

    py::class_<BitAddress>(m, "BitAddress")
        .def(py::init())
        .def(py::init<RegisterAddress, uint32_t>())
        .def(py::init<std::string, std::string>())
        .def("__repr__", &BitAddress::str)
        .def("str", &BitAddress::str)
        .def("address", &BitAddress::address)
        .def("bitPosition", &BitAddress::bitPosition)
        .def(py::self == py::self)
        .def(py::self != py::self);

    py::class_<RegisterValue>(m, "RegisterValue")
        .def(py::init<>())
        .def(py::init<const std::string &>())
        .def(py::init<uint32_t>())
        .def(py::init<const RegisterValue &>())
        .def("__repr__", &RegisterValue::str)
        .def("str", &RegisterValue::str)
        .def("uint32", [](const RegisterValue &v) { return static_cast<uint32_t>(v); })
        .def(py::self == py::self)
        .def(py::self != py::self)
        .def("__ior__", [](RegisterValue &v, uint32_t rhs) -> RegisterValue& {
            v |= rhs;
            return v;
        }, py::return_value_policy::reference_internal);
}
