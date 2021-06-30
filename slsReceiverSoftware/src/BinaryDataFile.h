#pragma once

#include "File.h"

class BinaryDataFile : private virtual slsDetectorDefs, public File {

  public:
    BinaryDataFile(int index);
    ~BinaryDataFile();

    void CloseFile() override;
    void CreateFirstBinaryDataFile(std::string filePath,
                                   std::string fileNamePrefix,
                                   uint64_t fileIndex, bool overWriteEnable,
                                   bool silentMode, int detIndex,
                                   int numUnitsPerDetector,
                                   uint32_t udpPortNumber,
                                   uint32_t maxFramesPerFile) override;

    void WriteToFile(char *buffer, int buffersize, uint64_t currentFrameNumber,
                     uint32_t numPacketsCaught) override;

  private:
    void CreateFile();

    uint32_t index_;
    FILE *fd_{nullptr};
    std::string fileName_;
    uint32_t numFramesInFile_{0};
    uint32_t subFileIndex_{0};

    std::string filePath_;
    std::string fileNamePrefix_;
    uint64_t fileIndex_{0};
    bool overWriteEnable_{false};
    bool silentMode_{false};
    int detIndex_{0};
    int numUnitsPerDetector_{0};
    uint32_t udpPortNumber_{0};
    uint32_t maxFramesPerFile_{0};
};