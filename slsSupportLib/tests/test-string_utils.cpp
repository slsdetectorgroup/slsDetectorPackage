#include "MySocketTCP.h"
#include "catch.hpp"
#include "logger.h"
#include <iostream>
#include <vector>

#include "string_utils.h"


TEST_CASE("copy a string") {

    char src[10] = "hej";
    REQUIRE(src[3]=='\0');

    char dst[20];

    sls::strcpy_safe(dst, src);
    REQUIRE(dst[0]=='h');
    REQUIRE(dst[1]=='e');
    REQUIRE(dst[2]=='j');
    REQUIRE(dst[3]=='\0');

}

	
#ifdef NDEBUG
//This test can only run in release since we assert on the length of the string
TEST_CASE("copy a long string"){
    auto src = "some very very long sting that does not fit";
    char dst[3];
    sls::strcpy_safe(dst, src);
    REQUIRE(dst[0]=='s');
    REQUIRE(dst[1]=='o');
    REQUIRE(dst[2]=='\0');
    
}
#endif
TEST_CASE("Concat") {
  std::vector<std::string> v{"one", "one", "one"};
  std::vector<std::string> v2{"one", "one", "one"};
  auto r = sls::concatenateIfDifferent(v);
  REQUIRE(r == std::string("one"));
  r.clear();

  // make sure we didn't modify the string
  REQUIRE(v == v2);

  SECTION("add a different value"){
      v.emplace_back("two");
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


TEST_CASE("Remove char from string"){
  char str[] = "sometest"; 
  sls::removeChar(str, 'e');
  REQUIRE(std::string(str) == "somtst");
}

TEST_CASE("Remove char from empty string"){
  char str[50] = {}; 
  sls::removeChar(str, 'e');
  REQUIRE(std::string(str) == "");
}

TEST_CASE("Many characters in a row"){
  char str[] = "someeequitellll::ongstring"; 
  sls::removeChar(str, 'l');
  REQUIRE(std::string(str) == "someeequite::ongstring");
}

// TEST_CASE("concat things not being strings")