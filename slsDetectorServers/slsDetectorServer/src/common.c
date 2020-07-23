#include "common.h"
#include "clogger.h"
#include "sls_detector_defs.h"

#include <string.h>
#include <libgen.h> // dirname
#include <unistd.h> // readlink

int ConvertToDifferentRange(int inputMin, int inputMax, int outputMin,
                            int outputMax, int inputValue, int *outputValue) {
    LOG(logDEBUG1, (" Input Value: %d (Input:(%d - %d), Output:(%d - %d))\n",
                    inputValue, inputMin, inputMax, outputMin, outputMax));

    // validate within bounds
    // eg. MAX1932 range is v(60 - 200) to dac(255 - 1), here inputMin >
    // inputMax (when dac to voltage)
    int smaller = inputMin;
    int bigger = inputMax;
    if (smaller > bigger) {
        smaller = inputMax;
        bigger = inputMin;
    }
    if ((inputValue < smaller) || (inputValue > bigger)) {
        LOG(logERROR, ("Input Value is outside bounds (%d to %d): %d\n",
                       smaller, bigger, inputValue));
        *outputValue = -1;
        return FAIL;
    }

    double value =
        ((double)(inputValue - inputMin) * (double)(outputMax - outputMin)) /
            (double)(inputMax - inputMin) +
        outputMin;

    // double to integer conversion (if decimal places, round to integer)
    if ((value - (int)value) > 0.0001) {
        value += 0.5;
    }
    *outputValue = value;

    LOG(logDEBUG1, (" Converted Output Value: %d\n", *outputValue));
    return OK;
}


int getAbsPath(char* buf, size_t bufSize, char* fname) {
    // get path of current binary
    char path[bufSize];
    memset(path, 0, bufSize);
    ssize_t len = readlink("/proc/self/exe", path, bufSize - 1);
    if (len < 0) {
        LOG(logWARNING, ("Could not readlink current binary for %s\n", fname));
        return FAIL;
    }
    path[len] = '\0';

    // get dir path and attach config file name
    char *dir = dirname(path);
    memset(buf, 0, bufSize);
    sprintf(buf, "%s/%s", dir, fname);
    LOG(logDEBUG1, ("full path for %s: %s\n", fname, buf)); 
    return OK;
}