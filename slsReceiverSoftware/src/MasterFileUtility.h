// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once

#include "MasterAttributes.h"

#ifdef HDF5C
#include "H5Cpp.h"
#include <mutex>
#ifndef H5_NO_NAMESPACE
using namespace H5;
#endif
#endif

namespace masterFileUtility {

std::string CreateMasterBinaryFile(const std::string filePath,
                                   const std::string fileNamePrefix,
                                   const uint64_t fileIndex,
                                   const bool overWriteEnable,
                                   const bool silentMode,
                                   MasterAttributes *attr);

#ifdef HDF5C
void LinkHDF5FileInMaster(const std::string &masterFileName,
                          const std::string &dataFilename,
                          const std::string &dataSetname,
                          const std::vector<std::string> parameterNames,
                          const bool silentMode, std::mutex *hdf5LibMutex);

std::string CreateMasterHDF5File(const std::string &filePath,
                                 const std::string &fileNamePrefix,
                                 const uint64_t fileIndex,
                                 const bool overWriteEnable,
                                 const bool silentMode, MasterAttributes *attr,
                                 std::mutex *hdf5LibMutex);

std::array<std::string, 2> CreateVirtualHDF5File(
    const std::string &filePath, const std::string &fileNamePrefix,
    const uint64_t fileIndex, const bool overWriteEnable, const bool silentMode,
    const int modulePos, const int numUnitsPerReadout,
    const uint32_t maxFramesPerFile, const uint64_t numImages,
    const uint32_t nPixelsX, const uint32_t nPixelsY,
    const uint32_t dynamicRange, const uint64_t numImagesCaught,
    const int numModX, const int numModY, const DataType dataType,
    const std::vector<std::string> parameterNames,
    const std::vector<DataType> parameterDataTypes, std::mutex *hdf5LibMutex,
    bool gotthard25um);
#endif
} // namespace masterFileUtility