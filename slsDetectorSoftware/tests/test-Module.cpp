// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "Detector.h"
#include "Module.h"
#include "SharedMemory.h"
#include "catch.hpp"

namespace sls {

using dt = slsDetectorDefs::detectorType;
TEST_CASE("Construction with a defined detector type") {
    Module m(dt::EIGER);
    REQUIRE(m.getDetectorType() == dt::EIGER);
    freeSharedMemory(0, 0); // clean up
    SharedMemory<sharedModule> moduleShm(0, 0);
    REQUIRE(moduleShm.exists() == false);
}

TEST_CASE("Read back detector type from shm") {
    // Create specific detector in order to create shm
    Module m(dt::JUNGFRAU);

    // New detector that reads type from shm
    Module m2;
    REQUIRE(m2.getDetectorType() == dt::JUNGFRAU);

    // Now both objects point to the same shm so we can only
    // free one!
    freeSharedMemory(0, 0);
    SharedMemory<sharedModule> moduleShm(0, 0);
    REQUIRE(moduleShm.exists() == false);
}

TEST_CASE("Is shm fixed pattern shm compatible") {
    Module m(dt::JUNGFRAU);

    // Should be true since we just created the shm
    REQUIRE(m.isFixedPatternSharedMemoryCompatible() == true);

    // Set shm version to 0
    SharedMemory<sharedModule> shm(0, 0);
    REQUIRE(shm.exists() == true);
    shm.openSharedMemory(true);
    shm()->shmversion = 0;

    // Should fail since version is set to 0
    REQUIRE(m.isFixedPatternSharedMemoryCompatible() == false);

    freeSharedMemory(0, 0);
    SharedMemory<sharedModule> moduleShm(0, 0);
    REQUIRE(moduleShm.exists() == false);
}

TEST_CASE("Get default control port") {
    Module m(dt::MYTHEN3);
    REQUIRE(m.getControlPort() == 1952);
    freeSharedMemory(0, 0);
    SharedMemory<sharedModule> moduleShm(0, 0);
    REQUIRE(moduleShm.exists() == false);
}

TEST_CASE("Get default stop port") {
    Module m(dt::GOTTHARD2);
    REQUIRE(m.getStopPort() == 1953);
    freeSharedMemory(0, 0);
    SharedMemory<sharedModule> moduleShm(0, 0);
    REQUIRE(moduleShm.exists() == false);
}

TEST_CASE("Get default receiver TCP port") {
    Module m(dt::MYTHEN3);
    REQUIRE(m.getReceiverPort() == 1954);
    freeSharedMemory(0, 0);
    SharedMemory<sharedModule> moduleShm(0, 0);
    REQUIRE(moduleShm.exists() == false);
}

} // namespace sls
