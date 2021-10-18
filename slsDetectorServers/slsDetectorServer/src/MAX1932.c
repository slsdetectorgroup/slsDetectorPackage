// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "MAX1932.h"
#include "blackfin.h"
#include "clogger.h"
#include "common.h"
#include "commonServerFunctions.h" // blackfin.h, ansi.h
#include "sls/sls_detector_defs.h"

/* MAX1932 HV DEFINES */

#define MAX1932_HV_NUMBITS   (8)
#define MAX1932_HV_DATA_OFST (0)
#define MAX1932_HV_DATA_MSK  (0x000000FF << MAX1932_HV_DATA_OFST)
// higher voltage requires lower dac value, 0 is off
#define MAX1932_MIN_DAC_VAL       (0xFF)
#define MAX1932_MAX_DAC_VAL       (0x1)
#define MAX1932_POWER_OFF_DAC_VAL (0x0)

// defines from the fpga
uint32_t MAX1932_Reg = 0x0;
uint32_t MAX1932_CsMask = 0x0;
uint32_t MAX1932_ClkMask = 0x0;
uint32_t MAX1932_DigMask = 0x0;
int MAX1932_DigOffset = 0x0;
int MAX1932_MinVoltage = 0;
int MAX1932_MaxVoltage = 0;

void MAX1932_SetDefines(uint32_t reg, uint32_t cmsk, uint32_t clkmsk,
                        uint32_t dmsk, int dofst, int minMV, int maxMV) {
    LOG(logINFOBLUE, ("Configuring High Voltage\n"));
    MAX1932_Reg = reg;
    MAX1932_CsMask = cmsk;
    MAX1932_ClkMask = clkmsk;
    MAX1932_DigMask = dmsk;
    MAX1932_DigOffset = dofst;
    MAX1932_MinVoltage = minMV;
    MAX1932_MaxVoltage = maxMV;
}

void MAX1932_Disable() {
    bus_w(MAX1932_Reg, (bus_r(MAX1932_Reg) | MAX1932_CsMask | MAX1932_ClkMask) &
                           ~(MAX1932_DigMask));
}

int MAX1932_Set(int *val) {
    LOG(logDEBUG1, ("Setting high voltage to %d\n", *val));
    if (*val < 0)
        return FAIL;

    int dacvalue = 0;

    // limit values (normally < 60 => 0 (off))
    if (*val < MAX1932_MinVoltage) {
        dacvalue = MAX1932_POWER_OFF_DAC_VAL;
        *val = 0;
    }
    // limit values (normally > 200 => 0x1 (max))
    else if (*val > MAX1932_MaxVoltage) {
        dacvalue = MAX1932_MAX_DAC_VAL;
        *val = MAX1932_MaxVoltage;
    }
    // convert value
    else {
        // no failure in conversion as limits handled (range from 0x1 to 0xFF)
        ConvertToDifferentRange(MAX1932_MinVoltage, MAX1932_MaxVoltage,
                                MAX1932_MIN_DAC_VAL, MAX1932_MAX_DAC_VAL, *val,
                                &dacvalue);
        dacvalue &= MAX1932_HV_DATA_MSK;
    }

    LOG(logINFO, ("\t%dV (dacval %d)\n", *val, dacvalue));
    serializeToSPI(MAX1932_Reg, dacvalue, MAX1932_CsMask, MAX1932_HV_NUMBITS,
                   MAX1932_ClkMask, MAX1932_DigMask, MAX1932_DigOffset, 0);
    return OK;
}
