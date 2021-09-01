#define _GNU_SOURCE // needed for strptime to be at the top
#include "common.h"
#include "clogger.h"
#include "sls/sls_detector_defs.h"

#include <libgen.h> // dirname
#include <string.h>
#include <unistd.h> // readlink

int ConvertToDifferentRange(int inputMin, int inputMax, int outputMin,
                            int outputMax, int inputValue, int *outputValue) {
    LOG(logDEBUG1, (" Input Value: %d (Input:(%d - %d), Output:(%d - %d))\n",
                    inputValue, inputMin, inputMax, outputMin, outputMax));

    // validate within bounds
    // eg. MAX1932 range is v(60 - 200) to dac(255 - 1), here inputMin >
    // inputMax (when dac to voltage)
    int smaller = inputMin;
    int bigger = inputMax;
    if (smaller > bigger) {
        smaller = inputMax;
        bigger = inputMin;
    }
    if ((inputValue < smaller) || (inputValue > bigger)) {
        LOG(logERROR, ("Input Value is outside bounds (%d to %d): %d\n",
                       smaller, bigger, inputValue));
        *outputValue = -1;
        return FAIL;
    }

    double value =
        ((double)(inputValue - inputMin) * (double)(outputMax - outputMin)) /
            (double)(inputMax - inputMin) +
        outputMin;

    // double to integer conversion (if decimal places, round to integer)
    if ((value - (int)value) > 0.0001) {
        value += 0.5;
    }
    *outputValue = value;

    LOG(logDEBUG1, (" Converted Output Value: %d\n", *outputValue));
    return OK;
}

int getAbsPath(char *buf, size_t bufSize, char *fname) {
    // get path of current binary
    char path[bufSize];
    memset(path, 0, bufSize);
    ssize_t len = readlink("/proc/self/exe", path, bufSize - 1);
    if (len < 0) {
        LOG(logWARNING, ("Could not readlink current binary for %s\n", fname));
        return FAIL;
    }
    path[len] = '\0';

    // get dir path and attach config file name
    char *dir = dirname(path);
    memset(buf, 0, bufSize);
    sprintf(buf, "%s/%s", dir, fname);
    LOG(logDEBUG1, ("full path for %s: %s\n", fname, buf));
    return OK;
}

int GetTimeFromString(char *buf, time_t *result) {
    struct tm t;
    if (NULL == strptime(buf, "%a %b %d %H:%M:%S %Z %Y", &t)) {
        return FAIL;
    }
    *result = mktime(&t);
    return OK;
}

void validate(int *ret, char *mess, int arg, int retval, char *modename,
              enum numberMode nummode) {
    if (*ret == OK && arg != GET_FLAG && retval != arg) {
        *ret = FAIL;
        if (nummode == HEX)
            sprintf(mess, "Could not %s. Set 0x%x, but read 0x%x\n", modename,
                    arg, retval);
        else
            sprintf(mess, "Could not %s. Set %d, but read %d\n", modename, arg,
                    retval);
        LOG(logERROR, (mess));
    }
}

void validate64(int *ret, char *mess, int64_t arg, int64_t retval,
                char *modename, enum numberMode nummode) {
    if (*ret == OK && arg != GET_FLAG && retval != arg) {
        *ret = FAIL;
        if (nummode == HEX)
            sprintf(mess, "Could not %s. Set 0x%llx, but read 0x%llx\n",
                    modename, (long long unsigned int)arg,
                    (long long unsigned int)retval);
        else
            sprintf(mess, "Could not %s. Set %lld, but read %lld\n", modename,
                    (long long unsigned int)arg,
                    (long long unsigned int)retval);
        LOG(logERROR, (mess));
    }
}

int getModuleIdInFile(int *ret, char *mess, char *fileName) {
    const int fileNameSize = 128;
    char fname[fileNameSize];
    if (getAbsPath(fname, fileNameSize, fileName) == FAIL) {
        *ret = FAIL;
        strcpy(mess, "Could not find detid file\n");
        LOG(logERROR, ("%s\n\n", mess));
        return -1;
    }

    // open id file
    FILE *fd = fopen(fname, "r");
    if (fd == NULL) {
        *ret = FAIL;
        strcpy(mess, "Could not find detid file\n");
        LOG(logERROR, ("%s\n\n", mess));
        return -1;
    }
    LOG(logDEBUG1, ("Reading det id file %s\n", fileName));

    // read line
    const size_t len = 256;
    char line[len];
    memset(line, 0, len);
    if (NULL == fgets(line, len, fd)) {
        *ret = FAIL;
        strcpy(mess, "Could not find detid file\n");
        LOG(logERROR, ("%s\n\n", mess));
        return -1;
    }
    // read id
    int retval = 0;
    if (sscanf(line, "%x", &retval) != 1) {
        *ret = FAIL;
        sprintf(mess,
                "Could not scan det id from on-board server "
                "id file. Line:[%s].\n",
                line);
        LOG(logERROR, ("%s\n\n", mess));
        return -1;
    }
    LOG(logINFOBLUE, ("Module Id: 0x%x (File)\n", retval));
    return retval;
}

int setModuleIdInFile(char *mess, int arg, char *fileName) {
    LOG(logINFOBLUE, ("Setting Module Id: 0x%x (File)\n", arg));

    const int fileNameSize = 128;
    char fname[fileNameSize];
    if (getAbsPath(fname, fileNameSize, fileName) == FAIL) {
        strcpy(mess, "Could not find detid file\n");
        LOG(logERROR, (mess));
        return FAIL;
    }

    // open id file
    FILE *fd = fopen(fname, "r");
    if (fd == NULL) {
        strcpy(mess, "Could not find detid file\n");
        LOG(logERROR, (mess));
        return FAIL;
    }
    LOG(logDEBUG1, ("Writing det id to file %s\n", fileName));

    // write id
    const size_t len = 256;
    char line[len];
    memset(line, 0, len);
    sprintf(line, "%x", arg);
    if (EOF == fputs(line, fd)) {
        strcpy(mess, "Could not write to detid file\n");
        LOG(logERROR, (mess));
        return FAIL;
    }
    return OK;
}
