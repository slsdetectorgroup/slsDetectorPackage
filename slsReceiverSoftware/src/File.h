#pragma once

#include "sls/sls_detector_defs.h"

struct MasterAttributes;

class File : private virtual slsDetectorDefs {

  public:
    File(slsDetectorDefs::fileFormat type);
    virtual ~File();

    fileFormat GetFileType();
    virtual uint32_t GetFilesInAcquisition() = 0;
    virtual void CloseFile() = 0;

    virtual void CreateMasterFile(std::string filePath,
                                  std::string fileNamePrefix,
                                  uint64_t fileIndex, bool overWriteEnable,
                                  bool silentMode, MasterAttributes *attr) = 0;

    virtual void
    CreateFirstBinaryDataFile(std::string filePath, std::string fileNamePrefix,
                              uint64_t fileIndex, bool overWriteEnable,
                              bool silentMode, int detIndex,
                              int numUnitsPerDetector, uint32_t udpPortNumber,
                              uint32_t maxFramesPerFile) = 0;

    virtual void CreateFirstHDF5DataFile(
        std::string filePath, std::string fileNamePrefix, uint64_t fileIndex,
        bool overWriteEnable, bool silentMode, int detIndex,
        int numUnitsPerDetector, uint32_t udpPortNumber,
        uint32_t maxFramesPerFile, uint64_t numImages, uint32_t nPIxelsX,
        uint32_t nPIxelsY, uint32_t dynamicRange) = 0;

    virtual void WriteToFile(char *buffer, int buffersize,
                             uint64_t currentFrameNumber,
                             uint32_t numPacketsCaught) = 0;

  protected:
    slsDetectorDefs::fileFormat type_;
};
