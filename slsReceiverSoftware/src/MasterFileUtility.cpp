// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "MasterFileUtility.h"
#include "sls/container_utils.h"

#include <iomanip>

namespace masterFileUtility {

std::string CreateMasterBinaryFile(const std::string filePath,
                                   const std::string fileNamePrefix,
                                   const uint64_t fileIndex,
                                   const bool overWriteEnable,
                                   const bool silentMode,
                                   MasterAttributes *attr) {
    // create file name
    std::ostringstream os;
    os << filePath << "/" << fileNamePrefix << "_master"
       << "_" << fileIndex << ".json";
    std::string fileName = os.str();

    // create file
    FILE *fd{nullptr};
    if (!overWriteEnable) {
        if (nullptr == (fd = fopen((const char *)fileName.c_str(), "wx"))) {
            fd = nullptr;
            throw sls::RuntimeError("Could not create binary master file " +
                                    fileName);
        }
    } else if (nullptr == (fd = fopen((const char *)fileName.c_str(), "w"))) {
        fd = nullptr;
        throw sls::RuntimeError(
            "Could not create/overwrite binary master file " + fileName);
    }

    rapidjson::StringBuffer s;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(s);
    attr->GetBinaryAttributes(&writer);
    std::string message = s.GetString();
    if (fwrite((void *)message.c_str(), 1, message.length(), fd) !=
        message.length()) {
        throw sls::RuntimeError(
            "Master binary file incorrect number of bytes written to file");
    }
    if (fd) {
        fclose(fd);
    }
    if (!silentMode) {
        LOG(logINFO) << "Master File: " << fileName;
    }
    return fileName;
}

#ifdef HDF5C
void LinkHDF5FileInMaster(const std::string &masterFileName,
                          const std::string &dataFilename,
                          const std::string &dataSetname,
                          const std::vector<std::string> parameterNames,
                          const bool silentMode, std::mutex *hdf5LibMutex) {

    std::lock_guard<std::mutex> lock(*hdf5LibMutex);
    std::unique_ptr<H5File> fd{nullptr};
    try {
        Exception::dontPrint(); // to handle errors

        FileAccPropList flist;
        flist.setFcloseDegree(H5F_CLOSE_STRONG);

        // open master file
        H5File masterfd(masterFileName.c_str(), H5F_ACC_RDWR,
                        FileCreatPropList::DEFAULT, flist);

        // open data file
        fd = sls::make_unique<H5File>(dataFilename.c_str(), H5F_ACC_RDONLY,
                                      FileCreatPropList::DEFAULT, flist);

        // create link for data dataset
        DataSet dset = fd->openDataSet(dataSetname.c_str());
        std::string linkname = std::string("/entry/data/") + dataSetname;
        if (H5Lcreate_external(dataFilename.c_str(), dataSetname.c_str(),
                               masterfd.getLocId(), linkname.c_str(),
                               H5P_DEFAULT, H5P_DEFAULT) < 0) {
            throw sls::RuntimeError(
                "Could not create link to data dataset in master");
        }

        // create link for parameter datasets
        for (unsigned int i = 0; i < parameterNames.size(); ++i) {
            DataSet pDset = fd->openDataSet(parameterNames[i].c_str());
            linkname = std::string("/entry/data/") + parameterNames[i];
            if (H5Lcreate_external(dataFilename.c_str(),
                                   parameterNames[i].c_str(),
                                   masterfd.getLocId(), linkname.c_str(),
                                   H5P_DEFAULT, H5P_DEFAULT) < 0) {
                throw sls::RuntimeError(
                    "Could not create link to parameter dataset in master");
            }
        }
        fd->close();
        masterfd.close();
    } catch (const Exception &error) {
        error.printErrorStack();
        if (fd != nullptr)
            fd->close();
        throw sls::RuntimeError("Could not link in master hdf5 file");
    }
    if (!silentMode) {
        LOG(logINFO) << "Linked in Master File: " << dataFilename;
    }
}

std::string CreateMasterHDF5File(const std::string &filePath,
                                 const std::string &fileNamePrefix,
                                 const uint64_t fileIndex,
                                 const bool overWriteEnable,
                                 const bool silentMode, MasterAttributes *attr,
                                 std::mutex *hdf5LibMutex) {

    std::ostringstream os;
    os << filePath << "/" << fileNamePrefix << "_master"
       << "_" << fileIndex << ".h5";
    std::string fileName = os.str();

    std::lock_guard<std::mutex> lock(*hdf5LibMutex);

    std::unique_ptr<H5File> fd{nullptr};
    try {
        Exception::dontPrint(); // to handle errors

        FileAccPropList flist;
        flist.setFcloseDegree(H5F_CLOSE_STRONG);

        unsigned int createFlags = H5F_ACC_EXCL;
        if (overWriteEnable) {
            createFlags = H5F_ACC_TRUNC;
        }
        fd = sls::make_unique<H5File>(fileName.c_str(), createFlags,
                                      FileCreatPropList::DEFAULT, flist);

        // attributes - version
        double dValue = HDF5_WRITER_VERSION;
        DataSpace dataspace_attr = DataSpace(H5S_SCALAR);
        Attribute attribute = fd->createAttribute(
            "version", PredType::NATIVE_DOUBLE, dataspace_attr);
        attribute.write(PredType::NATIVE_DOUBLE, &dValue);

        // Create a group in the file
        Group group1(fd->createGroup("entry"));
        Group group2(group1.createGroup("data"));
        Group group3(group1.createGroup("instrument"));
        Group group4(group3.createGroup("beam"));
        Group group5(group3.createGroup("detector"));
        Group group6(group1.createGroup("sample"));

        attr->WriteHDF5Attributes(fd.get(), &group5);
        fd->close();
    } catch (const Exception &error) {
        error.printErrorStack();
        if (fd != nullptr)
            fd->close();
        throw sls::RuntimeError(
            "Could not create/overwrite master HDF5 handles");
    }
    if (!silentMode) {
        LOG(logINFO) << "Master File: " << fileName;
    }
    return fileName;
}

std::array<std::string, 2> CreateVirtualHDF5File(
    const std::string &filePath, const std::string &fileNamePrefix,
    const uint64_t fileIndex, const bool overWriteEnable, const bool silentMode,
    const int modulePos, const int numUnitsPerReadout,
    const uint32_t maxFramesPerFile, const uint64_t numImages,
    const uint32_t nPixelsX, const uint32_t nPixelsY,
    const uint32_t dynamicRange, const uint64_t numImagesCaught,
    const int numModX, const int numModY, const DataType dataType,
    const std::vector<std::string> parameterNames,
    const std::vector<DataType> parameterDataTypes, std::mutex *hdf5LibMutex,
    bool gotthard25um) {

    // virtual file name
    std::ostringstream osfn;
    osfn << filePath << "/" << fileNamePrefix << "_virtual"
         << "_" << fileIndex << ".h5";
    std::string fileName = osfn.str();

    std::string dataSetName = "data";

    unsigned int paraSize = parameterNames.size();
    uint64_t numModZ = numModX;
    uint32_t nDimy = nPixelsY;
    uint32_t nDimz = ((dynamicRange == 4) ? (nPixelsX / 2) : nPixelsX);

    std::lock_guard<std::mutex> lock(*hdf5LibMutex);

    std::unique_ptr<H5File> fd{nullptr};
    try {
        Exception::dontPrint(); // to handle errors

        // file
        FileAccPropList fapl;
        fapl.setFcloseDegree(H5F_CLOSE_STRONG);
        if (!overWriteEnable)
            fd = sls::make_unique<H5File>(fileName.c_str(), H5F_ACC_EXCL,
                                          FileCreatPropList::DEFAULT, fapl);
        else
            fd = sls::make_unique<H5File>(fileName.c_str(), H5F_ACC_TRUNC,
                                          FileCreatPropList::DEFAULT, fapl);

        // attributes - version
        double dValue = HDF5_WRITER_VERSION;
        DataSpace dataspace_attr = DataSpace(H5S_SCALAR);
        Attribute attribute = fd->createAttribute(
            "version", PredType::NATIVE_DOUBLE, dataspace_attr);
        attribute.write(PredType::NATIVE_DOUBLE, &dValue);

        // virtual dataspace
        hsize_t vdsDims[3] = {numImagesCaught, numModY * nDimy,
                              numModZ * nDimz};
        DataSpace vdsDataSpace(3, vdsDims, nullptr);
        hsize_t vdsDimsPara[2] = {numImagesCaught,
                                  (unsigned int)numModY * numModZ};
        DataSpace vdsDataSpacePara(2, vdsDimsPara, nullptr);

        // property list (fill value and datatype)
        int fill_value = -1;
        DSetCreatPropList plist;
        plist.setFillValue(dataType, &fill_value);

        // property list for parameters (datatype)
        std::vector<DSetCreatPropList> plistPara(paraSize);

        // hyperslab (files)
        int numFiles = numImagesCaught / maxFramesPerFile;
        if (numImagesCaught % maxFramesPerFile)
            ++numFiles;
        uint64_t framesSaved = 0;
        for (int iFile = 0; iFile < numFiles; ++iFile) {

            uint64_t nDimx =
                ((numImagesCaught - framesSaved) > maxFramesPerFile)
                    ? maxFramesPerFile
                    : (numImagesCaught - framesSaved);

            hsize_t startLocation[3] = {framesSaved, 0, 0};
            hsize_t strideBetweenBlocks[3] = {1, 1, 1};
            hsize_t numBlocks[3] = {nDimx, nDimy, nDimz};
            hsize_t blockSize[3] = {1, 1, 1};

            hsize_t startLocationPara[2] = {framesSaved, 0};
            hsize_t strideBetweenBlocksPara[3] = {1, 1};
            hsize_t numBlocksPara[2] = {1, 1};
            hsize_t blockSizePara[3] = {nDimx, 1};

            // interleaving for g2
            if (gotthard25um) {
                strideBetweenBlocks[2] = 2;
            }

            for (unsigned int iReadout = 0; iReadout < numModY * numModZ;
                 ++iReadout) {

                // interleaving for g2 (startLocation is 0 and 1)
                if (gotthard25um) {
                    startLocation[2] = iReadout;
                }

                vdsDataSpace.selectHyperslab(H5S_SELECT_SET, numBlocks,
                                             startLocation, strideBetweenBlocks,
                                             blockSize);

                vdsDataSpacePara.selectHyperslab(
                    H5S_SELECT_SET, numBlocksPara, startLocationPara,
                    strideBetweenBlocksPara, blockSizePara);

                // source file name
                std::ostringstream os;
                os << filePath << "/" << fileNamePrefix << "_d"
                   << (modulePos * numUnitsPerReadout + iReadout) << "_f"
                   << iFile << '_' << fileIndex << ".h5";
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
                    osfn << "_f" << std::setfill('0') << std::setw(12) << iFile;
                std::string srcDatasetName = osfn.str();

                // source dataspace
                hsize_t srcDims[3] = {nDimx, nDimy, nDimz};
                hsize_t srcDimsMax[3] = {H5S_UNLIMITED, nDimy, nDimz};
                DataSpace srcDataSpace(3, srcDims, srcDimsMax);
                hsize_t srcDimsPara[1] = {nDimx};
                hsize_t srcDimsMaxPara[1] = {H5S_UNLIMITED};
                DataSpace srcDataSpacePara(1, srcDimsPara, srcDimsMaxPara);

                // mapping of property list
                plist.setVirtual(vdsDataSpace, relative_srcFileName.c_str(),
                                 srcDatasetName.c_str(), srcDataSpace);
                for (unsigned int p = 0; p < paraSize; ++p) {
                    plistPara[p].setVirtual(
                        vdsDataSpacePara, relative_srcFileName.c_str(),
                        parameterNames[p].c_str(), srcDataSpacePara);
                }

                // H5Sclose(srcDataspace);
                // H5Sclose(srcDataspace_para);

                if (!gotthard25um) {
                    startLocation[2] += nDimz;
                    if (startLocation[2] >= (numModZ * nDimz)) {
                        startLocation[2] = 0;
                        startLocation[1] += nDimy;
                    }
                }
                startLocationPara[1]++;
            }
            framesSaved += nDimx;
        }
        // datasets
        DataSet vdsDataSet(fd->createDataSet(dataSetName.c_str(), dataType,
                                             vdsDataSpace, plist));

        for (unsigned int p = 0; p < paraSize; ++p) {
            DataSet vdsDataSetPara(fd->createDataSet(
                parameterNames[p].c_str(), parameterDataTypes[p],
                vdsDataSpacePara, plistPara[p]));
        }

        fd->close();
    } catch (const Exception &error) {
        error.printErrorStack();
        if (fd) {
            fd->close();
        }
        throw sls::RuntimeError(
            "Could not create/overwrite virtual HDF5 handles");
    }
    if (!silentMode) {
        LOG(logINFO) << "Virtual File: " << fileName;
    }
    return std::array<std::string, 2>{fileName, dataSetName};
}
#endif

} // namespace masterFileUtility
