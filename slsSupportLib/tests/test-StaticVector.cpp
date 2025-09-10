// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "catch.hpp"
#include "sls/StaticVector.h"
#include "sls/TypeTraits.h"

#include <array>
#include <sstream>
#include <vector>

using sls::StaticVector;

TEST_CASE("StaticVector is a container") {
    REQUIRE(sls::is_container<StaticVector<int, 7>>::value == true);
}

TEST_CASE("Comparing StaticVector containers") {
    StaticVector<int, 5> a{0, 1, 2};
    StaticVector<int, 5> b{0, 1, 2};
    StaticVector<int, 5> c{0, 1, 2, 4};

    REQUIRE(a == b);
    REQUIRE_FALSE(a != b);
    REQUIRE_FALSE(a == c);
    REQUIRE(a != c);
    REQUIRE(c != a);
    REQUIRE_FALSE(c == a);
    REQUIRE_FALSE(b == c);
}

TEST_CASE("Compare array and fixed capacity container") {
    std::array<int, 3> arr{1, 2, 3};
    std::array<int, 3> arr2{1, 7, 3};
    StaticVector<int, 7> fcc{1, 2, 3};
    REQUIRE(fcc == arr);
    REQUIRE(arr == fcc);
    REQUIRE_FALSE(fcc != arr);
    REQUIRE_FALSE(arr != fcc);
    REQUIRE_FALSE(fcc == arr2);
    REQUIRE_FALSE(arr2 == fcc);
}

TEST_CASE("Compare vector and fixed capacity container") {
    std::vector<int> vec{1, 2, 3};
    std::vector<int> vec2{10, 2, 3};
    StaticVector<int, 7> fcc{1, 2, 3};
    REQUIRE(fcc == vec);
    REQUIRE(vec == fcc);
    REQUIRE_FALSE(fcc != vec);
    REQUIRE_FALSE(vec != fcc);
    REQUIRE_FALSE(fcc == vec2);
    REQUIRE_FALSE(vec2 == fcc);
}

TEST_CASE("Construct from vector") {
    std::vector<int> vec{1, 2, 3};
    StaticVector<int, 5> fcc{vec};
    REQUIRE(fcc == vec);
}

TEST_CASE("Copy construct from vector") {
    std::vector<int> vec{1, 2, 3};
    StaticVector<int, 5> fcc = vec;
    REQUIRE(fcc == vec);
}

TEST_CASE("Copy assignment from vector") {
    std::vector<int> vec{1, 2, 3};
    StaticVector<int, 5> fcc;
    fcc = vec;
    REQUIRE(fcc == vec);
}

TEST_CASE("Construct from array") {
    std::array<int, 3> arr{1, 2, 3};
    StaticVector<int, 5> fcc{arr};
    REQUIRE(fcc == arr);
}

TEST_CASE("Copy assign from array") {
    std::array<int, 3> arr{1, 2, 3};
    StaticVector<int, 5> fcc;
    fcc = arr;
    REQUIRE(fcc == arr);
}

TEST_CASE("Copy construct from array") {
    std::array<int, 3> arr{1, 2, 3};
    StaticVector<int, 5> fcc = arr;
    REQUIRE(fcc == arr);
}

TEST_CASE("Construct from a smaller StaticVector") {
    StaticVector<int, 3> sv{1, 2, 3};
    StaticVector<int, 5> sv2{sv};
    REQUIRE(sv == sv2);
}

TEST_CASE("Free function and method gives the same iterators") {
    StaticVector<int, 3> fcc{1, 2, 3};
    REQUIRE(std::begin(fcc) == fcc.begin());
}

