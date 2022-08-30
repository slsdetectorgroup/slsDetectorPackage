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

TEST_CASE("Channel file reading") {
    std::string fname = "/tmp/sls_test_channels.txt";
    std::ofstream outfile;
    outfile.open(fname.c_str(), std::ios_base::out);
    if (!outfile.is_open()) {
        throw RuntimeError("Could not open file " + fname +
                           "for writing to test channel4 file reading");
    }
    outfile << "0" << std::endl;
    outfile << "12, 15, 43" << std::endl;
    outfile << "40:45" << std::endl;
    outfile << "1279" << std::endl;
    outfile.close();
    std::vector<int> list;
    REQUIRE_NOTHROW(list = getChannelsFromFile(fname));
    std::vector<int> expected = {0, 12, 15, 40, 41, 42, 43, 44, 1279};
    REQUIRE(list == expected);
}

} // namespace sls
