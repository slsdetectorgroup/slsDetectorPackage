// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "programViaBlackfin.h"
#include "clogger.h"
#include "common.h"
#include "sls/ansi.h"
#include "slsDetectorServer_defs.h"

#include <string.h>
#include <sys/stat.h>
#include <sys/sysinfo.h>
#include <unistd.h> // usleep

/* global variables */
// clang-format off
#define MAX_TIME_FPGA_TOUCH_FLASH_US (10 * 1000 * 1000) // 10s

#define CMD_GPIO7_DEFINE "echo 7 > /sys/class/gpio/export"
#define CMD_GPIO9_DEFINE "echo 9 > /sys/class/gpio/export"
#define CMD_GPIO3_DEFINE "echo 3 > /sys/class/gpio/export"

// N config done
#define CMD_GPIO7_EXIST "/sys/class/gpio/gpio7"
// N Config
#define CMD_GPIO9_EXIST "/sys/class/gpio/gpio9"
// N CE (access to AS interface)
#define CMD_GPIO3_EXIST "/sys/class/gpio/gpio3"

#define CMD_GPIO9_DEFINE_OUT "echo out > /sys/class/gpio/gpio9/direction"
#define CMD_GPIO3_DEFINE_OUT "echo out > /sys/class/gpio/gpio3/direction"
#define CMD_GPIO7_DEFINE_IN "echo in > /sys/class/gpio/gpio7/direction"
#define CMD_GPIO9_DEFINE_IN "echo in > /sys/class/gpio/gpio9/direction"
#define CMD_GPIO3_DEFINE_IN "echo in > /sys/class/gpio/gpio3/direction"

// nConfig
#define CMD_GPIO9_DONT_TOUCH_FLASH "echo 0 > /sys/class/gpio/gpio9/value"
// nCE
#define CMD_GPIO3_DONT_TOUCH_FLASH "echo 1 > /sys/class/gpio/gpio3/value"
// CD
#define CMD_FPGA_PICKED_STATUS  "cat /sys/class/gpio/gpio7/value"

#define CMD_GET_FPGA_FLASH_DRIVE    "awk \'$4== \"\\\"bitfile(spi)\\\"\" {print $1}\' /proc/mtd"
#define CMD_GET_KERNEL_FLASH_DRIVE  "awk \'$4== \"\\\"linux\" {print $1}\' /proc/mtd"


#define CMD_GET_AMD_FLASH  "dmesg | grep Amd"

#define CMD_CREATE_DEVICE_FILE_PART1 "mknod"
#define CMD_CREATE_DEVICE_FILE_PART2 "c 90 6"

#define FLASH_BUFFER_MEMORY_SIZE (128 * 1024) // 500 KB
// clang-format on

#define FLASH_DRIVE_NAME_SIZE 16

#ifdef VIRTUAL
char flashDriveName[FLASH_DRIVE_NAME_SIZE] = "/tmp/SLS_mtd3";
#else
char flashDriveName[FLASH_DRIVE_NAME_SIZE] = {0};
#endif

char messageType[SHORT_STR_LENGTH] = {0};

extern int executeCommand(char *command, char *result, enum TLogLevel level);

int latestKernelVerified = -1;
#define KERNEL_DATE_VRSN_3GPIO "Fri Oct 29 00:00:00 2021"

