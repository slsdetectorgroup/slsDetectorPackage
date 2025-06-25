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
                          const bool silentMode, std::mutex *hdf5LibMutex, size_t multiRoiSize) {

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

        for (size_t iRoi = 0; iRoi != multiRoiSize; ++iRoi) {

            // create link for data dataset
            std::string datasetname = DATASET_NAME + '_' + std::to_string(iRoi);
            H5::DataSet dset = fd->openDataSet(datasetname);
            std::string linkname =
                std::string("/entry/data/") + datasetname;
            if (H5Lcreate_external(dataFilename.c_str(), datasetname.c_str(),
                                masterfd.getLocId(), linkname.c_str(),
                                H5P_DEFAULT, H5P_DEFAULT) < 0) {
                throw RuntimeError(
                    "Could not create link to data dataset in master");
            }

            // create link for parameter datasets
            for (unsigned int i = 0; i < parameterNames.size(); ++i) {
                std::string parameterDsetName = parameterNames[i] + '_' + std::to_string(iRoi);
                H5::DataSet pDset = fd->openDataSet(parameterDsetName.c_str());
                linkname = std::string("/entry/data/") + parameterDsetName;
                if (H5Lcreate_external(dataFilename.c_str(),
                                    parameterDsetName.c_str(),
                                    masterfd.getLocId(), linkname.c_str(),
                                    H5P_DEFAULT, H5P_DEFAULT) < 0) {
                    throw RuntimeError(
                        "Could not create link to parameter dataset in master");
                }
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

defs::ROI GetGlobalPortRoi(const int iPort, const defs::xy portSize, const int numPortsY) {
    defs::xy portPos = {(iPort / numPortsY), (iPort % numPortsY)};
    const int xmin = portSize.x * portPos.x;
    const int xmax = xmin + portSize.x - 1;
    const int ymin = portSize.y * portPos.y;
    const int ymax = ymin + portSize.y - 1;
    return defs::ROI{xmin, xmax, ymin, ymax}; 
}

int GetNumPortsInRoi(const defs::ROI roi, const defs::xy portSize) {
    if (portSize.x == 0 || portSize.y == 0) {
        throw RuntimeError("Port width or height cannot be zero");
    }
    int iPortXMin = roi.xmin / portSize.x;
    int iPortXMax = roi.xmax / portSize.x;
    int iPortYMin = roi.ymin / portSize.y;
    int iPortYMax = roi.ymax / portSize.y;
    return ((iPortXMax - iPortXMin + 1) * (iPortYMax - iPortYMin + 1));
}

/** Will not be called if dynamic range is 4 and roi enabled */
std::string CreateVirtualHDF5File(
    const std::string &filePath, const std::string &fileNamePrefix,
    const uint64_t fileIndex, const bool overWriteEnable, const bool silentMode,
    const int modulePos, const int numUnitsPerReadout,
    const uint32_t maxFramesPerFile, const int nPixelsX,
    const int nPixelsY, const uint32_t dynamicRange,
    const uint64_t numImagesCaught, const int numModX, const int numModY,
    const H5::DataType dataType, const std::vector<std::string> parameterNames,
    const std::vector<H5::DataType> parameterDataTypes,
    std::mutex *hdf5LibMutex, bool gotthard25um,
    std::vector<defs::ROI> multiRoi) {

    // cannot create
    if (!multiRoi.empty()) {
        if (dynamicRange == 4) {
            throw std::runtime_error("Skipping virtual hdf5 file since rx_roi is "
                                 "enabled and it is in 4 bit mode.");
        }
        if (gotthard25um && (numModX * numModY) == 2) {
            throw std::runtime_error("Skipping virtual hdf5 file since rx_roi is "
                                 "enabled and there are 2 Gotthard 25um modules.");
        }
    }


    // virtual file name
    std::ostringstream osfn;
    osfn << filePath << "/" << fileNamePrefix << "_virtual"
         << "_" << fileIndex << ".h5";
    std::string fileName = osfn.str();

    unsigned int paraSize = parameterNames.size();

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

        // complete detector in roi
        if (multiRoi.size() == 1 && multiRoi[0].completeRoi()) {
            int ny = nPixelsY * numModY;
            int nx = nPixelsX * numModX;
            if (nPixelsY == 1) {
                multiRoi.push_back(defs::ROI{0, nx - 1});
            } else {
                multiRoi.push_back(defs::ROI{0, nx - 1, 0, ny - 1});
            }
        }

        uint64_t depth = numImagesCaught;
        uint64_t nports = numModX * numModY;
        int numFiles = numImagesCaught / maxFramesPerFile;
        if (numImagesCaught % maxFramesPerFile)
                ++numFiles;
        defs::xy portSize{nPixelsX, nPixelsY};

        for (size_t iRoi = 0; iRoi != multiRoi.size(); ++iRoi) {
            uint64_t width = multiRoi[iRoi].width();
            uint64_t height = multiRoi[iRoi].height();
            uint64_t nportsInRoi = GetNumPortsInRoi(multiRoi[iRoi], portSize);

            // dataspace
            hsize_t vdsDims[DATA_RANK] = {depth, height, width};
            hsize_t vdsDimsPara[VDS_PARA_RANK] = {depth, nportsInRoi};
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
            uint64_t framesSaved = 0;
            for (int iFile = 0; iFile != numFiles; ++iFile) {
                
                uint64_t nImagesInFile = numImagesCaught - framesSaved;
                if ((numImagesCaught - framesSaved) > maxFramesPerFile)
                    nImagesInFile = maxFramesPerFile;

                // start location and blocksize recalculated later for each readout (because of irregular roi)
                hsize_t startLocation[DATA_RANK] = {framesSaved, 0, 0};
                hsize_t nx = static_cast<hsize_t>(nPixelsX);
                hsize_t ny = static_cast<hsize_t>(nPixelsY);
                hsize_t blockSize[DATA_RANK] = {nImagesInFile, nx, ny};
                hsize_t strideBetweenBlocks[DATA_RANK] = {1, 1, 1};
                hsize_t numBlocks[DATA_RANK] = {1, 1, 1};


                // start location of parameter datasets is recalcualted later
                hsize_t startLocationPara[VDS_PARA_RANK] = {framesSaved, 0};
                hsize_t strideBetweenBlocksPara[VDS_PARA_RANK] = {1, 1};
                hsize_t numBlocksPara[VDS_PARA_RANK] = {1, 1};
                hsize_t blockSizePara[VDS_PARA_RANK] = {nImagesInFile, 1};

                // interleaving for g2
                if (gotthard25um) {
                    strideBetweenBlocks[2] = 2;
                }

                for (unsigned int iReadout = 0; iReadout < nports; ++iReadout) {

                    // skip if roi does not overlap
                    auto globalPortRoi = GetGlobalPortRoi(iReadout, portSize, numModY);
                    if (!globalPortRoi.overlap(multiRoi[iRoi]))
                        continue; 

                    // calculate start location  (with roi)
                    int xmin = std::max(multiRoi[iRoi].xmin, globalPortRoi.xmin);
                    int xmax = std::min(multiRoi[iRoi].xmax, globalPortRoi.xmax);
                    int ymin = std::max(multiRoi[iRoi].ymin, globalPortRoi.ymin);
                    int ymax = std::min(multiRoi[iRoi].ymax, globalPortRoi.ymax);
                    uint32_t portRoiHeight  = ymax - ymin + 1;
                    uint32_t portRoiWidth  = xmax - xmin + 1;

                    // recalculating start location and block size
                    ++startLocationPara[1];
                    if (!gotthard25um) {
                        startLocation[1] = ymin;
                        startLocation[2] = xmin;
                        blockSize[1] = portRoiHeight;
                        blockSize[2] = portRoiWidth;
                    }
                    // interleaving for g2 (startLocation is 0 and 1)
                    else {
                        ++startLocation[2];
                    }

                    vdsDataSpace.selectHyperslab(
                        H5S_SELECT_SET, numBlocks, startLocation,
                        strideBetweenBlocks, blockSize);

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
                    hsize_t srcDims[DATA_RANK] = {nImagesInFile, portRoiHeight, portRoiWidth};
                    hsize_t srcDimsMax[DATA_RANK] = {H5S_UNLIMITED, portRoiHeight,
                                                     portRoiWidth};
                    H5::DataSpace srcDataSpace(DATA_RANK, srcDims, srcDimsMax);
                    hsize_t srcDimsPara[PARA_RANK] = {nImagesInFile};
                    hsize_t srcDimsMaxPara[PARA_RANK] = {H5S_UNLIMITED};
                    H5::DataSpace srcDataSpacePara(PARA_RANK, srcDimsPara,
                                                   srcDimsMaxPara);
                    // temporary fixfor corner case bug:
                    // (framescaught not multiple of framesperfile,
                    // virtual parameter datasets error loading (bad scalar
                    // value))
                    // TODO WHY????
                    /*if (nDimz != maxFramesPerFile) {
                        hsize_t count[1] = {nDimz};
                        hsize_t start[1] = {0};
                        srcDataSpacePara.selectHyperslab(
                            H5S_SELECT_SET, count, start,
                    strideBetweenBlocksPara, blockSizePara);
                    }*/

                    // mapping of property list
                    std::string datasetname = DATASET_NAME + '_' + std::to_string(iRoi);
                    plist.setVirtual(vdsDataSpace, relative_srcFileName.c_str(),
                                     datasetname.c_str(), srcDataSpace);
                    for (unsigned int p = 0; p < paraSize; ++p) {
                        std::string parameterDsetName = parameterNames[p] + '_' + std::to_string(iRoi);
                        plistPara[p].setVirtual(
                            vdsDataSpacePara, relative_srcFileName.c_str(),
                            parameterDsetName.c_str(), srcDataSpacePara);
                    }

                    // H5Sclose(srcDataspace);
                    // H5Sclose(srcDataspace_para);

                    ++startLocationPara[1];
                }
                framesSaved += nImagesInFile;
            }
            // datasets
            H5::DataSet vdsDataSet(
                fd->createDataSet(DATASET_NAME, dataType, vdsDataSpace, plist));

            for (unsigned int p = 0; p < paraSize; ++p) {
                H5::DataSet vdsDataSetPara(fd->createDataSet(
                    parameterNames[p].c_str(), parameterDataTypes[p],
                    vdsDataSpacePara, plistPara[p]));
            }
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
