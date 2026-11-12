// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once

#include "File.h"

#include <string>
#include <vector>

// root classes (only used as pointers in this header)
class TFile;
class TTree;
class TH2I;

namespace sls {

/**
 * Data file in root format (.root).
 *
 * Every file has a tree called "frames" with one entry per image. Its
 * branches are the members of sls_detector_header and the packets mask.
 *
 * How the image itself is saved depends on the file format option:
 * - ROOT_TTREE (default): as the branch "data" of the tree, which is a 2d
 *   array [rows][columns] of unsigned integers of the size of the dynamic
 *   range (as in the hdf5 file: 12 bit is saved as 16 bit and 4 bit is saved
 *   as it is sent by the detector, with 2 pixels in every byte).
 * - ROOT_TH2I: as a histogram called "frame_[frame number]" with one bin per
 *   pixel. The bins are signed 32 bit integers. Larger values are saved as
 *   the maximum of a signed 32 bit integer.
 */
class RootDataFile : private virtual slsDetectorDefs, public File {

  public:
    RootDataFile(const int index);
    ~RootDataFile();

    /**
     * Throws if the file format options are not valid for a root file.
     * Either no options (default) or a single option with a single value of
     * rootImageType.
     */
    static void
    ValidateOptions(const std::vector<std::vector<uint32_t>> &options);

    fileFormat GetFileFormat() const override;
    void CloseFile() override;
    void CreateFirstRootDataFile(
        const std::filesystem::path &filePath, const std::string &fNamePrefix,
        const uint64_t fIndex, const bool ovEnable, const bool sMode,
        const uint16_t uPortNumber, const uint32_t mFramesPerFile,
        const uint32_t nX, const uint32_t nY, const uint32_t dr,
        const std::vector<std::vector<uint32_t>> &options) override;

    void WriteToFile(char *imageData, sls_receiver_header &header,
                     const int imageSize, const uint64_t currentFrameNumber,
                     const uint32_t numPacketsCaught) override;

  private:
    void CreateFile();
    /** [ROOT_TTREE] copies the image to the address of the data branch */
    void FillImageBuffer(const char *imageData, const size_t imageSize);
    /** [ROOT_TH2I] sets the bins of the histogram to the pixels */
    void FillHistogram(const char *imageData, const size_t imageSize);

    uint32_t index;
    rootImageType imageType{ROOT_TTREE};

    TFile *fd{nullptr};
    /** one entry per image, owned by the file */
    TTree *tree{nullptr};
    /** [ROOT_TH2I] the same histogram is filled and written for every image */
    TH2I *histogram{nullptr};

    // addresses of the branches of the tree
    sls_detector_header detHeader{};
    bitset_storage packetsMask{};
    /** [ROOT_TTREE] image as it is saved */
    std::vector<char> imageBuffer;

    std::string fileName;
    std::filesystem::path m_filePath;
    uint32_t numFramesInFile{0};
    uint32_t subFileIndex{0};

    std::string fileNamePrefix;
    uint64_t fileIndex{0};
    bool overWriteEnable{false};
    bool silentMode{false};
    uint32_t udpPortNumber{0};
    uint32_t maxFramesPerFile{0};
    uint32_t nPixelsX{0};
    uint32_t nPixelsY{0};
    uint32_t dynamicRange{0};
};

} // namespace sls
