#include "HDF5DataFile.h"
#include "receiver_defs.h"
#include "sls/logger.h"

#include <iomanip>

HDF5DataFile::HDF5DataFile(int index, std::mutex *hdf5Lib)
    : File(HDF5), index_(index), hdf5Lib_(hdf5Lib) {

    parameterNames_ = std::vector<std::string>{
        "frame number",
        "exp length or sub exposure time",
        "packets caught",
        "bunch id",
        "timestamp",
        "mod id",
        "row",
        "column",
        "reserved",
        "debug",
        "round robin number",
        "detector type",
        "detector header version",
        "packets caught bit mask",
    };
    StrType strdatatype(PredType::C_S1, sizeof(bitset_storage));
    parameterDataTypes_ = std::vector<DataType>{
        PredType::STD_U64LE, PredType::STD_U32LE, PredType::STD_U32LE,
        PredType::STD_U64LE, PredType::STD_U64LE, PredType::STD_U16LE,
        PredType::STD_U16LE, PredType::STD_U16LE, PredType::STD_U16LE,
        PredType::STD_U32LE, PredType::STD_U16LE, PredType::STD_U8LE,
        PredType::STD_U8LE,  strdatatype};
}

HDF5DataFile::~HDF5DataFile() { CloseFile(); }

void HDF5DataFile::CloseFile() {
    std::lock_guard<std::mutex> lock(*hdf5Lib_);
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
    if (dataSpace_) {
        delete dataSpace_;
        dataSpace_ = nullptr;
    }
    if (dataSet_) {
        delete dataSet_;
        dataSet_ = nullptr;
    }
    if (dataSpacePara_) {
        delete dataSpacePara_;
        dataSpacePara_ = nullptr;
    }
    for (auto it : dataSetPara_)
        delete it;
    dataSetPara_.clear();
}

void HDF5DataFile::CreateFirstDataFile(
    std::string filePath, std::string fileNamePrefix, uint64_t fileIndex,
    bool overWriteEnable, bool silentMode, int detIndex,
    int numUnitsPerDetector, uint32_t udpPortNumber, uint32_t maxFramesPerFile,
    uint64_t numImages, uint32_t nPIxelsX, uint32_t nPIxelsY,
    uint32_t dynamicRange) {

    CreateDataFile(filePath, fileNamePrefix, fileIndex, overWriteEnable,
                   silentMode, detIndex, numUnitsPerDetector, udpPortNumber);

    maxFramesPerFile_ = maxFramesPerFile;
    numImages_ = numImages;
    nPixelsX_ = nPIxelsX;
    nPixelsY_ = nPIxelsY;
    dynamicRange_ = dynamicRange;
}

void HDF5DataFile::CreateDataFile(std::string filePath,
                                  std::string fileNamePrefix,
                                  uint64_t fileIndex, bool overWriteEnable,
                                  bool silentMode, int detIndex,
                                  int numUnitsPerDetector,
                                  uint32_t udpPortNumber) {

    std::ostringstream os;
    os << filePath << "/" << fileNamePrefix << "_d"
       << (detIndex * numUnitsPerDetector + index_) << "_f" << subFileIndex_
       << '_' << fileIndex << ".h5";
    fileName_ = os.str();

    std::lock_guard<std::mutex> lock(*hdf5Lib_);

    uint64_t framestosave =
        ((maxFramesPerFile_ == 0) ? numImages_ : // infinite images
             (((extNumImages_ - subFileIndex_) > maxFramesPerFile_)
                  ? // save up to maximum at a time
                  maxFramesPerFile_
                  : (extNumImages_ - subFileIndex_)));

    uint64_t nDimx = framestosave;
    uint32_t nDimy = nPixelsY_;
    uint32_t nDimz = ((dynamicRange_ == 4) ? (nPixelsX_ / 2) : nPixelsX_);

    try {
        Exception::dontPrint(); // to handle errors

        // file
        FileAccPropList fapl;
        fapl.setFcloseDegree(H5F_CLOSE_STRONG);
        fd_ = nullptr;
        if (!overWriteEnable)
            fd_ = new H5File(fileName_.c_str(), H5F_ACC_EXCL,
                             FileCreatPropList::DEFAULT, fapl);
        else
            fd_ = new H5File(fileName_.c_str(), H5F_ACC_TRUNC,
                             FileCreatPropList::DEFAULT, fapl);

        // attributes - version
        double dValue = HDF5_WRITER_VERSION;
        DataSpace dataspace_attr = DataSpace(H5S_SCALAR);
        Attribute attribute = fd_->createAttribute(
            "version", PredType::NATIVE_DOUBLE, dataspace_attr);
        attribute.write(PredType::NATIVE_DOUBLE, &dValue);

        // dataspace
        hsize_t srcdims[3] = {nDimx, nDimy, nDimz};
        hsize_t srcdimsmax[3] = {H5S_UNLIMITED, nDimy, nDimz};
        dataSpace_ = nullptr;
        dataSpace_ = new DataSpace(3, srcdims, srcdimsmax);

        // dataset name
        std::ostringstream osfn;
        osfn << "/data";
        if (numImages_ > 1)
            osfn << "_f" << std::setfill('0') << std::setw(12) << subFileIndex_;
        std::string dsetname = osfn.str();

        // dataset
        // fill value
        DSetCreatPropList plist;
        int fill_value = -1;
        plist.setFillValue(dataType, &fill_value);
        // always create chunked dataset as unlimited is only
        // supported with chunked layout
        hsize_t chunk_dims[3] = {MAX_CHUNKED_IMAGES, nDimy, nDimz};
        plist.setChunk(3, chunk_dims);
        dataSet_ = nullptr;
        dataSet_ = new DataSet(
            fd_->createDataSet(dsetname.c_str(), dataType, *dataSpace_, plist));

        // create parameter datasets
        hsize_t dims[1] = {nDimx};
        hsize_t dimsmax[1] = {H5S_UNLIMITED};
        dataSpacePara_ = nullptr;
        dataSpacePara_ = new DataSpace(1, dims, dimsmax);

        // always create chunked dataset as unlimited is only
        // supported with chunked layout
        DSetCreatPropList paralist;
        hsize_t chunkpara_dims[3] = {MAX_CHUNKED_IMAGES};
        paralist.setChunk(1, chunkpara_dims);

        for (unsigned int i = 0; i < parameterNames_.size(); ++i) {
            DataSet *ds = new DataSet(fd_->createDataSet(
                parameterNames_[i].c_str(), parameterDataTypes_[i],
                *dataSpacePara_, paralist));
            dataSetPara_.push_back(ds);
        }
    } catch (const Exception &error) {
        error.printErrorStack();
        CloseFile();
        throw sls::RuntimeError("Could not create HDF5 handles in object " +
                                index_);
    }
    if (!(silentMode)) {
        LOG(logINFO) << "[" << udpPortNumber
                     << "]: HDF5 File created: " << fileName_;
    }
}