int defineGPIOpins(char *mess) {
#ifdef VIRTUAL
    return OK;
#endif
    // only latest kernel can use gpio3 pins
    if (latestKernelVerified == -1) {
        if (FAIL == validateKernelVersion(KERNEL_DATE_VRSN_3GPIO)) {
            latestKernelVerified = 0;
            LOG(logWARNING,
                ("Kernel too old to use gpio3 (nCE). Update kernel to "
                 "guarantee error-free fpga programming. \n\tNot the end "
                 "of the world. Continuing with current kernel...\n"));
        } else {
            latestKernelVerified = 1;
        }
    }

    char retvals[MAX_STR_LENGTH] = {0};
    // define gpio7
    if (access(CMD_GPIO7_EXIST, F_OK) != 0) {
        if (executeCommand(CMD_GPIO7_DEFINE, retvals, logDEBUG1) == FAIL) {
            snprintf(mess, MAX_STR_LENGTH,
                     "Could not define gpio7 (CD) for fpga (%s)\n", retvals);
            LOG(logERROR, (mess));
            return FAIL;
        }
        LOG(logINFO, ("\tgpio7 (CD): defined\n"));
    } else {
        LOG(logINFO, ("\tgpio7 (CD): already defined\n"));
    }

    // define gpio7 direction
    if (executeCommand(CMD_GPIO7_DEFINE_IN, retvals, logDEBUG1) == FAIL) {
        snprintf(mess, MAX_STR_LENGTH,
                 "Could not set gpio7 (CD) as input for fpga (%s)\n", retvals);
        LOG(logERROR, (mess));
        return FAIL;
    }
    LOG(logINFO, ("\tgpio7 (CD): setting intput\n"));

    // define gpio9
    if (access(CMD_GPIO9_EXIST, F_OK) != 0) {
        if (executeCommand(CMD_GPIO9_DEFINE, retvals, logDEBUG1) == FAIL) {
            snprintf(mess, MAX_STR_LENGTH,
                     "Could not define gpio9 (nConfig) for fpga (%s)\n",
                     retvals);
            LOG(logERROR, (mess));
            return FAIL;
        }
        LOG(logINFO, ("\tgpio9 (nConfig): defined\n"));
    } else {
        LOG(logINFO, ("\tgpio9 (nConfig): already defined\n"));
    }

    // define gpio3 (not chip enable)
    if (latestKernelVerified == 1) {
        if (access(CMD_GPIO3_EXIST, F_OK) != 0) {
            if (executeCommand(CMD_GPIO3_DEFINE, retvals, logDEBUG1) == FAIL) {
                snprintf(mess, MAX_STR_LENGTH,
                         "Could not define gpio3 (nCE) for fpga (%s)\n",
                         retvals);
                LOG(logERROR, (mess));
                return FAIL;
            }
            LOG(logINFO, ("\tgpio3 (nCE): defined\n"));
        } else {
            LOG(logINFO, ("\tgpio3 (nCE): already defined\n"));
        }
    }

    return OK;
}

int FPGAdontTouchFlash(char *mess, int programming) {
#ifdef VIRTUAL
    return OK;
#endif
    char retvals[MAX_STR_LENGTH] = {0};
    // define gpio9 as output
    if (executeCommand(CMD_GPIO9_DEFINE_OUT, retvals, logDEBUG1) == FAIL) {
        snprintf(mess, MAX_STR_LENGTH,
                 "Could not set gpio9 (nConfig) as output for fpga (%s)\n",
                 retvals);
        LOG(logERROR, (mess));
        return FAIL;
    }
    LOG(logINFO, ("\tgpio9 (nConfig): setting output\n"));

    // define gpio3 as output
    if (programming && latestKernelVerified == 1) {
        if (executeCommand(CMD_GPIO3_DEFINE_OUT, retvals, logDEBUG1) == FAIL) {
            snprintf(mess, MAX_STR_LENGTH,
                     "Could not set gpio3 (nCE) as output for fpga (%s)\n",
                     retvals);
            LOG(logERROR, (mess));
            return FAIL;
        }
        LOG(logINFO, ("\tgpio3 (nCE): setting output\n"));
    }

    // tell FPGA to not: gpio9
    if (executeCommand(CMD_GPIO9_DONT_TOUCH_FLASH, retvals, logDEBUG1) ==
        FAIL) {
        snprintf(
            mess, MAX_STR_LENGTH,
            "Could not set gpio9 (nConfig) to not touch flash for fpga (%s)\n",
            retvals);
        LOG(logERROR, (mess));
        return FAIL;
    }
    LOG(logINFO, ("\tgpio9 (nConfig): fpga dont touch flash (Low)\n"));

    // tell FPGA to not: gpio3
    if (programming && latestKernelVerified == 1) {
        if (executeCommand(CMD_GPIO3_DONT_TOUCH_FLASH, retvals, logDEBUG1) ==
            FAIL) {
            snprintf(
                mess, MAX_STR_LENGTH,
                "Could not set gpio3 (nCE) to not touch flash for fpga (%s)\n",
                retvals);
            LOG(logERROR, (mess));
            return FAIL;
        }
        LOG(logINFO, ("\tgpio3 (nCE): fpga dont touch flash (High)\n"));
    }
    // usleep(100*1000);
    return OK;
}

