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

void HDF5MasterFile::LinkDataFile(std::string dataFilename,
                                  std::string dataSetname,
                                  const std::vector<std::string> parameterNames,
                                  const bool silentMode) {

    std::lock_guard<std::mutex> lock(*hdf5Lib_);
    try {
        Exception::dontPrint(); // to handle errors

        FileAccPropList flist;
        flist.setFcloseDegree(H5F_CLOSE_STRONG);

        // open master file
        H5File masterfd(fileName_.c_str(), H5F_ACC_RDWR,
                        FileCreatPropList::DEFAULT, flist);

        // open data file
        H5File fd(dataFilename.c_str(), H5F_ACC_RDONLY,
                  FileCreatPropList::DEFAULT, flist);

        // create link for data dataset
        DataSet dset = fd.openDataSet(dataSetname.c_str());
        std::string linkname = std::string("/entry/data/") + dataSetname;
        if (H5Lcreate_external(dataFilename.c_str(), dataSetname.c_str(),
                               masterfd.getLocId(), linkname.c_str(),
                               H5P_DEFAULT, H5P_DEFAULT) < 0) {
            throw sls::RuntimeError(
                "Could not create link to data dataset in master");
        }

        // create link for parameter datasets
        for (unsigned int i = 0; i < parameterNames.size(); ++i) {
            DataSet pDset = fd.openDataSet(parameterNames[i].c_str());
            linkname = std::string("/entry/data/") + parameterNames[i];
            if (H5Lcreate_external(dataFilename.c_str(),
                                   parameterNames[i].c_str(),
                                   masterfd.getLocId(), linkname.c_str(),
                                   H5P_DEFAULT, H5P_DEFAULT) < 0) {
                throw sls::RuntimeError(
                    "Could not create link to parameter dataset in master");
            }
        }
        fd.close();
        masterfd.close();
    } catch (const Exception &error) {
        error.printErrorStack();
        CloseFile();
        throw sls::RuntimeError("Could not link in master hdf5 file");
    }
    if (!silentMode) {
        LOG(logINFO) << "Linked in Master File: " << dataFilename;
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
    fileName_ = os.str();

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

        // TODO find a way to get complete group link
        attrGroupName_ = "/entry/instrument/detector";

        attr->WriteMasterHDF5Attributes(fd_, &group5);
        fd_->close();

    } catch (const Exception &error) {
        error.printErrorStack();
        CloseFile();
        throw sls::RuntimeError(
            "Could not create/overwrite master HDF5 handles");
    }
    if (!silentMode) {
        LOG(logINFO) << "Master File: " << fileName_;
    }
}

void HDF5MasterFile::UpdateMasterFile(MasterAttributes *attr, bool silentMode) {
    std::lock_guard<std::mutex> lock(*hdf5Lib_);

    try {
        Exception::dontPrint(); // to handle errors
        FileAccPropList flist;
        flist.setFcloseDegree(H5F_CLOSE_STRONG);
        fd_ = new H5File(fileName_.c_str(), H5F_ACC_RDWR,
                         FileCreatPropList::DEFAULT, flist);

        Group group = fd_->openGroup(attrGroupName_.c_str());
        attr->WriteFinalHDF5Attributes(fd_, &group);
        fd_->close();

    } catch (const Exception &error) {
        error.printErrorStack();
        CloseFile();
        throw sls::RuntimeError(
            "Could not create/overwrite master HDF5 handles");
    }
    if (!silentMode) {
        LOG(logINFO) << "Updated Master File";
    }
}