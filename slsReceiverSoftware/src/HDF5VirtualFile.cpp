#include "HDF5VirtualFile.h"
#include "sls/logger.h"

HDF5VirtualFile::HDF5VirtualFile(int index) : File(index, HDF5), fd_(0) {}

HDF5VirtualFile::~HDF5VirtualFile() { CloseFile(); }

void HDF5VirtualFile::CloseFile() {
    if (fd_ != 0) {
        if (H5Fclose(fd_) < 0) {
            LOG(logERROR) << "Could not close virtual HDF5 handles";
        }
        fd_ = 0;
    }
}