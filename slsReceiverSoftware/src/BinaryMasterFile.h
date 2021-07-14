#pragma once

#include "File.h"
#include "MasterAttributes.h"

class BinaryMasterFile : private virtual slsDetectorDefs, public File {

  public:
    BinaryMasterFile();
    ~BinaryMasterFile();

    void CloseFile() override;
    void CreateMasterFile(const std::string filePath,
                          const std::string fileNamePrefix,
                          const uint64_t fileIndex, const bool overWriteEnable,
                          const bool silentMode,
                          MasterAttributes *attr) override;

  private:
    FILE *fd_{nullptr};
    std::string fileName_;
};