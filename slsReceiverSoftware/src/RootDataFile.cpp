// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package

// clang-format off
// root headers first, as they must not see the macros of the sls headers
// (eg. RESET and BOLD in sls/ansi.h)
#include "TFile.h"
#include "TH2.h"
#include "TROOT.h"
#include "TTree.h"
// clang-format on

#include "RootDataFile.h"
#include "sls/ToString.h"

#include <algorithm>
#include <cstring>
#include <limits>
#include <mutex>

namespace sls {

namespace {

constexpr char TREE_NAME[] = "frames";
constexpr char IMAGE_BRANCH_NAME[] = "data";
constexpr char HISTOGRAM_NAME_PREFIX[] = "frame_";

/** default size in bytes of the buffer of a branch in root */
constexpr size_t DEFAULT_BASKET_SIZE = 32000;

/**
 * By default, root closes the file of a tree after 100 GB and continues in
 * a new file with another name. The receiver starts new files itself (max
 * frames per file), so this must never happen.
 */
constexpr Long64_t MAX_TREE_SIZE = 1LL << 50;

/** settings of root for the complete process, set only once */
void InitializeRoot() {
    static std::once_flag initialized;
    std::call_once(initialized, []() {
        // every port writes its file in its own thread
        ::ROOT::EnableThreadSafety();
        TTree::SetMaxTreeSize(MAX_TREE_SIZE);
        // by default, a histogram adds itself to the current directory of
        // the thread it is created in. Here it is written explicitly to the
        // file instead.
        TH1::AddDirectory(kFALSE);
    });
}

/** number of complete pixels in an image as it is sent by the detector */
size_t GetNumPixels(const size_t numBytes, const uint32_t dynamicRange) {
    switch (dynamicRange) {
    case 4:
        return numBytes * 2;
    case 12:
        // 2 pixels in 3 bytes
        return (numBytes / 3) * 2;
    case 16:
        return numBytes / 2;
    case 32:
        return numBytes / 4;
    default:
        return numBytes;
    }
}

/** value of a pixel in an image as it is sent by the detector */
uint32_t GetPixel(const uint8_t *data, const size_t pixel,
                  const uint32_t dynamicRange) {
    switch (dynamicRange) {
    case 4: {
        // 2 pixels in a byte, the first one in the upper 4 bits (as in gui)
        const uint8_t byte = data[pixel / 2];
        return (pixel % 2 == 0) ? ((byte >> 4) & 0xF) : (byte & 0xF);
    }
    case 12: {
        // 2 pixels in 3 bytes (as in the hdf5 file)
        const uint8_t *bytes = data + (pixel / 2) * 3;
        if (pixel % 2 == 0) {
            return bytes[0] | ((bytes[1] & 0xF) << 8);
        }
        return ((bytes[1] & 0xF0) >> 4) | (bytes[2] << 4);
    }
    case 16: {
        uint16_t value = 0;
        memcpy(&value, data + pixel * sizeof(value), sizeof(value));
        return value;
    }
    case 32: {
        uint32_t value = 0;
        memcpy(&value, data + pixel * sizeof(value), sizeof(value));
        return value;
    }
    default:
        return data[pixel];
    }
}

} // namespace

RootDataFile::RootDataFile(const int index) : index(index) { InitializeRoot(); }

RootDataFile::~RootDataFile() { CloseFile(); }

void RootDataFile::ValidateOptions(
    const std::vector<std::vector<uint32_t>> &options) {
    // default
    if (options.empty()) {
        return;
    }
    if (options.size() != 1 || options[0].size() != 1 ||
        options[0][0] >= static_cast<uint32_t>(NUM_ROOT_IMAGE_TYPES)) {
        throw RuntimeError("Invalid file format options " + ToString(options) +
                           " for root file format. Options: " +
                           ToString(ROOT_TTREE) + ", " + ToString(ROOT_TH2I));
    }
}

slsDetectorDefs::fileFormat RootDataFile::GetFileFormat() const { return ROOT; }

void RootDataFile::CloseFile() {
    if (fd != nullptr) {
        // writes the tree (replaces the one saved automatically while
        // filling)
        fd->Write(nullptr, TObject::kOverwrite);
        fd->Close();
        delete fd;
        fd = nullptr;
    }
    // deleted by the file
    tree = nullptr;
    delete histogram;
    histogram = nullptr;
}

void RootDataFile::CreateFirstRootDataFile(
    const std::filesystem::path &filePath, const std::string &fNamePrefix,
    const uint64_t fIndex, const bool ovEnable, const bool sMode,
    const uint16_t uPortNumber, const uint32_t mFramesPerFile,
    const uint32_t nX, const uint32_t nY, const uint32_t dr,
    const std::vector<std::vector<uint32_t>> &options) {

    ValidateOptions(options);
    imageType = options.empty() ? ROOT_TTREE
                                : static_cast<rootImageType>(options[0][0]);
    if (nX == 0 || nY == 0) {
        throw RuntimeError("Could not create root file for an image of " +
                           std::to_string(nX) + " x " + std::to_string(nY) +
                           " pixels");
    }

    subFileIndex = 0;
    numFramesInFile = 0;

    m_filePath = filePath;
    fileNamePrefix = fNamePrefix;
    fileIndex = fIndex;
    overWriteEnable = ovEnable;
    silentMode = sMode;
    udpPortNumber = uPortNumber;
    maxFramesPerFile = mFramesPerFile;
    nPixelsX = nX;
    nPixelsY = nY;
    dynamicRange = dr;

    CreateFile();
}

void RootDataFile::CreateFile() {
    numFramesInFile = 0;

    std::filesystem::path p =
        m_filePath / (fileNamePrefix + "_f" + std::to_string(subFileIndex) +
                      '_' + std::to_string(fileIndex) + ".root");
    fileName = p.string();

    // CREATE fails if the file exists
    fd = TFile::Open(fileName.c_str(), overWriteEnable ? "RECREATE" : "CREATE");
    if (fd == nullptr || fd->IsZombie()) {
        delete fd;
        fd = nullptr;
        if (!overWriteEnable) {
            throw RuntimeError("Could not create/overwrite file " + fileName);
        }
        throw RuntimeError("Could not create file " + fileName);
    }

    // header of every image
    tree = new TTree(TREE_NAME, "one entry per image");
    tree->SetDirectory(fd);
    tree->Branch("frameNumber", &detHeader.frameNumber, "frameNumber/l");
    tree->Branch("expLength", &detHeader.expLength, "expLength/i");
    tree->Branch("packetNumber", &detHeader.packetNumber, "packetNumber/i");
    tree->Branch("detSpec1", &detHeader.detSpec1, "detSpec1/l");
    tree->Branch("timestamp", &detHeader.timestamp, "timestamp/l");
    tree->Branch("modId", &detHeader.modId, "modId/s");
    tree->Branch("row", &detHeader.row, "row/s");
    tree->Branch("column", &detHeader.column, "column/s");
    tree->Branch("detSpec2", &detHeader.detSpec2, "detSpec2/s");
    tree->Branch("detSpec3", &detHeader.detSpec3, "detSpec3/i");
    tree->Branch("detSpec4", &detHeader.detSpec4, "detSpec4/s");
    tree->Branch("detType", &detHeader.detType, "detType/b");
    tree->Branch("version", &detHeader.version, "version/b");
    const std::string maskLeaf =
        "packetsMask[" + std::to_string(sizeof(packetsMask)) + "]/b";
    tree->Branch("packetsMask", packetsMask, maskLeaf.c_str());

    // image
    if (imageType == ROOT_TH2I) {
        // one bin per pixel
        histogram = new TH2I(HISTOGRAM_NAME_PREFIX, HISTOGRAM_NAME_PREFIX,
                             nPixelsX, 0, nPixelsX, nPixelsY, 0, nPixelsY);
        histogram->SetDirectory(nullptr);
    } else {
        // unsigned integers of the size of the dynamic range
        // (b: 8 bit, s: 16 bit, i: 32 bit)
        char leafType = 'b';
        size_t bytesPerValue = 1;
        switch (dynamicRange) {
        case 12:
        case 16:
            leafType = 's';
            bytesPerValue = 2;
            break;
        case 32:
            leafType = 'i';
            bytesPerValue = 4;
            break;
        default:
            break;
        }
        // 4 bit is saved as it is sent by the detector (2 pixels in a byte)
        const size_t numColumns =
            (dynamicRange == 4) ? (nPixelsX + 1) / 2 : nPixelsX;
        imageBuffer.assign(nPixelsY * numColumns * bytesPerValue, 0);

        const std::string imageLeaf =
            std::string(IMAGE_BRANCH_NAME) + '[' + std::to_string(nPixelsY) +
            "][" + std::to_string(numColumns) + "]/" + leafType;
        // the buffer of the branch holds at least one image
        const size_t basketSize =
            std::min(std::max(DEFAULT_BASKET_SIZE, imageBuffer.size()),
                     static_cast<size_t>(std::numeric_limits<Int_t>::max()));
        if (tree->Branch(IMAGE_BRANCH_NAME, imageBuffer.data(),
                         imageLeaf.c_str(),
                         static_cast<Int_t>(basketSize)) == nullptr) {
            CloseFile();
            throw RuntimeError("Could not create branch " + imageLeaf +
                               " in file " + fileName);
        }
    }

    if (!silentMode) {
        LOG(logINFO) << "[" << udpPortNumber
                     << "]: Root File created: " << fileName;
    }
}

void RootDataFile::WriteToFile(char *imageData, sls_receiver_header &header,
                               const int imageSize,
                               const uint64_t currentFrameNumber,
                               const uint32_t numPacketsCaught) {
    // check if maxframesperfile = 0 for infinite
    if (maxFramesPerFile && (numFramesInFile >= maxFramesPerFile)) {
        CloseFile();
        ++subFileIndex;
        CreateFile();
    }
    if (fd == nullptr || tree == nullptr) {
        throw RuntimeError(std::to_string(index) +
                           " : No root file to write image number " +
                           std::to_string(currentFrameNumber));
    }
    ++numFramesInFile;

    // header of the image (the branches read from these members)
    detHeader = header.detHeader;
    // contiguous representation of bit mask
    memset(packetsMask, 0, sizeof(packetsMask));
    for (int i = 0; i < MAX_NUM_PACKETS; ++i)
        packetsMask[i >> 3] |= (header.packetsMask[i] << (i & 7));

    // image
    const size_t numBytes = imageSize > 0 ? static_cast<size_t>(imageSize) : 0;
    bool success = true;
    if (imageType == ROOT_TH2I) {
        FillHistogram(imageData, numBytes);
        const std::string name = HISTOGRAM_NAME_PREFIX +
                                 std::to_string(detHeader.frameNumber);
        histogram->SetNameTitle(name.c_str(), name.c_str());
        success = (fd->WriteTObject(histogram) > 0);
    } else {
        FillImageBuffer(imageData, numBytes);
    }

    // one entry in the tree
    if (tree->Fill() < 0) {
        success = false;
    }
    if (!success) {
        throw RuntimeError(std::to_string(index) +
                           " : Write to file failed for image number " +
                           std::to_string(currentFrameNumber));
    }
}

void RootDataFile::FillImageBuffer(const char *imageData,
                                   const size_t imageSize) {
    // 12 bit is saved as 16 bit, pixels without data are 0
    if (dynamicRange == 12) {
        const auto *data = reinterpret_cast<const uint8_t *>(imageData);
        const size_t numPixels = imageBuffer.size() / sizeof(uint16_t);
        const size_t numPixelsInData = GetNumPixels(imageSize, dynamicRange);
        for (size_t pixel = 0; pixel != numPixels; ++pixel) {
            const uint16_t value =
                (pixel < numPixelsInData)
                    ? static_cast<uint16_t>(
                          GetPixel(data, pixel, dynamicRange))
                    : 0;
            memcpy(imageBuffer.data() + pixel * sizeof(value), &value,
                   sizeof(value));
        }
        return;
    }

    // saved as it is sent by the detector, bytes without data are 0
    const size_t numBytes = std::min(imageSize, imageBuffer.size());
    memcpy(imageBuffer.data(), imageData, numBytes);
    std::fill(imageBuffer.begin() + numBytes, imageBuffer.end(), 0);
}

void RootDataFile::FillHistogram(const char *imageData,
                                 const size_t imageSize) {
    const auto *data = reinterpret_cast<const uint8_t *>(imageData);
    const size_t numPixelsInData = GetNumPixels(imageSize, dynamicRange);
    // bins are signed 32 bit integers
    const auto maxValue =
        static_cast<uint32_t>(std::numeric_limits<Int_t>::max());

    size_t pixel = 0;
    for (uint32_t y = 0; y != nPixelsY; ++y) {
        for (uint32_t x = 0; x != nPixelsX; ++x) {
            // pixels without data are 0
            const uint32_t value = (pixel < numPixelsInData)
                                       ? GetPixel(data, pixel, dynamicRange)
                                       : 0;
            // bin 0 is the underflow bin
            histogram->SetBinContent(x + 1, y + 1, std::min(value, maxValue));
            ++pixel;
        }
    }
    // otherwise the number of calls to SetBinContent of all the images
    histogram->SetEntries(static_cast<Double_t>(pixel));
}

} // namespace sls
