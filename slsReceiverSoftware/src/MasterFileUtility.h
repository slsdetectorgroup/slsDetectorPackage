// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once

#include "MasterAttributes.h"

#include <mutex>

namespace sls {

namespace masterFileUtility {

std::string CreateMasterBinaryFile(const std::string &filePath,
                                   const std::string &fileNamePrefix,
                                   const uint64_t fileIndex,
                                   const bool overWriteEnable,
                                   const bool silentMode,
                                   MasterAttributes *attr);

#ifdef HDF5C
void LinkHDF5FileInMaster(std::string &masterFileName,
                          std::string &dataFilename,
                          std::vector<std::string> parameterNames,
                          const bool silentMode, std::mutex *hdf5LibMutex);

std::string CreateMasterHDF5File(const std::string &filePath,
                                 const std::string &fileNamePrefix,
                                 const uint64_t fileIndex,
                                 const bool overWriteEnable,
                                 const bool silentMode, MasterAttributes *attr,
                                 std::mutex *hdf5LibMutex);

std::string CreateVirtualHDF5File(
    const std::string &filePath, const std::string &fileNamePrefix,
    const uint64_t fileIndex, const bool overWriteEnable, const bool silentMode,
    const int modulePos, const int numUnitsPerReadout,
    const uint32_t maxFramesPerFile, const uint32_t nPixelsX,
    const uint32_t nPixelsY, const uint32_t dynamicRange,
    const uint64_t numImagesCaught, const int numModX, const int numModY,
    const H5::DataType dataType, const std::vector<std::string> parameterNames,
    const std::vector<H5::DataType> parameterDataTypes,
    std::mutex *hdf5LibMutex, bool gotthard25um);
#endif
} // namespace masterFileUtility

} // namespace sls
