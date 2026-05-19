// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once

#include <optional>
#include <rapidjson/document.h>
#include <string>

#ifdef HDF5C
#include "H5Cpp.h"
#endif

namespace sls::test::master_file {

struct JsonContext {
    rapidjson::Document doc;
};
#ifdef HDF5C
struct H5Context {
    H5::H5File file;
    explicit H5Context(const std::string &path) : file(path, H5F_ACC_RDONLY) {}
};
#endif

} // namespace sls::test::master_file
