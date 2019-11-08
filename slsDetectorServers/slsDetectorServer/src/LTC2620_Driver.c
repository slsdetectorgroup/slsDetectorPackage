#include "LTC2620_Driver.h"
#include "clogger.h"
#include "common.h"
#include "sls_detector_defs.h"

#include <string.h>

/* LTC2620 DAC DEFINES */
#define LTC2620_D_PWR_DOWN_VAL                (-100)
#define LTC2620_D_MAX_DAC_VAL                 (4095) // 12 bits
#define LTC2620_D_MAX_STEPS                   (LTC2620_D_MAX_DAC_VAL + 1)


// defines from the fpga
int LTC2620_D_HardMaxVoltage = 0;
char LTC2620_D_DriverFileName[MAX_STR_LENGTH];
int LTC2620_D_NumDacs = 0;

void LTC2620_D_SetDefines(int hardMaxV, char* driverfname, int numdacs) {
    FILE_LOG(logINFOBLUE, ("Configuring DACs (LTC2620) to %s (numdacs:%d, hard max: %dmV)\n", driverfname, numdacs, hardMaxV));
    LTC2620_D_HardMaxVoltage = hardMaxV;
    memset(LTC2620_D_DriverFileName, 0, MAX_STR_LENGTH);
    strcpy(LTC2620_D_DriverFileName, driverfname);
    LTC2620_D_NumDacs = numdacs;
}

int LTC2620_D_GetMaxNumSteps() {
    return LTC2620_D_MAX_STEPS;
}

int LTC2620_D_VoltageToDac(int voltage, int* dacval) {
    return ConvertToDifferentRange(0, LTC2620_D_HardMaxVoltage, 0, LTC2620_D_MAX_DAC_VAL,
            voltage, dacval);
}

int LTC2620_D_DacToVoltage(int dacval, int* voltage) {
    return ConvertToDifferentRange(  0, LTC2620_D_MAX_DAC_VAL, 0, LTC2620_D_HardMaxVoltage,
            dacval, voltage);
}


int LTC2620_D_SetDACValue (int dacnum, int val, int mV, char* dacname, int* dacval) {
    FILE_LOG(logDEBUG1, ("dacnum:%d, val:%d, ismV:%d\n", dacnum, val, mV));
    // validate index
    if (dacnum < 0 || dacnum >= LTC2620_D_NumDacs) {
        FILE_LOG(logERROR, ("Dac index %d is out of bounds (0 to %d)\n", dacnum, LTC2620_D_NumDacs - 1));
        return FAIL;
    }

    // get
    if (val < 0 && val != LTC2620_D_PWR_DOWN_VAL)
        return FAIL;

    // convert to dac or get mV value
    *dacval = val;
    int dacmV = val;
    int ret = OK;
    if (mV) {
        ret = LTC2620_D_VoltageToDac(val, dacval);
    } else if (val >= 0) {
        // do not convert power down dac val
        ret = LTC2620_D_DacToVoltage(val, &dacmV);
    }

    // conversion out of bounds
    if (ret == FAIL) {
        FILE_LOG(logERROR, ("Setting Dac %d %s is out of bounds\n", dacnum, (mV ? "mV" : "dac units")));
        return FAIL;
    }

    // set
    if ( (*dacval >= 0) || (*dacval == LTC2620_D_PWR_DOWN_VAL)) {
        FILE_LOG(logINFO, ("Setting DAC %2d [%-12s] : %d dac (%d mV)\n",dacnum, dacname, *dacval, dacmV));
 
        char fname[MAX_STR_LENGTH];
        sprintf(fname, "%s%d", LTC2620_D_DriverFileName, dacnum);
        FILE_LOG(logDEBUG1, ("fname %s\n",fname));
        
        //open file
        FILE* fd=fopen(fname,"w");
        if (fd==NULL) {
            FILE_LOG(logERROR, ("Could not open file %s for writing to set dac %d\n", fname, dacnum));
            return FAIL;
        }
        //convert to string, add 0 and write to file
        fprintf(fd, "%d\n", *dacval);
        fclose(fd);
    }
    return OK;
}
