// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "LTC2620_Driver.h"
#include "clogger.h"
#include "common.h"
#include "sls/sls_detector_defs.h"

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
char LTC2620_D_PowerDownDriverFileName[MAX_STR_LENGTH];
int LTC2620_D_NumDacs = 0;
int LTC2620_D_NumDacsOnly = 0;

void LTC2620_D_SetDefines(int hardMinV, int hardMaxV, char *driverfname,
                          int numdacs, int numpowers,
                          char *powerdownDriverfname) {
    LOG(logINFOBLUE,
        ("Configuring DACs (LTC2620) to %s\n\t (numdacs:%d, hard min:%d, hard "
         "max: %dmV)\n",
         driverfname, numdacs, hardMinV, hardMaxV));
    LTC2620_D_HardMinVoltage = hardMinV;
    LTC2620_D_HardMaxVoltage = hardMaxV;
    memset(LTC2620_D_DriverFileName, 0, MAX_STR_LENGTH);
    strcpy(LTC2620_D_DriverFileName, driverfname);
    memset(LTC2620_D_PowerDownDriverFileName, 0, MAX_STR_LENGTH);
    strcpy(LTC2620_D_PowerDownDriverFileName, powerdownDriverfname);
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

int LTC2620_D_SetDACValue(int dacnum, int val, int mV, char *dacname,
                          int *dacval) {
    LOG(logDEBUG1, ("dacnum:%d, val:%d, ismV:%d\n", dacnum, val, mV));

    // validate index
    if (dacnum < 0 || dacnum >= LTC2620_D_NumDacs) {
        LOG(logERROR, ("Dac index %d is out of bounds (0 to %d)\n", dacnum,
                       LTC2620_D_NumDacs - 1));
        return FAIL;
    }

    // validate set
    if (val < 0 && val != LTC2620_D_PWR_DOWN_VAL)
        return FAIL;

    int ret = OK;
    *dacval = val;
#ifndef VIRTUAL
    char fnameFormat[MAX_STR_LENGTH];
    memset(fnameFormat, 0, MAX_STR_LENGTH);
    strcpy(fnameFormat, LTC2620_D_DriverFileName);
#endif

    // power down dac (different file name)
    if (val == LTC2620_D_PWR_DOWN_VAL) {
#if defined(XILINX_CHIPTESTBOARDD) && !defined(VIRTUAL)
        LOG(logINFO, ("Powering down DAC %2d [%-6s] \n", dacnum, dacname));
        strcpy(fnameFormat, LTC2620_D_PowerDownDriverFileName);
#endif
    }

    // proper value to set
    else {
        // convert to dac or get mV value
        int dacmV = val;
        if (mV) {
            ret = LTC2620_D_VoltageToDac(val, dacval);
        }

        // mV only for print out (dont convert to mV for power regulators)
        else if (val >= 0 && dacnum < LTC2620_D_NumDacsOnly) {
            // do not convert power down dac val
            ret = LTC2620_D_DacToVoltage(val, &dacmV);
        }

        // conversion out of bounds
        if (ret == FAIL) {
            LOG(logERROR, ("Setting Dac %d %s is out of bounds\n", dacnum,
                           (mV ? "mV" : "dac units")));
            return FAIL;
        }

        // print and set
#ifdef XILINX_CHIPTESTBOARDD
        if (*dacval >= 0) {
            // also print mV
            if (dacnum < LTC2620_D_NumDacsOnly) {
                LOG(logINFO, ("Setting DAC %2d [%-6s] : %d dac (%d mV)\n",
                              dacnum, dacname, *dacval, dacmV));
            }
            // do not print mV for power regulators
            else {
                LOG(logINFO, ("Setting Power DAC%2d [%-6s] : %d dac \n", dacnum,
                              dacname, *dacval));
            }
        }
#else
        if ((*dacval >= 0) || (*dacval == LTC2620_D_PWR_DOWN_VAL)) {
            LOG(logINFO, ("Setting DAC %2d [%-12s] : %d dac (%d mV)\n", dacnum,
                          dacname, *dacval, dacmV));
        }
#endif
    }

    // set in file
#ifndef VIRTUAL
    char fname[MAX_STR_LENGTH];
    memset(fname, 0, MAX_STR_LENGTH);
#ifdef XILINX_CHIPTESTBOARDD
    sprintf(fname, fnameFormat, dacnum);
#else
    sprintf(fname, "%s%d", fnameFormat, dacnum);
#endif
    LOG(logDEBUG1, ("fname %s\n", fname));

    // open file
    FILE *fd = fopen(fname, "w");
    if (fd == NULL) {
        LOG(logERROR, ("Could not open file %s for writing to set dac %d\n",
                       fname, dacnum));
        return FAIL;
    }
    // convert to string, add 0 and write to file
#ifdef XILINX_CHIPTESTBOARDD
    // not changing *dacval from -100 (cant write -100 to file: invalid arg)
    int writeValue = *dacval;
    if (writeValue == LTC2620_D_PWR_DOWN_VAL)
        writeValue = 1;
    fprintf(fd, "%d\n", writeValue);
#else
    fprintf(fd, "%d\n", *dacval);
#endif
    fclose(fd);
#endif
    return OK;
}
