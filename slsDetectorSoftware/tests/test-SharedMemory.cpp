
#include "SharedMemory.h"
#include "catch.hpp"
#include "string_utils.h"

struct Data {
    int x;
    double y;
    char mess[50];
};

TEST_CASE("Create SharedMemory read and write") {

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
}

TEST_CASE("Open existing SharedMemory and read") {

    SharedMemory<double> shm(0, -1);
    shm.CreateSharedMemory();
    *shm() = 5.3;
    shm.UnmapSharedMemory();

    SharedMemory<double> shm2(0, -1);
    shm2.OpenSharedMemory();
    CHECK(*shm2() == 5.3);

    shm2.RemoveSharedMemory();
}

TEST_CASE("Two shared memories with the same name throws") {

    SharedMemory<double> shm0(0, -1);
    SharedMemory<double> shm1(0, -1);

    shm0.CreateSharedMemory();
    CHECK_THROWS(shm1.CreateSharedMemory());
    shm0.RemoveSharedMemory();
}

TEST_CASE("Open two shared memories to the same place") {

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