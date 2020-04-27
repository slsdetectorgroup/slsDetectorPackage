
#include "catch.hpp"
#include "Container3.h"
using sls::Container3;

TEST_CASE("Default construction gives container of size 0") {
    Container3<int> c;
    CHECK(c.size() == 0);
    CHECK(c.size(0) == 0);
    CHECK(c.size(1) == 0);
    CHECK(c.size(2) == 0);
}

TEST_CASE("Construct container with size") {

    Container3<int> c(3, 4, 5);
    CHECK(c.size() == 3 * 4 * 5);
    CHECK(c.size(0) == 3);
    CHECK(c.size(1) == 4);
    CHECK(c.size(2) == 5);
}

TEST_CASE("Constructor with default value") {
    constexpr int val = 7;
    Container3<int> c({1, 1, 5}, val);
    for (size_t i = 0; i < c.size(); ++i) {
        CHECK(c[i] == val);
    }
}

TEST_CASE("Container3 can be iterated with range for") {
    constexpr int val = 7;
    Container3<int> c({1, 1, 5}, val);
    for (const auto& item : c) {
        CHECK(item == val);
    }
}

TEST_CASE(
    "() gives access to an element for both const and non const objects") {
    Container3<int> c{1, 1, 1};
    CHECK(c(0, 0, 0) == 0);
    const Container3<int> c2{1, 1, 1};
    CHECK(c2(0, 0, 0) == 0);
}

TEST_CASE("() can be used to modify object") {
    Container3<int> c{1, 1, 1};
    c(0, 0, 0) = 7;
    CHECK(c(0, 0, 0) == 7);
}

TEST_CASE("at() can be used to modify object") {
    Container3<int> c{1, 1, 1};
    c.at(0, 0, 0) = 7;
    CHECK(c(0, 0, 0) == 7);
}

TEST_CASE("at throws outsize range for both const and non const objects") {
    Container3<int> c{1, 1, 1};
    const Container3<double> c2{1, 1, 1};

    CHECK_THROWS(c.at(5, 5, 5));
    CHECK_THROWS(c2.at(5, 5, 5));
}

TEST_CASE("Set values") {
    Container3<int> c(2, 3, 4);
    size_t count = 0;
    for (size_t i = 0; i < c.size(0); ++i) {
        for (size_t j = 0; j < c.size(1); ++j) {
            for (size_t k = 0; k < c.size(2); ++k) {
                c(i, j, k) = count;
                CHECK(c(i, j, k) == count);
                CHECK(c[count] == count);
                ++count;
            }
        }
    }
}

TEST_CASE("Check if index is valid") {
    Container3<int> c(2, 3, 4);

    CHECK(c.is_valid_index(1,1,1));
    CHECK(c.is_valid_index(1,2,3));
    CHECK(c.is_valid_index(1,2,3));

    CHECK_FALSE(c.is_valid_index(1,7,1));
    CHECK_FALSE(c.is_valid_index(1,1,4));
    CHECK_FALSE(c.is_valid_index(3,1,1));

    
}

TEST_CASE("Copy data from one container to another") {
    Container3<int> c(2, 1, 2);
    for (size_t i = 0; i < c.size(); ++i) {
        c[i] = i;
    }
    Container3<int> c2(3, 3, 3);
    c2.copy_data(c);

    for (size_t i = 0; i < c.size(0); ++i) {
        for (size_t j = 0; j < c.size(1); ++j) {
            for (size_t k = 0; k < c.size(2); ++k) {
                CHECK(c2(i, j, k) == c(i, j, k));
            }
        }
    }
}

TEST_CASE("Copy assignment copies values") {
    Container3<int> c(2, 3, 4);
    size_t count = 0;
    for (size_t i = 0; i < c.size(0); ++i) {
        for (size_t j = 0; j < c.size(1); ++j) {
            for (size_t k = 0; k < c.size(2); ++k) {
                c(i, j, k) = count;
                CHECK(c(i, j, k) == count);
                CHECK(c[count] == count);
                ++count;
            }
        }
    }

    Container3<int> c2;

    c2 = c;
    count = 0;
    for (size_t i = 0; i < c.size(0); ++i) {
        for (size_t j = 0; j < c.size(1); ++j) {
            for (size_t k = 0; k < c.size(2); ++k) {
                c2(i, j, k) = count;
                CHECK(c(i, j, k) == count);
                CHECK(c[count] == count);
                ++count;
            }
        }
    }
}

TEST_CASE("Copy constructor copies values") {
    Container3<int> c(18, 23, 4);
    size_t count = 0;
    for (size_t i = 0; i < c.size(0); ++i) {
        for (size_t j = 0; j < c.size(1); ++j) {
            for (size_t k = 0; k < c.size(2); ++k) {
                c(i, j, k) = count;
                CHECK(c(i, j, k) == count);
                CHECK(c[count] == count);
                ++count;
            }
        }
    }

    Container3<int> c2 = c;

    count = 0;
    for (size_t i = 0; i < c.size(0); ++i) {

        for (size_t j = 0; j < c.size(1); ++j) {
            for (size_t k = 0; k < c.size(2); ++k) {
                c2(i, j, k) = count;
                CHECK(c(i, j, k) == count);
                CHECK(c[count] == count);
                ++count;
            }
        }
    }
}

