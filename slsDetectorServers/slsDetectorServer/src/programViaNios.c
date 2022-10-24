// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "programViaNios.h"
#include "clogger.h"
#include "common.h"
#include "sls/ansi.h"
#include "slsDetectorServer_defs.h"

#include <string.h>
#include <sys/stat.h>
#include <unistd.h> // usleep

/* global variables */

#define CMD_GET_FPGA_FLASH_DRIVE                                               \
    "awk \'$5== \"Application\" {print $1}\' /proc/mtd"
#define CMD_GET_KERNEL_FLASH_DRIVE                                             \
    "awk \'$5== \"Linux\" && $9 != \"Backup\\\"\" {print $1}\' /proc/mtd"
#define FLASH_DRIVE_NAME_SIZE 16

#ifdef VIRTUAL
char flashDriveName[FLASH_DRIVE_NAME_SIZE] = "/tmp/SLS_mtd3";
#else
char flashDriveName[FLASH_DRIVE_NAME_SIZE] = {0};
#endif

char messageType[SHORT_STR_LENGTH] = {0};
#define MICROCONTROLLER_FILE "/dev/ttyAL0"

extern int executeCommand(char *command, char *result, enum TLogLevel level);

void NotifyServerStartSuccess() {
    LOG(logINFOBLUE, ("Server started successfully\n"));
    char command[255];
    memset(command, 0, 255);
    sprintf(command, "echo r > %s", MICROCONTROLLER_FILE);
    system(command);
}

void rebootControllerAndFPGA() {
    LOG(logDEBUG1, ("Reseting FPGA...\n"));
    char command[255];
    memset(command, 0, 255);
    sprintf(command, "echo z > %s", MICROCONTROLLER_FILE);
    system(command);
}

int eraseAndWriteToFlash(char *mess, enum PROGRAM_INDEX index,
                         char *functionType, char *checksum, char *fpgasrc,
                         uint64_t fsize) {

    memset(messageType, 0, sizeof(messageType));
    strcpy(messageType, functionType);

    if (getDrive(mess, index) == FAIL) {
        return FAIL;
    }

    FILE *flashfd = NULL;
    if (openFileForFlash(mess, &flashfd) == FAIL) {
        return FAIL;
    }

    if (eraseFlash(mess) == FAIL) {
        fclose(flashfd);
        return FAIL;
    }

    if (writeToFlash(mess, fsize, flashfd, fpgasrc) == FAIL) {
        return FAIL;
    }

    /* remove condition when flash fpga fixed */
    if (index == PROGRAM_KERNEL) {
        if (verifyChecksumFromFlash(mess, messageType, checksum, flashDriveName,
                                    fsize) == FAIL) {
            return FAIL;
        }
    }

    if (index == PROGRAM_KERNEL) {
        char retvals[MAX_STR_LENGTH] = {0};
        if (executeCommand("sync", retvals, logDEBUG1) == FAIL) {
            snprintf(mess, MAX_STR_LENGTH,
                     "Could not update %s. (could not sync)\n", messageType);
            // LOG(logERROR, (mess)); already printed in executecommand
            return FAIL;
        }
    }

    return OK;
}

