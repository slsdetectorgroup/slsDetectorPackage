// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once

#include <bitset>
#include <vector>
namespace sls {
template <typename T> std::vector<int> getSetBits(T val) {
    constexpr size_t bitsPerByte = 8;
    constexpr size_t numBits = sizeof(T) * bitsPerByte;
    std::bitset<numBits> bs(val);
    std::vector<int> set_bits;
    set_bits.reserve(bs.count());
    for (size_t i = 0; i < bs.size(); ++i) {
        if (bs[i]) {
            set_bits.push_back(static_cast<int>(i));
        }
    }
    return set_bits;
}
} // namespace sls
