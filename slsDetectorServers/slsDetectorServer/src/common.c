#include "common.h"
#include "clogger.h"
#include "sls_detector_defs.h"

int ConvertToDifferentRange(int inputMin, int inputMax, int outputMin, int outputMax,
        int inputValue, int* outputValue) {
    LOG(logDEBUG1, (" Input Value: %d (Input:(%d - %d), Output:(%d - %d))\n",
            inputValue, inputMin, inputMax, outputMin, outputMax));

    // validate within bounds
    // eg. MAX1932 range is v(60 - 200) to dac(255 - 1), here inputMin > inputMax (when dac to voltage)
    int smaller = inputMin;
    int bigger = inputMax;
    if (smaller > bigger) {
        smaller = inputMax;
        bigger = inputMin;
    }
    if ((inputValue < smaller) || (inputValue > bigger)) {
        LOG(logERROR, ("Input Value is outside bounds (%d to %d): %d\n", smaller, bigger, inputValue));
        *outputValue = -1;
        return FAIL;
    }

    double value = ((double)(inputValue - inputMin) * (double)(outputMax - outputMin))
            / (double)(inputMax - inputMin) + outputMin;

    // double to integer conversion (if decimal places, round to integer)
    if ((value - (int)value) > 0.0001) {
        value += 0.5;
    }
    *outputValue = value;

    LOG(logDEBUG1, (" Converted Output Value: %d\n", *outputValue));
    return OK;
}

