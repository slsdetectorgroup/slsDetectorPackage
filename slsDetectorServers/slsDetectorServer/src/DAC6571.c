// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "DAC6571.h"
#include "clogger.h"
#include "common.h"
#include "sls/sls_detector_defs.h"

#include "string.h"

/* DAC6571 HV DEFINES */
#define DAC6571_MIN_DAC_VAL (0x0)
#define DAC6571_MAX_DAC_VAL (0x3FF)

// defines from the hardware
int DAC6571_HardMaxVoltage = 0;
char DAC6571_DriverFileName[MAX_STR_LENGTH];

#ifdef VIRTUAL
int highvoltage = 0;
#endif

void DAC6571_SetDefines(int hardMaxV, char *driverfname) {
    LOG(logINFOBLUE, ("Configuring High Voltage to %s (hard max: %dV)\n",
                      driverfname, hardMaxV));
    DAC6571_HardMaxVoltage = hardMaxV;
    memset(DAC6571_DriverFileName, 0, MAX_STR_LENGTH);
    strcpy(DAC6571_DriverFileName, driverfname);
#ifdef VIRTUAL
    highvoltage = 0;
#endif
}

int DAC6571_Set(int val, char *mess) {
    LOG(logDEBUG1, ("Setting high voltage to %d\n", val));
    if (val < 0) {
        sprintf(mess, "Invalid value. Cannot be negative.\n");
        LOG(logERROR, (mess));
        return FAIL;
    }

    int dacvalue = 0;

    // convert value
    if (ConvertToDifferentRange(0, DAC6571_HardMaxVoltage, DAC6571_MIN_DAC_VAL,
                                DAC6571_MAX_DAC_VAL, val, &dacvalue) == FAIL) {
        sprintf(mess,
                "Could not convert %d high voltage to a valid dac value\n",
                val);
        LOG(logERROR, (mess));
        return FAIL;
    }
    LOG(logINFO, ("\t%dV (dacval %d)\n", val, dacvalue));

#ifdef VIRTUAL
    highvoltage = dacvalue;
#else
    // open file
    FILE *fd = fopen(DAC6571_DriverFileName, "w");
    if (fd == NULL) {
        sprintg(mess,
                "Could not open file %s for writing to set high voltage\n",
                DAC6571_DriverFileName);
        LOG(logERROR, (mess));
        return FAIL;
    }
    // convert to string, add 0 and write to file
    fprintf(fd, "%d\n", dacvalue);
    fclose(fd);
#endif

    return OK;
}

int DAC6571_Get(int *retval, char *mess) {
    LOG(logDEBUG1, ("Getting high voltage\n"));
    int dacvalue = 0;

#ifdef VIRTUAL
    dacvalue = highvoltage;
#else
    if (readParameterFromFile(DAC6571_DriverFileName, "high voltage",
                              &dacvalue) == FAIL) {
        sprintf(mess, "Could not get high voltage\n");
        LOG(logERROR, (mess));
        return FAIL;
    }
#endif

    // convert value
    if (ConvertToDifferentRange(DAC6571_MIN_DAC_VAL, DAC6571_MAX_DAC_VAL, 0,
                                DAC6571_HardMaxVoltage, dacvalue,
                                retval) == FAIL) {
        sprintf(mess,
                "Could not convert %d dac value to a valid high voltage\n",
                dacvalue);
        LOG(logERROR, (mess));
        return FAIL;
    }
    LOG(logINFO, ("\t%dV (dacval %d)\n", (*retval), dacvalue));

#ifndef VIRTUAL
    // open file
    FILE *fd = fopen(DAC6571_DriverFileName, "w");
    if (fd == NULL) {
        sprintf(mess,
                "Could not open file %s for writing to get high voltage\n",
                DAC6571_DriverFileName);
        LOG(logERROR, (mess));
        return FAIL;
    }
    // convert to string, add 0 and write to file
    fprintf(fd, "%d\n", dacvalue);
    fclose(fd);
#endif

    return OK;
}