int getDrive(char *mess, enum PROGRAM_INDEX index) {
#ifdef VIRTUAL
    return OK;
#endif
    LOG(logDEBUG1, ("Finding flash drive...\n"));
    // getting the drive
    // # cat /proc/mtd
    // dev:    size   erasesize  name
    // mtd0: 00580000 00010000 "qspi BootInfo + Factory Image"
    // mtd1: 00580000 00010000 "qspi Application Image"
    // mtd2: 00800000 00010000 "qspi Linux Kernel with initramfs"
    // mtd3: 00800000 00010000 "qspi Linux Kernel with initramfs Backup"
    // mtd4: 02500000 00010000 "qspi ubi filesystem"
    // mtd5: 04000000 00010000 "qspi Complete Flash"

    char cmd[MAX_STR_LENGTH] = {0};
    char retvals[MAX_STR_LENGTH] = {0};

    if (index == PROGRAM_FPGA) {
        strcpy(cmd, CMD_GET_FPGA_FLASH_DRIVE);
    } else {
        strcpy(cmd, CMD_GET_KERNEL_FLASH_DRIVE);
    }
    if (executeCommand(cmd, retvals, logDEBUG1) == FAIL) {
        snprintf(mess, MAX_STR_LENGTH,
                 "Could not %s. (could not get flash drive: %s)\n", messageType,
                 retvals);
        // LOG(logERROR, (mess)); already printed in executecommand
        return FAIL;
    }

    if (strlen(retvals) == 0) {
        LOG(logERROR, ("Could not %s. Could not get mtd drive, script returned "
                       "empty string\n",
                       messageType));
        return FAIL;
    }

    char *pch = strtok(retvals, ":");
    if (pch == NULL) {
        sprintf(
            mess,
            "Could not %s. Could not get mtd drive to flash (strtok fail).\n",
            messageType);
        LOG(logERROR, (mess));
        return FAIL;
    }

    memset(flashDriveName, 0, sizeof(flashDriveName));
    strcpy(flashDriveName, "/dev/");
    strcat(flashDriveName, pch);
    LOG(logINFO, ("\tFlash drive found: %s\n", flashDriveName));
    return OK;
}

int openFileForFlash(char *mess, FILE **flashfd) {
#ifndef VIRTUAL
    // check if its a normal file or special file
    struct stat buf;
    if (stat(flashDriveName, &buf) == -1) {
        sprintf(mess, "Could not %s. Unable to find the flash drive %s\n",
                messageType, flashDriveName);
        LOG(logERROR, (mess));
        return FAIL;
    }
    // zero = normal file (not char drive special file)
    if (!S_ISCHR(buf.st_mode)) {
        // memory is not permanent
        sprintf(mess,
                "Could not %s. The flash drive found is a normal file. "
                "Reboot board using 'rebootcontroller' command to load "
                "proper device tree\n",
                messageType);
        LOG(logERROR, (mess));
        return FAIL;
    }
    LOG(logINFO, ("\tValidated flash drive (not a normal file)\n"));
#endif

    *flashfd = fopen(flashDriveName, "w");
    if (*flashfd == NULL) {
        sprintf(mess,
                "Could not %s. Unable to open flash drive %s in write mode\n",
                messageType, flashDriveName);
        LOG(logERROR, (mess));
        return FAIL;
    }
    LOG(logINFO, ("\tFlash ready for writing\n"));
    return OK;
}

int eraseFlash(char *mess) {
    LOG(logINFO, ("\tErasing Flash...\n"));

#ifdef VIRTUAL
    return OK;
#endif
    char cmd[MAX_STR_LENGTH] = {0};
    char retvals[MAX_STR_LENGTH] = {0};

    if (snprintf(cmd, MAX_STR_LENGTH, "flash_erase %s 0 0", flashDriveName) >=
        MAX_STR_LENGTH) {
        sprintf(mess, "Could not %s. Command to erase flash is too long\n",
                messageType);
        LOG(logERROR, (mess));
        return FAIL;
    }
    if (executeCommand(cmd, retvals, logDEBUG1) == FAIL) {
        snprintf(mess, MAX_STR_LENGTH,
                 "Could not %s. (could not erase flash: %s)\n", messageType,
                 retvals);
        // LOG(logERROR, (mess)); already printed in executecommand
        return FAIL;
    }

    LOG(logINFO, ("\tFlash erased\n"));
    return OK;
}

int writeToFlash(char *mess, ssize_t fsize, FILE *flashfd, char *buffer) {
    LOG(logINFO, ("\tWriting to Flash...\n"));

    ssize_t bytesWritten = fwrite((void *)buffer, sizeof(char), fsize, flashfd);
    if (bytesWritten != fsize) {
        fclose(flashfd);
        sprintf(mess,
                "Could not %s. Incorrect bytes written to flash  %lu "
                "[expected: %lu]\n",
                messageType, (long int)bytesWritten, (long int)fsize);
        LOG(logERROR, (mess));
        return FAIL;
    }
    fclose(flashfd);
    LOG(logINFO, ("\tWrote %ld bytes to flash\n", bytesWritten));
    return OK;
}
