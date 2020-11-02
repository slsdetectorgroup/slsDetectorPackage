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

void DAC6571_SetDefines(int hardMaxV, char *driverfname) {
    LOG(logINFOBLUE, ("Configuring High Voltage to %s (hard max: %dV)\n",
                      driverfname, hardMaxV));
    DAC6571_HardMaxVoltage = hardMaxV;
    memset(DAC6571_DriverFileName, 0, MAX_STR_LENGTH);
    strcpy(DAC6571_DriverFileName, driverfname);
}

int DAC6571_Set(int val) {
    LOG(logDEBUG1, ("Setting high voltage to %d\n", val));
    if (val < 0)
        return FAIL;

    int dacvalue = 0;

    // convert value
    ConvertToDifferentRange(0, DAC6571_HardMaxVoltage, DAC6571_MIN_DAC_VAL,
                            DAC6571_MAX_DAC_VAL, val, &dacvalue);

    LOG(logINFO, ("\t%dV (dacval %d)\n", val, dacvalue));

#ifndef VIRTUAL
    // open file
    FILE *fd = fopen(DAC6571_DriverFileName, "w");
    if (fd == NULL) {
        LOG(logERROR,
            ("Could not open file %s for writing to set high voltage\n",
             DAC6571_DriverFileName));
        return FAIL;
    }
    // convert to string, add 0 and write to file
    fprintf(fd, "%d\n", dacvalue);
    fclose(fd);
#endif

    return OK;
}
