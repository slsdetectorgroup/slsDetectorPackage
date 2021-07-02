#pragma once

#include "File.h"

#include <mutex>

class HDF5MasterFile : private virtual slsDetectorDefs, public File {

  public:
    HDF5MasterFile(std::mutex *hdf5Lib);
    ~HDF5MasterFile();

    void CloseFile() override;
    void CreateMasterFile(const std::string filePath,
                          const std::string fileNamePrefix,
                          const uint64_t fileIndex, const bool overWriteEnable,
                          const bool silentMode,
                          MasterAttributes *attr) override;

  private:
    std::mutex *hdf5Lib_;
    H5File *fd_{nullptr};
};