SCENARIO("StaticVectors can be sized and resized", "[support]") {

    GIVEN("A default constructed container") {
        constexpr size_t n_elem = 5;
        StaticVector<int, n_elem> vec;

        REQUIRE(vec.empty());
        REQUIRE(vec.size() == 0); // NOLINT
        REQUIRE(vec.capacity() == n_elem);
        REQUIRE(sizeof(vec) == sizeof(int) * n_elem + sizeof(size_t));

        WHEN("an item is pushed back") {
            vec.push_back(42);

            THEN("the element is stored and the size increases but the "
                 "capacity remains fixed") {
                REQUIRE(vec[0] == 42);
                REQUIRE(vec.size() == 1);
                REQUIRE_FALSE(vec.empty());
                REQUIRE(vec.capacity() == 5);
            }
        }

        WHEN("We try to push back more elements than the capcity") {
            for (size_t i = 0; i != vec.capacity(); ++i)
                vec.push_back(static_cast<int>(i));
            THEN("It throws") { REQUIRE_THROWS(vec.push_back(92)); }
        }

        WHEN("a vector is assigned to the fixed container") {
            std::vector<int> standard_vector{1, 2, 8, 0};
            vec = standard_vector;
            THEN("their values and size match") {
                REQUIRE(vec.size() == standard_vector.size());
                for (size_t i = 0; i != vec.size(); ++i) {
                    REQUIRE(vec[i] == standard_vector[i]);
                }
            }
        }
    }

    GIVEN("A StaticVector constructed form a std::initializer list") {
        StaticVector<int, 10> vec{23, 52, 11};
        REQUIRE(vec.size() == 3);
        REQUIRE(vec.capacity() == 10);
        REQUIRE(vec[0] == 23);
        REQUIRE(vec[1] == 52);
        REQUIRE(vec[2] == 11);

        WHEN("The container is resized to a smaller size") {
            vec.resize(2);
            THEN("The size changes but not the values or capacity") {
                REQUIRE(vec.size() == 2);
                REQUIRE(vec[0] == 23);
                REQUIRE(vec[1] == 52);
            }
        }

        WHEN("The container is resized to a larger size") {
            vec.resize(7);
            THEN("The size changes but not the values") {
                REQUIRE(vec.size() == 7);
                REQUIRE(vec[0] == 23);
                REQUIRE(vec[1] == 52);
                REQUIRE(vec[2] == 11);
            }
        }

        WHEN("We try to resize beyond the capacity") {
            THEN("it throws") { CHECK_THROWS(vec.resize(25)); }
        }
        WHEN("We call front and back") {
            THEN("They return referenced to the first and last element") {
                REQUIRE(vec.front() == 23);
                REQUIRE(&vec.front() == &vec[0]);
                REQUIRE(vec.back() == 11);
                REQUIRE(&vec.back() == &vec[2]);
            }
        }
    }

    GIVEN("An std::vector of size 3") {
        std::vector<int> standard_vector{5, 2, 1};
        WHEN("we construct a fixed capacity container from it") {
            StaticVector<int, 5> vec(standard_vector);
            THEN("size and data matches") {
                REQUIRE(vec.size() == 3);
                REQUIRE(vec[0] == 5);
                REQUIRE(vec[1] == 2);
                REQUIRE(vec[2] == 1);
            }
            THEN("we can compare the vector and fixed container") {
                REQUIRE(vec == standard_vector);
            }
        }
    }
}

SCENARIO("Comparison of StaticVectors", "[support]") {
    GIVEN("Two containers containers that are equal at the start") {
        StaticVector<int, 5> a{0, 1, 2};
        StaticVector<int, 5> b{0, 1, 2};
        REQUIRE(a == b);
        REQUIRE_FALSE(a != b);

        WHEN("We push back one element") {
            a.push_back(4);
            THEN("they are not equal anymore") { REQUIRE(a != b); }
        }
        WHEN("Compared to a StaticVector with different capacity") {
            StaticVector<int, 8> c{0, 1, 2};
            THEN("The comparison still holds") {
                REQUIRE(a == c);
                REQUIRE_FALSE(a != c);
            }
        }
        WHEN("we make a copy of one container") {
            auto c = a;
            THEN("The comparison holds") {
                REQUIRE(c == a);
                REQUIRE(&c != &a);
                REQUIRE(&a[0] != &c[0]);
            }
        }
        WHEN("Compared to an std::vector") {
            std::vector<int> standard_vector{0, 1, 2};
            THEN("The comparison also holds for both orderings") {
                REQUIRE(a == standard_vector);
                REQUIRE(standard_vector == a);
                REQUIRE_FALSE(a != standard_vector);
                REQUIRE_FALSE(standard_vector != a);

                standard_vector.push_back(3);
                REQUIRE(a != standard_vector);
            }
        }
    }
}

