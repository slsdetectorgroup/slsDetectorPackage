// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#define _GNU_SOURCE // needed for strptime to be at the top
#include "common.h"
#include "clogger.h"
#include "sls/sls_detector_defs.h"
#include "slsDetectorServer_defs.h"

#include <libgen.h> // dirname
#include <string.h>
#include <sys/stat.h>    // stat
#include <sys/utsname.h> // uname
#include <unistd.h>      // readlink

extern int executeCommand(char *command, char *result, enum TLogLevel level);

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
    if (fname[0] == '/') {
        strcpy(buf, fname);
        return OK;
    }
    // get path of current binary
    char path[bufSize];
    memset(path, 0, bufSize);
    ssize_t len = readlink("/proc/self/exe", path, bufSize - 1);
    if (len < 0) {
        LOG(logWARNING, ("Could not readlink current binary for %s\n", fname));
        return FAIL;
    }
    path[len] = '\0';

    // get dir path and attach file name
    char *dir = dirname(path);
    memset(buf, 0, bufSize);
    if (!strcmp(dir, "/")) {
        sprintf(buf, "/%s", fname);
    } else {
        sprintf(buf, "%s/%s", dir, fname);
    }
    LOG(logDEBUG1, ("full path for %s: %s\n", fname, buf));
    return OK;
}

int getTimeFromString(char *buf, time_t *result) {
    char buffer[255] = {0};
    strcpy(buffer, buf);
    // remove timezone as strptime cannot validate timezone despite
    // documentation (for blackfin)
    LOG(logDEBUG, ("kernel v %s\n", buffer));
    char timezone[8] = {0};
    strcpy(timezone, "CEST");
    char *res = strstr(buffer, timezone);
    // remove CET as well
    if (res == NULL) {
        strcpy(timezone, "CET");
        res = strstr(buffer, timezone);
    }
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
    /*  Do not check as it fails with nios
    if (*result == (time_t)-1) {
        LOG(logERROR, ("Could not convert time structure to time_t\n"));
        return FAIL;
    }*/
    return OK;
}

int getKernelVersion(char *retvals) {
    struct utsname buf;
    memset(&buf, 0, sizeof(buf));
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
#ifndef ARMPROCESSOR
    // remove first word (#version number)
    const char *ptr = strstr(version, " ");
    if (ptr == NULL) {
        LOG(logERROR, ("Could not parse kernel version\n"));
        return FAIL;
    }
    strcpy(currentVersion, ptr + 1);
#else
    // remove first two words (#version number and SMP)
    const char *ptr = strstr(version, "SMP ");
    if (ptr == NULL) {
        LOG(logERROR, ("Could not parse kernel version\n"));
        return FAIL;
    }
    strcpy(currentVersion, ptr + 4);
#endif
#endif
    currentVersion[sizeof(currentVersion) - 1] = '\0';

    // convert kernel date string into time
    time_t kernelDate;
    if (getTimeFromString(currentVersion, &kernelDate) == FAIL) {
        LOG(logERROR,
            ("Could not parse retrieved kernel date, %s\n", currentVersion));
        return FAIL;
    }
    LOG(logDEBUG, ("Kernel Date: [%s]\n", ctime(&kernelDate)));

    // convert expected date into time
    time_t expDate;
    if (getTimeFromString(expectedVersion, &expDate) == FAIL) {
        LOG(logERROR,
            ("Could not parse expected kernel date, %s\n", expectedVersion));
        return FAIL;
    }
    LOG(logDEBUG, ("Expected Date: [%s]\n", ctime(&expDate)));

    // compare if kernel time is older than expected time
    if (kernelDate < expDate) {
        LOG(logERROR, ("Kernel Version Incompatible (too old)!\nExpected: '%s'"
                       "\nGot     : '%s'\n",
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
#if defined(JUNGFRAUD) || defined(MOENCHD)
        *ret = OK;
        LOG(logWARNING, ("Could not find detid file to set module id. "
                         "Continuing without.\n"));
        return 0;
#else
        *ret = FAIL;
        strcpy(mess, "Could not find detid file\n");
        LOG(logERROR, ("%s\n\n", mess));
        return -1;
#endif
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
    LOG(logINFO, ("\tVerifying Checksum from memory...\n"));
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
                          "copied program (buffer)");
}

int verifyChecksumFromFile(char *mess, char *functionType, char *clientChecksum,
                           char *fname) {
    LOG(logINFO, ("\tVerifying Checksum of file...\n"));

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
                          "copied program (file)");
}

int verifyChecksumFromFlash(char *mess, char *functionType,
                            char *clientChecksum, char *fname, ssize_t fsize) {
    LOG(logINFO, ("\tVerifying Checksum from flash...\n"));

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
        ssize_t bytesToRead = readUnitSize;
        if ((readUnitSize + totalBytesRead) > fsize) {
            bytesToRead = fsize - totalBytesRead;
        }
        bytes = fread(buf, 1, bytesToRead, fp);
        totalBytesRead += bytes;
    }
    LOG(logINFO, ("\tRead %lu bytes to calculate checksum\n", totalBytesRead));
    fclose(fp);
    return verifyChecksum(mess, functionType, clientChecksum, &c, "flash");
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
                "checksum:%s, copied checksum:%s. Please try again before "
                "rebooting.\n",
                functionType, msg, clientChecksum, checksum);
        LOG(logERROR, (mess));
        return FAIL;
    }
    LOG(logINFO, ("\tChecksum of %s verified\n", msg));
    return OK;
}

