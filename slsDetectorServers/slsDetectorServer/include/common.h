// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once

#include "sls/md5.h"
#include <stdint.h> // int64_t
#include <stdio.h>
#include <sys/types.h>
#include <time.h>

#define UPDATE_FILE "update.txt"
#ifdef VIRTUAL
#define TEMP_PROG_FOLDER_NAME "/tmp/"
#else
#define TEMP_PROG_FOLDER_NAME           "/var/tmp/"
#define TEMP_PROG_FOLDER_NAME_ALL_FILES "/var/tmp/*"
#endif

#define TEMP_PROG_FILE_NAME TEMP_PROG_FOLDER_NAME "tmp.rawbin"

enum numberMode { DEC, HEX };
enum PROGRAM_INDEX { PROGRAM_FPGA, PROGRAM_KERNEL, PROGRAM_SERVER };

/**
 * Convert a value from a range to a different range (eg voltage to dac or vice
 * versa)
 * @param inputMin input minimum
 * @param inputMax input maximum
 * @param outputMin output minimum
 * @param outputMax output maximum
 * @param inputValue input value
 * @param outputValue pointer to output value
 * @returns FAIL if input value is out of bounds, else OK
 */
int ConvertToDifferentRange(int inputMin, int inputMax, int outputMin,
                            int outputMax, int inputValue, int *outputValue);

int getAbsPath(char *buf, size_t bufSize, char *fname);

int getTimeFromString(char *buf, time_t *result);

int getKernelVersion(char *retvals);

int validateKernelVersion(char *expectedVersion);

void validate(int *ret, char *mess, int arg, int retval, char *modename,
              enum numberMode nummode);
void validate64(int *ret, char *mess, int64_t arg, int64_t retval,
                char *modename, enum numberMode nummode);

int getModuleIdInFile(int *ret, char *mess, char *fileName);
int verifyChecksumFromBuffer(char *mess, char *functionType,
                             char *clientChecksum, char *buffer, ssize_t bytes);
int verifyChecksumFromFile(char *mess, char *functionType, char *clientChecksum,
                           char *fname);
int verifyChecksumFromFlash(char *mess, char *functionType,
                            char *clientChecksum, char *fname, ssize_t fsize);
int verifyChecksum(char *mess, char *functionType, char *clientChecksum,
                   MD5_CTX *c, char *msg);
int setupDetectorServer(char *mess, char *sname);

int writeBinaryFile(char *mess, char *fname, char *buffer,
                    const uint64_t filesize, char *errorPrefix);

int moveBinaryFile(char *mess, char *dest, char *src, char *errorPrefix);

int createEmptyFile(char *mess, char *fname, char *errorPrefix);
int deleteFile(char *mess, char *fname, char *errorPrefix);

int deleteOldServers(char *mess, char *newServerPath, char *errorPrefix);

int readParameterFromFile(char *fname, char *parameterName, int *value);

int createAbsoluteDirectory(char *mess, const char *absPath, char *errorPrefix);
int deleteAbsoluteDirectory(char *mess, const char *absPath, char *errorPrefix);

int deleteItem(char *mess, int isFile, const char *absPath, char *errorPrefix);