int FPGATouchFlash(char *mess, int programming) {
#ifdef VIRTUAL
    return OK;
#endif
    char retvals[MAX_STR_LENGTH] = {0};
    // tell FPGA to touch flash to program itself
    if (executeCommand(CMD_GPIO9_DEFINE_IN, retvals, logDEBUG1) == FAIL) {
        snprintf(mess, MAX_STR_LENGTH,
                 "Could not set gpio9 (nConfig) as input for fpga (%s)\n",
                 retvals);
        LOG(logERROR, (mess));
        return FAIL;
    }
    LOG(logINFO, ("\tgpio9 (nConfig): setting input\n"));

    if (programming && latestKernelVerified == 1) {
        if (executeCommand(CMD_GPIO3_DEFINE_IN, retvals, logDEBUG1) == FAIL) {
            snprintf(mess, MAX_STR_LENGTH,
                     "Could not set gpio3 (nCE) as input for fpga (%s)\n",
                     retvals);
            LOG(logERROR, (mess));
            return FAIL;
        }
        LOG(logINFO, ("\tgpio3 (nCE): setting input\n"));
    }
    return OK;
}

int resetFPGA(char *mess) {
    LOG(logINFOBLUE, ("Reseting FPGA\n"));
#ifdef VIRTUAL
    return OK;
#endif
    if (FPGAdontTouchFlash(mess, 0) == FAIL) {
        return FAIL;
    }
    if (FPGATouchFlash(mess, 0) == FAIL) {
        return FAIL;
    }
    usleep(CTRL_SRVR_INIT_TIME_US);
    return OK;
}

int emptyTempFolder(char *mess) {
#ifdef VIRTUAL
    return OK;
#else
    char cmd[MAX_STR_LENGTH] = {0};
    char retvals[MAX_STR_LENGTH] = {0};

    if (snprintf(cmd, MAX_STR_LENGTH, "rm -fr %s",
                 TEMP_PROG_FOLDER_NAME_ALL_FILES) >= MAX_STR_LENGTH) {
        sprintf(mess,
                "Could not update %s. Command to empty %s folder is too long\n",
                messageType, TEMP_PROG_FOLDER_NAME);
        LOG(logERROR, (mess));
        return FAIL;
    }
    if (executeCommand(cmd, retvals, logDEBUG1) == FAIL) {
        snprintf(mess, MAX_STR_LENGTH,
                 "Could not update %s. (could not empty %s folder: %s)\n",
                 messageType, TEMP_PROG_FOLDER_NAME, retvals);
        LOG(logERROR, (mess));
        return FAIL;
    }
    LOG(logINFO, ("\tEmptied temp folder(%s)\n", TEMP_PROG_FOLDER_NAME));
    return OK;
#endif
}

int allowUpdate(char *mess, char *functionType) {
    LOG(logINFO, ("\tVerifying %s allowed...\n", functionType));

#ifdef VIRTUAL
    return OK;
#endif
    char retvals[MAX_STR_LENGTH] = {0};
    if (executeCommand(CMD_GET_AMD_FLASH, retvals, logDEBUG1) == FAIL) {
        // no amd found
        if (strstr(retvals, "No result") != NULL) {
            LOG(logINFO, ("\tNot Amd Flash\n"));
            return OK;
        }
        // could not figure out if amd
        snprintf(
            mess, MAX_STR_LENGTH,
            "Could not update %s. (Could not figure out if Amd flash: %s)\n",
            functionType, retvals);
        LOG(logERROR, (mess));
        return FAIL;
    }
    // amd, only current kernel works with amd flash
    if (validateKernelVersion(KERNEL_DATE_VRSN_3GPIO) == FAIL) {
        getKernelVersion(retvals);
        snprintf(mess, MAX_STR_LENGTH,
                 "Could not update %s. Kernel version %s is too old to "
                 "update the Amd flash/ root directory. Most likely, blackfin "
                 "needs rescue or replacement. Please contact us.\n",
                 functionType, retvals);
        LOG(logERROR, (mess));
        return FAIL;
    }
    LOG(logINFO, ("\tAmd flash with compatible kernel version\n"));
    return OK;
}

