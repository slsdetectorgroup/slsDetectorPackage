// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "programFpgaNios.h"
#include "clogger.h"
#include "common.h"
#include "sls/ansi.h"
#include "slsDetectorServer_defs.h"

#include <string.h>
#include <unistd.h> // usleep

/* global variables */

#define CMD_GET_FLASH    "awk \'$5== \"Application\" {print $1}\' /proc/mtd"

#define  FLASH_DRIVE_NAME_SIZE   16
char flashDriveName[FLASH_DRIVE_NAME_SIZE] = {0};
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

int eraseAndWriteToFlash(char *mess, char *checksum, char *fpgasrc,
                         uint64_t fsize) {

    if (verifyChecksumFromBuffer(mess, checksum, fpgasrc, fsize) == FAIL) {
        return FAIL;
    }

    if (getDrive(mess) == FAIL) {
        return FAIL;
    }

    FILE *flashfd = NULL;
    if (openFileForFlash(&flashfd, mess) == FAIL) {
        return FAIL;
    }

   if (eraseFlash(mess) == FAIL) {
        fclose(flashfd);
        return FAIL;
    }

    if (writeToFlash(fsize, flashfd, fpgasrc, mess) == FAIL) {
        return FAIL;
    }
    /* ignoring this until a consistent way to read from nios flash
    if (verifyChecksumFromFlash(mess, checksum, flashDriveName, fsize) ==
        FAIL) {
        return FAIL;
    }
    */
    return OK;
}

int getDrive(char *mess) {
#ifdef VIRTUAL
    strcpy(flashDriveName, "/tmp/SLS_mtd3");
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
    strcpy(cmd, CMD_GET_FLASH);
    if (executeCommand(cmd, retvals, logDEBUG1) == FAIL) {
        strcpy(mess, "Could not program fpga. (could not get flash drive: ");
        strncat(mess, retvals, sizeof(mess) - strlen(mess) - 1);
        strcat(mess, "\n");
        LOG(logERROR, (mess));
        return FAIL;
    }

    char *pch = strtok(retvals, ":");
    if (pch == NULL) {
        strcpy(mess, "Could not get mtd drive to flash (strtok fail).\n");
        LOG(logERROR, (mess));
        return FAIL;
    }

    memset(flashDriveName, 0, sizeof(flashDriveName));
    strcpy(flashDriveName, "/dev/");
    strcat(flashDriveName, pch);
    LOG(logINFO, ("\tFlash drive found: %s\n", flashDriveName));
    return OK;
}

int openFileForFlash(FILE **flashfd, char *mess) {
    *flashfd = fopen(flashDriveName, "w");
    if (*flashfd == NULL) {
        sprintf(mess, "Unable to open flash drive %s in write mode\n",
                flashDriveName);
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
    sprintf(cmd, "flash_erase %s 0 0", flashDriveName);
    if (FAIL == executeCommand(cmd, retvals, logDEBUG1)) {
        strcpy(mess, "Could not program fpga. (could not erase flash: ");
        strncat(mess, retvals, sizeof(mess) - strlen(mess) - 1);
        strcat(mess, "\n");
        LOG(logERROR, (mess));
        return FAIL;
    }

    LOG(logINFO, ("\tFlash erased\n"));
    return OK;
}

int writeToFlash(ssize_t fsize, FILE *flashfd, char *buffer, char *mess) {
    LOG(logINFO, ("\tWriting to Flash...\n"));

    ssize_t bytesWritten = fwrite((void *)buffer, sizeof(char), fsize, flashfd);
    if (bytesWritten != fsize) {
        fclose(flashfd);
        sprintf(
            mess,
            "Could not program fpga. Incorrect bytes written to flash  %lu [expected: %lu]\n",
            (long int)bytesWritten, (long int)fsize);
        LOG(logERROR, (mess));
        return FAIL;
    }
    LOG(logINFO, ("\tWritten to Flash\n"));
    return OK;
}
