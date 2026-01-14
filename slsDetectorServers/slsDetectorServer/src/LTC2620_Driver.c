// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "LTC2620_Driver.h"
#include "clogger.h"
#include "common.h"

// to include power down file name suffix
#ifdef XILINX_CHIPTESTBOARDD
#include "slsDetectorServer_defs.h"
#else
#include "sls/sls_detector_defs.h"
#endif

#include <string.h>

/* LTC2620 DAC DEFINES */
#define LTC2620_D_PWR_DOWN_VAL (-100)
#define LTC2620_D_MIN_DAC_VAL  (0)
#define LTC2620_D_MAX_DAC_VAL  (4095) // 12 bits
#define LTC2620_D_MAX_STEPS    (LTC2620_D_MAX_DAC_VAL + 1)

// defines from the fpga
int LTC2620_D_HardMinVoltage = 0;
int LTC2620_D_HardMaxVoltage = 0;
char LTC2620_D_DriverFileName[MAX_STR_LENGTH];
int LTC2620_D_NumDacs = 0;
int LTC2620_D_NumDacsOnly = 0;

void LTC2620_D_SetDefines(int hardMinV, int hardMaxV, char *driverfname,
                          int numdacs, int numpowers) {
    LOG(logINFOBLUE,
        ("Configuring DACs (LTC2620) to %s\n\t (numdacs:%d, hard min:%d, hard "
         "max: %dmV)\n",
         driverfname, numdacs, hardMinV, hardMaxV));
    LTC2620_D_HardMinVoltage = hardMinV;
    LTC2620_D_HardMaxVoltage = hardMaxV;
    memset(LTC2620_D_DriverFileName, 0, MAX_STR_LENGTH);
    strcpy(LTC2620_D_DriverFileName, driverfname);
    LTC2620_D_NumDacs = numdacs;
    LTC2620_D_NumDacsOnly = numdacs - numpowers;
}

int LTC2620_D_GetMaxNumSteps() { return LTC2620_D_MAX_STEPS; }

int LTC2620_D_GetPowerDownValue() { return LTC2620_D_PWR_DOWN_VAL; }

int LTC2620_D_GetMinInput() { return LTC2620_D_MIN_DAC_VAL; }

int LTC2620_D_GetMaxInput() { return LTC2620_D_MAX_DAC_VAL; }

int LTC2620_D_VoltageToDac(int voltage, int *dacval) {
    return ConvertToDifferentRange(LTC2620_D_HardMinVoltage,
                                   LTC2620_D_HardMaxVoltage, 0,
                                   LTC2620_D_MAX_DAC_VAL, voltage, dacval);
}

int LTC2620_D_DacToVoltage(int dacval, int *voltage) {
    return ConvertToDifferentRange(0, LTC2620_D_MAX_DAC_VAL,
                                   LTC2620_D_HardMinVoltage,
                                   LTC2620_D_HardMaxVoltage, dacval, voltage);
}

int LTC2620_D_WriteDACValue(int dacnum, int dacvalue, char *dacname) {
    LOG(logDEBUG1, ("dacnum:%d, val:%d\n", dacnum, dacvalue));

    // validate index
    if (dacnum < 0 || dacnum >= LTC2620_D_NumDacs) {
        LOG(logERROR, ("Dac index %d is out of bounds (0 to %d)\n", dacnum,
                       LTC2620_D_NumDacs - 1));
        return FAIL;
    }

    // validate value
    if ((dacvalue < 0 && dacvalue != LTC2620_D_PWR_DOWN_VAL) ||
        (dacvalue > LTC2620_D_MAX_DAC_VAL)) {
        LOG(logERROR,
            ("Dac %d %s: Invalid dac value %d\n", dacnum, dacname, dacvalue));
        return FAIL;
    }

    // print info
    if (dacvalue == LTC2620_D_PWR_DOWN_VAL) {
        LOG(logDEBUG, ("\tPowering down DAC %2d [%-6s] \n", dacnum, dacname));
    } else {
        LOG(logINFO, ("\tSetting DAC %2d [%-6s] to %d dac units\n", dacnum,
                      dacname, dacvalue));
    }

#ifdef VIRTUAL
    return OK;
#endif

    // file name
    char fname[MAX_STR_LENGTH];
    memset(fname, 0, MAX_STR_LENGTH);
    snprintf(fname, MAX_STR_LENGTH, "%s%d", LTC2620_D_DriverFileName, dacnum);
#ifdef XILINX_CHIPTESTBOARDD
    // different file for power down
    if (dacvalue == LTC2620_D_PWR_DOWN_VAL) {
        snprintf(fname, MAX_STR_LENGTH, "%s%d%s", LTC2620_D_DriverFileName,
                 dacnum, DAC_POWERDOWN_DRIVER_FILE_SUFFIX);
    }
#endif
    LOG(logDEBUG1, ("fname %s\n", fname));

    // open file
    FILE *fd = fopen(fname, "w");
    if (fd == NULL) {
        LOG(logERROR,
            ("Could not open file %s for writing to set dac %d [%s] \n", fname,
             dacnum, dacname));
        return FAIL;
    }

    // write to file
#ifndef XILINX_CHIPTESTBOARDD
    fprintf(fd, "%d\n", dacvalue);
#else
    // cant write -100 to file: invalid arg
    if (dacvalue == LTC2620_D_PWR_DOWN_VAL) {
        fprintf(fd, "1\n");
    } else {
        fprintf(fd, "%d\n", dacvalue);
    }
#endif
    fclose(fd);

    return OK;
}

int LTC2620_D_SetDACValue(int dacnum, int val, int mV, char *dacname,
                          int *dacval) {
    LOG(logDEBUG1,
        ("dacnum:%d (%s), val:%d, ismV:%d\n", dacnum, dacname, val, mV));

    // invalid index
    if (dacnum >= LTC2620_D_NumDacs) {
        LOG(logERROR, ("Dac index %d is out of bounds (0 to %d)\n", dacnum,
                       LTC2620_D_NumDacs - 1));
        return FAIL;
    }

    if (mV && dacnum >= LTC2620_D_NumDacsOnly) {
        LOG(logERROR, ("Cannot convert to dac units for power regulator %d %s "
                       "here. Expecting dac units here.\n",
                       dacnum, dacname));
        return FAIL;
    }

    *dacval = val;
    if (val == LTC2620_D_GetPowerDownValue()) {
        LOG(logINFO, ("\tPowering down DAC %2d [%-6s] \n", dacnum, dacname));
    } else {

        // invalid negative value
        if (val < 0) {
            LOG(logERROR, ("Invalid value %d for dac[%d - %s]\n", val,
                           (int)dacnum, dacname));
            return FAIL;
        }

        // convert to dac units or mV (print)
        if (dacnum < LTC2620_D_NumDacsOnly) {
            int dacmV = val;
            int ret = OK;
            if (mV) {
                ret = LTC2620_D_VoltageToDac(val, dacval);
            } else {
                ret = LTC2620_D_DacToVoltage(val, &dacmV);
            }
            // conversion out of bounds
            if (ret == FAIL) {
                LOG(logERROR, ("Setting Dac %d %s is out of bounds\n", dacnum,
                               (mV ? "mV" : "dac units")));
                return FAIL;
            }
            LOG(logINFO, ("Setting DAC %2d [%-6s] : %d dac (%d mV)\n", dacnum,
                          dacname, *dacval, dacmV));
        }
    }

    return LTC2620_D_WriteDACValue(dacnum, *dacval, dacname);
}