SCENARIO("Sorting, removing and other manipulation of a container",
         "[support]") {
    GIVEN("An unsorted container") {
        StaticVector<int, 5> a{14, 12, 90, 12};
        WHEN("We sort it") {
            std::sort(a.begin(), a.end());
            THEN("Elements appear sorted") {
                REQUIRE(a[0] == 12);
                REQUIRE(a[1] == 12);
                REQUIRE(a[2] == 14);
                REQUIRE(a[3] == 90);
            }
        }
        WHEN("Sorting is done using free function for begin and end") {
            std::sort(std::begin(a), std::end(a));
            THEN("it also works") {
                REQUIRE(a[0] == 12);
                REQUIRE(a[1] == 12);
                REQUIRE(a[2] == 14);
                REQUIRE(a[3] == 90);
            }
        }
        WHEN("Erasing elements of a certain value") {
            a.erase(std::remove(std::begin(a), std::end(a), 12));
            THEN("all elements of that value are removed") {
                REQUIRE(a.size() == 2);
                REQUIRE(a[0] == 14);
                REQUIRE(a[1] == 90);
            }
        }
    }
}

SCENARIO("Assigning containers to each other", "[support]") {
    GIVEN("Two containers") {
        StaticVector<int, 3> a{1, 2, 3};
        StaticVector<int, 3> b{4, 5, 6};
        WHEN("a is assigned to b") {
            a = b;
            THEN("A deep copy is made and both containers are equal") {
                REQUIRE(a == b);
                REQUIRE(&a != &b);
                REQUIRE(&a[0] != &b[0]);
                REQUIRE(a[0] == 4);
                REQUIRE(a[1] == 5);
                REQUIRE(a[2] == 6);
            }
        }
        WHEN("A new object is create from an old one") {
            StaticVector<int, 3> c(a);
            THEN("A deep copy is also made") {
                REQUIRE(c == a);
                REQUIRE(&c != &a);
                REQUIRE(&c[0] != &a[0]);
                REQUIRE(c[0] == 1);
                REQUIRE(c[1] == 2);
                REQUIRE(c[2] == 3);
            }
        }
        WHEN("We create a const StaticVector") {
            const StaticVector<int, 5> c(a);
            THEN("The values are still the same using const operators") {
                REQUIRE(c[0] == 1);
                REQUIRE(c[1] == 2);
                REQUIRE(c[2] == 3);
                REQUIRE(c.front() == 1);
                REQUIRE(c.back() == 3);
            }
        }
    }
}

SCENARIO("Converting to vector", "[support]") {
    GIVEN("a StaticVector") {
        StaticVector<int, 5> a{1, 2, 3};
        WHEN("Converted into a vector") {
            std::vector<int> b(a);
            THEN("Data and size matches") {
                REQUIRE(a == b);
                REQUIRE(a.size() == b.size());
            }
        }
    }
}

TEST_CASE("StaticVector") {
    StaticVector<int, 5> vec;
    vec.push_back(3);
    vec.push_back(8);
    REQUIRE(ToString(vec) == "[3, 8]");
}

TEST_CASE("StaticVector stream") {
    StaticVector<int, 5> vec;
    vec.push_back(33);
    vec.push_back(85667);
    vec.push_back(2);
    std::ostringstream oss;
    oss << vec;
    REQUIRE(oss.str() == "[33, 85667, 2]");
}