int setupDetectorServer(char *mess, char *sname) {
    char cmd[MAX_STR_LENGTH] = {0};
    char retvals[MAX_STR_LENGTH] = {0};

    // give permissions
    if (snprintf(cmd, MAX_STR_LENGTH, "chmod 777 %s", sname) >=
        MAX_STR_LENGTH) {
        strcpy(mess, "Could not copy detector server. Command to give "
                     "permissions to server is too long\n");
        LOG(logERROR, (mess));
        return FAIL;
    }

    if (executeCommand(cmd, retvals, logDEBUG1) == FAIL) {
        snprintf(mess, MAX_STR_LENGTH,
                 "Could not copy detector server (permissions). %s\n", retvals);
        LOG(logERROR, (mess));
        return FAIL;
    }
    LOG(logINFO, ("\tPermissions modified\n"));

    // symbolic link
    const int fileNameSize = 128;
    char linkname[fileNameSize];
    if (getAbsPath(linkname, fileNameSize, LINKED_SERVER_NAME) == FAIL) {
        sprintf(
            mess,
            "Could not copy detector server. Could not get abs path of current "
            "process\n");
        LOG(logERROR, (mess));
        return FAIL;
    }

    if (snprintf(cmd, MAX_STR_LENGTH, "ln -sf %s %s", sname, linkname) >=
        MAX_STR_LENGTH) {
        strcpy(mess, "Could not copy detector server. Command to "
                     "create symbolic link too long\n");
        LOG(logERROR, (mess));
        return FAIL;
    }
    if (executeCommand(cmd, retvals, logDEBUG1) == FAIL) {
        snprintf(mess, MAX_STR_LENGTH,
                 "Could not copy detector server (symbolic link). %s\n",
                 retvals);
        LOG(logERROR, (mess));
        return FAIL;
    }
    LOG(logINFO, ("\tSymbolic link created %s -> %s\n", linkname, sname));

    // blackfin boards (respawn) (only kept for backwards compatibility)
#ifndef VIRTUAL
#if defined(JUNGFRAUD) || defined(MOENCHD) || defined(CHIPTESTBOARDD) ||       \
    defined(GOTTHARDD)
    // delete every line with DetectorServer in /etc/inittab
    strcpy(cmd, "sed -i '/DetectorServer/d' /etc/inittab");
    if (executeCommand(cmd, retvals, logDEBUG1) == FAIL) {
        snprintf(mess, MAX_STR_LENGTH,
                 "Could not copy detector server (del respawning). %s\n",
                 retvals);
        LOG(logERROR, (mess));
        return FAIL;
    }
    LOG(logINFO, ("\tinittab: DetectoServer line deleted\n"));

    // add new link name to /etc/inittab
    if (snprintf(cmd, MAX_STR_LENGTH,
                 "echo 'ttyS0::respawn:%s' >> /etc/inittab",
                 linkname) >= MAX_STR_LENGTH) {
        strcpy(mess, "Could not copy detector server. Command "
                     "to add new server for spawning is too long\n");
        LOG(logERROR, (mess));
        return FAIL;
    }
    if (executeCommand(cmd, retvals, logDEBUG1) == FAIL) {
        snprintf(mess, MAX_STR_LENGTH,
                 "Could not copy detector server (respawning). %s\n", retvals);
        LOG(logERROR, (mess));
        return FAIL;
    }
    LOG(logINFO, ("\tinittab: updated for respawning\n"));

#endif
#endif

    // sync
    strcpy(cmd, "sync");
    if (executeCommand(cmd, retvals, logDEBUG1) == FAIL) {
        snprintf(mess, MAX_STR_LENGTH,
                 "Could not copy detector server (sync). %s\n", retvals);
        LOG(logERROR, (mess));
        return FAIL;
    }
    LOG(logINFO, ("\tsync\n"));
    return OK;
}

