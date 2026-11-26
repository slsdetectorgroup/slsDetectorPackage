// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once

#include "File.h"
#include <mutex>

namespace sls {

class HDF5DataFile : private virtual slsDetectorDefs, public File {

  public:
    HDF5DataFile(const int index, std::mutex *hdf5Lib);
    ~HDF5DataFile();

    /**
     * Throws if a filter id is invalid, used more than once or not available.
     * Each filter is a list with the HDF5 filter id followed by its
     * parameters (cd_values). The caller must hold the hdf5 library mutex.
     */
    static void
    ValidateFilters(const std::vector<std::vector<uint32_t>> &filters);

    fileFormat GetFileFormat() const override;
    std::string GetFileName() const override;
    uint32_t GetFilesInAcquisition() const override;
    H5::DataType GetPDataType() const override;
    std::vector<std::string> GetParameterNames() const override;
    std::vector<H5::DataType> GetParameterDataTypes() const override;

    void CloseFile() override;

    void CreateFirstHDF5DataFile(const std::filesystem::path &filePath,
                                 const std::string &fNamePrefix,
                                 const uint64_t fIndex, const bool owEnable,
                                 const bool sMode, const uint16_t uPortNumber,
                                 const uint32_t mFramesPerFile,
                                 const uint64_t nImages, const uint32_t nX,
                                 const uint32_t nY, const uint32_t dr,
                                 const std::vector<std::vector<uint32_t>>
                                     &hdf5Filters) override;

    void WriteToFile(char *imageData, sls_receiver_header &header,
                     const int imageSize, const uint64_t currentFrameNumber,
                     const uint32_t numPacketsCaught) override;

  private:
    void CreateFile();
    void Convert12to16Bit(uint16_t *dst, uint8_t *src);
    void WriteImageDatasets(const uint64_t currentFrameNumber, char *buffer);
    void WriteParameterDatasets(const uint64_t currentFrameNumber,
                                sls_receiver_header rheader);
    void ExtendDataset();

    int index;
    std::mutex *hdf5Lib;
    H5::H5File *fd{nullptr};
    std::string fileName;
    H5::DataSpace *dataSpace{nullptr};
    H5::DataSet *dataSet{nullptr};
    H5::DataType dataType{H5::PredType::STD_U16LE};

    H5::DataSpace *dataSpacePara{nullptr};
    std::vector<H5::DataSet *> dataSetPara{nullptr};
    std::vector<std::string> parameterNames;
    std::vector<H5::DataType> parameterDataTypes;

    uint32_t subFileIndex{0};
    uint32_t numFramesInFile{0};
    uint32_t numFilesInAcquisition{0};
    uint32_t maxFramesPerFile{0};
    uint64_t numImages{0};
    uint64_t extNumImages{0};
    uint32_t nPixelsX{0};
    uint32_t nPixelsY{0};
    uint32_t dynamicRange{0};
    std::vector<std::vector<uint32_t>> filters;

    std::filesystem::path m_filePath{};
    std::string fileNamePrefix{};
    uint64_t fileIndex{0};
    bool overWriteEnable{false};
    bool silentMode{false};
    uint16_t udpPortNumber{0};

    static const int EIGER_NUM_PIXELS{256 * 2 * 256};
    static const int EIGER_16_BIT_IMAGE_SIZE{EIGER_NUM_PIXELS * 2};
};

} // namespace sls
