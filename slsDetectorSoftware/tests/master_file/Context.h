// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once

#include "catch.hpp"
#include <filesystem>
#include <fstream>
#include <optional>
#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <sstream>
#include <string>

#ifdef HDF5C
#include "H5Cpp.h"
#endif

namespace sls::test::master_file {

struct JsonContext {
    rapidjson::Document doc;

    explicit JsonContext(const std::string &path) {
        parse_binary_master_attributes(path);
    }

  private:
    void parse_binary_master_attributes(const std::string &file_path) {
        REQUIRE(std::filesystem::exists(file_path));

        std::ifstream file(file_path);
        REQUIRE(file.is_open());

        std::stringstream buffer;
        buffer << file.rdbuf();

        std::string json_str = buffer.str();

        rapidjson::ParseResult result = doc.Parse(json_str.c_str());

        if (!result) {
            std::cout << "JSON parse error: " << GetParseError_En(result.Code())
                      << " (at offset " << result.Offset() << ")" << std::endl;

            size_t offset = result.Offset();
            std::string context =
                json_str.substr(std::max(0, (int)offset - 20), 40);
            std::cout << "Context around error: \"" << context << "\""
                      << std::endl;
        }

        REQUIRE(result);
    }
};

#ifdef HDF5C
struct H5Context {
    H5::H5File file;
    explicit H5Context(const std::string &path) : file(path, H5F_ACC_RDONLY) {}
};
#endif

} // namespace sls::test::master_file
