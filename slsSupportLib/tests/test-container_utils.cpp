#include "catch.hpp"
#include "container_utils.h"
#include <exception>
#include <string>
#include <vector>

using sls::allEqual;
using sls::allEqualTo;
using sls::anyEqualTo;

using sls::allEqualToWithTol;
using sls::allEqualWithTol;
using sls::anyEqualToWithTol;

using sls::sum;

TEST_CASE("Equality of an empty vector") {
  std::vector<int> v;
  REQUIRE(v.empty());
  REQUIRE_FALSE(allEqual(v));
  REQUIRE_FALSE(allEqualWithTol(v, 2));
  REQUIRE_FALSE(allEqualTo(v, 5));
  REQUIRE_FALSE(anyEqualTo(v, 5));
  REQUIRE_FALSE(anyEqualToWithTol(v, 5, 1));
}

TEST_CASE("Equality of a vector with one element") {
  std::vector<int> v{5};
  REQUIRE(v.size() == 1);
  REQUIRE(allEqual(v));
  REQUIRE(allEqualWithTol(v, 1));
  REQUIRE(allEqualTo(v, 5));
  REQUIRE(allEqualToWithTol(v, 5, 2));
  REQUIRE(anyEqualTo(v, 5));
  REQUIRE(anyEqualToWithTol(v, 5, 1));
}

TEST_CASE("A larger vector of the same elements") {
  std::vector<int> v(101, 5);
  REQUIRE(v.size() == 101);
  REQUIRE(allEqual(v));
  REQUIRE(allEqualWithTol(v, 1));
  REQUIRE(allEqualTo(v, 5));
  REQUIRE(anyEqualTo(v, 5));

  SECTION(
      "Push back another element to create a vector where not all are equal") {
    v.push_back(7);
    REQUIRE(v.size() == 102);
    REQUIRE_FALSE(allEqual(v));

    REQUIRE_FALSE(allEqualWithTol(v, 1));
    REQUIRE(allEqualWithTol(v, 3));

    REQUIRE_FALSE(allEqualTo(v, 5));

    REQUIRE_FALSE(allEqualToWithTol(v, 5, 1));
    REQUIRE(allEqualToWithTol(v, 5, 3));
    REQUIRE(anyEqualTo(v, 5));
  }
}

TEST_CASE("A vector of double with different values") {
  std::vector<double> v{1.2, 2., 4.2, 4, 1.1};

  REQUIRE(allEqual(v) == false);
  REQUIRE(allEqualWithTol(v, 0.3) == false);
  REQUIRE(allEqualWithTol(v, 3.2));
}

TEST_CASE("Sum of empty vector") {
  std::vector<float> v;
  REQUIRE(sls::sum(v) == Approx(0));
}

TEST_CASE("Sum of vector") {
  std::vector<double> v{1.2, 2., 4.2, 4, 1.13};
  REQUIRE(sls::sum(v) == Approx(12.53));
}

TEST_CASE("Minus one if different") {
  std::vector<double> v;
  REQUIRE(v.empty());
  double d = -1;
  REQUIRE(sls::minusOneIfDifferent(v) == d);

  SECTION("single element") {
    v.push_back(7.3);
    REQUIRE(v.size() == 1);
    REQUIRE(sls::minusOneIfDifferent(v) == Approx(7.3));
  }
  SECTION("different elements") {
    v.push_back(7.3);
    v.push_back(1.0);
    v.push_back(62.1);
    REQUIRE(sls::minusOneIfDifferent(v) == Approx(-1.0));
  }
}

TEST_CASE("minus one does not have side effects"){
    std::vector<int> v{1,1,1};
    int i = sls::minusOneIfDifferent(v);
    REQUIRE(i==1);
    i=5;
    REQUIRE(v[0]==1);
}

TEST_CASE("Concat") {
  std::vector<std::string> v{"one", "one", "one"};
  std::vector<std::string> v2{"one", "one", "one"};
  auto r = sls::concatenateIfDifferent(v);
  REQUIRE(r == std::string("one"));
  r.clear();

  // make sure we didn't modify the string
  REQUIRE(v == v2);

  SECTION("add a different value"){
      v.push_back("two");
      REQUIRE(v!=v2);
      REQUIRE( sls::concatenateIfDifferent(v) == "one+one+one+two+");
  }
}

TEST_CASE("split a string with end delimiter"){
  std::string s("abra+kadabra+");
  auto r =sls::split(s, '+');
  REQUIRE(r.size()==2);
  REQUIRE(r[0]=="abra");
  REQUIRE(r[1]=="kadabra");
}

TEST_CASE("split a string without end delimiter"){
  std::string s("abra+kadabra+filibom");
  auto r =sls::split(s, '+');
  REQUIRE(r.size()==3);
  REQUIRE(r[0]=="abra");
  REQUIRE(r[1]=="kadabra");
  REQUIRE(r[2]=="filibom");
}

TEST_CASE("concatenate non empty strings"){
  std::vector<std::string> vec{"hej", "kalas", "", "foto"};
  REQUIRE(vec.size()==4);
  auto ret = sls::concatenateNonEmptyStrings(vec);
  REQUIRE(ret == "hej+kalas+foto+");
}

TEST_CASE("concatenate non empty strings with only emty"){
  std::vector<std::string> vec{"", "", ""};
  REQUIRE(vec.size()==3);
  auto ret = sls::concatenateNonEmptyStrings(vec);
  REQUIRE(ret.empty());
}

TEST_CASE("concatenate non empty strings with one element"){
  std::vector<std::string> vec{"", "hej", "", "", ""};
  REQUIRE(vec.size()==5);
  auto ret = sls::concatenateNonEmptyStrings(vec);
  REQUIRE(ret=="hej+");
}