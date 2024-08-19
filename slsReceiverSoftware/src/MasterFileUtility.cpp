// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "MasterFileUtility.h"
#include "sls/container_utils.h"

#include <iomanip>

namespace sls {

namespace masterFileUtility {

std::string CreateMasterBinaryFile(const std::string &filePath,
                                   const std::string &fileNamePrefix,
                                   const uint64_t fileIndex,
                                   const bool overWriteEnable,
                                   const bool silentMode,
                                   MasterAttributes *attr) {
    std::ostringstream os;
    os << filePath << "/" << fileNamePrefix << "_master"
       << "_" << fileIndex << ".json";
    std::string fileName = os.str();

    std::string mode = "w";
    if (!overWriteEnable)
        mode = "wx";
    FILE *fd = fopen(fileName.c_str(), mode.c_str());
    if (!fd) {
        throw RuntimeError("Could not create/overwrite binary master file " +
                           fileName);
    }

    rapidjson::StringBuffer s;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(s);
    attr->GetBinaryAttributes(&writer);
    if (fwrite(s.GetString(), strlen(s.GetString()), 1, fd) != 1) {
        throw RuntimeError(
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
void LinkHDF5FileInMaster(std::string &masterFileName,
                          std::string &dataFilename,
                          std::vector<std::string> parameterNames,
                          const bool silentMode, std::mutex *hdf5LibMutex) {

    std::lock_guard<std::mutex> lock(*hdf5LibMutex);
    std::unique_ptr<H5::H5File> fd{nullptr};
    try {
        H5::Exception::dontPrint(); // to handle errors

        H5::FileAccPropList flist;
        flist.setFcloseDegree(H5F_CLOSE_STRONG);

        // open master file
        H5::H5File masterfd(masterFileName.c_str(), H5F_ACC_RDWR,
                            H5::FileCreatPropList::DEFAULT, flist);

        // open data file
        fd = make_unique<H5::H5File>(dataFilename.c_str(), H5F_ACC_RDONLY,
                                     H5::FileCreatPropList::DEFAULT, flist);

        // create link for data dataset
        H5::DataSet dset = fd->openDataSet(DATASET_NAME);
        std::string linkname =
            std::string("/entry/data/") + std::string(DATASET_NAME);
        if (H5Lcreate_external(dataFilename.c_str(), DATASET_NAME,
                               masterfd.getLocId(), linkname.c_str(),
                               H5P_DEFAULT, H5P_DEFAULT) < 0) {
            throw RuntimeError(
                "Could not create link to data dataset in master");
        }

        // create link for parameter datasets
        for (unsigned int i = 0; i < parameterNames.size(); ++i) {
            H5::DataSet pDset = fd->openDataSet(parameterNames[i].c_str());
            linkname = std::string("/entry/data/") + parameterNames[i];
            if (H5Lcreate_external(dataFilename.c_str(),
                                   parameterNames[i].c_str(),
                                   masterfd.getLocId(), linkname.c_str(),
                                   H5P_DEFAULT, H5P_DEFAULT) < 0) {
                throw RuntimeError(
                    "Could not create link to parameter dataset in master");
            }
        }
        fd->close();
        masterfd.close();
    } catch (const H5::Exception &error) {
        error.printErrorStack();
        if (fd != nullptr)
            fd->close();
        throw RuntimeError("Could not link in master hdf5 file");
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

    std::unique_ptr<H5::H5File> fd{nullptr};
    try {
        H5::Exception::dontPrint(); // to handle errors

        H5::FileAccPropList flist;
        flist.setFcloseDegree(H5F_CLOSE_STRONG);

        unsigned int createFlags = H5F_ACC_EXCL;
        if (overWriteEnable) {
            createFlags = H5F_ACC_TRUNC;
        }
        fd = make_unique<H5::H5File>(fileName.c_str(), createFlags,
                                     H5::FileCreatPropList::DEFAULT, flist);

        // attributes - version
        double dValue = HDF5_WRITER_VERSION;
        H5::DataSpace dataspace_attr = H5::DataSpace(H5S_SCALAR);
        H5::Attribute attribute = fd->createAttribute(
            "version", H5::PredType::NATIVE_DOUBLE, dataspace_attr);
        attribute.write(H5::PredType::NATIVE_DOUBLE, &dValue);

        // Create a group in the file
        H5::Group group1(fd->createGroup("entry"));
        H5::Group group2(group1.createGroup("data"));
        H5::Group group3(group1.createGroup("instrument"));
        H5::Group group4(group3.createGroup("beam"));
        H5::Group group5(group3.createGroup("detector"));
        H5::Group group6(group1.createGroup("sample"));

        attr->WriteHDF5Attributes(fd.get(), &group5);
        fd->close();
    } catch (const H5::Exception &error) {
        error.printErrorStack();
        if (fd != nullptr)
            fd->close();
        throw RuntimeError("Could not create/overwrite master HDF5 handles");
    }
    if (!silentMode) {
        LOG(logINFO) << "Master File: " << fileName;
    }
    return fileName;
}

std::string CreateVirtualHDF5File(
    const std::string &filePath, const std::string &fileNamePrefix,
    const uint64_t fileIndex, const bool overWriteEnable, const bool silentMode,
    const int modulePos, const int numUnitsPerReadout,
    const uint32_t maxFramesPerFile, const uint32_t nPixelsX,
    const uint32_t nPixelsY, const uint32_t dynamicRange,
    const uint64_t numImagesCaught, const int numModX, const int numModY,
    const H5::DataType dataType, const std::vector<std::string> parameterNames,
    const std::vector<H5::DataType> parameterDataTypes,
    std::mutex *hdf5LibMutex, bool gotthard25um) {

    // virtual file name
    std::ostringstream osfn;
    osfn << filePath << "/" << fileNamePrefix << "_virtual"
         << "_" << fileIndex << ".h5";
    std::string fileName = osfn.str();

    unsigned int paraSize = parameterNames.size();
    uint64_t numModZ = numModX;
    uint32_t nDimy = nPixelsY;
    uint32_t nDimz = ((dynamicRange == 4) ? (nPixelsX / 2) : nPixelsX);

    std::lock_guard<std::mutex> lock(*hdf5LibMutex);

    std::unique_ptr<H5::H5File> fd{nullptr};
    try {
        H5::Exception::dontPrint(); // to handle errors

        // file
        H5::FileAccPropList fapl;
        fapl.setFcloseDegree(H5F_CLOSE_STRONG);
        if (!overWriteEnable)
            fd = make_unique<H5::H5File>(fileName.c_str(), H5F_ACC_EXCL,
                                         H5::FileCreatPropList::DEFAULT, fapl);
        else
            fd = make_unique<H5::H5File>(fileName.c_str(), H5F_ACC_TRUNC,
                                         H5::FileCreatPropList::DEFAULT, fapl);

        // attributes - version
        double dValue = HDF5_WRITER_VERSION;
        H5::DataSpace dataspace_attr = H5::DataSpace(H5S_SCALAR);
        H5::Attribute attribute = fd->createAttribute(
            "version", H5::PredType::NATIVE_DOUBLE, dataspace_attr);
        attribute.write(H5::PredType::NATIVE_DOUBLE, &dValue);

        // dataspace
        hsize_t vdsDims[DATA_RANK] = {numImagesCaught, numModY * nDimy,
                                      numModZ * nDimz};
        hsize_t vdsDimsPara[VDS_PARA_RANK] = {numImagesCaught,
                                              numModY * numModZ};
        H5::DataSpace vdsDataSpace(DATA_RANK, vdsDims, nullptr);
        H5::DataSpace vdsDataSpacePara(VDS_PARA_RANK, vdsDimsPara, nullptr);

        // property list
        H5::DSetCreatPropList plist;
        uint64_t fill_value = -1;
        plist.setFillValue(dataType, &fill_value);
        std::vector<H5::DSetCreatPropList> plistPara(paraSize);
        // ignoring last fill (string)
        for (unsigned int i = 0; i != plistPara.size() - 1; ++i) {
            plistPara[i].setFillValue(parameterDataTypes[i], &fill_value);
        }

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

            hsize_t startLocation[DATA_RANK] = {framesSaved, 0, 0};
            hsize_t strideBetweenBlocks[DATA_RANK] = {1, 1, 1};
            hsize_t numBlocks[DATA_RANK] = {nDimx, nDimy, nDimz};
            hsize_t blockSize[DATA_RANK] = {1, 1, 1};

            hsize_t startLocationPara[VDS_PARA_RANK] = {framesSaved, 0};
            hsize_t strideBetweenBlocksPara[VDS_PARA_RANK] = {1, 1};
            hsize_t numBlocksPara[VDS_PARA_RANK] = {nDimx, 1};
            hsize_t blockSizePara[VDS_PARA_RANK] = {1, 1};

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

                // source dataspace
                hsize_t srcDims[DATA_RANK] = {nDimx, nDimy, nDimz};
                hsize_t srcDimsMax[DATA_RANK] = {H5S_UNLIMITED, nDimy, nDimz};
                H5::DataSpace srcDataSpace(DATA_RANK, srcDims, srcDimsMax);
                hsize_t srcDimsPara[PARA_RANK] = {nDimx};
                hsize_t srcDimsMaxPara[PARA_RANK] = {H5S_UNLIMITED};
                H5::DataSpace srcDataSpacePara(PARA_RANK, srcDimsPara,
                                               srcDimsMaxPara);
                // temporary fixfor corner case bug:
                // (framescaught not multiple of framesperfile,
                // virtual parameter datasets error loading (bad scalar value))
                if (nDimx != maxFramesPerFile) {
                    hsize_t count[1] = {nDimx};
                    hsize_t start[1] = {0};
                    srcDataSpacePara.selectHyperslab(
                        H5S_SELECT_SET, count, start, strideBetweenBlocksPara,
                        blockSizePara);
                }

                // mapping of property list
                plist.setVirtual(vdsDataSpace, relative_srcFileName.c_str(),
                                 DATASET_NAME, srcDataSpace);
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
                ++startLocationPara[1];
            }
            framesSaved += nDimx;
        }
        // datasets
        H5::DataSet vdsDataSet(
            fd->createDataSet(DATASET_NAME, dataType, vdsDataSpace, plist));

        for (unsigned int p = 0; p < paraSize; ++p) {
            H5::DataSet vdsDataSetPara(fd->createDataSet(
                parameterNames[p].c_str(), parameterDataTypes[p],
                vdsDataSpacePara, plistPara[p]));
        }

        fd->close();
    } catch (const H5::Exception &error) {
        error.printErrorStack();
        if (fd) {
            fd->close();
        }
        throw RuntimeError("Could not create/overwrite virtual HDF5 handles");
    }
    if (!silentMode) {
        LOG(logINFO) << "Virtual File: " << fileName;
    }
    return fileName;
}
#endif

} // namespace masterFileUtility

} // namespace sls
