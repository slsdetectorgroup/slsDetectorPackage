#include "programFpgaNios.h"
#include "ansi.h"
#include "clogger.h"
#include "slsDetectorServer_defs.h"

#include <string.h>
#include <unistd.h> // usleep

/* global variables */
#define MTDSIZE 10
char mtdvalue[MTDSIZE] = {0};
#define MICROCONTROLLER_FILE "/dev/ttyAL0"

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

int findFlash(char *mess) {
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
    char output[255];
    memset(output, 0, 255);
    FILE *fp = popen("awk \'$5== \"Application\" {print $1}\' /proc/mtd", "r");
    if (fp == NULL) {
        strcpy(mess, "popen returned NULL. Need that to get mtd drive.\n");
        LOG(logERROR, (mess));
        return RO_TRIGGER_IN_FALLING_EDGE;
    }
    if (fgets(output, sizeof(output), fp) == NULL) {
        strcpy(mess, "fgets returned NULL. Need that to get mtd drive.\n");
        LOG(logERROR, (mess));
        return FAIL;
    }
    pclose(fp);
    memset(mtdvalue, 0, MTDSIZE);
    strcpy(mtdvalue, "/dev/");
    char *pch = strtok(output, ":");
    if (pch == NULL) {
        strcpy(mess, "Could not get mtd value\n");
        LOG(logERROR, (mess));
        return FAIL;
    }
    strcat(mtdvalue, pch);
    LOG(logINFO, ("\tFlash drive found: %s\n", mtdvalue));
    return OK;
}

void eraseFlash() {
    LOG(logDEBUG1, ("Erasing Flash...\n"));
    char command[255];
    memset(command, 0, 255);
    sprintf(command, "flash_erase %s 0 0", mtdvalue);
    system(command);
    LOG(logINFO, ("\tFlash erased\n"));
}

int eraseAndWriteToFlash(char *mess, char *fpgasrc, uint64_t fsize) {
    if (findFlash(mess) == FAIL) {
        return FAIL;
    }
    eraseFlash();

    // open file pointer to flash
    FILE *filefp = fopen(mtdvalue, "w");
    if (filefp == NULL) {
        sprintf(mess, "Unable to open %s in write mode\n", mtdvalue);
        LOG(logERROR, (mess));
        return FAIL;
    }
    LOG(logINFO, ("\tFlash ready for writing\n"));

    // write to flash
    if (writeFPGAProgram(mess, fpgasrc, fsize, filefp) == FAIL) {
        fclose(filefp);
        return FAIL;
    }

    fclose(filefp);
    return OK;
}

int writeFPGAProgram(char *mess, char *fpgasrc, uint64_t fsize, FILE *filefp) {
    LOG(logDEBUG1, ("Writing to flash...\n"
                    "\taddress of fpgasrc:%p\n"
                    "\tfsize:%lu\n\tpointer:%p\n",
                    (void *)fpgasrc, fsize, (void *)filefp));

    uint64_t retval = fwrite((void *)fpgasrc, sizeof(char), fsize, filefp);
    if (retval != fsize) {
        sprintf(
            mess,
            "Could not write FPGA source to flash (size:%llu), write %llu\n",
            (long long unsigned int)fsize, (long long unsigned int)retval);
        LOG(logERROR, (mess));
        return FAIL;
    }
    LOG(logINFO, ("\tProgram written to flash\n"));
    return OK;
}
