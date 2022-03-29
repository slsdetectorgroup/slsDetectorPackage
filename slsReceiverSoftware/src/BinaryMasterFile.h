// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once

#include "MasterAttributes.h"

class BinaryMasterFile : private virtual slsDetectorDefs {

  public:
    static std::string CreateMasterFile(const std::string filePath,
                                        const std::string fileNamePrefix,
                                        const uint64_t fileIndex,
                                        const bool overWriteEnable,
                                        const bool silentMode,
                                        MasterAttributes *attr);

  private:
    static std::string GetMasterAttributes(MasterAttributes *attr);
};