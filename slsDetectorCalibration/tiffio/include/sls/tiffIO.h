// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once
#include <cstdint>
void *WriteToTiff(float *imgData, const char *imgname, int nrow, int ncol);
float *ReadFromTiff(const char *imgname, uint32_t &nrow, uint32_t &ncol);

