
#include "catch.hpp"
#include "container_utils.h"
#include "DetectorImpl.h"
#include "Module.h"
#include "string_utils.h"

#include <iostream>

using namespace sls;

SCENARIO("Multi detector operation", "[detector]") {

    DetectorImpl::freeSharedMemory(20, -1);

    GIVEN("An empty multi detector") {
        DetectorImpl m(20);
        THEN("the size is zero") {
            CHECK(m.getNumberOfDetectors() == 0);
            CHECK(m.getDataBytes() == 0);
            CHECK(m.getTotalNumberOfChannels() == 0);
        }

        WHEN("we add a detector") {
            m.addSlsDetector(sls::make_unique<Module>(
                slsDetectorDefs::detectorType::EIGER, 20, 0));
            THEN("the size and number of detector changes") {
                CHECK(m.getNumberOfDetectors() == 1);
                CHECK(m.getTotalNumberOfChannels() == 256 * 1024);
            }

            WHEN("we add another detector") {
                m.addSlsDetector(sls::make_unique<Module>(
                    slsDetectorDefs::detectorType::EIGER, 20, 1));
                THEN("the size and number of detector changes") {
                    CHECK(m.getNumberOfDetectors() == 2);
                    CHECK(m.getTotalNumberOfChannels() == 2 * 256 * 1024);
                }

                WHEN("We set the trimen") {
                    std::vector<int> energies{5000, 6000, 7000, 8000, 9000};
                    m.setTrimEn(energies);
                    THEN("we read back the same values") {
                        CHECK(m.getTrimEn() == energies);
                    }
                }
                WHEN("We set the trimen to different values") {
                    std::vector<int> en0{5000, 6000, 7000, 8000, 9000};
                    std::vector<int> en1{6000, 7000, 8000, 9000};
                    m.setTrimEn(en0, 0);
                    m.setTrimEn(en1, 1);
                    THEN("we read back the same values") {
                        CHECK(m.getTrimEn(0) == en0);
                        CHECK(m.getTrimEn(1) == en1);
                        CHECK(m.getTrimEn() == std::vector<int>{-1});
                    }
                }
            }
        }
        m.freeSharedMemory();
    }
}

TEST_CASE("Set and get partialFramesPadding", "[detector][somenewtag]"){

    DetectorImpl::freeSharedMemory(20, -1);
    DetectorImpl m(20);
    m.addSlsDetector(sls::make_unique<Module>(
        slsDetectorDefs::detectorType::EIGER, 20, 0));
    m.addSlsDetector(sls::make_unique<Module>(
        slsDetectorDefs::detectorType::EIGER, 20, 1));

    m.setPartialFramesPadding(false);
    CHECK(m.getPartialFramesPadding() == 0);

    m.setPartialFramesPadding(true);
    CHECK(m.getPartialFramesPadding() == 1);

    m.setPartialFramesPadding(false, 0);
    CHECK(m.getPartialFramesPadding() == -1);

    m.freeSharedMemory();
}
