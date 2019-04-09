
#include "SharedMemory.h"
#include "catch.hpp"
#include "string_utils.h"

#include <iostream>

struct Data {
    int x;
    double y;
    char mess[50];
};

using namespace sls;

TEST_CASE("Create SharedMemory read and write", "[detector]") {

    SharedMemory<Data> shm(0, -1);
    shm.CreateSharedMemory();
    CHECK(shm.GetName() == "/slsDetectorPackage_multi_0");

    shm()->x = 3;
    shm()->y = 5.7;
    sls::strcpy_safe(shm()->mess, "Some string");

    CHECK(shm()->x == 3);
    CHECK(shm()->y == 5.7);
    CHECK(std::string(shm()->mess) == "Some string");

    shm.UnmapSharedMemory();
    shm.RemoveSharedMemory();

    CHECK(shm.IsExisting() == false);
}

TEST_CASE("Open existing SharedMemory and read", "[detector]") {

    {
        SharedMemory<double> shm(0, -1);
        shm.CreateSharedMemory();
        *shm() = 5.3;
    }

    SharedMemory<double> shm2(0, -1);
    shm2.OpenSharedMemory();
    CHECK(*shm2() == 5.3);

    shm2.RemoveSharedMemory();
}

TEST_CASE("Creating a second shared memory with the same name throws",
          "[detector]") {

    SharedMemory<double> shm0(0, -1);
    SharedMemory<double> shm1(0, -1);

    shm0.CreateSharedMemory();
    CHECK_THROWS(shm1.CreateSharedMemory());
    shm0.RemoveSharedMemory();
}

TEST_CASE("Open two shared memories to the same place", "[detector]") {

    //Create the first shared memory
    SharedMemory<Data> shm(0, -1);
    shm.CreateSharedMemory();
    shm()->x = 5;
    CHECK(shm()->x == 5);

    //Open the second shared memory with the same name
    SharedMemory<Data> shm2(0, -1);
    shm2.OpenSharedMemory();
    CHECK(shm2()->x == 5);
    CHECK(shm.GetName() == shm2.GetName());

    //Check that they still point to the same place
    shm2()->x = 7;
    CHECK(shm()->x == 7);

    //Remove only needs to be done once since they refer
    //to the same memory
    shm2.RemoveSharedMemory();
    CHECK(shm.IsExisting() == false);
    CHECK(shm2.IsExisting() == false);
}

TEST_CASE("Move SharedMemory", "[detector]") {

    SharedMemory<Data> shm(0,-1);
    CHECK(shm.GetName() == "/slsDetectorPackage_multi_0");
    shm.CreateSharedMemory();
    shm()->x = 9;

    CHECK(shm.size()== sizeof(Data));

    SharedMemory<Data> shm2(1,-1);
    shm2 = std::move(shm); //shm is now a moved from object!

    CHECK(shm2()->x == 9);
    CHECK(shm() == nullptr);
    CHECK(shm.size() == 0);

    CHECK(shm2.GetName() == "/slsDetectorPackage_multi_0");
    shm2.RemoveSharedMemory();

}

TEST_CASE("Create several shared memories", "[detector]") {
    constexpr int N = 5;
    std::vector<SharedMemory<int>> v;
    v.reserve(N);
    for (int i = 0; i != N; ++i) {
        v.emplace_back(i, -1);
        CHECK(v[i].IsExisting() == false);
        v[i].CreateSharedMemory();
        *v[i]() = i;
        CHECK(*v[i]() == i);
    }

    for (int i = 0; i != N; ++i) {
        CHECK(*v[i]() == i);
        CHECK(v[i].GetName() == std::string("/slsDetectorPackage_multi_")+std::to_string(i));
    }

    for (int i = 0; i != N; ++i) {
        v[i].RemoveSharedMemory();
        CHECK(v[i].IsExisting() == false);
    }
}
