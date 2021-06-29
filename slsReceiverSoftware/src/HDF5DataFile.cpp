#include "HDF5DataFile.h"
#include "sls/logger.h"

HDF5DataFile::HDF5DataFile(int index) : File(index, HDF5) {}

HDF5DataFile::~HDF5DataFile() { CloseFile(); }

void HDF5DataFile::CloseFile() {
    try {
        Exception::dontPrint(); // to handle errors
        if (fd_) {
            fd_->close();
            delete fd_;
            fd_ = nullptr;
        }
    } catch (const Exception &error) {
        LOG(logERROR) << "Could not close data HDF5 handles of index "
                      << index_;
        error.printErrorStack();
    }
}