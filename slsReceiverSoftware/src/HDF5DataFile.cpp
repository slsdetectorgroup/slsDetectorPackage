// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "HDF5DataFile.h"
#include "receiver_defs.h"

#include <iomanip>

namespace sls {

HDF5DataFile::HDF5DataFile(int index, std::mutex *hdf5Lib)
    : index(index), hdf5Lib(hdf5Lib) {

    parameterNames = std::vector<std::string>{
        "frame number",
        "exp length or sub exposure time",
        "packets caught",
        "detector specific 1",
        "timestamp",
        "mod id",
        "row",
        "column",
        "detector specific 2",
        "detector specific 3",
        "detector specific 4",
        "detector type",
        "detector header version",
        "packets caught bit mask",
    };
    H5::StrType strdatatype(H5::PredType::C_S1, sizeof(bitset_storage));
    parameterDataTypes = std::vector<H5::DataType>{
        H5::PredType::STD_U64LE, H5::PredType::STD_U32LE,
        H5::PredType::STD_U32LE, H5::PredType::STD_U64LE,
        H5::PredType::STD_U64LE, H5::PredType::STD_U16LE,
        H5::PredType::STD_U16LE, H5::PredType::STD_U16LE,
        H5::PredType::STD_U16LE, H5::PredType::STD_U32LE,
        H5::PredType::STD_U16LE, H5::PredType::STD_U8LE,
        H5::PredType::STD_U8LE,  strdatatype};
}

HDF5DataFile::~HDF5DataFile() { CloseFile(); }

std::string HDF5DataFile::GetFileName() const { return fileName; }

uint32_t HDF5DataFile::GetFilesInAcquisition() const {
    return numFilesInAcquisition;
}

H5::DataType HDF5DataFile::GetPDataType() const { return dataType; }

std::vector<std::string> HDF5DataFile::GetParameterNames() const {
    return parameterNames;
}
std::vector<H5::DataType> HDF5DataFile::GetParameterDataTypes() const {
    return parameterDataTypes;
}

slsDetectorDefs::fileFormat HDF5DataFile::GetFileFormat() const { return HDF5; }

void HDF5DataFile::CloseFile() {
    std::lock_guard<std::mutex> lock(*hdf5Lib);
    try {
        H5::Exception::dontPrint(); // to handle errors
        if (fd) {
            fd->close();
            delete fd;
            fd = nullptr;
        }
    } catch (const H5::Exception &error) {
        LOG(logERROR) << "Could not close data HDF5 handles of index " << index;
        error.printErrorStack();
    }
    if (dataSpace) {
        delete dataSpace;
        dataSpace = nullptr;
    }
    if (dataSet) {
        delete dataSet;
        dataSet = nullptr;
    }
    if (dataSpacePara) {
        delete dataSpacePara;
        dataSpacePara = nullptr;
    }
    for (auto it : dataSetPara)
        delete it;
    dataSetPara.clear();
}

void HDF5DataFile::CreateFirstHDF5DataFile(
    const std::string &fNamePrefix, const uint64_t fIndex, const bool owEnable,
    const bool sMode, const uint16_t uPortNumber, const uint32_t mFramesPerFile,
    const uint64_t nImages, const uint32_t nX, const uint32_t nY,
    const uint32_t dr) {

    subFileIndex = 0;
    numFramesInFile = 0;
    extNumImages = nImages;
    numFilesInAcquisition = 0;

    maxFramesPerFile = mFramesPerFile;
    numImages = nImages;
    nPixelsX = nX;
    nPixelsY = nY;
    dynamicRange = dr;

    fileNamePrefix = fNamePrefix;
    fileIndex = fIndex;
    overWriteEnable = owEnable;
    silentMode = sMode;
    udpPortNumber = uPortNumber;

    switch (dynamicRange) {
    case 12:
    case 16:
        dataType = H5::PredType::STD_U16LE;
        break;
    case 32:
        dataType = H5::PredType::STD_U32LE;
        break;
    default:
        dataType = H5::PredType::STD_U8LE;
        break;
    }

    CreateFile();
}

void HDF5DataFile::CreateFile() {
    numFramesInFile = 0;
    numFilesInAcquisition++;

    std::ostringstream os;
    os << fileNamePrefix << "_f" << subFileIndex << '_' << fileIndex << ".h5";
    fileName = os.str();

    std::lock_guard<std::mutex> lock(*hdf5Lib);

    uint64_t framestosave =
        ((maxFramesPerFile == 0) ? numImages : // infinite images
             (((extNumImages - subFileIndex) > maxFramesPerFile)
                  ? // save up to maximum at a time
                  maxFramesPerFile
                  : (extNumImages - subFileIndex)));

    uint64_t nDimx = framestosave;
    uint32_t nDimy = nPixelsY;
    uint32_t nDimz = ((dynamicRange == 4) ? (nPixelsX / 2) : nPixelsX);

    try {
        H5::Exception::dontPrint(); // to handle errors

        // file
        H5::FileAccPropList fapl;
        fapl.setFcloseDegree(H5F_CLOSE_STRONG);
        fd = nullptr;
        if (!overWriteEnable)
            fd = new H5::H5File(fileName.c_str(), H5F_ACC_EXCL,
                                H5::FileCreatPropList::DEFAULT, fapl);
        else
            fd = new H5::H5File(fileName.c_str(), H5F_ACC_TRUNC,
                                H5::FileCreatPropList::DEFAULT, fapl);

        // attributes - version
        double dValue = HDF5_WRITER_VERSION;
        H5::DataSpace dataspace_attr = H5::DataSpace(H5S_SCALAR);
        H5::Attribute attribute = fd->createAttribute(
            "version", H5::PredType::NATIVE_DOUBLE, dataspace_attr);
        attribute.write(H5::PredType::NATIVE_DOUBLE, &dValue);

        // dimensions
        hsize_t dims[DATA_RANK] = {nDimx, nDimy, nDimz};
        hsize_t dimsMax[DATA_RANK] = {H5S_UNLIMITED, nDimy, nDimz};
        hsize_t dimsPara[PARA_RANK] = {nDimx};
        hsize_t dimsMaxPara[PARA_RANK] = {H5S_UNLIMITED};
        // always create chunked dataset as unlimited is only
        // supported with chunked layout
        hsize_t dimsChunk[DATA_RANK] = {MAX_CHUNKED_IMAGES, nDimy, nDimz};
        hsize_t dimsChunkPara[PARA_RANK] = {MAX_CHUNKED_IMAGES};

        // dataspace
        dataSpace = nullptr;
        dataSpace = new H5::DataSpace(DATA_RANK, dims, dimsMax);
        dataSpacePara = nullptr;
        dataSpacePara = new H5::DataSpace(PARA_RANK, dimsPara, dimsMaxPara);

        // property list
        H5::DSetCreatPropList plist;
        H5::DSetCreatPropList plistPara;
        uint64_t fill_value = -1;
        plist.setFillValue(dataType, &fill_value);
        // plistPara.setFillValue(dataType, &fill_value);
        plist.setChunk(DATA_RANK, dimsChunk);
        plistPara.setChunk(PARA_RANK, dimsChunkPara);

        // dataset
        dataSet = nullptr;
        dataSet = new H5::DataSet(
            fd->createDataSet(DATASET_NAME, dataType, *dataSpace, plist));
        for (unsigned int i = 0; i < parameterNames.size(); ++i) {
            H5::DataSet *ds = new H5::DataSet(fd->createDataSet(
                parameterNames[i].c_str(), parameterDataTypes[i],
                *dataSpacePara, plistPara));
            dataSetPara.push_back(ds);
        }
    } catch (const H5::Exception &error) {
        error.printErrorStack();
        CloseFile();
        throw RuntimeError("Could not create HDF5 handles in object " + index);
    }
    if (!silentMode) {
        LOG(logINFO) << "[" << udpPortNumber
                     << "]: HDF5 File created: " << fileName;
    }
}

void HDF5DataFile::WriteToFile(char *imageData, sls_receiver_header &header,
                               const int imageSize,
                               const uint64_t currentFrameNumber,
                               const uint32_t numPacketsCaught) {

    // check if maxframesperfile = 0 for infinite
    if (maxFramesPerFile && (numFramesInFile >= maxFramesPerFile)) {
        CloseFile();
        ++subFileIndex;
        CreateFile();
    }
    ++numFramesInFile;

    // extend dataset (when receiver start followed by many status starts
    // (jungfrau)))
    if (currentFrameNumber >= extNumImages) {
        ExtendDataset();
    }

    WriteImageDatasets(currentFrameNumber, imageData);
    WriteParameterDatasets(currentFrameNumber, header);
}

void HDF5DataFile::Convert12to16Bit(uint16_t *dst, uint8_t *src) {
    for (int i = 0; i < EIGER_NUM_PIXELS; ++i) {
        *dst = (uint16_t)(*src++ & 0xFF);
        *dst++ |= (uint16_t)((*src & 0xF) << 8u);
        ++i;
        *dst = (uint16_t)((*src++ & 0xF0) >> 4u);
        *dst++ |= (uint16_t)((*src++ & 0xFF) << 4u);
    }
}

void HDF5DataFile::WriteImageDatasets(const uint64_t currentFrameNumber,
                                      char *buffer) {
    // expand 12 bit to 16 bits
    char *revBuffer = buffer;
    if (dynamicRange == 12) {
        revBuffer = (char *)malloc(EIGER_16_BIT_IMAGE_SIZE);
        if (revBuffer == nullptr) {
            throw RuntimeError("Could not allocate memory for 12 bit to "
                               "16 bit conversion in object " +
                               std::to_string(index));
        }
        Convert12to16Bit((uint16_t *)revBuffer, (uint8_t *)buffer);
    }

    std::lock_guard<std::mutex> lock(*hdf5Lib);

    uint64_t nDimx =
        ((maxFramesPerFile == 0) ? currentFrameNumber
                                 : currentFrameNumber % maxFramesPerFile);
    uint32_t nDimy = nPixelsY;
    uint32_t nDimz = ((dynamicRange == 4) ? (nPixelsX / 2) : nPixelsX);

    hsize_t count[3] = {1, nDimy, nDimz};
    hsize_t start[3] = {nDimx, 0, 0};
    hsize_t dims2[2] = {nDimy, nDimz};
    try {
        H5::Exception::dontPrint(); // to handle errors

        dataSpace->selectHyperslab(H5S_SELECT_SET, count, start);
        H5::DataSpace memspace(2, dims2);
        dataSet->write(revBuffer, dataType, memspace, *dataSpace);
        memspace.close();
        if (dynamicRange == 12) {
            free(revBuffer);
        }
    } catch (const H5::Exception &error) {
        if (dynamicRange == 12) {
            free(revBuffer);
        }
        LOG(logERROR) << "Could not write to file in object " << index;
        error.printErrorStack();
        throw RuntimeError("Could not write to file in object " +
                           std::to_string(index));
    }
}

void HDF5DataFile::WriteParameterDatasets(const uint64_t currentFrameNumber,
                                          sls_receiver_header rheader) {
    std::lock_guard<std::mutex> lock(*hdf5Lib);

    uint64_t fnum =
        ((maxFramesPerFile == 0) ? currentFrameNumber
                                 : currentFrameNumber % maxFramesPerFile);

    sls_detector_header header = rheader.detHeader;
    hsize_t count[PARA_RANK] = {1};
    hsize_t start[PARA_RANK] = {fnum};
    int i = 0;
    try {
        H5::Exception::dontPrint(); // to handle errors
        dataSpacePara->selectHyperslab(H5S_SELECT_SET, count, start);
        H5::DataSpace memspace(H5S_SCALAR);
        dataSetPara[0]->write(&header.frameNumber, parameterDataTypes[0],
                              memspace, *dataSpacePara);
        i = 1;
        dataSetPara[1]->write(&header.expLength, parameterDataTypes[1],
                              memspace, *dataSpacePara);
        i = 2;
        dataSetPara[2]->write(&header.packetNumber, parameterDataTypes[2],
                              memspace, *dataSpacePara);
        i = 3;
        dataSetPara[3]->write(&header.detSpec1, parameterDataTypes[3], memspace,
                              *dataSpacePara);
        i = 4;
        dataSetPara[4]->write(&header.timestamp, parameterDataTypes[4],
                              memspace, *dataSpacePara);
        i = 5;
        dataSetPara[5]->write(&header.modId, parameterDataTypes[5], memspace,
                              *dataSpacePara);
        i = 6;
        dataSetPara[6]->write(&header.row, parameterDataTypes[6], memspace,
                              *dataSpacePara);
        i = 7;
        dataSetPara[7]->write(&header.column, parameterDataTypes[7], memspace,
                              *dataSpacePara);
        i = 8;
        dataSetPara[8]->write(&header.detSpec2, parameterDataTypes[8], memspace,
                              *dataSpacePara);
        i = 9;
        dataSetPara[9]->write(&header.detSpec3, parameterDataTypes[9], memspace,
                              *dataSpacePara);
        i = 10;
        dataSetPara[10]->write(&header.detSpec4, parameterDataTypes[10],
                               memspace, *dataSpacePara);
        i = 11;
        dataSetPara[11]->write(&header.detType, parameterDataTypes[11],
                               memspace, *dataSpacePara);
        i = 12;
        dataSetPara[12]->write(&header.version, parameterDataTypes[12],
                               memspace, *dataSpacePara);
        i = 13;

        // contiguous bitset
        if (sizeof(sls_bitset) == sizeof(bitset_storage)) {
            dataSetPara[13]->write((char *)&(rheader.packetsMask),
                                   parameterDataTypes[13], memspace,
                                   *dataSpacePara);
        }

        // not contiguous bitset
        else {
            // get contiguous representation of bit mask
            bitset_storage storage;
            memset(storage, 0, sizeof(bitset_storage));
            sls_bitset bits = rheader.packetsMask;
            for (int i = 0; i < MAX_NUM_PACKETS; ++i)
                storage[i >> 3] |= (bits[i] << (i & 7));
            // write bitmask
            dataSetPara[13]->write((char *)storage, parameterDataTypes[13],
                                   memspace, *dataSpacePara);
        }
        i = 14;
    } catch (const H5::Exception &error) {
        error.printErrorStack();
        throw RuntimeError(
            "Could not write parameters (index:" + std::to_string(i) +
            ") to file in object " + std::to_string(index));
    }
}

void HDF5DataFile::ExtendDataset() {
    std::lock_guard<std::mutex> lock(*hdf5Lib);

    try {
        H5::Exception::dontPrint(); // to handle errors

        hsize_t dims[DATA_RANK];
        dataSpace->getSimpleExtentDims(dims);
        dims[0] += numImages;
        dataSet->extend(dims);
        delete dataSpace;
        dataSpace = nullptr;
        dataSpace = new H5::DataSpace(dataSet->getSpace());

        hsize_t dimsPara[PARA_RANK] = {dims[0]};
        for (unsigned int i = 0; i < dataSetPara.size(); ++i)
            dataSetPara[i]->extend(dimsPara);
        delete dataSpacePara;
        dataSpacePara = nullptr;
        dataSpacePara = new H5::DataSpace(dataSetPara[0]->getSpace());

    } catch (const H5::Exception &error) {
        error.printErrorStack();
        throw RuntimeError("Could not extend dataset in object " +
                           std::to_string(index));
    }
    if (!silentMode) {
        LOG(logINFO) << index << " Extending HDF5 dataset by " << extNumImages
                     << ", Total x Dimension: " << (extNumImages + numImages);
    }
    extNumImages += numImages;
}

} // namespace sls
