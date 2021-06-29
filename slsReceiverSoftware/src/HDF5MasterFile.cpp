#include "HDF5MasterFile.h"
#include "MasterAttributes.h"
#include "sls/logger.h"

HDF5MasterFile::HDF5MasterFile(int index, std::mutex *hdf5Lib)
    : File(index, HDF5), hdf5Lib_(hdf5Lib) {}

HDF5MasterFile::~HDF5MasterFile() { CloseFile(); }

void HDF5MasterFile::CloseFile() {
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

void HDF5MasterFile::CreateMasterFile(MasterAttributes *attr,
                                      std::string filePath,
                                      std::string fileNamePrefix,
                                      uint64_t fileIndex, bool overWriteEnable,
                                      bool silentMode) {

    std::ostringstream os;
    os << filePath << "/" << fileNamePrefix << "_master"
       << "_" << fileIndex << ".h5";
    fileName_ = os.str();

    if (!(silentMode)) {
        LOG(logINFO) << "Master File: " << fileName_;
    }

    std::lock_guard<std::mutex> lock(*hdf5Lib_);

    try {
        Exception::dontPrint(); // to handle errors

        FileAccPropList flist;
        flist.setFcloseDegree(H5F_CLOSE_STRONG);
        fd_ = nullptr;
        if (!(overWriteEnable))
            fd_ = new H5File(fileName_.c_str(), H5F_ACC_EXCL,
                             FileCreatPropList::DEFAULT, flist);
        else
            fd_ = new H5File(fileName_.c_str(), H5F_ACC_TRUNC,
                             FileCreatPropList::DEFAULT, flist);

        // Create a group in the file
        Group group1(fd_->createGroup("entry"));
        Group group2(group1.createGroup("data"));
        Group group3(group1.createGroup("instrument"));
        Group group4(group3.createGroup("beam"));
        Group group5(group3.createGroup("detector"));
        Group group6(group1.createGroup("sample"));

        attr->WriteMasterHDF5Attributes(fd_, &group5);
        fd_->close();

    } catch (const Exception &error) {
        error.printErrorStack();
        CloseFile();
        throw sls::RuntimeError("Could not create master HDF5 handles");
    }
}