#pragma once

#include <stdint.h>
#include <stdio.h>

#define NIOS_MAX_APP_IMAGE_SIZE (0x00580000)

/** Notify microcontroller of successful server start up */
void NotifyServerStartSuccess();

/** reset fpga and controller(only implemented for >= v1.1 boards) */
void rebootControllerAndFPGA();

int eraseAndWriteToFlash(char *mess, char *checksum, char *fpgasrc, uint64_t fsize);
int getDrive(char *mess);
int openFileForFlash(FILE **flashfd, char *mess);
int eraseFlash(char *mess);
int writeToFlash(ssize_t fsize, FILE *flashfd, char *buffer, char *mess);