TEST_CASE("Copy assignment create disjoint objects") {
    Container3<int> c(2, 3, 4);
    Container3<int> c2;
    c2 = c;
    c2(1, 1, 1) = 72;
    CHECK(c2(1, 1, 1) == 72);
    CHECK(c(1, 1, 1) != 72);
}

TEST_CASE("Move constructor") {
    Container3<int> c(2, 3, 4);

    // explicit move only for testing
    Container3<int> c2(std::move(c));

    // Moved from object is in an ok state
    CHECK(c.data() == nullptr);
    for (size_t i = 0; i < 3; ++i) {
        CHECK(c.size(i) == 0);
    }

    // new object has the correct size and owns some data
    CHECK(c2.data() != nullptr);
    CHECK(c2.size(0) == 2);
    CHECK(c2.size(1) == 3);
    CHECK(c2.size(2) == 4);
}

TEST_CASE("Move assignment ") {
    Container3<int> c(2, 3, 4);

    Container3<int> c2(5, 5, 5);

    c2 = std::move(c);
    // Moved from object is in an ok state
    CHECK(c.data() == nullptr);
    for (size_t i = 0; i < 3; ++i) {
        CHECK(c.size(i) == 0);
    }

    // new object has the correct size and owns some data
    CHECK(c2.data() != nullptr);
    CHECK(c2.size(0) == 2);
    CHECK(c2.size(1) == 3);
    CHECK(c2.size(2) == 4);
}

TEST_CASE("Resize to a larger size") {
    Container3<int> c(2, 3, 4);

    // Assign values
    auto shape = c.shape();
    CHECK(shape == std::array<size_t, 3>{2,3,4});

    size_t count = 0;
    for (size_t i = 0; i < shape[0]; ++i) {
        for (size_t j = 0; j < shape[1]; ++j) {
            for (size_t k = 0; k < shape[2]; ++k) {
                c(i, j, k) = count;
                CHECK(c(i, j, k) == count);
                ++count;
            }
        }
    }

    c.resize(3, 4, 5);
    CHECK(c.shape() == std::array<size_t, 3>{3,4,5});

    // Values should remain the same in the old region
    count = 0;
    for (size_t i = 0; i < shape[0]; ++i) {
        for (size_t j = 0; j < shape[1]; ++j) {
            for (size_t k = 0; k < shape[2]; ++k) {
                CHECK(c(i, j, k) == count);
                ++count;
            }
        }
    }

    // Default constructed values outsize

    CHECK(c(2, 2, 2) == 0);
    CHECK(c(2, 3, 2) == 0);
}

TEST_CASE("Inserting values outside range with a empty receiver") {
    Container3<int> c;
    c.at_can_grow(0, 0, 0) = 5;

    CHECK(c(0, 0, 0) == 5);
    CHECK(c.size() == 1);
    CHECK(c.size(0) == 1);
    CHECK(c.size(1) == 1);
    CHECK(c.size(2) == 1);

    c.at_can_grow(0, 0, 1) = 6;
    CHECK(c.size() == 2);
    CHECK(c.size(0) == 1);
    CHECK(c.size(1) == 1);
    CHECK(c.size(2) == 2);
}

TEST_CASE("Inserting a value outside of the current size") {
    Container3<int> c{1, 2, 3};
    for (size_t i = 0; i < c.size(); ++i) {
        c[i] = i;
    }

    Container3<int> copy;
    copy = c;

    c.at_can_grow(2, 2, 2) = 7;

    CHECK(c.size(0) == 3);
    CHECK(c.size(1) == 3);
    CHECK(c.size(2) == 3);

    for (size_t i = 0; i < copy.size(0); ++i) {
        for (size_t j = 0; j < copy.size(0); ++j) {
            for (size_t k = 0; k < copy.size(0); ++k) {
                CHECK(copy(i, j, k) == c(i, j, k));
            }
        }
    }
}

TEST_CASE("Clear sets size to zero and clears memory"){
    Container3<int> c(5,5,5);
    CHECK(c.shape() == std::array<size_t, 3>{5,5,5});

    c.clear();
    CHECK(c.shape() == std::array<size_t, 3>{0,0,0});
    CHECK(c.size() == 0);
}

TEST_CASE("Put unique pointer in Container3"){
    Container3<std::unique_ptr<int>> c(3,1,1);
    c(0,0,0) = std::unique_ptr<int>(new int);

    CHECK(c(0,0,0) != nullptr);
    CHECK(c(1,0,0) == nullptr);
    CHECK(c(2,0,0) == nullptr);

    *c(0,0,0) = 5;
    CHECK(*c(0,0,0) == 5);
}

TEST_CASE("Resize with unique ptr"){
    Container3<std::unique_ptr<int>> c(2,1,1);
    c(1,0,0) = std::unique_ptr<int>(new int);
    *c(1,0,0) =  7;

    c.resize(3,1,1);
    CHECK(c.size() == 3);
    CHECK(*c(1,0,0) == 7);


}
