// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once
#include <chrono>

#include "sls/TypeTraits.h"
namespace sls {
namespace time {
using ns = std::chrono::nanoseconds;
using us = std::chrono::microseconds;
using ms = std::chrono::milliseconds;
using s = std::chrono::seconds;

// Absolute value of std::chrono::duration
template <class Rep, class Period>
constexpr std::chrono::duration<Rep, Period>
abs(std::chrono::duration<Rep, Period> d) {
    return d >= d.zero() ? d : -d;
}

static_assert(sizeof(ns) == 8, "ns needs to be 64bit");

} // namespace time
} // namespace sls