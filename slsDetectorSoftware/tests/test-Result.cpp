#include "Result.h"
#include "TypeTraits.h"
#include "catch.hpp"
#include <string>

using sls::Result;

TEST_CASE("Result looks and behaves like a standard container") {
    REQUIRE(sls::is_container<Result<int>>::value == true);
}

TEST_CASE("Default construction is possible and gives an empty result") {
    Result<int> res;
    REQUIRE(res.size() == 0);
    REQUIRE(res.empty() == true);
}

TEST_CASE("Result can be constructed from std::initializer_list") {
    Result<int> res{1, 2, 5};
    REQUIRE(res.empty() == false);
    REQUIRE(res.size() == 3);
    REQUIRE(res[0] == 1);
    REQUIRE(res[1] == 2);
    REQUIRE(res[2] == 5);
}

TEST_CASE("Like vector it can be constructed from size and value") {
    Result<int> res(5, 7);
    REQUIRE(res.size() == 5);
    REQUIRE(res[0] == 7);
    REQUIRE(res[1] == 7);
    REQUIRE(res[2] == 7);
    REQUIRE(res[3] == 7);
    REQUIRE(res[4] == 7);
}

TEST_CASE("Result can be iterated using modern syntax"){
    Result<int> res{0,1,2,3,4,5};

    int i = 0;
    for (const auto& r:res)
        REQUIRE(r == i++);

}

TEST_CASE("Calling squash on an empty Result produces default value") {
    Result<double> res;
    REQUIRE(res.squash() == 0.);

    Result<unsigned> res2;
    REQUIRE(res2.squash() == 0u);

    Result<std::string> res3;
    REQUIRE(res3.squash() == "");
}

TEST_CASE("When equal squash gives the front value") {
    Result<int> res{3, 3, 3};
    REQUIRE(res.squash() == 3);
}

TEST_CASE("When elements are not equal squash gives default value") {
    Result<int> res{3, 3, 3, 5};
    REQUIRE(res.squash() == 0);
}

TEST_CASE("String compare with squash") {
    Result<std::string> res{"hej", "hej", "hej"};
    REQUIRE(res.squash() == "hej");
}

TEST_CASE("tsquash throws for different elements") {
    Result<int> res{1, 2, 3};
    REQUIRE_THROWS(res.tsquash("something is wrong"));
}

TEST_CASE("tsquash returns front element when equal") {
    Result<std::string> res{"word", "word"};
    REQUIRE(res.tsquash("error") == "word");
}

TEST_CASE("When provided squash gives default value if elements differ") {
    Result<int> res{5, 5, 5};
    REQUIRE(res.squash(-1) == 5);

    res.push_back(7); // adding a different value
    REQUIRE(res.squash(-1) == -1);
}

TEST_CASE("It is possible to update elements by index access") {
    Result<int> res{1, 2, 3};
    res[0] = 5;
    REQUIRE(res[0] == 5);
    REQUIRE(res[1] == 2);
    REQUIRE(res[2] == 3);
}

TEST_CASE("Check if elements are equal") {
    Result<float> res;
    REQUIRE(res.equal() == false); // no elements to compare

    res.push_back(1.2); // one element "all" equal
    REQUIRE(res.equal() == true);

    res.push_back(1.2); // two elements
    REQUIRE(res.equal() == true);

    res.push_back(1.3); // three elements 1.2, 1.2, 1.3
    REQUIRE(res.equal() == false);
}

TEST_CASE("Result can be converted to std::vector") {
    Result<short> res{1, 2, 3, 4, 5};
    std::vector<short> vec{1, 2, 3, 4, 5};
    std::vector<short> vec2 = res;
    REQUIRE(vec2 == vec);
}

TEST_CASE("Result can be printed using <<") {
    Result<int> res{1, 2, 3};
    std::ostringstream os;
    os << res;
    REQUIRE(os.str() == "[1, 2, 3]");
}

TEST_CASE("Convert from Result<int> to Result<ns>") {
    // This function is used when the detector class
    // returns time as integers
    using ns = std::chrono::nanoseconds;
    Result<int> res{10, 50, 236};

    Result<std::chrono::nanoseconds> res2 = res;
    REQUIRE(res2[0] == ns(10));
    REQUIRE(res2[1] == ns(50));
    REQUIRE(res2[2] == ns(236));
}
