// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#define _GNU_SOURCE // needed for strptime to be at the top
#include "common.h"
#include "clogger.h"
#include "sls/sls_detector_defs.h"

#include <libgen.h> // dirname
#include <string.h>
#include <sys/utsname.h> // uname
#include <unistd.h>      // readlink

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

int getTimeFromString(char *buf, time_t *result) {
    char buffer[255] = {0};
    strcpy(buffer, buf);
    // remove timezone as strptime cannot validate timezone despite
    // documentation (for blackfin)
    LOG(logDEBUG, ("kernel v %s\n", buffer));
    const char *timezone = {"CEST"};
    char *res = strstr(buffer, timezone);
    if (res != NULL) {
        size_t cestPos = res - buffer;
        size_t pos = cestPos + strlen(timezone) + 1;
        while (pos != strlen(buffer)) {
            buffer[cestPos] = buffer[pos];
            ++cestPos;
            ++pos;
        }
        buffer[cestPos] = '\0';
    }
    LOG(logDEBUG, ("kernel v after removing CEST %s\n", buffer));

    // convert to time structure
    struct tm t;
    if (NULL == strptime(buffer, "%a %b %d %H:%M:%S %Y", &t)) {
        return FAIL;
    }

    // print time structure
    LOG(logDEBUG,
        ("%d %d %d %d:%d:%d %d (day date month H:M:S year)\n", t.tm_wday,
         t.tm_mday, t.tm_mon, t.tm_year + 1900, t.tm_hour, t.tm_min, t.tm_sec));

    *result = mktime(&t);
    return OK;
}

int getKernelVersion(char *retvals) {
    struct utsname buf = {0};
    if (uname(&buf) == -1) {
        strcpy(retvals, "Failed to get utsname structure from uname\n");
        LOG(logERROR, (retvals));
        return FAIL;
    }
    strcpy(retvals, buf.version);
    LOG(logINFOBLUE, ("Kernel Version: %s\n", retvals));
    return OK;
}

int validateKernelVersion(char *expectedVersion) {
    // extract kernel date string
    char version[255] = {0};
    if (getKernelVersion(version) == FAIL) {
        LOG(logERROR, ("Could not validate kernel version\n"));
        return FAIL;
    }
    LOG(logDEBUG, ("utsname.version:%s\n", version));

    char currentVersion[255] = {0};
#ifdef VIRTUAL
    strcpy(currentVersion, expectedVersion);
#else
    // remove first word (#version number)
    const char *ptr = strchr(version, ' ');
    if (ptr == NULL) {
        LOG(logERROR, ("Could not parse kernel version\n"));
        return FAIL;
    }
    strcpy(currentVersion, version + (ptr - version + 1));
#endif

    // convert kernel date string into time
    time_t kernelDate;
    if (getTimeFromString(currentVersion, &kernelDate) == FAIL) {
        LOG(logERROR,
            ("Could not parse retrieved kernel date, %s\n", currentVersion));
        return FAIL;
    }

    // convert expected date into time
    time_t expDate;
    if (getTimeFromString(expectedVersion, &expDate) == FAIL) {
        LOG(logERROR,
            ("Could not parse expected kernel date, %s\n", expectedVersion));
        return FAIL;
    }

    // compare if kernel time is older than expected time
    if (kernelDate < expDate) {
        LOG(logERROR, ("Kernel Version Incompatible (too old)! Expected: [%s], "
                       "Got [%s]\n",
                       expectedVersion, currentVersion));
        return FAIL;
    }

    LOG(logINFOBLUE, ("Kernel Version Compatible: %s [min.: %s]\n",
                      currentVersion, expectedVersion));
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
    LOG(logINFOBLUE, ("Reading det id file %s\n", fileName));

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
    if (sscanf(line, "%u", &retval) != 1) {
        *ret = FAIL;
        sprintf(mess,
                "Could not scan det id from on-board server "
                "id file. Line:[%s].\n",
                line);
        LOG(logERROR, ("%s\n\n", mess));
        return -1;
    }
    LOG(logINFOBLUE, ("Module Id: %d (File)\n", retval));
    return retval;
}

int verifyChecksumFromBuffer(char *mess, char *functionType,
                             char *clientChecksum, char *buffer,
                             ssize_t bytes) {
    LOG(logINFO, ("\tVerifying Checksum...\n"));
    MD5_CTX c;
    if (!MD5_Init_SLS(&c)) {
        sprintf(mess,
                "Could not %s. Unable to calculate checksum (MD5_Init_SLS)\n",
                functionType);
        LOG(logERROR, (mess));
        return FAIL;
    }
    if (!MD5_Update_SLS(&c, buffer, bytes)) {
        sprintf(mess,
                "Could not %s. Unable to calculate checksum (MD5_Update_SLS)\n",
                functionType);
        LOG(logERROR, (mess));
        return FAIL;
    }
    return verifyChecksum(mess, functionType, clientChecksum, &c,
                          "copied program");
}

