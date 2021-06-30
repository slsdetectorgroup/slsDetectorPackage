#pragma once

#include "File.h"

class BinaryDataFile : private virtual slsDetectorDefs, public File {

  public:
    BinaryDataFile(int index);
    ~BinaryDataFile();

    void CloseFile() override;
    void CreateDataFile(std::string filePath, std::string fileNamePrefix,
                        uint64_t fileIndex, bool overWriteEnable,
                        bool silentMode, int detIndex, int numUnitsPerDetector,
                        uint32_t udpPortNumber) override;

  private:
    uint32_t index_;
    FILE *fd_{nullptr};
    std::string fileName_;
    uint32_t numFramesInFile_{0};
    uint32_t subFileIndex_{0}; // to do reset in createfirstdatafile
};