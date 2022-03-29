// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once

#include "MasterAttributes.h"

#include <mutex>

class HDF5MasterFile : private virtual slsDetectorDefs {
  public:
    void LinkDataFile(const std::string &masterFileName,
                      const std::string &dataFilename,
                      const std::string &dataSetname,
                      const std::vector<std::string> parameterNames,
                      const bool silentMode, std::mutex *hdf5LibMutex);
    std::string CreateMasterFile(const std::string &filePath,
                                 const std::string &fileNamePrefix,
                                 const uint64_t fileIndex,
                                 const bool overWriteEnable,
                                 const bool silentMode, MasterAttributes *attr,
                                 std::mutex *hdf5LibMutex);
};