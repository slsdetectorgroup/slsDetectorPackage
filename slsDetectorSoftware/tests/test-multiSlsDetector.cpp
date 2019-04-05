
#include "catch.hpp"
#include "container_utils.h"
#include "multiSlsDetector.h"
#include "slsDetector.h"
#include "string_utils.h"

#include <iostream>

using namespace sls;

SCENARIO("Multi detector operation", "[detector]") {

    multiSlsDetector::freeSharedMemory(0, -1);

    GIVEN("An empty multi detector") {
        multiSlsDetector m(0);
        THEN("the size is zero") {
            CHECK(m.getNumberOfDetectors() == 0);
            CHECK(m.getDataBytes() == 0);
            CHECK(m.getTotalNumberOfChannels() == 0);
        }

        WHEN("we add a detector") {
            m.addSlsDetector(sls::make_unique<slsDetector>(
                slsDetectorDefs::detectorType::EIGER, 0, 0));
            THEN("the size and number of detector changes") {
                CHECK(m.getNumberOfDetectors() == 1);
                CHECK(m.getTotalNumberOfChannels() == 256 * 1024);
            }

            WHEN("we add another detector") {
                m.addSlsDetector(sls::make_unique<slsDetector>(
                    slsDetectorDefs::detectorType::EIGER, 0, 1));
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
