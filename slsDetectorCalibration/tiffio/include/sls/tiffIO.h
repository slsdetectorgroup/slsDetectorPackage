// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once
#include <cstdint>

//Write 32bit float data to tiff file
//Always returns nullptr, prints message on failure
void *WriteToTiff(float *imgData, const char *imgname, int ncol, int nrow);

//Read 32bit float data from tiff file, returns pointer to data and sets
//image dimensions in the out parameters nrow, ncol. 
//Returns nullptr on failure
//The caller is responsible to deallocate the memory that the returned
//pointer points to. 
float *ReadFromTiff(const char *imgname, uint32_t &ncol, uint32_t &nrow);

