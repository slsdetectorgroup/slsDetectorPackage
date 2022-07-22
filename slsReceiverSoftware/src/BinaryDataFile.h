// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once

#include "File.h"

namespace sls {

class BinaryDataFile : private virtual slsDetectorDefs, public File {

  public:
    BinaryDataFile(const int index);
    ~BinaryDataFile();

    void CloseFile() override;
    void CreateFirstBinaryDataFile(const std::string filePath,
                                   const std::string fileNamePrefix,
                                   const uint64_t fileIndex,
                                   const bool overWriteEnable,
                                   const bool silentMode, const int modulePos,
                                   const int numUnitsPerReadout,
                                   const uint32_t udpPortNumber,
                                   const uint32_t maxFramesPerFile) override;

    void WriteToFile(char *imageData, sls_receiver_header& header, const int imageSize, const uint64_t currentFrameNumber, const uint32_t numPacketsCaught) override;

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
    int numUnitsPerReadout_{0};
    uint32_t udpPortNumber_{0};
    uint32_t maxFramesPerFile_{0};
};

} // namespace sls
