#include "catch.hpp"
#include "MaskGenerator.h"

using sls::MaskGenerator;
using sls::Container3;

TEST_CASE("Default construction gives an mask of size 0") {
    auto m = MaskGenerator().mask();
    CHECK(m.shape() == std::array<size_t, 3>{0, 0, 0});
    CHECK(m.size() == 0);
}

TEST_CASE("Default behaviour with shape is all true and same shape") {
    Container3<int> c{1, 2, 3};
    auto m = MaskGenerator().mask(c.shape());

    CHECK(m.shape() == std::array<size_t, 3>{1, 2, 3});
    for (auto &i : m)
        CHECK(i == true);
}

TEST_CASE("With std::vector we give back the first index and 0, 0") {

    Container3<int> rec{14, 1, 3};

    auto m = MaskGenerator(std::vector<size_t>{0, 3, 5}).mask(rec.shape());
    CHECK(m.shape() == std::array<size_t, 3>{14, 1, 3});

    CHECK(m(0, 0, 0) == true);
    CHECK(m(3, 0, 0) == true);
    CHECK(m(5, 0, 0) == true);

    std::vector<size_t> positions(rec.size(0));
    std::iota(begin(positions), end(positions), 0);
    positions.erase(std::remove(positions.begin(), positions.end(), 0),
                    positions.end());
    positions.erase(std::remove(positions.begin(), positions.end(), 3),
                    positions.end());
    positions.erase(std::remove(positions.begin(), positions.end(), 5),
                    positions.end());

    for (auto i : positions) {
        for (size_t j = 0; j < rec.size(1); ++j) {
            for (size_t k = 0; k < rec.size(2); ++k) {
                REQUIRE(m(i, j, k) == false);
            }
        }
    }
}

TEST_CASE("With single number we get that detector x,0,0") {
    Container3<int> rec{2, 2, 1};
    auto m = MaskGenerator(1).mask(rec);
    CHECK(m(1, 0, 0) == true);
    CHECK(m(1, 1, 0) == false);
    CHECK(m(0, 1, 0) == false);
    CHECK(m(0, 0, 0) == false);
}

TEST_CASE("With two numbers we get x,y,0") {
    Container3<int> rec{2, 2, 1};
    auto m = MaskGenerator(1, 1).mask(rec);
    CHECK(m(1, 1, 0) == true);
    CHECK(m(1, 0, 0) == false);
    CHECK(m(0, 1, 0) == false);
    CHECK(m(0, 0, 0) == false);
}

TEST_CASE("With three numbers we get x,y,z") {
    Container3<int> rec{9, 7, 5};
    auto m = MaskGenerator(3, 4, 1).mask(rec);
    REQUIRE(m.shape() == rec.shape());
    REQUIRE(m(3, 4, 1) == true);

    for (size_t i = 0; i < rec.size(0); ++i) {
        for (size_t j = 0; j < rec.size(1); ++j) {
            for (size_t k = 0; k < rec.size(2); ++k) {
                if (!(i == 3 && j == 4 && k == 1)) {
                    REQUIRE(m(i, j, k) == false);
                }
            }
        }
    }
}