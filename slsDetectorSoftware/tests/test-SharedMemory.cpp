
#include "catch.hpp"
#include "SharedMemory.h"
#include "string_utils.h"


TEST_CASE("SharedMemory") {
    struct Data{
        int x;
        double y;
        char mess[50];
    };

    
    SharedMemory<Data> shm(0,-1);
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