#pragma once

#include "File.h"
#include "MasterAttributes.h"

class BinaryMasterFile : private virtual slsDetectorDefs, public File {

  public:
    BinaryMasterFile();
    ~BinaryMasterFile();

    void CloseFile() override;
    void CreateMasterFile(std::string filePath, std::string fileNamePrefix,
                          uint64_t fileIndex, bool overWriteEnable,
                          bool silentMode, MasterAttributes *attr) override;

  private:
    FILE *fd_{nullptr};
    std::string fileName_;
};