#pragma once

#include "File.h"

#include "H5Cpp.h"
#ifndef H5_NO_NAMESPACE
using namespace H5;
#endif

class HDF5VirtualFile : private virtual slsDetectorDefs, public File {

  public:
    HDF5VirtualFile(int index);
    ~HDF5VirtualFile();

    void CloseFile() override;

  private:
    hid_t fd_;
};