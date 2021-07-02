#include "HDF5MasterFile.h"
#include "MasterAttributes.h"

HDF5MasterFile::HDF5MasterFile(std::mutex *hdf5Lib)
    : File(HDF5), hdf5Lib_(hdf5Lib) {}

HDF5MasterFile::~HDF5MasterFile() { CloseFile(); }

void HDF5MasterFile::CloseFile() {
    std::lock_guard<std::mutex> lock(*hdf5Lib_);
    try {
        Exception::dontPrint(); // to handle errors
        if (fd_) {
            fd_->close();
            delete fd_;
            fd_ = nullptr;
        }
    } catch (const Exception &error) {
        LOG(logERROR) << "Could not close master HDF5 handles";
        error.printErrorStack();
    }
}

void HDF5MasterFile::CreateMasterFile(const std::string filePath,
                                      const std::string fileNamePrefix,
                                      const uint64_t fileIndex,
                                      const bool overWriteEnable,
                                      const bool silentMode,
                                      MasterAttributes *attr) {

    std::ostringstream os;
    os << filePath << "/" << fileNamePrefix << "_master"
       << "_" << fileIndex << ".h5";
    std::string fileName = os.str();

    std::lock_guard<std::mutex> lock(*hdf5Lib_);

    try {
        Exception::dontPrint(); // to handle errors

        FileAccPropList flist;
        flist.setFcloseDegree(H5F_CLOSE_STRONG);
        fd_ = nullptr;
        if (!(overWriteEnable))
            fd_ = new H5File(fileName.c_str(), H5F_ACC_EXCL,
                             FileCreatPropList::DEFAULT, flist);
        else
            fd_ = new H5File(fileName.c_str(), H5F_ACC_TRUNC,
                             FileCreatPropList::DEFAULT, flist);

        // attributes - version
        double dValue = HDF5_WRITER_VERSION;
        DataSpace dataspace_attr = DataSpace(H5S_SCALAR);
        Attribute attribute = fd_->createAttribute(
            "version", PredType::NATIVE_DOUBLE, dataspace_attr);
        attribute.write(PredType::NATIVE_DOUBLE, &dValue);

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
        throw sls::RuntimeError(
            "Could not create/overwrite master HDF5 handles");
    }
    if (!silentMode) {
        LOG(logINFO) << "Master File: " << fileName;
    }
}