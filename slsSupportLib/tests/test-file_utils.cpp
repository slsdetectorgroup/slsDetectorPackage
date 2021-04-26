#include "catch.hpp"
#include "sls/file_utils.h"
#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <vector>

TEST_CASE("Get size of empty file") {
    char fname[] = "temfile_XXXXXX";
    int fh = mkstemp(fname);
    std::ifstream ifs(fname);
    auto size = sls::getFileSize(ifs);
    REQUIRE(size == 0);
}

TEST_CASE("Get size of file with data") {
    constexpr size_t n_bytes = 137;
    std::vector<char> data(n_bytes);
    char fname[] = "temfile_XXXXXX";
    int fh = mkstemp(fname);
    write(fh, data.data(), n_bytes);

    std::ifstream ifs(fname);
    auto size = sls::getFileSize(ifs);
    REQUIRE(size == n_bytes);
    REQUIRE(ifs.tellg() == 0); //getting size resets pos!
}

