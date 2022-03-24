// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "BinaryMasterFile.h"
#include "MasterAttributes.h"

BinaryMasterFile::BinaryMasterFile() : File(BINARY) {}

BinaryMasterFile::~BinaryMasterFile() { CloseFile(); }

void BinaryMasterFile::CloseFile() {
    if (fd_) {
        fclose(fd_);
    }
    fd_ = nullptr;
}

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
    fileName_ = os.str();

    // create file
    if (!overWriteEnable) {
        if (nullptr == (fd_ = fopen((const char *)fileName_.c_str(), "wx"))) {
            fd_ = nullptr;
            throw sls::RuntimeError("Could not create binary master file " +
                                    fileName_);
        }
    } else if (nullptr == (fd_ = fopen((const char *)fileName_.c_str(), "w"))) {
        fd_ = nullptr;
        throw sls::RuntimeError(
            "Could not create/overwrite binary master file " + fileName_);
    }
    if (!silentMode) {
        LOG(logINFO) << "Master File: " << fileName_;
    }
    attr->WriteMasterBinaryAttributes(fd_);
    CloseFile();
}

void BinaryMasterFile::UpdateMasterFile(MasterAttributes *attr,
                                        bool silentMode) {
    if (nullptr == (fd_ = fopen((const char *)fileName_.c_str(), "a"))) {
        fd_ = nullptr;
        throw sls::RuntimeError("Could not append binary master file " +
                                fileName_);
    }
    attr->WriteFinalBinaryAttributes(fd_);
    CloseFile();
    if (!silentMode) {
        LOG(logINFO) << "Updated Master File";
    }
}
