// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once

#include "common.h"

#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>

#define BLACKFIN_DEFINED

int defineGPIOpins(char *mess);
int FPGAdontTouchFlash(char *mess, int programming);
int FPGATouchFlash(char *mess, int programming);
int resetFPGA(char *mess);

int emptyTempFolder(char *mess);
int allowUpdate(char *mess, char *functionType);
/**
 * deletes old file
 * verify memory available to copy
 * open file to copy
 */
int preparetoCopyProgram(char *mess, char *functionType, FILE **fd,
                         uint64_t fsize);
int eraseAndWriteToFlash(char *mess, enum PROGRAM_INDEX index,
                         char *functionType, char *clientChecksum,
                         ssize_t fsize, int forceDeleteNormalFile);
int getDrive(char *mess, enum PROGRAM_INDEX index);
/** Notify fpga not to touch flash, open src and flash drive to write */
int openFileForFlash(char *mess, enum PROGRAM_INDEX index, FILE **flashfd,
                     FILE **srcfd, int forceDeleteNormalFile);
int checkNormalFile(char *mess, enum PROGRAM_INDEX index,
                    int forceDeleteNormalFile);
int eraseFlash(char *mess);
/* write from tmp file to flash */
int writeToFlash(char *mess, ssize_t fsize, FILE *flashfd, FILE *srcfd);
/** Notify fpga to pick up firmware from flash and wait for status confirmation
 */
int waitForFPGAtoTouchFlash(char *mess);
