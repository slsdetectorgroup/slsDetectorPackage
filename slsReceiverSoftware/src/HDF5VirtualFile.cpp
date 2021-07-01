#include "HDF5VirtualFile.h"

HDF5VirtualFile::HDF5VirtualFile(std::mutex *hdf5Lib)
    : File(HDF5), hdf5Lib_(hdf5Lib) {}

HDF5VirtualFile::~HDF5VirtualFile() { CloseFile(); }

void HDF5VirtualFile::CloseFile() {
    std::lock_guard<std::mutex> lock(*hdf5Lib_);
    /*if (fd_ != 0) {
        if (H5Fclose(fd_) < 0) {
            LOG(logERROR) << "Could not close virtual HDF5 handles";
        }
        fd_ = 0;
    }*/
    try {
        Exception::dontPrint(); // to handle errors
        if (fd_) {
            fd_->close();
            delete fd_;
            fd_ = nullptr;
        }
    } catch (const Exception &error) {
        LOG(logERROR) << "Could not close virtual HDF5 handles of index";
        error.printErrorStack();
    }
}