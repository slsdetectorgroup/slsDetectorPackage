// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once
#include <pybind11/pybind11.h>
#include "sls/Result.h"
// Add type_typecaster to pybind for our wrapper type
namespace pybind11 {
namespace detail {
template <typename Type, typename Alloc>
struct type_caster<sls::Result<Type, Alloc>>
    : list_caster<sls::Result<Type, Alloc>, Type> {};
} // namespace detail
} // namespace pybind11