// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package

#include "sls/tiffIO.h"
#include <iostream>
#include <tiffio.h>

void *WriteToTiff(float *imgData, const char *imgname, int ncol, int nrow) {
    constexpr uint32_t sampleperpixel = 1;
    TIFF *tif = TIFFOpen(imgname, "w");
    if (tif) {
        TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, ncol);
        TIFFSetField(tif, TIFFTAG_IMAGELENGTH, nrow);
        TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, sampleperpixel);
        TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, 32);
        TIFFSetField(tif, TIFFTAG_ORIENTATION, ORIENTATION_BOTLEFT);
        TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
        TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
        TIFFSetField(tif, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_IEEEFP);
        TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP,
                     TIFFDefaultStripSize(tif, ncol * sampleperpixel));

        for (int irow = 0; irow < nrow; irow++) {
            TIFFWriteScanline(tif, &imgData[irow * ncol], irow, 0);
        }
        TIFFClose(tif);
    } else {
        std::cout << "could not open file " << imgname << " for writing\n";
    }
    return nullptr;
}

float *ReadFromTiff(const char *imgname, uint32_t &ncol, uint32_t &nrow) {
    TIFF *tif = TIFFOpen(imgname, "r");
    if (tif) {
        TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &ncol);
        TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &nrow);
        float *imgData = new float[ncol * nrow];
        for (uint32_t irow = 0; irow < nrow; ++irow) {
            TIFFReadScanline(tif, &imgData[irow * ncol], irow);
        }
        TIFFClose(tif);
        return imgData;
    } else {
        std::cout << "could not open file " << imgname << " for reading\n";
        return nullptr;
    }
}