int preparetoCopyProgram(char *mess, char *functionType, FILE **fd,
                         uint64_t fsize) {

    if (emptyTempFolder(mess) == FAIL) {
        return FAIL;
    }

    // check available memory to copy program
    {
        struct sysinfo info;
        sysinfo(&info);
        if (fsize >= info.freeram) {
            sprintf(mess,
                    "Could not %s. Not enough memory to copy "
                    "program. [File size:%ldMB, free RAM: %ldMB]\n",
                    functionType, (long int)(fsize / (1024 * 1024)),
                    (long int)(info.freeram / (1024 * 1024)));
            LOG(logERROR, (mess));
            return FAIL;
        }
    }

    // open file to copy program
    *fd = fopen(TEMP_PROG_FILE_NAME, "w");
    if (*fd == NULL) {
        sprintf(mess, "Could not %s. Unable to open %s in write mode\n",
                functionType, TEMP_PROG_FILE_NAME);
        LOG(logERROR, (mess));
        return FAIL;
    }
    LOG(logINFO, ("\tGoing to copy program to %s\n", TEMP_PROG_FILE_NAME));
    return OK;
}

int eraseAndWriteToFlash(char *mess, enum PROGRAM_INDEX index,
                         char *functionType, char *clientChecksum,
                         ssize_t fsize, int forceDeleteNormalFile) {

    memset(messageType, 0, sizeof(messageType));
    strcpy(messageType, functionType);

    if (getDrive(mess, index) == FAIL) {
        return FAIL;
    }

    FILE *flashfd = NULL;
    FILE *srcfd = NULL;
    if (openFileForFlash(mess, index, &flashfd, &srcfd,
                         forceDeleteNormalFile) == FAIL) {
        return FAIL;
    }

    if (index == PROGRAM_FPGA) {
        if (FPGAdontTouchFlash(mess, 1) == FAIL) {
            return FAIL;
        }
    }

    if (eraseFlash(mess) == FAIL) {
        fclose(flashfd);
        fclose(srcfd);
        return FAIL;
    }

    if (writeToFlash(mess, fsize, flashfd, srcfd) == FAIL) {
        return FAIL;
    }

    if (emptyTempFolder(mess) == FAIL) {
        return FAIL;
    }

    /* remove condition when flash fpga fixed */
    if (index == PROGRAM_KERNEL) {
        if (verifyChecksumFromFlash(mess, messageType, clientChecksum,
                                    flashDriveName, fsize) == FAIL) {
            return FAIL;
        }
    }

    if (index == PROGRAM_FPGA) {
        if (waitForFPGAtoTouchFlash(mess) == FAIL) {
            return FAIL;
        }
    }

    // kernel
    else {
        char retvals[MAX_STR_LENGTH] = {0};
        if (executeCommand("sync", retvals, logDEBUG1) == FAIL) {
            snprintf(mess, MAX_STR_LENGTH,
                     "Could not update %s. (could not sync)\n", messageType);
            LOG(logERROR, (mess));
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
    // root:/>  cat /proc/mtd
    // dev:    size   erasesize  name
    // mtd0: 00040000 00020000 "bootloader(nor)"
    // mtd1: 00100000 00020000 "linux kernel(nor)"
    // mtd2: 002c0000 00020000 "file system(nor)"
    // mtd3: 01000000 00010000 "bitfile(spi)"

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
        LOG(logERROR, (mess));
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
        sprintf(mess,
                "Could not %s. Could not get mtd drive to flash (strtok "
                "fail).\n",
                messageType);
        LOG(logERROR, (mess));
        return FAIL;
    }

    memset(flashDriveName, 0, sizeof(flashDriveName));
    sprintf(flashDriveName, "/dev/%s", pch);
    LOG(logINFO, ("\tFlash drive found: %s\n", flashDriveName));
    return OK;
}

int openFileForFlash(char *mess, enum PROGRAM_INDEX index, FILE **flashfd,
                     FILE **srcfd, int forceDeleteNormalFile) {
    // open src file
    *srcfd = fopen(TEMP_PROG_FILE_NAME, "r");
    if (*srcfd == NULL) {
        sprintf(mess,
                "Could not %s. Unable to open temp program file %s in read "
                "mode\n",
                messageType, TEMP_PROG_FILE_NAME);
        LOG(logERROR, (mess));
        return FAIL;
    }
    LOG(logDEBUG1, ("Temp file ready for reading\n"));

    if (checkNormalFile(mess, index, forceDeleteNormalFile) == FAIL) {
        fclose(*srcfd);
        return FAIL;
    }

    // open flash drive for writing
    *flashfd = fopen(flashDriveName, "w");
    if (*flashfd == NULL) {
        fclose(*srcfd);
        sprintf(mess,
                "Could not %s. Unable to open flash drive %s in write "
                "mode\n",
                messageType, flashDriveName);
        LOG(logERROR, (mess));
        return FAIL;
    }
    LOG(logINFO, ("\tFlash ready for writing\n"));
    return OK;
}

int checkNormalFile(char *mess, enum PROGRAM_INDEX index,
                    int forceDeleteNormalFile) {
#ifndef VIRTUAL
    // check if its a normal file or special file
    struct stat buf;
    if (stat(flashDriveName, &buf) == -1) {
        sprintf(mess, "Could not %s. Unable to find the flash drive %s\n",
                messageType, flashDriveName);
        LOG(logERROR, (mess));
        return FAIL;
    }
    // zero = normal file (not char special drive file)
    if (!S_ISCHR(buf.st_mode)) {
        // kernel memory is not permanent
        if (index != PROGRAM_FPGA) {
            sprintf(mess,
                    "Could not %s. The flash drive found is a normal file. "
                    "Reboot board using 'rebootcontroller' command to load "
                    "proper device tree\n",
                    messageType);
            LOG(logERROR, (mess));
            return FAIL;
        }

        // user does not allow to fix it (default)
        if (forceDeleteNormalFile == 0) {
            sprintf(mess,
                    "Could not %s. The flash drive %s found for fpga "
                    "programming is a normal file. To "
                    "fix this (by deleting this file, creating the flash drive "
                    "and proceeding with "
                    "programming), re-run the programming command "
                    "'programfpga' with parameter "
                    "'--force-delete-normal-file'\n",
                    messageType, flashDriveName);
            LOG(logERROR, (mess));
            return FAIL;
        }

        // fpga memory stays after a reboot, user allowed to fix it
        LOG(logWARNING,
            ("Flash drive invalidated (normal file). Fixing it...\n"));

        // user allows to fix it, so force delete normal file
        char cmd[MAX_STR_LENGTH] = {0};
        char retvals[MAX_STR_LENGTH] = {0};

        if (snprintf(cmd, MAX_STR_LENGTH, "rm %s", flashDriveName) >=
            MAX_STR_LENGTH) {
            sprintf(mess,
                    "Could not update %s. Command to delete normal file %s is "
                    "too long\n",
                    messageType, flashDriveName);
            LOG(logERROR, (mess));
            return FAIL;
        }
        if (executeCommand(cmd, retvals, logDEBUG1) == FAIL) {
            snprintf(
                mess, MAX_STR_LENGTH,
                "Could not update %s. (could not delete normal file %s: %s)\n",
                messageType, flashDriveName, retvals);
            LOG(logERROR, (mess));
            return FAIL;
        }
        LOG(logINFO, ("\tDeleted Normal File (%s)\n", flashDriveName));

        // create special drive
        if (snprintf(cmd, MAX_STR_LENGTH, "%s %s %s",
                     CMD_CREATE_DEVICE_FILE_PART1, flashDriveName,
                     CMD_CREATE_DEVICE_FILE_PART2) >= MAX_STR_LENGTH) {
            sprintf(mess,
                    "Could not update %s. Command to create special file %s is "
                    "too long\n",
                    messageType, flashDriveName);
            LOG(logERROR, (mess));
            return FAIL;
        }
        if (executeCommand(cmd, retvals, logDEBUG1) == FAIL) {
            snprintf(
                mess, MAX_STR_LENGTH,
                "Could not update %s. (could not create special file %s: %s)\n",
                messageType, flashDriveName, retvals);
            LOG(logERROR, (mess));
            return FAIL;
        }
        LOG(logINFO, ("\tSpecial File created (%s)\n", flashDriveName));
    } else {
        LOG(logINFO, ("\tValidated flash drive (not a normal file)\n"));
    }
#endif
    return OK;
}

int eraseFlash(char *mess) {
    LOG(logINFO, ("\tErasing Flash...\n"));

#ifdef VIRTUAL
    return OK;
#endif
    char cmd[MAX_STR_LENGTH] = {0};
    char retvals[MAX_STR_LENGTH] = {0};
    if (snprintf(cmd, MAX_STR_LENGTH, "flash_eraseall %s", flashDriveName) >=
        MAX_STR_LENGTH) {
        sprintf(mess, "Could not %s. Command to erase flash is too long\n",
                messageType);
        LOG(logERROR, (mess));
        return FAIL;
    }
    if (executeCommand(cmd, retvals, logDEBUG1) == FAIL) {
        snprintf(mess, MAX_STR_LENGTH,
                 "Could not update %s. (could not erase flash: %s)\n",
                 messageType, retvals);
        LOG(logERROR, (mess));
        return FAIL;
    }
    LOG(logINFO, ("\tFlash erased\n"));
    return OK;
}

int writeToFlash(char *mess, ssize_t fsize, FILE *flashfd, FILE *srcfd) {
    LOG(logDEBUG1, ("writing to flash\n"));

    char *buffer = malloc(FLASH_BUFFER_MEMORY_SIZE);
    if (buffer == NULL) {
        fclose(flashfd);
        fclose(srcfd);
        sprintf(mess,
                "Could not %s. Memory allocation to write to "
                "flash failed.\n",
                messageType);
        LOG(logERROR, (mess));
        return FAIL;
    }
    LOG(logINFO, ("\tWriting to Flash...\n"));

    int oldProgress = 0;
    ssize_t totalBytes = 0;
    ssize_t bytes =
        fread((void *)buffer, sizeof(char), FLASH_BUFFER_MEMORY_SIZE, srcfd);

    while (bytes > 0) {

        ssize_t bytesWritten =
            fwrite((void *)buffer, sizeof(char), bytes, flashfd);
        totalBytes += bytesWritten;

        if (bytesWritten != bytes) {
            free(buffer);
            fclose(flashfd);
            fclose(srcfd);
            sprintf(mess,
                    "Could not %s. Could not write to flash (bytes "
                    "written:%ld, expected: "
                    "%ld, total written:%ld)\n",
                    messageType, (long int)bytesWritten, (long int)bytes,
                    (long int)totalBytes);
            LOG(logERROR, (mess));
            return FAIL;
        }

        // print progress
        if (fsize > 0) {
            int progress = (int)(((double)(totalBytes) / fsize) * 100);
            if (oldProgress != progress) {
                printf("%d%%\r", progress);
                fflush(stdout);
                oldProgress = progress;
            }
        } else
            printf(".");

        bytes = fread((void *)buffer, sizeof(char), FLASH_BUFFER_MEMORY_SIZE,
                      srcfd);
    }
    if (fsize <= 0) {
        printf("\n");
    }
    free(buffer);
    fclose(flashfd);
    fclose(srcfd);
    LOG(logINFO, ("\tWrote %ld bytes to flash\n", totalBytes));

    if (totalBytes != fsize) {
        sprintf(mess,
                "Could not %s. Incorrect bytes written to flash %lu "
                "[expected: %lu]\n",
                messageType, totalBytes, fsize);
        LOG(logERROR, (mess));
        return FAIL;
    }
    return OK;
}

int waitForFPGAtoTouchFlash(char *mess) {
    // touch and program
    if (FPGATouchFlash(mess, 1) == FAIL) {
        return FAIL;
    }

#ifdef VIRTUAL
    return OK;
#endif
    LOG(logINFO, ("\tWaiting for FPGA to program from flash... \n\t[gpio7 (CD) "
                  "should be High when done]\n"));
    int timeSpent = 0;

    int result = 0;
    while (result == 0) {
        // time taken for fpga to pick up from flash
        usleep(1000);
        timeSpent += 1000;
        if (timeSpent >= MAX_TIME_FPGA_TOUCH_FLASH_US) {
            sprintf(
                mess,
                "Could not program fpga. (exceeded max time allowed: %ds)\n",
                MAX_TIME_FPGA_TOUCH_FLASH_US / (1000 * 1000));
            LOG(logERROR, (mess));
            return FAIL;
        }

        // read gpio status
        char retvals[MAX_STR_LENGTH] = {0};
        if (FAIL ==
            executeCommand(CMD_FPGA_PICKED_STATUS, retvals, logDEBUG1)) {
            snprintf(
                mess, MAX_STR_LENGTH,
                "Could not program fpga. (could not read gpio status: %s)\n",
                retvals);
            // LOG(logERROR, (mess)); already printed in executecommand
            return FAIL;
        }

        // convert to int
        if (sscanf(retvals, "%d\n", &result) != 1) {
            snprintf(mess, MAX_STR_LENGTH,
                     "Could not program fpga. (could not scan int for gpio "
                     "status: [%s])\n",
                     retvals);
            LOG(logERROR, (mess));
            return FAIL;
        }
        LOG(logDEBUG1, ("gpi07 (CD)returned %d\n", result));
    }
    LOG(logINFO,
        ("\tFPGA has picked up the program from flash. gpio7 (CD) is High\n"));
    return OK;
}
