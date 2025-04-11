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

class File : private virtual slsDetectorDefs {

  public:
    File(){};
    virtual ~File(){};

    virtual fileFormat GetFileFormat() const = 0;
    virtual void CloseFile() = 0;

#ifdef HDF5C
    virtual std::string GetFileName() const {
        LOG(logERROR) << "This is a generic function GetFileName that "
                         "should be overloaded by a derived class";
        return std::string{};
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
        LOG(logERROR) << "This is a generic function GetParameterNames that "
                         "should be overloaded by a derived class";
        return std::vector<std::string>{};
    };

    virtual std::vector<H5::DataType> GetParameterDataTypes() const {
        LOG(logERROR)
            << "This is a generic function GetParameterDataTypes that "
               "should be overloaded by a derived class";
        return std::vector<H5::DataType>{};
    };

    virtual void CreateFirstHDF5DataFile(
        const std::string &fileNamePrefix, const uint64_t fileIndex,
        const bool overWriteEnable, const bool silentMode,
        const uint16_t udpPortNumber, const uint32_t maxFramesPerFile,
        const uint64_t numImages, const uint32_t nPixelsX,
        const uint32_t nPixelsY, const uint32_t dynamicRange) {
        LOG(logERROR)
            << "This is a generic function CreateFirstHDF5DataFile that "
               "should be overloaded by a derived class";
    };
#endif
    virtual void CreateFirstBinaryDataFile(const std::string &fileNamePrefix,
                                           const uint64_t fileIndex,
                                           const bool overWriteEnable,
                                           const bool silentMode,
                                           const uint16_t udpPortNumber,
                                           const uint32_t maxFramesPerFile) {
        LOG(logERROR)
            << "This is a generic function CreateFirstBinaryDataFile that "
               "should be overloaded by a derived class";
    };

    virtual void WriteToFile(char *imageData, sls_receiver_header &header,
                             const int imageSize,
                             const uint64_t currentFrameNumber,
                             const uint32_t numPacketsCaught) = 0;
};

} // namespace sls
