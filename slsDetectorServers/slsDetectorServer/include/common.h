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
        int inputValue, int* outputValue);
