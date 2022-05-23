// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "catch.hpp"
#include "sls/file_utils.h"
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <vector>

namespace sls {

TEST_CASE("Get size of empty file") {
    char fname[] = "temfile_XXXXXX";
    std::ifstream ifs(fname);
    auto size = getFileSize(ifs);
    REQUIRE(size <= 0); // -1 or zero
}

TEST_CASE("Get size of file with data") {
    constexpr size_t n_bytes = 137;
    std::vector<char> data(n_bytes);
    char fname[] = "temfile_XXXXXX";
    int fh = mkstemp(fname);
    write(fh, data.data(), n_bytes);

    std::ifstream ifs(fname);
    auto size = getFileSize(ifs);
    REQUIRE(size == n_bytes);
    REQUIRE(ifs.tellg() == 0); // getting size resets pos!
}

} // namespace sls
