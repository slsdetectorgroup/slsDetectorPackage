
#include "catch.hpp"
#include "container_utils.h"
#include "slsDetector.h"
#include "string_utils.h"

#include <iostream>

using namespace sls;

TEST_CASE("Set and get trimen", "[detector]") {
    // Free shared memory to be sure that we start in a clean state
    slsDetector::freeSharedMemory(0, 0);

    // Create a detector and check that the type is set correctly
    slsDetector d(slsDetectorDefs::detectorType::EIGER, 0, 0);
    CHECK(d.getDetectorTypeAsEnum() == slsDetectorDefs::detectorType::EIGER);

    // At the beginning there should be no trimen set
    auto res = d.getTrimEn();
    CHECK(res.size() == 0);

    std::vector<int> energies{5200, 6400, 8500, 9900, 12000};
    d.setTrimEn(energies);
    auto res2 = d.getTrimEn();

    // Check that the size and every element matches what we set
    CHECK(res2.size() == energies.size());
    for (size_t i = 0; i != res2.size(); ++i)
        CHECK(res2[i] == energies[i]);

    

    // Setting trimen with too many vales throws an exception and keeps the
    // old values
    std::vector<int> too_many(150, 1000);
    CHECK_THROWS(d.setTrimEn(too_many));
    auto res3 = d.getTrimEn();
    CHECK(res3.size() == energies.size());
    for (size_t i = 0; i != res3.size(); ++i)
        CHECK(res3[i] == energies[i]);

    // Setting trimen without arguments resets to zero
    d.setTrimEn();
    CHECK(d.getTrimEn().size() == 0);

    // Clean up before next test
    d.freeSharedMemory();
}
