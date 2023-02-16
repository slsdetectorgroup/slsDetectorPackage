// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package

#include "SharedMemory.h"
#include "catch.hpp"
#include "sls/string_utils.h"

#include <iostream>

namespace sls {

struct Data {
    int x;
    double y;
    char mess[50];
};

constexpr int shm_id = 10;

TEST_CASE("Create SharedMemory read and write", "[detector]") {

    SharedMemory<Data> shm(shm_id, -1);
    shm.createSharedMemory();
    CHECK(shm.getName() == std::string("/slsDetectorPackage_detector_") +
                               std::to_string(shm_id));

    shm()->x = 3;
    shm()->y = 5.7;
    strcpy_safe(shm()->mess, "Some string");

    CHECK(shm()->x == 3);
    CHECK(shm()->y == 5.7);
    CHECK(std::string(shm()->mess) == "Some string");

    shm.unmapSharedMemory();
    shm.removeSharedMemory();

    CHECK(shm.exists() == false);
}

TEST_CASE("Open existing SharedMemory and read", "[detector]") {

    {
        SharedMemory<double> shm(shm_id, -1);
        shm.createSharedMemory();
        *shm() = 5.3;
    }

    SharedMemory<double> shm2(shm_id, -1);
    shm2.openSharedMemory(true);
    CHECK(*shm2() == 5.3);

    shm2.removeSharedMemory();
}

TEST_CASE("Creating a second shared memory with the same name throws",
          "[detector]") {

    SharedMemory<double> shm0(shm_id, -1);
    SharedMemory<double> shm1(shm_id, -1);

    shm0.createSharedMemory();
    CHECK_THROWS(shm1.createSharedMemory());
    shm0.removeSharedMemory();
}

TEST_CASE("Open two shared memories to the same place", "[detector]") {

    // Create the first shared memory
    SharedMemory<Data> shm(shm_id, -1);
    shm.createSharedMemory();
    shm()->x = 5;
    CHECK(shm()->x == 5);

    // Open the second shared memory with the same name
    SharedMemory<Data> shm2(shm_id, -1);
    shm2.openSharedMemory(true);
    CHECK(shm2()->x == 5);
    CHECK(shm.getName() == shm2.getName());

    // Check that they still point to the same place
    shm2()->x = 7;
    CHECK(shm()->x == 7);

    // Remove only needs to be done once since they refer
    // to the same memory
    shm2.removeSharedMemory();
    CHECK(shm.exists() == false);
    CHECK(shm2.exists() == false);
}

TEST_CASE("Move SharedMemory", "[detector]") {

    SharedMemory<Data> shm(shm_id, -1);
    CHECK(shm.getName() == std::string("/slsDetectorPackage_detector_") +
                               std::to_string(shm_id));
    shm.createSharedMemory();
    shm()->x = 9;

    SharedMemory<Data> shm2(shm_id + 1, -1);
    shm2 = std::move(shm); // shm is now a moved from object!

    CHECK(shm2()->x == 9);
    REQUIRE_THROWS(
        shm()); // trying to access should throw instead of returning a nullptr
    CHECK(shm2.getName() == std::string("/slsDetectorPackage_detector_") +
                                std::to_string(shm_id));
    shm2.removeSharedMemory();
}

TEST_CASE("Create several shared memories", "[detector]") {
    constexpr int N = 5;
    std::vector<SharedMemory<int>> v;
    v.reserve(N);
    for (int i = 0; i != N; ++i) {
        v.emplace_back(shm_id + i, -1);
        CHECK(v[i].exists() == false);
        v[i].createSharedMemory();
        *v[i]() = i;
        CHECK(*v[i]() == i);
    }

    for (int i = 0; i != N; ++i) {
        CHECK(*v[i]() == i);
        CHECK(v[i].getName() == std::string("/slsDetectorPackage_detector_") +
                                    std::to_string(i + shm_id));
    }

    for (int i = 0; i != N; ++i) {
        v[i].removeSharedMemory();
        CHECK(v[i].exists() == false);
    }
}

TEST_CASE("Create create a shared memory with a tag") {
    SharedMemory<int> shm(0, -1, "ctbdacs");
    REQUIRE(shm.getName() == "/slsDetectorPackage_detector_0_ctbdacs");
}

TEST_CASE("Create create a shared memory with a tag when SLSDETNAME is set") {

    // if SLSDETNAME is already set we unset it but
    // save the value
    std::string old_slsdetname;
    if (getenv(SHM_ENV_NAME))
        old_slsdetname = getenv(SHM_ENV_NAME);
    unsetenv(SHM_ENV_NAME);
    setenv(SHM_ENV_NAME, "myprefix", 1);

    SharedMemory<int> shm(0, -1, "ctbdacs");
    REQUIRE(shm.getName() == "/slsDetectorPackage_detector_0_myprefix_ctbdacs");

    // Clean up after us
    if (old_slsdetname.empty())
        unsetenv(SHM_ENV_NAME);
    else
        setenv(SHM_ENV_NAME, old_slsdetname.c_str(), 1);
}

TEST_CASE("map int64 to int32 throws") {
    SharedMemory<int32_t> shm(shm_id, -1);
    shm.createSharedMemory();
    *shm() = 7;

    SharedMemory<int64_t> shm2(shm_id, -1);
    REQUIRE_THROWS(shm2.openSharedMemory(true));

    shm.removeSharedMemory();
}

} // namespace sls
