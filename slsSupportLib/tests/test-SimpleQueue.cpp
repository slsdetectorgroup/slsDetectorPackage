#include "catch.hpp"

#include "sls/SimpleQueue.h"
using sls::SimpleQueue;

#include <vector>


TEST_CASE("Push fails on full queue"){
    uint32_t size = 3;
    SimpleQueue<double> q(size);

    double a = 5.3;
    REQUIRE(q.push(a));
    REQUIRE(q.push(a));
    REQUIRE(q.push(a));
    REQUIRE_FALSE(q.push(a));

}


TEST_CASE("Pop fails on empty queue"){
    uint32_t size = 3;
    SimpleQueue<double> q(size);

    double a = 5.3;
    REQUIRE_FALSE(q.pop(a));

}

TEST_CASE("Add and remove elements"){

    uint32_t size = 10;
    SimpleQueue<int> q(size);

    std::vector<int> ints{5,6,8,1,3};

    for (auto& i : ints)
        q.push(i);

    REQUIRE(q.sizeGuess() == 5);

    int r = 0;
    REQUIRE(q.pop(r));
    REQUIRE(r == 5);
    REQUIRE(q.sizeGuess() == 4);

    REQUIRE(q.pop(r));
    REQUIRE(r == 6);
    REQUIRE(q.sizeGuess() == 3);

    REQUIRE(q.pop(r));
    REQUIRE(r == 8);
    REQUIRE(q.sizeGuess() == 2);

    REQUIRE(q.pop(r));
    REQUIRE(r == 1);
    REQUIRE(q.sizeGuess() == 1);

    REQUIRE(q.pop(r));
    REQUIRE(r == 3);
    REQUIRE(q.sizeGuess() == 0);

    REQUIRE_FALSE(q.pop(r));


}