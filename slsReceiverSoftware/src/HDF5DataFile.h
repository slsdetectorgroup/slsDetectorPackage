#pragma once

#include "File.h"

#include "H5Cpp.h"
#ifndef H5_NO_NAMESPACE
using namespace H5;
#endif

#include <mutex>

class HDF5DataFile : private virtual slsDetectorDefs, public File {

  public:
    HDF5DataFile(int index, std::mutex *hdf5Lib);
    ~HDF5DataFile();

    uint32_t GetFilesInAcquisition() override;

    void CloseFile() override;

    void CreateFirstHDF5DataFile(std::string filePath,
                                 std::string fileNamePrefix, uint64_t fileIndex,
                                 bool overWriteEnable, bool silentMode,
                                 int detIndex, int numUnitsPerDetector,
                                 uint32_t udpPortNumber,
                                 uint32_t maxFramesPerFile, uint64_t numImages,
                                 uint32_t nPIxelsX, uint32_t nPIxelsY,
                                 uint32_t dynamicRange) override;

    void WriteToFile(char *buffer, int buffersize, uint64_t currentFrameNumber,
                     uint32_t numPacketsCaught) override;

  private:
    void CreateFile();
    void WriteDataFile(uint64_t currentFrameNumber, char *buffer);
    void WriteParameterDatasets(uint64_t currentFrameNumber,
                                sls_receiver_header *rheader);
    void ExtendDataset();

    int index_;
    std::mutex *hdf5Lib_;
    H5File *fd_{nullptr};
    std::string fileName_;
    DataSpace *dataSpace_{nullptr};
    DataSet *dataSet_{nullptr};
    DataType dataType_{PredType::STD_U16LE};

    DataSpace *dataSpacePara_{nullptr};
    std::vector<DataSet *> dataSetPara_{nullptr};
    std::vector<std::string> parameterNames_;
    std::vector<DataType> parameterDataTypes_;

    uint32_t subFileIndex_{0};
    uint32_t numFramesInFile_{0};
    uint32_t numFilesInAcquisition_{0};
    uint32_t maxFramesPerFile_{0};
    uint64_t numImages_{0};
    uint64_t extNumImages_{0};
    uint32_t nPixelsX_{0};
    uint32_t nPixelsY_{0};
    uint32_t dynamicRange_{0};

    std::string filePath_;
    std::string fileNamePrefix_;
    uint64_t fileIndex_{0};
    bool overWriteEnable_{false};
    bool silentMode_{false};
    int detIndex_{0};
    int numUnitsPerDetector_{0};
    uint32_t udpPortNumber_{0};
};