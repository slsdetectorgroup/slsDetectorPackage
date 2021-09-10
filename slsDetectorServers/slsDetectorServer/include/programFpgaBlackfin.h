#pragma once

#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>

#define TEMP_PROG_FILE_NAME "/var/tmp/tmp.rawbin"

void defineGPIOpins();
void FPGAdontTouchFlash();
void FPGATouchFlash();
void resetFPGA();

/**
 * deletes old file
 * verify memory available to copy
 * open file to copy
 */
int preparetoCopyFPGAProgram(FILE **fd, uint64_t fsize, char *mess);
int copyToFlash(ssize_t fsize, char *clientChecksum, char *mess);
int getDrive(char *mess);
/** Notify fpga not to touch flash, open src and flash drive to write */
int openFileForFlash(FILE **flashfd, FILE **srcfd, char *mess);
int eraseFlash(char *mess);
/* write from tmp file to flash */
int writeToFlash(ssize_t fsize, FILE *flashfd, FILE *srcfd, char *mess);
/** Notify fpga to pick up firmware from flash and wait for status confirmation
 */
int waitForFPGAtoTouchFlash(char *mess);
