// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "BinaryMasterFile.h"
#include "MasterAttributes.h"

void BinaryMasterFile::CreateMasterFile(const std::string filePath,
                                        const std::string fileNamePrefix,
                                        const uint64_t fileIndex,
                                        const bool overWriteEnable,
                                        const bool silentMode,
                                        MasterAttributes *attr) {
    // create file name
    std::ostringstream os;
    os << filePath << "/" << fileNamePrefix << "_master"
       << "_" << fileIndex << ".json";
    std::string fileName = os.str();

    // create file
    FILE *fd{nullptr};
    if (!overWriteEnable) {
        if (nullptr == (fd = fopen((const char *)fileName.c_str(), "wx"))) {
            fd = nullptr;
            throw sls::RuntimeError("Could not create binary master file " +
                                    fileName);
        }
    } else if (nullptr == (fd = fopen((const char *)fileName.c_str(), "w"))) {
        fd = nullptr;
        throw sls::RuntimeError(
            "Could not create/overwrite binary master file " + fileName);
    }
    if (!silentMode) {
        LOG(logINFO) << "Master File: " << fileName;
    }
    attr->WriteMasterBinaryAttributes(fd);
    if (fd) {
        fclose(fd);
    }
}
