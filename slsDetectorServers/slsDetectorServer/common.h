#pragma once

/**
 * Convert voltage to dac units
 * @param voltage value in mv
 * @param dacval pointer to value converted to dac units
 * @param vmin minimum voltage in mV
 * @param vmax maximum voltage in mV
 * @param maximum number of steps
 * @returns FAIL when voltage outside limits, OK if conversion successful
 */
int Common_VoltageToDac(int voltage, int* dacval, int vmin, int vmax, int nsteps) {

    // validate
    if ((voltage < vmin) || (voltage > vmax)) {
        FILE_LOG(logERROR, ("Voltage value (to convert to dac value) is outside bounds (%d to %d mV): %d\n", vmin, vmax, voltage));
        return FAIL;
    }

    // convert
    *dacval = (int)(((voltage - vmin) / (vmax - vmin)) * (nsteps - 1) + 0.5);
    return OK;
}
/**
 * Convert dac units to voltage
 * @param dacval dac units
 * @param voltage pointer to value converted to mV
 * @param vmin minimum voltage in mV
 * @param vmax maximum voltage in mV
 * @param maximum number of steps
 * @returns FAIL when voltage outside limits, OK if conversion successful
 */
int Common_DacToVoltage(int dacval, int* voltage, int vmin, int vmax, int nsteps) {

    // validate
    if ((dacval < 0) || (dacval >= nsteps)) {
        FILE_LOG(logERROR, ("Dac units (to convert to voltage) is outside bounds (0 to %d): %d\n", nsteps - 1, dacval));
        return FAIL;
    }

    // convert
    *voltage = vmin + (vmax - vmin) * dacval / (nsteps - 1);
    return OK;
}
