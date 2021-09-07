#pragma once

#include <stdint.h>
#include <stdio.h>

#define NIOS_MAX_APP_IMAGE_SIZE (0x00580000)

/** Notify microcontroller of successful server start up */
void NotifyServerStartSuccess();

/** reset fpga and controller(only implemented for >= v1.1 boards) */
void rebootControllerAndFPGA();
int findFlash(char *mess);
void eraseFlash();
int eraseAndWriteToFlash(char *mess, char *checksum, char *fpgasrc,
                         uint64_t fsize);
int writeFPGAProgram(char *mess, char *fpgasrc, uint64_t fsize, FILE *filefp);
