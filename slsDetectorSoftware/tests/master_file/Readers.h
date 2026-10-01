// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once

#include "Context.h"
#include "sls/sls_detector_defs.h"

namespace sls::test::master_file {

template <typename Context, typename T> struct Reader;
template <typename Context, typename T> struct AttributeReader;

enum class AccessType { Dataset, Attribute };

template <typename Context, typename T>
T read(const Context &ctx, const std::string &name,
       AccessType access = AccessType::Dataset) {
    return Reader<Context, T>::read(ctx, name, access);
}

inline void check_size(size_t actual, size_t expected, const std::string &name,
                       const std::string &doc) {
    if (actual != expected) {
        throw sls::RuntimeError(
            doc + " array " + name + " has " + std::to_string(actual) +
            " elements instead of " + std::to_string(expected));
    }
}

} // namespace sls::test::master_file
