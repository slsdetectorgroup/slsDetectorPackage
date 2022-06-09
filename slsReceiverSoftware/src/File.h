// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once

#include "sls/logger.h"
#include "sls/sls_detector_defs.h"

#include <array>

#ifdef HDF5C
#include "H5Cpp.h"
#endif

namespace sls {

struct MasterAttributes;

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

    virtual H5::DataType GetPDataType() const {
        LOG(logERROR) << "This is a generic function GetPDataType that "
                         "should be overloaded by a derived class";
        return H5::PredType::STD_U16LE;
    }

    virtual std::vector<std::string> GetParameterNames() const {
        LOG(logERROR)
            << "This is a generic function GetFilesInAcquisition that "
               "should be overloaded by a derived class";
        return std::vector<std::string>{};
    };

    virtual std::vector<H5::DataType> GetParameterDataTypes() const {
        LOG(logERROR)
            << "This is a generic function GetFilesInAcquisition that "
               "should be overloaded by a derived class";
        return std::vector<H5::DataType>{};
    };

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
                             const uint32_t numPacketsCaught) = 0;

  protected:
    slsDetectorDefs::fileFormat format_;
};

} // namespace sls
