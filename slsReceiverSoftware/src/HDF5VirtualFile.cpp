// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "HDF5VirtualFile.h"
#include "receiver_defs.h"

#include <iomanip>

HDF5VirtualFile::HDF5VirtualFile(std::mutex *hdf5Lib, bool g25)
    : File(HDF5), hdf5Lib_(hdf5Lib), gotthard25um(g25) {}

HDF5VirtualFile::~HDF5VirtualFile() { CloseFile(); }

std::array<std::string, 2> HDF5VirtualFile::GetFileAndDatasetName() const {
    return std::array<std::string, 2>{fileName_, dataSetName_};
}

void HDF5VirtualFile::CloseFile() {
    std::lock_guard<std::mutex> lock(*hdf5Lib_);
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

void HDF5VirtualFile::CreateVirtualFile(
    const std::string filePath, const std::string fileNamePrefix,
    const uint64_t fileIndex, const bool overWriteEnable, const bool silentMode,
    const int modulePos, const int numUnitsPerReadout,
    const uint32_t maxFramesPerFile, const uint64_t numImages,
    const uint32_t nPixelsX, const uint32_t nPixelsY,
    const uint32_t dynamicRange, const uint64_t numImagesCaught,
    const int numModX, const int numModY, const DataType dataType,
    const std::vector<std::string> parameterNames,
    const std::vector<DataType> parameterDataTypes) {
    // virtual file name
    std::ostringstream osfn;
    osfn << filePath << "/" << fileNamePrefix << "_virtual"
         << "_" << fileIndex << ".h5";
    fileName_ = osfn.str();

    unsigned int paraSize = parameterNames.size();
    uint64_t numModZ = numModX;
    uint32_t nDimy = nPixelsY;
    uint32_t nDimz = ((dynamicRange == 4) ? (nPixelsX / 2) : nPixelsX);

    std::lock_guard<std::mutex> lock(*hdf5Lib_);

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

        // virtual data dataspace
        hsize_t vdsDims[3] = {numImagesCaught, numModY * nDimy,
                              numModZ * nDimz};
        DataSpace vdsDataSpace(3, vdsDims, nullptr);

        // virtual parameter dataspace
        hsize_t vdsDimsPara[2] = {numImagesCaught,
                                  (unsigned int)numModY * numModZ};
        DataSpace vdsDataSpacePara(2, vdsDimsPara, nullptr);

        // property list (fill value and datatype)
        int fill_value = -1;
        DSetCreatPropList plist;
        plist.setFillValue(dataType, &fill_value);

        // property list for parameters (datatype)
        std::vector<DSetCreatPropList> plistPara(paraSize);

        // hyperslab
        int numMajorHyperslab = numImagesCaught / maxFramesPerFile;
        if (numImagesCaught % maxFramesPerFile)
            ++numMajorHyperslab;
        uint64_t framesSaved = 0;
        // loop through files
        for (int hyperSlab = 0; hyperSlab < numMajorHyperslab; ++hyperSlab) {

            uint64_t nDimx =
                ((numImagesCaught - framesSaved) > maxFramesPerFile)
                    ? maxFramesPerFile
                    : (numImagesCaught - framesSaved);
            // starting location
            hsize_t start[3] = {framesSaved, 0, 0};
            // number of elements separating each block
            hsize_t stride[3] = {1, 1, 1};
            // number of blocks
            hsize_t count[3] = {1, 1, 1};
            // block size
            hsize_t block[3] = {nDimx, nDimy, nDimz};

            // starting location
            hsize_t startPara[2] = {framesSaved, 0};
            // number of elements separating each block
            hsize_t stridePara[3] = {1, 1};
            // number of blocks
            hsize_t countPara[2] = {1, 1};
            // block size
            hsize_t blockPara[3] = {nDimx, 1};

            // loop through readouts (image)
            for (unsigned int i = 0; i < numModY * numModZ; ++i) {

                // setect data hyperslabs
                vdsDataSpace.selectHyperslab(H5S_SELECT_SET, count, start,
                                             stride, block);

                // select parameter hyperslabs
                vdsDataSpacePara.selectHyperslab(H5S_SELECT_SET, countPara,
                                                 startPara, stridePara,
                                                 blockPara);

                // source file name
                std::ostringstream os;
                os << filePath << "/" << fileNamePrefix << "_d"
                   << (modulePos * numUnitsPerReadout + i) << "_f" << hyperSlab
                   << '_' << fileIndex << ".h5";
                std::string srcFileName = os.str();
                LOG(logDEBUG1) << srcFileName;

                // find relative path
                std::string relative_srcFileName = srcFileName;
                {
                    size_t p = srcFileName.rfind('/', srcFileName.length());
                    if (p != std::string::npos)
                        relative_srcFileName = (srcFileName.substr(
                            p + 1, srcFileName.length() - p));
                }

                // source dataset name
                std::ostringstream osfn;
                osfn << "/data";
                if (numImages > 1)
                    osfn << "_f" << std::setfill('0') << std::setw(12)
                         << hyperSlab;
                std::string srcDatasetName = osfn.str();

                // source data dataspace
                hsize_t srcDims[3] = {nDimx, nDimy, nDimz};
                hsize_t srcDimsMax[3] = {H5S_UNLIMITED, nDimy, nDimz};
                DataSpace srcDataSpace(3, srcDims, srcDimsMax);

                // source parameter dataspace
                hsize_t srcDimsPara[1] = {nDimx};
                hsize_t srcDimsMaxPara[1] = {H5S_UNLIMITED};
                DataSpace srcDataSpacePara(1, srcDimsPara, srcDimsMaxPara);

                // mapping of data property list
                plist.setVirtual(vdsDataSpace, relative_srcFileName.c_str(),
                                 srcDatasetName.c_str(), srcDataSpace);

                // mapping of parameter property list
                for (unsigned int p = 0; p < paraSize; ++p) {
                    plistPara[p].setVirtual(
                        vdsDataSpacePara, relative_srcFileName.c_str(),
                        parameterNames[p].c_str(), srcDataSpacePara);
                }

                // H5Sclose(srcDataspace);
                // H5Sclose(srcDataspace_para);
                start[2] += nDimz;
                if (start[2] >= (numModZ * nDimz)) {
                    start[2] = 0;
                    start[1] += nDimy;
                }
                startPara[1]++;
            }
            framesSaved += nDimx;
        }
        // data dataset
        dataSetName_ = "data";
        DataSet vdsDataSet(fd_->createDataSet(dataSetName_.c_str(), dataType,
                                              vdsDataSpace, plist));

        // parameter dataset
        for (unsigned int p = 0; p < paraSize; ++p) {
            DataSet vdsDataSetPara(fd_->createDataSet(
                parameterNames[p].c_str(), parameterDataTypes[p],
                vdsDataSpacePara, plistPara[p]));
        }

        fd_->close();
    } catch (const Exception &error) {
        error.printErrorStack();
        CloseFile();
        throw sls::RuntimeError(
            "Could not create/overwrite virtual HDF5 handles");
    }
    if (!silentMode) {
        LOG(logINFO) << "Virtual File: " << fileName_;
    }
}
