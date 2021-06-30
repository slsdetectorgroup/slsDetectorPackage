#pragma once

#include "File.h"

#include "H5Cpp.h"
#ifndef H5_NO_NAMESPACE
using namespace H5;
#endif

#include <mutex>

class HDF5VirtualFile : private virtual slsDetectorDefs, public File {

  public:
    HDF5VirtualFile(std::mutex *hdf5Lib);
    ~HDF5VirtualFile();

    void CloseFile() override;

  private:
    std::mutex *hdf5Lib_;
    // hid_t fd_;
    H5File *fd_{nullptr};
};