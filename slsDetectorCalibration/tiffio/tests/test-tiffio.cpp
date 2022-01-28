

#include "catch.hpp"
#include "sls/tiffIO.h"
#include <cstdio>
#include <ftw.h>
#include <iostream>
#include <vector>

/* Call-back to the 'remove()' function called by nftw() */
static int remove_callback(const char *pathname,
                           __attribute__((unused)) const struct stat *sbuf,
                           __attribute__((unused)) int type,
                           __attribute__((unused)) struct FTW *ftwb) {
    return remove(pathname);
}

TEST_CASE("Write and read back data from tiff file") {

    std::vector<float> data{1, 2, 3, 4, 5, 6, 7, 8, 9};

    /* Create the temporary directory */
    char tmp[] = "/tmp/tmpdir.XXXXXX";
    char *tmp_dirname = mkdtemp(tmp);

    if (tmp_dirname == NULL) {
        perror("tempdir: error: Could not create tmp directory");
        CHECK(false);
    }

    std::string fname = std::string(tmp_dirname) + std::string("/test.tif");
    std::cout << "Writing to: " << fname<< '\n';
    
    WriteToTiff(data.data(), fname.c_str(), 3, 3);

    //Readback
    uint32_t nrow, ncol;
    float* ptr = ReadFromTiff(fname.c_str(), nrow, ncol);
    CHECK(nrow == 3);
    CHECK(ncol == 3);
    uint32_t size = nrow*ncol;
    for (uint32_t i = 0; i!=size; ++i){
        CHECK(data[i] == ptr[i]);
    }

    delete[] ptr;

    /* Delete the temporary directory */
    if (nftw(tmp_dirname, remove_callback, FOPEN_MAX,
             FTW_DEPTH | FTW_MOUNT | FTW_PHYS) == -1) {
        perror("tempdir: error: ");
        exit(EXIT_FAILURE);
    }
}