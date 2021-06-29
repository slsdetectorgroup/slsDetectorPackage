#pragma once

#include "File.h"

class BinaryDataFile : private virtual slsDetectorDefs, public File {

  public:
    BinaryDataFile(int index);
    ~BinaryDataFile();

    void CloseFile() override;

  private:
    FILE *fd_{nullptr};
};