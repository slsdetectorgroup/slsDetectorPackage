// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "HDF5DataFile.h"
#include "receiver_defs.h"

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

std::array<std::string, 2> HDF5DataFile::GetFileAndDatasetName() const {
    return std::array<std::string, 2>{fileName_, dataSetName_};
}

uint32_t HDF5DataFile::GetFilesInAcquisition() const {
    return numFilesInAcquisition_;
}

DataType HDF5DataFile::GetPDataType() const { return dataType_; }

std::vector<std::string> HDF5DataFile::GetParameterNames() const {
    return parameterNames_;
}
std::vector<DataType> HDF5DataFile::GetParameterDataTypes() const {
    return parameterDataTypes_;
}

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

void HDF5DataFile::CreateFirstHDF5DataFile(
    const std::string filePath, const std::string fileNamePrefix,
    const uint64_t fileIndex, const bool overWriteEnable, const bool silentMode,
    const int modulePos, const int numUnitsPerReadout,
    const uint32_t udpPortNumber, const uint32_t maxFramesPerFile,
    const uint64_t numImages, const uint32_t nPixelsX, const uint32_t nPixelsY,
    const uint32_t dynamicRange) {

    subFileIndex_ = 0;
    numFramesInFile_ = 0;
    extNumImages_ = numImages;
    numFilesInAcquisition_ = 0;

    maxFramesPerFile_ = maxFramesPerFile;
    numImages_ = numImages;
    nPixelsX_ = nPixelsX;
    nPixelsY_ = nPixelsY;
    dynamicRange_ = dynamicRange;

    filePath_ = filePath;
    fileNamePrefix_ = fileNamePrefix;
    fileIndex_ = fileIndex;
    overWriteEnable_ = overWriteEnable;
    silentMode_ = silentMode;
    detIndex_ = modulePos;
    numUnitsPerReadout_ = numUnitsPerReadout;
    udpPortNumber_ = udpPortNumber;

    switch (dynamicRange_) {
    case 16:
        dataType_ = PredType::STD_U16LE;
        break;
    case 32:
        dataType_ = PredType::STD_U32LE;
        break;
    default:
        dataType_ = PredType::STD_U8LE;
        break;
    }

    CreateFile();
}

void HDF5DataFile::CreateFile() {

    numFilesInAcquisition_++;

    std::ostringstream os;
    os << filePath_ << "/" << fileNamePrefix_ << "_d"
       << (detIndex_ * numUnitsPerReadout_ + index_) << "_f" << subFileIndex_
       << '_' << fileIndex_ << ".h5";
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
        if (!overWriteEnable_)
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
        dataSetName_ = osfn.str();

        // dataset
        // fill value
        DSetCreatPropList plist;
        int fill_value = -1;
        plist.setFillValue(dataType_, &fill_value);
        // always create chunked dataset as unlimited is only
        // supported with chunked layout
        hsize_t chunk_dims[3] = {MAX_CHUNKED_IMAGES, nDimy, nDimz};
        plist.setChunk(3, chunk_dims);
        dataSet_ = nullptr;
        dataSet_ = new DataSet(fd_->createDataSet(
            dataSetName_.c_str(), dataType_, *dataSpace_, plist));

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
    if (!silentMode_) {
        LOG(logINFO) << "[" << udpPortNumber_
                     << "]: HDF5 File created: " << fileName_;
    }
}

void HDF5DataFile::WriteToFile(char *buffer, const int buffersize,
                               const uint64_t currentFrameNumber,
                               const uint32_t numPacketsCaught) {

    // check if maxframesperfile = 0 for infinite
    if (maxFramesPerFile_ && (numFramesInFile_ >= maxFramesPerFile_)) {
        CloseFile();
        ++subFileIndex_;
        CreateFile();
    }
    numFramesInFile_++;

    // extend dataset (when receiver start followed by many status starts
    // (jungfrau)))
    if (currentFrameNumber >= extNumImages_) {
        ExtendDataset();
    }

    WriteDataFile(currentFrameNumber, buffer + sizeof(sls_receiver_header));
    WriteParameterDatasets(currentFrameNumber, (sls_receiver_header *)(buffer));
}

void HDF5DataFile::WriteDataFile(const uint64_t currentFrameNumber,
                                 char *buffer) {
    std::lock_guard<std::mutex> lock(*hdf5Lib_);

    uint64_t nDimx =
        ((maxFramesPerFile_ == 0) ? currentFrameNumber
                                  : currentFrameNumber % maxFramesPerFile_);
    uint32_t nDimy = nPixelsY_;
    uint32_t nDimz = ((dynamicRange_ == 4) ? (nPixelsX_ / 2) : nPixelsX_);

    hsize_t count[3] = {1, nDimy, nDimz};
    hsize_t start[3] = {nDimx, 0, 0};
    hsize_t dims2[2] = {nDimy, nDimz};
    try {
        Exception::dontPrint(); // to handle errors

        dataSpace_->selectHyperslab(H5S_SELECT_SET, count, start);
        DataSpace memspace(2, dims2);
        dataSet_->write(buffer, dataType_, memspace, *dataSpace_);
        memspace.close();
    } catch (const Exception &error) {
        LOG(logERROR) << "Could not write to file in object " << index_;
        error.printErrorStack();
        throw sls::RuntimeError("Could not write to file in object " +
                                std::to_string(index_));
    }
}

