// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once
#include <cstdint>
#include <string>

namespace sls {

/**
   @short data structure to hold the detector data after postprocessing
 */
class detectorData {
  public:
    detectorData(double progressIndex, std::string fileName, int nx, int ny,
                 char *data, int databytes, int dynamicRange,
                 uint64_t fileIndex, bool completeImage)
        : progressIndex(progressIndex), fileName(fileName),
          fileIndex(fileIndex), nx(nx), ny(ny), data(data),
          databytes(databytes), dynamicRange(dynamicRange),
          completeImage(completeImage){};

    detectorData(double progressIndex, std::string fileName, int nx, int ny,
                 char *data, int databytes, int dynamicRange,
                 uint64_t fileIndex, bool completeImage,
                 std::array<int, 4> rxRoi)
        : progressIndex(progressIndex), fileName(fileName),
          fileIndex(fileIndex), nx(nx), ny(ny), data(data),
          databytes(databytes), dynamicRange(dynamicRange),
          completeImage(completeImage), rxRoi(rxRoi){};
    /**
     * data has to be deleted by caller
     */
    ~detectorData(){};

    int64_t getChannel(int i) {
        int off = dynamicRange / 8;
        if (off == 1) {
            char val = *(data + i);
            return val;
        }
        if (off == 2) {
            int16_t val = *((int16_t *)(data + i * off));
            return val;
        }
        if (off == 4) {
            int32_t val = *((int32_t *)(data + i * off));
            return val;
        }
        if (off == 8) {
            int64_t val = *((int64_t *)(data + i * off));
            return val;
        }
        return -1;
    }
    // private:
    double progressIndex;
    std::string fileName;
    uint64_t fileIndex;
    int nx;
    int ny;
    char *data{nullptr};
    int databytes;
    int dynamicRange;
    bool completeImage;
    std::array<int, 4> rxRoi{{-1, -1, -1, -1}};
};

} // namespace sls
