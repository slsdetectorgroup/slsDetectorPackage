#include "BinaryDataFile.h"
#include "sls/logger.h"

BinaryDataFile::BinaryDataFile(int index) : File(index, BINARY) {}

BinaryDataFile::~BinaryDataFile() { CloseFile(); }

void BinaryDataFile::CloseFile() {
    if (fd_) {
        fclose(fd_);
    }
    fd_ = nullptr;
}