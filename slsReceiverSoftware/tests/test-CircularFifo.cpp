#include "sls/CircularFifo.h"
#include "catch.hpp"
#include <vector>

using sls::CircularFifo;

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
        CHECK(fifo.getDataValue() == i + 1);
        CHECK(fifo.getFreeValue() == 4 - i);
    }

    CHECK(fifo.isEmpty() == false);
    CHECK(fifo.isFull() == true);

    for (size_t i = 0; i != vec.size(); ++i) {
        fifo.pop(p);
        CHECK(*p == vec[i]);
        CHECK(fifo.getDataValue() == 4 - i);
        CHECK(fifo.getFreeValue() == i + 1);
    }

    CHECK(fifo.isEmpty() == true);
    CHECK(fifo.isFull() == false);
}