// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once

#include "common.h"

#include <stdint.h>
#include <stdio.h>

#define NIOS_MAX_APP_IMAGE_SIZE (0x00580000)

/** Notify microcontroller of successful server start up */
void NotifyServerStartSuccess();

/** reset fpga and controller(only implemented for >= v1.1 boards) */
void rebootControllerAndFPGA();

int eraseAndWriteToFlash(char *mess, enum PROGRAM_INDEX index,
                         char *functionType, char *checksum, char *fpgasrc,
                         uint64_t fsize);
int getDrive(char *mess, enum PROGRAM_INDEX index);
int openFileForFlash(char *mess, FILE **flashfd);
int eraseFlash(char *mess);
int writeToFlash(char *mess, ssize_t fsize, FILE *flashfd, char *buffer);