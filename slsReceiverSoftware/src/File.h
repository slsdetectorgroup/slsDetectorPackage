#pragma once

#include "sls/logger.h"
#include "sls/sls_detector_defs.h"

struct MasterAttributes;

class File : private virtual slsDetectorDefs {

  public:
    File(const slsDetectorDefs::fileFormat format);
    virtual ~File();

    fileFormat GetFileFormat() const;
    virtual void CloseFile() = 0;

    virtual uint32_t GetFilesInAcquisition() const {
        LOG(logERROR)
            << "This is a generic function GetFilesInAcquisition that "
               "should be overloaded by a derived class";
        return 0;
    };

    virtual void CreateMasterFile(const std::string filePath,
                                  const std::string fileNamePrefix,
                                  const uint64_t fileIndex,
                                  const bool overWriteEnable,
                                  const bool silentMode,
                                  MasterAttributes *attr) {
        LOG(logERROR) << "This is a generic function CreateMasterFile that "
                         "should be overloaded by a derived class";
    };

    virtual void CreateFirstBinaryDataFile(
        const std::string filePath, const std::string fileNamePrefix,
        const uint64_t fileIndex, const bool overWriteEnable,
        const bool silentMode, const int modulePos,
        const int numUnitsPerReadout, const uint32_t udpPortNumber,
        const uint32_t maxFramesPerFile) {
        LOG(logERROR)
            << "This is a generic function CreateFirstBinaryDataFile that "
               "should be overloaded by a derived class";
    };

    virtual void CreateFirstHDF5DataFile(
        const std::string filePath, const std::string fileNamePrefix,
        const uint64_t fileIndex, const bool overWriteEnable,
        const bool silentMode, const int modulePos,
        const int numUnitsPerReadout, const uint32_t udpPortNumber,
        const uint32_t maxFramesPerFile, const uint64_t numImages,
        const uint32_t nPIxelsX, const uint32_t nPIxelsY,
        const uint32_t dynamicRange) {
        LOG(logERROR)
            << "This is a generic function CreateFirstHDF5DataFile that "
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
