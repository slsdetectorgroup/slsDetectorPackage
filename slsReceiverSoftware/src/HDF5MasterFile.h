#pragma once

#include "File.h"

#include "H5Cpp.h"
#ifndef H5_NO_NAMESPACE
using namespace H5;
#endif

#include <mutex>

class HDF5MasterFile : private virtual slsDetectorDefs, public File {

  public:
    HDF5MasterFile(std::mutex *hdf5Lib);
    ~HDF5MasterFile();

    void CloseFile() override;
    void CreateMasterFile(std::string filePath, std::string fileNamePrefix,
                          uint64_t fileIndex, bool overWriteEnable,
                          bool silentMode, MasterAttributes *attr) override;

  private:
    std::mutex *hdf5Lib_;
    H5File *fd_{nullptr};
    std::string fileName_;
};