// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#ifndef MY_TIFF_IO_H
#define MY_TIFF_IO_H

#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdio.h>
#include <string>
#include <vector>

/*****************************************************************************/
//
// CBFlib must be installed to use this program
//
/*****************************************************************************/
#include <tiffio.h>

#undef cbf_failnez
#define cbf_failnez(x)                                                         \
    {                                                                          \
        int err;                                                               \
        err = (x);                                                             \
        if (err) {                                                             \
            fprintf(stderr, "\nCBFlib fatal error %x \n", err);                \
            exit(-1);                                                          \
        }                                                                      \
    }

void *WriteToTiff(float *imgData, const char *imgname, int nrow, int ncol);

float *ReadFromTiff(const char *imgname, uint32_t &nrow, uint32_t &ncol);

#endif
