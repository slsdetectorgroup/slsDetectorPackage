// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "BinaryDataFile.h"

namespace sls {

BinaryDataFile::BinaryDataFile(const int index) : index(index) {}

BinaryDataFile::~BinaryDataFile() { CloseFile(); }

slsDetectorDefs::fileFormat BinaryDataFile::GetFileFormat() const {
    return BINARY;
}

void BinaryDataFile::CloseFile() {
    if (fd) {
        fclose(fd);
    }
    fd = nullptr;
}

void BinaryDataFile::CreateFirstBinaryDataFile(const std::string &fNamePrefix,
                                               const uint64_t fIndex,
                                               const bool ovEnable,
                                               const bool sMode,
                                               const uint16_t uPortNumber,
                                               const uint32_t mFramesPerFile) {

    subFileIndex = 0;
    numFramesInFile = 0;

    fileNamePrefix = fNamePrefix;
    fileIndex = fIndex;
    overWriteEnable = ovEnable;
    silentMode = sMode;
    udpPortNumber = uPortNumber;
    maxFramesPerFile = mFramesPerFile;

    CreateFile();
}

void BinaryDataFile::CreateFile() {
    numFramesInFile = 0;

    std::ostringstream os;
    os << fileNamePrefix << "_f" << subFileIndex << '_' << fileIndex << ".raw";
    fileName = os.str();

    if (!overWriteEnable) {
        if (nullptr == (fd = fopen(fileName.c_str(), "wx"))) {
            fd = nullptr;
            throw RuntimeError("Could not create/overwrite file " + fileName);
        }
    } else if (nullptr == (fd = fopen(fileName.c_str(), "w"))) {
        fd = nullptr;
        throw RuntimeError("Could not create file " + fileName);
    }
    // setting to no file buffering
    setvbuf(fd, nullptr, _IONBF, 0);

    if (!silentMode) {
        LOG(logINFO) << "[" << udpPortNumber
                     << "]: Binary File created: " << fileName;
    }
}

void BinaryDataFile::WriteToFile(char *imageData, sls_receiver_header &header,
                                 const int imageSize,
                                 const uint64_t currentFrameNumber,
                                 const uint32_t numPacketsCaught) {
    // check if maxframesperfile = 0 for infinite
    if (maxFramesPerFile && (numFramesInFile >= maxFramesPerFile)) {
        CloseFile();
        ++subFileIndex;
        CreateFile();
    }
    ++numFramesInFile;

    // write to file
    size_t ret = 0;

    // contiguous bitset (write header + image)
    if (sizeof(sls_bitset) == sizeof(bitset_storage)) {
        ret = fwrite(&header, sizeof(sls_receiver_header) + imageSize, 1, fd);
    }

    // not contiguous bitset
    else {
        // write detector header
        ret = fwrite(&header, sizeof(sls_detector_header), 1, fd);

        // get contiguous representation of bit mask
        bitset_storage storage;
        memset(storage, 0, sizeof(bitset_storage));
        sls_bitset bits = header.packetsMask;
        for (int i = 0; i < MAX_NUM_PACKETS; ++i)
            storage[i >> 3] |= (bits[i] << (i & 7));
        // write bitmask
        ret += fwrite(storage, sizeof(bitset_storage), 1, fd);

        // write data
        ret += fwrite(imageData, imageSize, 1, fd);
    }

    // if write error
    if (ret != 1) {
        throw RuntimeError(std::to_string(index) +
                           " : Write to file failed for image number " +
                           std::to_string(currentFrameNumber));
    }
}

} // namespace sls
