// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once

#include "Context.h"

#ifdef HDF5C
#include "H5Cpp.h"
#endif

#include <rapidjson/document.h>

namespace sls::test::master_file {
template <typename Context, typename T> struct Reader;

template <typename Context, typename T>
T read(const Context &ctx, const std::string &name) {
    return Reader<Context, T>::read(ctx, name);
}

} // namespace sls::test::master_file
