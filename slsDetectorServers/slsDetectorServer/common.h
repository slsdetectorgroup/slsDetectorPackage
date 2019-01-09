#pragma once

/**
 * Convert a value from a range to a different range (eg voltage to dac or vice versa)
 * @param inputMin input minimum
 * @param inputMax input maximum
 * @param outputMin output minimum
 * @param outputMax output maximum
 * @param inputValue input value
 * @param outputValue pointer to output value
 * @returns FAIL if input value is out of bounds, else OK
 */
int ConvertToDifferentRange(int inputMin, int inputMax, int outputMin, int outputMax,
        int inputValue, int* outputValue) {
    FILE_LOG(logDEBUG1, ("\tInput Value: %d\n", inputValue));

    // validate within bounds
    // eg. MAX1932 range is v(60 - 200) to dac(255 - 1), here inputMin > inputMax (when dac to voltage)
    int smaller = inputMin;
    int bigger = inputMax;
    if (smaller > bigger) {
        smaller = inputMax;
        bigger = inputMin;
    }
    if ((inputValue < smaller) || (inputValue > bigger)) {
        FILE_LOG(logERROR, ("Input Value is outside bounds (%d to %d): %d\n", inputValue, smaller, bigger));
        *outputValue = -1;
        return FAIL;
    }

    double value = double((inputValue - inputMin) * (outputMax - outputMin))
            / double(inputMax - inputMin) + outputMin;

    // double to integer conversion (if decimal places, round to integer)
    if ((value - (int)value) > 0.0001) {
        value += 0.5;
    }
    *outputValue = value;

    FILE_LOG(logDEBUG1, ("\tConverted Ouput Value: %d\n", *outputValue));
    return OK;
}
