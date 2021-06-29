#pragma once

#include "File.h"

#include "H5Cpp.h"
#ifndef H5_NO_NAMESPACE
using namespace H5;
#endif

class HDF5DataFile : private virtual slsDetectorDefs, public File {

  public:
    HDF5DataFile(int index);
    ~HDF5DataFile();

    void CloseFile() override;

  private:
    H5File *fd_{nullptr};
};