int verifyChecksumFromFile(char *mess, char *functionType, char *clientChecksum,
                           char *fname) {
    LOG(logINFO, ("\tVerifying Checksum...\n"));

    FILE *fp = fopen(fname, "r");
    if (fp == NULL) {
        sprintf(
            mess,
            "Could not %s. Unable to open %s in read mode to get checksum\n",
            functionType, fname);
        LOG(logERROR, (mess));
        return FAIL;
    }

    MD5_CTX c;
    if (!MD5_Init_SLS(&c)) {
        fclose(fp);
        sprintf(mess,
                "Could not %s. Unable to calculate checksum (MD5_Init_SLS)\n",
                functionType);
        LOG(logERROR, (mess));
        return FAIL;
    }
    const int readUnitSize = 128;
    char buf[readUnitSize];
    ssize_t bytes = fread(buf, 1, readUnitSize, fp);
    ssize_t totalBytesRead = bytes;
    while (bytes > 0) {
        if (!MD5_Update_SLS(&c, buf, bytes)) {
            fclose(fp);
            sprintf(
                mess,
                "Could not %s. Unable to calculate checksum (MD5_Update_SLS)\n",
                functionType);
            LOG(logERROR, (mess));
            return FAIL;
        }
        bytes = fread(buf, 1, readUnitSize, fp);
        totalBytesRead += bytes;
    }
    LOG(logINFO, ("\tRead %lu bytes to calculate checksum\n", totalBytesRead));
    fclose(fp);
    return verifyChecksum(mess, functionType, clientChecksum, &c,
                          "copied program");
}

int verifyChecksumFromFlash(char *mess, char *functionType,
                            char *clientChecksum, char *fname, ssize_t fsize) {
    LOG(logINFO, ("\tVerifying FlashChecksum...\n"));

    FILE *fp = fopen(fname, "r");
    if (fp == NULL) {
        sprintf(
            mess,
            "Could not %s. Unable to open %s in read mode to get checksum\n",
            functionType, fname);
        LOG(logERROR, (mess));
        return FAIL;
    }

    MD5_CTX c;
    if (!MD5_Init_SLS(&c)) {
        fclose(fp);
        sprintf(mess,
                "Could not %s. Unable to calculate checksum (MD5_Init_SLS)\n",
                functionType);
        LOG(logERROR, (mess));
        return FAIL;
    }
    const int readUnitSize = 128;
    char buf[readUnitSize];
    ssize_t bytes = fread(buf, 1, readUnitSize, fp);
    ssize_t totalBytesRead = bytes;
    int oldProgress = 0;

    while (bytes > 0) {
        int progress = (int)(((double)(totalBytesRead) / fsize) * 100);
        if (oldProgress != progress) {
            printf("%d%%\r", progress);
            fflush(stdout);
            oldProgress = progress;
        }

        if (!MD5_Update_SLS(&c, buf, bytes)) {
            fclose(fp);
            sprintf(
                mess,
                "Could not %s. Unable to calculate checksum (MD5_Update_SLS)\n",
                functionType);
            LOG(logERROR, (mess));
            return FAIL;
        }

        // read only until a particular size (drive)
        if (fsize != 0 && totalBytesRead >= fsize) {
            LOG(logINFO,
                ("\tReached %lu bytes. Not reading more\n", totalBytesRead));
            break;
        }
        // for less than 128 bytes
        if ((readUnitSize + totalBytesRead) > fsize) {
            readUnitSize = fsize - totalBytesRead;
        }
        bytes = fread(buf, 1, readUnitSize, fp);
        totalBytesRead += bytes;
    }
    LOG(logINFO, ("\tRead %lu bytes to calculate checksum\n", totalBytesRead));
    fclose(fp);
    int ret = verifyChecksum(mess, functionType, clientChecksum, &c, "flash");
    if (ret == OK) {
        LOG(logINFO, ("\tChecksum in Flash verified\n"));
    }
    return ret;
}

int verifyChecksum(char *mess, char *functionType, char *clientChecksum,
                   MD5_CTX *c, char *msg) {
    unsigned char out[MD5_DIGEST_LENGTH];
    if (!MD5_Final_SLS(out, c)) {
        sprintf(mess,
                "Could not %s. Unable to calculate checksum (MD5_Final_SLS)\n",
                functionType);
        LOG(logERROR, (mess));
        return FAIL;
    }

    char checksum[512];
    memset(checksum, 0, 512);
    for (int i = 0; i != MD5_DIGEST_LENGTH; ++i) {
        char part[16];
        memset(part, 0, 16);
        sprintf(part, "%02x", out[i]);
        strcat(checksum, part);
    }

    LOG(logDEBUG1,
        ("\nC checksum: %s\nS checksum: %s\n", clientChecksum, checksum));

    // compare checksum
    if (strcmp(clientChecksum, checksum)) {
        sprintf(mess,
                "Could not %s. Checksum of %s does not match. Client "
                "checksum:%s, copied checksum:%s\n",
                functionType, msg, clientChecksum, checksum);
        LOG(logERROR, (mess));
        return FAIL;
    }
    LOG(logINFO, ("\tChecksum verified\n"));
    return OK;
}