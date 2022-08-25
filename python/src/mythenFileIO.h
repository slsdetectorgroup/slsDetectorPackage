// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once
#include "py_headers.h"

#include <cassert>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <vector>

namespace py = pybind11;

template <size_t bit_index0, size_t bit_index1>
std::vector<int> ExtractBits(const std::vector<uint64_t> &data, int dr = 24) {
    constexpr int mask0 = (1 << bit_index0);
    constexpr int mask1 = (1 << bit_index1);
    constexpr int NumStrips = 2;
    const int NumCompleteSamples = data.size() / dr;
    const int NumCounters = NumCompleteSamples * NumStrips;

    std::vector<int> result(NumCounters);

    auto ptr = data.data();

    auto strip0 = result.data();
    auto strip1 = strip0 + NumCompleteSamples;

    for (int j = 0; j != NumCompleteSamples; ++j) {
        for (int i = 0; i != dr; ++i) {
            int bit0 = (*ptr & mask0) >> bit_index0;
            int bit1 = (*ptr++ & mask1) >> bit_index1;
            *strip0 |= bit0 << i;
            *strip1 |= bit1 << i;
        }
        strip0++;
        strip1++;
    }
    return result;
}

template <size_t bit_index>
std::vector<int> ExtractBits(const std::vector<uint64_t> &data, int dr = 24) {
    constexpr int mask = (1 << bit_index);
    const int NumCompleteSamples = data.size() / dr;
    std::vector<int> result(NumCompleteSamples);
    auto ptr = data.data();
    for (auto &r : result) {
        for (int i = 0; i != dr; ++i) {
            int bit = (*ptr++ & mask) >> bit_index;
            r |= bit << i;
        }
    }
    return result;
}

std::vector<uint64_t> ReadFile(const std::string &fname, int byte_offset = 8,
                               int dr = 24) {
    const int element_size = static_cast<int>(sizeof(uint64_t));
    // const int byte_offset = element_size * offset;
    const int expected_size = dr * element_size * 32 * 3;
    std::ifstream fs(fname, std::ios::binary | std::ios::ate);
    if (!fs.is_open()) {
        throw std::runtime_error("File not found: " + std::string(fname));
    }
    auto data_size = static_cast<long>(fs.tellg()) - byte_offset;
    if (data_size != expected_size) {
        auto diff = data_size - expected_size;
        std::cout << "WARNING: data size is: " << data_size
                  << " expected size is: " << expected_size << ", "
                  << std::abs(diff) << " bytes "
                  << ((diff < 0) ? "missing" : "too many") << '\n';
    }
    std::vector<uint64_t> data(expected_size / element_size);
    fs.seekg(byte_offset, std::ios::beg);
    fs.read(reinterpret_cast<char *>(data.data()), expected_size);
    return data;
}

py::array_t<uint64_t> read_my302_file(const std::string &fname, int offset = 8,
                                      int dr = 24) {
    auto data = ExtractBits<17, 6>(ReadFile(fname, offset, dr));
    return py::array(data.size(), data.data());
}
