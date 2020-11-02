#include "programFpgaBlackfin.h"
#include "sls/ansi.h"
#include "clogger.h"
#include "slsDetectorServer_defs.h"

#include <string.h>
#include <unistd.h> // usleep

/* global variables */
#define MTDSIZE                      10
#define MAX_TIME_FPGA_TOUCH_FLASH_US (10 * 1000 * 1000) // 10s

int gpioDefined = 0;
char mtdvalue[MTDSIZE] = {0};

void defineGPIOpins() {
    if (!gpioDefined) {
        // define the gpio pins
        system("echo 7 > /sys/class/gpio/export");
        system("echo 9 > /sys/class/gpio/export");
        // define their direction
        system("echo in  > /sys/class/gpio/gpio7/direction");
        system("echo out > /sys/class/gpio/gpio9/direction");
        LOG(logINFO, ("gpio pins defined\n"));
        gpioDefined = 1;
    } else
        LOG(logDEBUG1, ("gpio pins already defined earlier\n"));
}

void FPGAdontTouchFlash() {
    // tell FPGA to not touch flash
    system("echo 0 > /sys/class/gpio/gpio9/value");
    // usleep(100*1000);
}

void FPGATouchFlash() {
    // tell FPGA to touch flash to program itself
    system("echo 1 > /sys/class/gpio/gpio9/value");
}

void resetFPGA() {
    LOG(logINFOBLUE, ("Reseting FPGA\n"));
    FPGAdontTouchFlash();
    FPGATouchFlash();
    usleep(CTRL_SRVR_INIT_TIME_US);
}

void eraseFlash() {
    LOG(logDEBUG1, ("Erasing Flash\n"));
    char command[255];
    memset(command, 0, 255);
    sprintf(command, "flash_eraseall %s", mtdvalue);
    system(command);
    LOG(logINFO, ("Flash erased\n"));
}

int startWritingFPGAprogram(FILE **filefp) {
    LOG(logDEBUG1, ("Start Writing of FPGA program\n"));

    // getting the drive
    // root:/>  cat /proc/mtd
    // dev:    size   erasesize  name
    // mtd0: 00040000 00020000 "bootloader(nor)"
    // mtd1: 00100000 00020000 "linux kernel(nor)"
    // mtd2: 002c0000 00020000 "file system(nor)"
    // mtd3: 01000000 00010000 "bitfile(spi)"
    char output[255];
    memset(output, 0, 255);
    FILE *fp = popen(
        "awk \'$4== \"\\\"bitfile(spi)\\\"\" {print $1}\' /proc/mtd", "r");
    if (fp == NULL) {
        LOG(logERROR, ("popen returned NULL. Need that to get mtd drive.\n"));
        return 1;
    }
    if (fgets(output, sizeof(output), fp) == NULL) {
        LOG(logERROR, ("fgets returned NULL. Need that to get mtd drive.\n"));
        return 1;
    }
    pclose(fp);
    memset(mtdvalue, 0, MTDSIZE);
    strcpy(mtdvalue, "/dev/");
    char *pch = strtok(output, ":");
    if (pch == NULL) {
        LOG(logERROR, ("Could not get mtd value\n"));
        return 1;
    }
    strcat(mtdvalue, pch);
    LOG(logINFO, ("Flash drive found: %s\n", mtdvalue));

    FPGAdontTouchFlash();

    // writing the program to flash
    *filefp = fopen(mtdvalue, "w");
    if (*filefp == NULL) {
        LOG(logERROR, ("Unable to open %s in write mode\n", mtdvalue));
        return 1;
    }
    LOG(logINFO, ("Flash ready for writing\n"));

    return 0;
}

int stopWritingFPGAprogram(FILE *filefp) {
    LOG(logDEBUG1, ("Stopping of writing FPGA program\n"));

    if (filefp != NULL) {
        fclose(filefp);
    }

    // touch and program
    FPGATouchFlash();

    LOG(logINFO, ("Waiting for FPGA to program from flash\n"));
    // waiting for success or done
    char output[255];
    int res = 0;
    int timeSpent = 0;
    while (res == 0) {
        // time taken for fpga to pick up from flash
        usleep(1000);
        timeSpent += 1000;
        if (timeSpent >= MAX_TIME_FPGA_TOUCH_FLASH_US) {
            return 1;
        }
        FILE *sysFile = popen("cat /sys/class/gpio/gpio7/value", "r");
        fgets(output, sizeof(output), sysFile);
        pclose(sysFile);
        sscanf(output, "%d", &res);
        LOG(logDEBUG1, ("gpi07 returned %d\n", res));
    }
    LOG(logINFO, ("FPGA has picked up the program from flash\n"));
    return 0;
}

int writeFPGAProgram(char *fpgasrc, uint64_t fsize, FILE *filefp) {
    LOG(logDEBUG1,
        ("Writing of FPGA Program\n"
         "\taddress of fpgasrc:%p\n"
         "\tfsize:%llu\n\tpointer:%p\n",
         (void *)fpgasrc, (long long unsigned int)fsize, (void *)filefp));

    if (fwrite((void *)fpgasrc, sizeof(char), fsize, filefp) != fsize) {
        LOG(logERROR, ("Could not write FPGA source to flash (size:%llu)\n",
                       (long long unsigned int)fsize));
        return 1;
    }
    LOG(logDEBUG1, ("program written to flash\n"));
    return 0;
}
