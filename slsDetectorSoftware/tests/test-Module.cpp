#include "Module.h"
#include "catch.hpp"

using dt = slsDetectorDefs::detectorType;
TEST_CASE("Construction with a defined detector type") { 
    sls::Module m(dt::EIGER);
    REQUIRE(m.getDetectorType() == dt::EIGER);
}