int writeBinaryFile(char *mess, char *fname, char *buffer,
                    const uint64_t filesize, char *errorPrefix) {
    LOG(logINFO, ("\tWriting Detector Server Binary...\n"));

    FILE *fp = fopen(fname, "wb");
    if (fp == NULL) {
        sprintf(mess,
                "Could not %s. (opening file to write(%s). "
                "Maybe it is being used? Try another name?\n",
                errorPrefix, fname);
        LOG(logERROR, (mess));
        return FAIL;
    }

    size_t bytesWritten = 0;
    size_t unitSize = 128;
    int oldProgress = 0;

    while (bytesWritten < filesize) {
        // print progress
        int progress = (int)(((double)(bytesWritten) / filesize) * 100);
        if (oldProgress != progress) {
            printf("%d%%\r", progress);
            fflush(stdout);
            oldProgress = progress;
        }

        // for less than 128 bytes
        ssize_t writeSize = unitSize;
        if ((unitSize + bytesWritten) > filesize) {
            writeSize = filesize - bytesWritten;
        }
        size_t bytes = fwrite((char *)buffer + bytesWritten, 1, writeSize, fp);

        // write
        if (bytes != (size_t)writeSize) {
            sprintf(mess,
                    "Could not %s. Expected to write %lu "
                    "bytes, wrote %lu bytes). No space left? \n",
                    errorPrefix, (long unsigned int)filesize,
                    (long unsigned int)bytesWritten);
            LOG(logERROR, (mess));
            return FAIL;
        }
        bytesWritten += bytes;
        LOG(logDEBUG1,
            ("bytesWritten:%lu filesize:%lu\n", bytesWritten, filesize));
    }
    if (fclose(fp) != 0) {
        sprintf(mess, "Could not %s. (closing file pointer)\n", errorPrefix);
        LOG(logERROR, (mess));
        return FAIL;
    }
    LOG(logINFO, ("\tWritten binary to %s (%lu bytes)\n", fname,
                  (long unsigned int)bytesWritten));
    return OK;
}

int moveBinaryFile(char *mess, char *dest, char *src, char *errorPrefix) {

    char cmd[MAX_STR_LENGTH] = {0};
    char retvals[MAX_STR_LENGTH] = {0};

    // one can move into the current process binary (will not interfere in
    // kernel mode)

    if (snprintf(cmd, MAX_STR_LENGTH, "mv %s %s", src, dest) >=
        MAX_STR_LENGTH) {
        sprintf(mess, "Could not %s. Command to move binary is too long\n",
                errorPrefix);
        LOG(logERROR, (mess));
        return FAIL;
    }

    if (executeCommand(cmd, retvals, logDEBUG1) == FAIL) {
        snprintf(mess, MAX_STR_LENGTH, "Could not %s. (moving). %s\n",
                 errorPrefix, retvals);
        LOG(logERROR, (mess));
        return FAIL;
    }
    LOG(logINFO, ("\tMoved file from %s to %s\n", src, dest));
    return OK;
}

int createEmptyFile(char *mess, char *fname, char *errorPrefix) {
    const int fileNameSize = 128;
    char fullname[fileNameSize];
    if (getAbsPath(fullname, fileNameSize, fname) == FAIL) {
        sprintf(mess,
                "Could not %s. Could not get abs path of current "
                "process\n",
                errorPrefix);
        LOG(logERROR, (mess));
        return FAIL;
    }

    char cmd[MAX_STR_LENGTH] = {0};
    char retvals[MAX_STR_LENGTH] = {0};

    if (snprintf(cmd, MAX_STR_LENGTH, "touch %s", fullname) >= MAX_STR_LENGTH) {
        sprintf(mess, "Could not %s. Command to create is too long\n",
                errorPrefix);
        LOG(logERROR, (mess));
        return FAIL;
    }

    if (executeCommand(cmd, retvals, logDEBUG1) == FAIL) {
        snprintf(mess, MAX_STR_LENGTH,
                 "Could not %s. (creating empty file %s): %s\n", errorPrefix,
                 fullname, retvals);
        LOG(logERROR, (mess));
        return FAIL;
    }
    LOG(logINFO, ("\tEmpty file created: %s (%s)\n", fullname, errorPrefix));
    return OK;
}

