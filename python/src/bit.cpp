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
        .def(py::init<uint32_t>())
        .def(py::init<const RegisterAddress &>())
        .def("__repr__",
             [](const RegisterAddress &addr) {
                 return "RegisterAddress(" + addr.str() + ")";
             })
        .def("__str__", &RegisterAddress::str)
        .def("value", &RegisterAddress::value)
        .def("__int__", &RegisterAddress::value)
        .def(py::self == py::self)
        .def(py::self != py::self)
        .def("__add__",&RegisterAddress::operator+)
        .def("__radd__",&RegisterAddress::operator+)
        .def("__iadd__",&RegisterAddress::operator+=, py::return_value_policy::reference_internal);

    py::class_<BitAddress>(m, "BitAddress")
        .def(py::init())
        .def(py::init<RegisterAddress, uint32_t>())
        .def("__repr__",
             [](const BitAddress &addr) {
                 return "BitAddress(" + addr.str() + ")";
             })
        .def("__str__", &BitAddress::str)
        .def("address", &BitAddress::address)
        .def("bitPosition", &BitAddress::bitPosition)
        .def(py::self == py::self)
        .def(py::self != py::self);

    py::class_<RegisterValue>(m, "RegisterValue")
        .def(py::init<>())
        .def(py::init<uint32_t>())
        .def(py::init<const RegisterValue &>())
        .def("__repr__",
             [](const RegisterValue &val) {
                 return "RegisterValue(" + val.str() + ")";
             })
        .def("__str__", &RegisterValue::str)
        .def("value", &RegisterValue::value)
        .def("__int__", &RegisterValue::value)
        .def(py::self == py::self)
        .def(py::self != py::self)
        .def("__or__", [](const RegisterValue &lhs,
                          const RegisterValue &rhs) { return lhs | rhs; })
        .def("__or__",
             [](const RegisterValue &lhs, uint32_t rhs) { return lhs | rhs; })
        .def(
            "__ior__",
            [](RegisterValue &lhs, uint32_t rhs) -> RegisterValue & {
                lhs |= rhs;
                return lhs;
            },
            py::return_value_policy::reference_internal);
}
