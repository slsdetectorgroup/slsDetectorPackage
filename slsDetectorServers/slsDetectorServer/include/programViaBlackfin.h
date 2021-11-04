// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once

#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>

#define TEMP_PROG_FOLDER_NAME "/var/tmp/"
#define TEMP_PROG_FILE_NAME   TEMP_PROG_FOLDER_NAME "tmp.rawbin"

void defineGPIOpins();
int FPGAdontTouchFlash();
int FPGATouchFlash();
int resetFPGA();

int emptyTempFolder(char *mess);
/**
 * deletes old file
 * verify memory available to copy
 * open file to copy
 */
int preparetoCopyProgram(FILE **fd, uint64_t fsize, char *mess);
int copyToFlash(enum PROGRAMINDEX index, char *mType, ssize_t fsize,
                char *clientChecksum, char *mess);
int getDrive(enum PROGRAMINDEX index, char *mess);
/** Notify fpga not to touch flash, open src and flash drive to write */
int openFileForFlash(FILE **flashfd, FILE **srcfd, char *mess);
int eraseFlash(char *mess);
/* write from tmp file to flash */
int writeToFlash(ssize_t fsize, FILE *flashfd, FILE *srcfd, char *mess);
/** Notify fpga to pick up firmware from flash and wait for status confirmation
 */
int waitForFPGAtoTouchFlash(char *mess);
