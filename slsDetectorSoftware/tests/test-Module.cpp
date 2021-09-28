#include "Module.h"
#include "SharedMemory.h"
#include "catch.hpp"

using dt = slsDetectorDefs::detectorType;
TEST_CASE("Construction with a defined detector type") {
    sls::Module m(dt::EIGER);
    REQUIRE(m.getDetectorType() == dt::EIGER);
    m.freeSharedMemory(); // clean up
}

TEST_CASE("Read back detector type from shm") {
    // Create specific detector in order to create shm
    sls::Module m(dt::JUNGFRAU);

    // New detector that reads type from shm
    sls::Module m2;
    REQUIRE(m2.getDetectorType() == dt::JUNGFRAU);

    // Now both objects point to the same shm so we can only
    // free one!
    m2.freeSharedMemory();
}

TEST_CASE("Is shm fixed pattern shm compatible") {
    sls::Module m(dt::JUNGFRAU);

    // Should be true since we just created the shm
    REQUIRE(m.isFixedPatternSharedMemoryCompatible() == true);

    // Set shm version to 0
    sls::SharedMemory<sls::sharedModule> shm(0, 0);
    REQUIRE(shm.IsExisting() == true);
    shm.OpenSharedMemory();
    shm()->shmversion = 0;

    // Should fail since version is set to 0
    REQUIRE(m.isFixedPatternSharedMemoryCompatible() == false);

    m.freeSharedMemory();
}

TEST_CASE("Get default control port") {
    sls::Module m(dt::MYTHEN3);
    REQUIRE(m.getControlPort() == 1952);
    m.freeSharedMemory();
}

TEST_CASE("Get default stop port") {
    sls::Module m(dt::GOTTHARD2);
    REQUIRE(m.getStopPort() == 1953);
    m.freeSharedMemory();
}

TEST_CASE("Get default receiver TCP port") {
    sls::Module m(dt::MYTHEN3);
    REQUIRE(m.getReceiverPort() == 1954);
    m.freeSharedMemory();
}