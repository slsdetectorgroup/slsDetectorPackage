#include "MySocketTCP.h"
#include "catch.hpp"
// #include "multiSlsDetector.h"
#include "logger.h"
#include <iostream>
#include <vector>

#include "string_utils.h"

#define VERBOSE

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

TEST_CASE("copy a long string"){
    auto src = "some very very long sting that does not fit";
    char dst[3];
    sls::strcpy_safe(dst, src);
    REQUIRE(dst[0]=='s');
    REQUIRE(dst[1]=='o');
    REQUIRE(dst[3]=='\0');
}
