#include "catch.hpp"
#include "multiSlsDetector.h"

#include <iostream>
TEST_CASE("Initialize a detector") {
multiSlsDetector det(0, true, true);
  std::cout << "Size: " << det.getNumberOfDetectors() << std::endl;
  std::cout << "Hostname: " << det.getHostname() << std::endl;
  REQUIRE(false);
  
}