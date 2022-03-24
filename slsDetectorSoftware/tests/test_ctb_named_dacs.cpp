#include "catch.hpp"
#include <string>

#include <stdlib.h>

#include "SharedMemory.h"
#include "ctb_named_dacs.h"
using namespace sls;
#include <fstream>
TEST_CASE("Create name for shm dac file") {
    // if SLSDETNAME is already set we unset it but
    // save the value
    std::string old_slsdetname;
    if (getenv(SHM_ENV_NAME))
        old_slsdetname = getenv(SHM_ENV_NAME);
    unsetenv(SHM_ENV_NAME);

    int det_id = 0;
    REQUIRE(ctb_dac_fname(det_id) == "/dev/shm/slsDetectorPackage_detector_0_ctbdacs");

    setenv(SHM_ENV_NAME, "myprefix", 1);
    REQUIRE(ctb_dac_fname(det_id) ==
            "/dev/shm/slsDetectorPackage_detector_0_myprefix_ctbdacs");

    // Clean up after us
    if (old_slsdetname.empty())
        unsetenv(SHM_ENV_NAME);
    else
        setenv(SHM_ENV_NAME, old_slsdetname.c_str(), 1);
}

TEST_CASE("Get ctb dac names returns an empty vector when not set"){
    int large_unlikely_number = 123203;
    auto vec = get_ctb_dac_names(large_unlikely_number);
    REQUIRE(vec.empty());
}

TEST_CASE("Remove file fails silently when file is not present"){
    int large_unlikely_number = 123203;
    REQUIRE_NOTHROW(remove_ctb_dacnames(large_unlikely_number));
}

TEST_CASE("Read dacs from file then remove file"){
    int large_unlikely_number = 998765;
    std::ofstream out(ctb_dac_fname(large_unlikely_number));
    std::string names = "[first, second, third]";
    out.write(&names[0], names.size());
    out.close();

    auto dacnames = get_ctb_dac_names(large_unlikely_number);
    REQUIRE(dacnames.size() == 3);
    REQUIRE(dacnames[0] == "first");
    REQUIRE(dacnames[1] == "second");
    REQUIRE(dacnames[2] == "third");

    remove_ctb_dacnames(large_unlikely_number);

    std::ifstream in(ctb_dac_fname(large_unlikely_number));
    REQUIRE_FALSE(in);

}