int deleteFile(char *mess, char *fname, char *errorPrefix) {
    const int fileNameSize = 128;
    char fullname[fileNameSize];
    if (getAbsPath(fullname, fileNameSize, fname) == FAIL) {
        sprintf(mess,
                "Could not %s. Could not get abs path of current "
                "process\n",
                errorPrefix);
        LOG(logERROR, (mess));
        return FAIL;
    }

    return deleteItem(mess, 1, fullname, errorPrefix);
}

int deleteOldServers(char *mess, char *newServerPath, char *errorPrefix) {
    LOG(logINFO, ("\tChecking if current binary is to be deleted ...\n"))
    // get path of current binary (get file name if link)
    char currentBinary[MAX_STR_LENGTH];
    memset(currentBinary, 0, MAX_STR_LENGTH);
    ssize_t len = readlink("/proc/self/exe", currentBinary, MAX_STR_LENGTH - 1);
    if (len < 0) {
        LOG(logWARNING, ("(%s): Could not delete old servers. Could not "
                         "readlink current binary\n",
                         errorPrefix));
        return FAIL;
    }
    currentBinary[len] = '\0';
    LOG(logDEBUG1, ("Current binary:%s\n", currentBinary));

    // delete file
    if (deleteFile(mess, currentBinary, errorPrefix) == FAIL) {
        LOG(logWARNING, ("(%s). Could not delete old servers\n", errorPrefix));
        return FAIL;
    }
    return OK;
}

int readParameterFromFile(char *fname, char *parameterName, int *value) {
    LOG(logDEBUG1, ("fname:%s parameter:%s\n", fname, parameterName));
    // open file
    FILE *fd = fopen(fname, "r");
    if (fd == NULL) {
#ifdef VIRTUAL
        LOG(logWARNING, ("Could not open file for reading [%s]\n", fname));
        return OK;
#endif
        LOG(logERROR, ("Could not open file for reading [%s]\n", fname));
        return FAIL;
    }

    const size_t LZ = 256;
    char line[LZ];
    memset(line, 0, LZ);

    if (NULL == fgets(line, LZ, fd)) {
        LOG(logERROR, ("Could not read from file %s\n", fname));
        *value = -1;
        return FAIL;
    }

    *value = -1;
    if (sscanf(line, "%d", value) != 1) {
        LOG(logERROR, ("Could not scan %s from %s\n", parameterName, line));
        return FAIL;
    }

    fclose(fd);
    return OK;
}

int createAbsoluteDirectory(char *mess, const char *absPath,
                            char *errorPrefix) {
    // check if folder exists
    if (access(absPath, F_OK) == 0) {
        LOG(logINFO, ("Folder %s already exists\n", absPath));
        return OK;
    }

    // folder does not exist, create it
    if (mkdir(absPath, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0) {
        sprintf(mess, "Could not %s. Could not create folder %s\n", errorPrefix,
                absPath);
        LOG(logERROR, (mess));
        return FAIL;
    }
    LOG(logINFO, ("\tCreated folder: %s (%s)\n", absPath, errorPrefix));

    return OK;
}

int deleteAbsoluteDirectory(char *mess, const char *absPath,
                            char *errorPrefix) {
    return deleteItem(mess, 0, absPath, errorPrefix);
}

int deleteItem(char *mess, int isFile, const char *absPath, char *errorPrefix) {
    // item does not exist
    if (access(absPath, F_OK) != 0) {
        LOG(logINFO, ("\t%s does not exist anyway: %s (%s)\n",
                      (isFile ? "File" : "Folder"), absPath, errorPrefix));
        return OK;
    }

    // delete item
    char cmd[MAX_STR_LENGTH] = {0};
    char retvals[MAX_STR_LENGTH] = {0};
    if (snprintf(cmd, MAX_STR_LENGTH, "rm %s %s", (isFile ? "-f" : "-rf"),
                 absPath) >= MAX_STR_LENGTH) {
        sprintf(mess, "Could not %s. Command to delete is too long\n",
                errorPrefix);
        LOG(logERROR, (mess));
        return FAIL;
    }

    if (executeCommand(cmd, retvals, logDEBUG1) == FAIL) {
        snprintf(mess, MAX_STR_LENGTH, "Could not %s. (deleting %s %s). %s\n",
                 errorPrefix, (isFile ? "file" : "folder"), absPath, retvals);
        LOG(logERROR, (mess));
        return FAIL;
    }
    LOG(logINFO, ("\tDeleted %s: %s (%s)\n", (isFile ? "file" : "folder"),
                  absPath, errorPrefix));

    return OK;
}