void HDF5DataFile::WriteParameterDatasets(const uint64_t currentFrameNumber,
                                          sls_receiver_header *rheader) {
    std::lock_guard<std::mutex> lock(*hdf5Lib_);

    uint64_t fnum =
        ((maxFramesPerFile_ == 0) ? currentFrameNumber
                                  : currentFrameNumber % maxFramesPerFile_);

    sls_detector_header header = rheader->detHeader;
    hsize_t count[1] = {1};
    hsize_t start[1] = {fnum};
    int i = 0;
    try {
        Exception::dontPrint(); // to handle errors
        dataSpacePara_->selectHyperslab(H5S_SELECT_SET, count, start);
        DataSpace memspace(H5S_SCALAR);
        dataSetPara_[0]->write(&header.frameNumber, parameterDataTypes_[0],
                               memspace, *dataSpacePara_);
        i = 1;
        dataSetPara_[1]->write(&header.expLength, parameterDataTypes_[1],
                               memspace, *dataSpacePara_);
        i = 2;
        dataSetPara_[2]->write(&header.packetNumber, parameterDataTypes_[2],
                               memspace, *dataSpacePara_);
        i = 3;
        dataSetPara_[3]->write(&header.bunchId, parameterDataTypes_[3],
                               memspace, *dataSpacePara_);
        i = 4;
        dataSetPara_[4]->write(&header.timestamp, parameterDataTypes_[4],
                               memspace, *dataSpacePara_);
        i = 5;
        dataSetPara_[5]->write(&header.modId, parameterDataTypes_[5], memspace,
                               *dataSpacePara_);
        i = 6;
        dataSetPara_[6]->write(&header.row, parameterDataTypes_[6], memspace,
                               *dataSpacePara_);
        i = 7;
        dataSetPara_[7]->write(&header.column, parameterDataTypes_[7], memspace,
                               *dataSpacePara_);
        i = 8;
        dataSetPara_[8]->write(&header.reserved, parameterDataTypes_[8],
                               memspace, *dataSpacePara_);
        i = 9;
        dataSetPara_[9]->write(&header.debug, parameterDataTypes_[9], memspace,
                               *dataSpacePara_);
        i = 10;
        dataSetPara_[10]->write(&header.roundRNumber, parameterDataTypes_[10],
                                memspace, *dataSpacePara_);
        i = 11;
        dataSetPara_[11]->write(&header.detType, parameterDataTypes_[11],
                                memspace, *dataSpacePara_);
        i = 12;
        dataSetPara_[12]->write(&header.version, parameterDataTypes_[12],
                                memspace, *dataSpacePara_);
        i = 13;

        // contiguous bitset
        if (sizeof(sls_bitset) == sizeof(bitset_storage)) {
            dataSetPara_[13]->write((char *)&(rheader->packetsMask),
                                    parameterDataTypes_[13], memspace,
                                    *dataSpacePara_);
        }

        // not contiguous bitset
        else {
            // get contiguous representation of bit mask
            bitset_storage storage;
            memset(storage, 0, sizeof(bitset_storage));
            sls_bitset bits = rheader->packetsMask;
            for (int i = 0; i < MAX_NUM_PACKETS; ++i)
                storage[i >> 3] |= (bits[i] << (i & 7));
            // write bitmask
            dataSetPara_[13]->write((char *)storage, parameterDataTypes_[13],
                                    memspace, *dataSpacePara_);
        }
        i = 14;
    } catch (const Exception &error) {
        error.printErrorStack();
        throw sls::RuntimeError(
            "Could not write parameters (index:" + std::to_string(i) +
            ") to file in object " + std::to_string(index_));
    }
}

void HDF5DataFile::ExtendDataset() {
    std::lock_guard<std::mutex> lock(*hdf5Lib_);

    try {
        Exception::dontPrint(); // to handle errors

        hsize_t dims[3];
        dataSpace_->getSimpleExtentDims(dims);
        dims[0] += numImages_;

        dataSet_->extend(dims);
        delete dataSpace_;
        dataSpace_ = nullptr;
        dataSpace_ = new DataSpace(dataSet_->getSpace());

        hsize_t dims_para[1] = {dims[0]};
        for (unsigned int i = 0; i < dataSetPara_.size(); ++i)
            dataSetPara_[i]->extend(dims_para);
        delete dataSpacePara_;
        dataSpacePara_ = nullptr;
        dataSpacePara_ = new DataSpace(dataSetPara_[0]->getSpace());

    } catch (const Exception &error) {
        error.printErrorStack();
        throw sls::RuntimeError("Could not extend dataset in object " +
                                std::to_string(index_));
    }
    if (!silentMode_) {
        LOG(logINFO) << index_ << " Extending HDF5 dataset by " << extNumImages_
                     << ", Total x Dimension: " << (extNumImages_ + numImages_);
    }
    extNumImages_ += numImages_;
}