// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
/*
This file contains Python bindings for the RegisterAddr, BitPosition and RegisterValue
classes.
*/
#include "py_headers.h"
#include "sls/bit_utils.h"

namespace py = pybind11;

using sls::RegisterAddress;
using sls::RegisterValue;
using sls::BitPosition;

void init_bit(py::module &m) {

    py::class_<RegisterAddress>(m, "RegisterAddress")
        .def(py::init())
        .def(py::init<const std::string &>())
        .def(py::init<uint32_t>())
        .def(py::init<const RegisterAddress &>())
        .def("__repr__", &RegisterAddress::str)
        .def("str", &RegisterAddress::str);
        .def("uint32", [](const RegisterAddress &v) { return static_cast<uint32_t>(v); })
        .def(py::self == py::self)
        .def(py::self != py::self)

    py::class_<BitPosition>(m, "BitPosition")
        .def(py::init())
        .def(py::init<RegisterAddress address, int>())
        .def("__repr__", &BitPosition::str)
        .def("str", &BitPosition::str);
        .def("address", &BitPosition::address)
        .def("bitPosition", &BitPosition::bitPosition)
        .def("setAddress", &BitPosition::setAddress)
        .def("setBitPosition", &BitPosition::setBitPosition)
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
