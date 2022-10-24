// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "catch.hpp"
#include "sls/CircularFifo.h"
#include <vector>

namespace sls {

TEST_CASE("Empty buffer") {
    CircularFifo<char> fifo(0);

    // Since the fifo can hold zero elements
    // its both empty and full
    CHECK(fifo.isEmpty() == true);
    CHECK(fifo.isFull() == true);

    // push fails
    char *c = new char;
    *c = 'h';
    CHECK(fifo.push(c, true) == false);

    // pop fails
    CHECK(fifo.pop(c, true) == false);

    delete c;
}

TEST_CASE("Push pop") {
    CircularFifo<int> fifo(5);

    std::vector<int> vec{3, 7, 12, 3, 4};
    int *p = &vec[0];

    for (size_t i = 0; i != vec.size(); ++i) {
        fifo.push(p);
        ++p;
        CHECK(fifo.getDataValue() == (int)(i + 1));
        CHECK(fifo.getFreeValue() == (int)(4 - i));
    }

    CHECK(fifo.isEmpty() == false);
    CHECK(fifo.isFull() == true);

    for (size_t i = 0; i != vec.size(); ++i) {
        fifo.pop(p);
        CHECK(*p == vec[i]);
        CHECK(fifo.getDataValue() == (int)(4 - i));
        CHECK(fifo.getFreeValue() == (int)(i + 1));
    }

    CHECK(fifo.isEmpty() == true);
    CHECK(fifo.isFull() == false);
}

} // namespace sls
