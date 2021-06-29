#pragma once

#include "File.h"
#include "MasterAttributes.h"

class BinaryMasterFile : private virtual slsDetectorDefs, public File {

  public:
    BinaryMasterFile(int index);
    ~BinaryMasterFile();

    void CloseFile() override;
    void CreateMasterFile(MasterAttributes *attr, std::string filePath,
                          std::string fileNamePrefix, uint64_t fileIndex,
                          bool overWriteEnable, bool silentMode) override;

  private:
    FILE *fd_{nullptr};
    std::string fileName_;
};