// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "BinaryDataFile.h"

BinaryDataFile::BinaryDataFile(const int index) : File(BINARY), index_(index) {}

BinaryDataFile::~BinaryDataFile() { CloseFile(); }

void BinaryDataFile::CloseFile() {
    if (fd_) {
        fclose(fd_);
    }
    fd_ = nullptr;
}

void BinaryDataFile::CreateFirstBinaryDataFile(
    const std::string filePath, const std::string fileNamePrefix,
    const uint64_t fileIndex, const bool overWriteEnable, const bool silentMode,
    const int modulePos, const int numUnitsPerReadout,
    const uint32_t udpPortNumber, const uint32_t maxFramesPerFile) {

    subFileIndex_ = 0;
    numFramesInFile_ = 0;

    filePath_ = filePath;
    fileNamePrefix_ = fileNamePrefix;
    fileIndex_ = fileIndex;
    overWriteEnable_ = overWriteEnable;
    silentMode_ = silentMode;
    detIndex_ = modulePos;
    numUnitsPerReadout_ = numUnitsPerReadout;
    udpPortNumber_ = udpPortNumber;
    maxFramesPerFile_ = maxFramesPerFile;

    CreateFile();
}

void BinaryDataFile::CreateFile() {
    numFramesInFile_ = 0;

    std::ostringstream os;
    os << filePath_ << "/" << fileNamePrefix_ << "_d"
       << (detIndex_ * numUnitsPerReadout_ + index_) << "_f" << subFileIndex_
       << '_' << fileIndex_ << ".raw";
    fileName_ = os.str();

    if (!overWriteEnable_) {
        if (nullptr == (fd_ = fopen((const char *)fileName_.c_str(), "wx"))) {
            fd_ = nullptr;
            throw sls::RuntimeError("Could not create/overwrite file " +
                                    fileName_);
        }
    } else if (nullptr == (fd_ = fopen((const char *)fileName_.c_str(), "w"))) {
        fd_ = nullptr;
        throw sls::RuntimeError("Could not create file " + fileName_);
    }
    // setting to no file buffering
    setvbuf(fd_, nullptr, _IONBF, 0);

    if (!silentMode_) {
        LOG(logINFO) << "[" << udpPortNumber_
                     << "]: Binary File created: " << fileName_;
    }
}

void BinaryDataFile::WriteToFile(char *buffer, const int buffersize,
                                 const uint64_t currentFrameNumber,
                                 const uint32_t numPacketsCaught) {
    // check if maxframesperfile = 0 for infinite
    if (maxFramesPerFile_ && (numFramesInFile_ >= maxFramesPerFile_)) {
        CloseFile();
        ++subFileIndex_;
        CreateFile();
    }
    numFramesInFile_++;

    // write to file
    int ret = 0;

    // contiguous bitset
    if (sizeof(sls_bitset) == sizeof(bitset_storage)) {
        ret = fwrite(buffer, 1, buffersize, fd_);
    }

    // not contiguous bitset
    else {
        // write detector header
        ret = fwrite(buffer, 1, sizeof(sls_detector_header), fd_);

        // get contiguous representation of bit mask
        bitset_storage storage;
        memset(storage, 0, sizeof(bitset_storage));
        sls_bitset bits = *(sls_bitset *)(buffer + sizeof(sls_detector_header));
        for (int i = 0; i < MAX_NUM_PACKETS; ++i)
            storage[i >> 3] |= (bits[i] << (i & 7));
        // write bitmask
        ret += fwrite((char *)storage, 1, sizeof(bitset_storage), fd_);

        // write data
        ret += fwrite(buffer + sizeof(sls_detector_header), 1,
                      buffersize - sizeof(sls_receiver_header), fd_);
    }

    // if write error
    if (ret != buffersize) {
        throw sls::RuntimeError(std::to_string(index_) +
                                " : Write to file failed for image number " +
                                std::to_string(currentFrameNumber));
    }
}