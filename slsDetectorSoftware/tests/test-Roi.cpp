#include "catch.hpp"
#include <string>

#include <stdlib.h>

#include "DetectorImpl.h"
#include "sls/Detector.h"
#include "Module.h"

#include <fstream>

namespace sls {

TEST_CASE("ROI validation") {
    DetectorImpl det(0, false, false);
    det.addTestModule(slsDetectorDefs::JUNGFRAU);
    std::cout << "size:"<< det.size() << std::endl;
    DetectorImpl detector(0, false, false);

    SECTION("Valid non-overlapping ROIs") {
        std::vector<slsDetectorDefs::ROI> rois = {
            {0, 10, 0, 10},
            {20, 30, 0, 10},
            {0, 10, 20, 30},
            {40, 50, 0, 10}
        };
        REQUIRE_NOTHROW(DetectorImplTestHelper::testValidateRois(detector, rois));
    }

    SECTION("Overlapping ROIs should throw") {
        std::vector<slsDetectorDefs::ROI> rois = {
            {0, 10, 0, 10},
            {5, 15, 0, 10} 
        };
        REQUIRE_THROWS_AS(DetectorImplTestHelper::testValidateRois(detector, rois),
                          sls::RuntimeError);
    }

    SECTION("Invalid ROI (xmin > xmax)") {
        std::vector<slsDetectorDefs::ROI> rois = {
            {12, 8, 0, 10}
        };
        REQUIRE_THROWS_AS(DetectorImplTestHelper::testValidateRois(detector, rois),
                          sls::RuntimeError);
    }

    SECTION("Invalid ROI (ymin > ymax)") {
        std::vector<slsDetectorDefs::ROI> rois = {
            {0, 10, 20, 5}
        };
        REQUIRE_THROWS_AS(DetectorImplTestHelper::testValidateRois(detector, rois),
                          sls::RuntimeError);
    }

    SECTION("Invalid ROI (outside detector bounds)") {
        // Assuming detector has 100x100 dimensions
        detector.setNumberOfChannels({100, 100});
        std::vector<slsDetectorDefs::ROI> rois = {
            {95, 105, 0, 10} // xmax out of bounds
        };
        REQUIRE_THROWS_AS(DetectorImplTestHelper::testValidateRois(detector, rois),
                          sls::RuntimeError);
    }

    SECTION("Valid ROI for 1D detector") {
        detector.setNumberOfChannels({100, 1});
        std::vector<slsDetectorDefs::ROI> rois = {
            {10, 20, -1, -1}
        };
        REQUIRE_NOTHROW(DetectorImplTestHelper::testValidateRois(detector, rois));
    }

    SECTION("Invalid Y for 1D detector") {
        detector.setNumberOfChannels({100, 1});
        std::vector<slsDetectorDefs::ROI> rois = {
            {10, 20, 0, 10} // Y should be -1 for 1D
        };
        REQUIRE_THROWS_AS(DetectorImplTestHelper::testValidateRois(detector, rois),
                          sls::RuntimeError);
    }
}


} // namespace sls
