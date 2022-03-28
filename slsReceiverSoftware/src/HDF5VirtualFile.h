// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once

#include "File.h"

#include <mutex>

class HDF5VirtualFile : private virtual slsDetectorDefs, public File {

  public:
    HDF5VirtualFile(std::mutex *hdf5LibMutex, bool g25);
    ~HDF5VirtualFile();

    std::array<std::string, 2> GetFileAndDatasetName() const override;
    void CloseFile() override;
    void CreateVirtualFile(
        const std::string &filePath, const std::string &fileNamePrefix,
        const uint64_t fileIndex, const bool overWriteEnable,
        const bool silentMode, const int modulePos,
        const int numUnitsPerReadout, const uint32_t maxFramesPerFile,
        const uint64_t numImages, const uint32_t nPixelsX,
        const uint32_t nPixelsY, const uint32_t dynamicRange,
        const uint64_t numImagesCaught, const int numModX, const int numModY,
        const DataType dataType, const std::vector<std::string> parameterNames,
        const std::vector<DataType> parameterDataTypes) override;

  private:
    std::mutex *hdf5Lib_;
    H5File *fd_{nullptr};
    std::string fileName_;
    std::string dataSetName_;
    bool gotthard25um;
};