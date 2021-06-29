#pragma once

#include "sls/sls_detector_defs.h"

struct MasterAttributes;

class File : private virtual slsDetectorDefs {

  public:
    File(int index, slsDetectorDefs::fileFormat type);
    virtual ~File();

    fileFormat GetFileType();

    virtual void CloseFile() = 0;
    virtual void CreateMasterFile(MasterAttributes *attr, std::string filePath,
                                  std::string fileNamePrefix,
                                  uint64_t fileIndex, bool overWriteEnable,
                                  bool silentMode) = 0;

  protected:
    int index_;
    slsDetectorDefs::fileFormat type_;
};
