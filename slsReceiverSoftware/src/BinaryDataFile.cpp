#include "BinaryDataFile.h"
#include "sls/logger.h"

BinaryDataFile::BinaryDataFile(int index) : File(BINARY), index_(index) {}

BinaryDataFile::~BinaryDataFile() { CloseFile(); }

void BinaryDataFile::CloseFile() {
    if (fd_) {
        fclose(fd_);
    }
    fd_ = nullptr;
}

void BinaryDataFile::CreateDataFile(std::string filePath,
                                    std::string fileNamePrefix,
                                    uint64_t fileIndex, bool overWriteEnable,
                                    bool silentMode, int detIndex,
                                    int numUnitsPerDetector,
                                    uint32_t udpPortNumber) {
    numFramesInFile_ = 0;

    std::ostringstream os;
    os << filePath << "/" << fileNamePrefix << "_d"
       << (detIndex * numUnitsPerDetector + index_) << "_f" << subFileIndex_
       << '_' << fileIndex << ".raw";
    fileName_ = os.str();

    if (!overWriteEnable) {
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

    if (!(silentMode)) {
        LOG(logINFO) << "[" << udpPortNumber
                     << "]: Binary File created: " << fileName_;
    }
}