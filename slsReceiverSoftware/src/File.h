// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once

#include "sls/logger.h"
#include "sls/sls_detector_defs.h"

struct MasterAttributes;

#ifdef HDF5C
#include "H5Cpp.h"
#ifndef H5_NO_NAMESPACE
using namespace H5;
#endif
#endif

#include <array>

class File : private virtual slsDetectorDefs {

  public:
    File(const slsDetectorDefs::fileFormat format);
    virtual ~File();

    fileFormat GetFileFormat() const;
    virtual void CloseFile() = 0;

#ifdef HDF5C
    virtual std::array<std::string, 2> GetFileAndDatasetName() const {
        LOG(logERROR)
            << "This is a generic function GetFilesInAcquisition that "
               "should be overloaded by a derived class";
        return std::array<std::string, 2>{};
    }

    virtual uint32_t GetFilesInAcquisition() const {
        LOG(logERROR)
            << "This is a generic function GetFilesInAcquisition that "
               "should be overloaded by a derived class";
        return 0;
    };

    virtual DataType GetPDataType() const {
        LOG(logERROR) << "This is a generic function GetPDataType that "
                         "should be overloaded by a derived class";
        return PredType::STD_U16LE;
    }

    virtual std::vector<std::string> GetParameterNames() const {
        LOG(logERROR)
            << "This is a generic function GetFilesInAcquisition that "
               "should be overloaded by a derived class";
        return std::vector<std::string>{};
    };

    virtual std::vector<DataType> GetParameterDataTypes() const {
        LOG(logERROR)
            << "This is a generic function GetFilesInAcquisition that "
               "should be overloaded by a derived class";
        return std::vector<DataType>{};
    };

    virtual void CreateVirtualFile(
        const std::string filePath, const std::string fileNamePrefix,
        const uint64_t fileIndex, const bool overWriteEnable,
        const bool silentMode, const int modulePos,
        const int numUnitsPerReadout, const uint32_t maxFramesPerFile,
        const uint64_t numImages, const uint32_t nPixelsX,
        const uint32_t nPixelsY, const uint32_t dynamicRange,
        const uint64_t numImagesCaught, const int numModX, const int numModY,
        const DataType dataType, const std::vector<std::string> parameterNames,
        const std::vector<DataType> parameterDataTypes) {
        LOG(logERROR) << "This is a generic function CreateVirtualFile that "
                         "should be overloaded by a derived class";
    }

    virtual void CreateFirstHDF5DataFile(
        const std::string filePath, const std::string fileNamePrefix,
        const uint64_t fileIndex, const bool overWriteEnable,
        const bool silentMode, const int modulePos,
        const int numUnitsPerReadout, const uint32_t udpPortNumber,
        const uint32_t maxFramesPerFile, const uint64_t numImages,
        const uint32_t nPixelsX, const uint32_t nPixelsY,
        const uint32_t dynamicRange) {
        LOG(logERROR) << "This is a generic function CreateFirstDataFile that "
                         "should be overloaded by a derived class";
    };

    virtual void LinkDataFile(std::string dataFilename, std::string dataSetname,
                              const std::vector<std::string> parameterNames,
                              const bool silentMode) {
        LOG(logERROR) << "This is a generic function LinkDataFile that "
                         "should be overloaded by a derived class";
    };
#endif
    virtual void CreateFirstBinaryDataFile(
        const std::string filePath, const std::string fileNamePrefix,
        const uint64_t fileIndex, const bool overWriteEnable,
        const bool silentMode, const int modulePos,
        const int numUnitsPerReadout, const uint32_t udpPortNumber,
        const uint32_t maxFramesPerFile) {
        LOG(logERROR) << "This is a generic function CreateFirstDataFile that "
                         "should be overloaded by a derived class";
    };

    virtual void WriteToFile(char *buffer, const int buffersize,
                             const uint64_t currentFrameNumber,
                             const uint32_t numPacketsCaught) {
        LOG(logERROR) << "This is a generic function WriteToFile that "
                         "should be overloaded by a derived class";
    };

  protected:
    slsDetectorDefs::fileFormat format_;
};
