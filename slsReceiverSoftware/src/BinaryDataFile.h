// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once

#include "File.h"

namespace sls {

class BinaryDataFile : private virtual slsDetectorDefs, public File {

  public:
    BinaryDataFile(const int index);
    ~BinaryDataFile();

    fileFormat GetFileFormat() const override;
    void CloseFile() override;
    void CreateFirstBinaryDataFile(const std::string &fNamePrefix,
                                   const uint64_t fIndex, const bool ovEnable,
                                   const bool sMode, const uint16_t uPortNumber,
                                   const uint32_t mFramesPerFile) override;

    void WriteToFile(char *imageData, sls_receiver_header &header,
                     const int imageSize, const uint64_t currentFrameNumber,
                     const uint32_t numPacketsCaught) override;

  private:
    void CreateFile();

    uint32_t index;
    FILE *fd{nullptr};
    std::string fileName;
    uint32_t numFramesInFile{0};
    uint32_t subFileIndex{0};

    std::string fileNamePrefix;
    uint64_t fileIndex{0};
    bool overWriteEnable{false};
    bool silentMode{false};
    uint32_t udpPortNumber{0};
    uint32_t maxFramesPerFile{0};
};

} // namespace sls
