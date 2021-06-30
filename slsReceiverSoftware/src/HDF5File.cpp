/************************************************
 * @file HDF5File.cpp
 * @short sets/gets properties for the HDF5 file,
 * creates/closes the file and writes data to it
 ***********************************************/
#include "HDF5File.h"
#include "Fifo.h"
#include "MasterAttributes.h"
#include "receiver_defs.h"

#include <iomanip>
#include <iostream>
#include <libgen.h> //basename
#include <string.h>

void HDF5File::EndofAcquisition(bool anyPacketsCaught,
                                uint64_t numImagesCaught) {
    // not created before
    if (!virtualfd && anyPacketsCaught) {
        // called only by the one maser receiver
        if (master && masterfd != nullptr) {
            // only one file and one sub image (link current file in master)
            if (((numFilesinAcquisition == 1) && (numDetY * numDetX) == 1)) {
                // dataset name
                std::ostringstream oss;
                oss << "/data";
                if ((*numImages > 1)) {
                    oss << "_f" << std::setfill('0') << std::setw(12) << 0;
                }
                std::string dsetname = oss.str();

                LinkVirtualInMaster(currentFileName, dsetname);
            }
            // create virutal file
            else {
                CreateVirtualDataFile(
                    // infinite images in 1 file, then maxfrperfile =
                    // numImagesCaught
                    ((*maxFramesPerFile == 0) ? numImagesCaught + 1
                                              : *maxFramesPerFile),
                    numImagesCaught + 1);
            }
        }
    }
    numFilesinAcquisition = 0;
}

void HDF5File::CreateVirtualDataFile(uint32_t maxFramesPerFile, uint64_t numf) {

    std::ostringstream osfn;
    osfn << *filePath << "/" << *fileNamePrefix;
    osfn << "_virtual";
    osfn << "_" << *fileIndex;
    osfn << ".h5";
    std::string vname = osfn.str();

    if (!(*silentMode)) {
        LOG(logINFO) << "Virtual File: " << vname;
    }

    int numDetz = numDetX;
    uint32_t nDimy = nPixelsY;
    uint32_t nDimz = ((*dynamicRange == 4) ? (nPixelsX / 2) : nPixelsX);

    std::lock_guard<std::mutex> lock(HDF5File::hdf5Lib);

    try {
        // file
        hid_t dfal = H5Pcreate(H5P_FILE_ACCESS);
        if (dfal < 0)
            throw sls::RuntimeError(
                "Could not create file access property for virtual file " +
                vname);
        if (H5Pset_fclose_degree(dfal, H5F_CLOSE_STRONG) < 0)
            throw sls::RuntimeError(
                "Could not set strong file close degree for virtual file " +
                vname);
        virtualfd = H5Fcreate(vname.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, dfal);
        if (virtualfd < 0)
            throw sls::RuntimeError("Could not create virtual file " + vname);

        // attributes - version
        hid_t dataspace_attr = H5Screate(H5S_SCALAR);
        if (dataspace_attr < 0)
            throw sls::RuntimeError(
                "Could not create dataspace for attribute in virtual file " +
                vname);
        hid_t attrid = H5Acreate2(virtualfd, "version", H5T_NATIVE_DOUBLE,
                                  dataspace_attr, H5P_DEFAULT, H5P_DEFAULT);
        if (attrid < 0)
            throw sls::RuntimeError(
                "Could not create attribute in virtual file " + vname);
        double attr_data = HDF5_WRITER_VERSION;
        if (H5Awrite(attrid, H5T_NATIVE_DOUBLE, &attr_data) < 0)
            throw sls::RuntimeError(
                "Could not write attribute in virtual file " + vname);
        if (H5Aclose(attrid) < 0)
            throw sls::RuntimeError(
                "Could not close attribute in virtual file " + vname);

        // virtual dataspace
        hsize_t vdsdims[3] = {numf, numDetY * nDimy, numDetz * nDimz};
        hid_t vdsDataspace = H5Screate_simple(3, vdsdims, nullptr);
        if (vdsDataspace < 0)
            throw sls::RuntimeError(
                "Could not create virtual dataspace in virtual file " + vname);
        hsize_t vdsdims_para[2] = {numf, (unsigned int)numDetY * numDetz};
        hid_t vdsDataspace_para = H5Screate_simple(2, vdsdims_para, nullptr);
        if (vdsDataspace_para < 0)
            throw sls::RuntimeError("Could not create virtual dataspace "
                                    "(parameters) in virtual file " +
                                    vname);

        // fill values
        hid_t dcpl = H5Pcreate(H5P_DATASET_CREATE);
        if (dcpl < 0)
            throw sls::RuntimeError(
                "Could not create file creation properties in virtual file " +
                vname);
        int fill_value = -1;
        if (H5Pset_fill_value(dcpl, GetDataTypeinC(datatype), &fill_value) < 0)
            throw sls::RuntimeError(
                "Could not create fill value in virtual file " + vname);
        std::vector<hid_t> dcpl_para(parameterNames.size());
        for (unsigned int i = 0; i < parameterNames.size(); ++i) {
            dcpl_para[i] = H5Pcreate(H5P_DATASET_CREATE);
            if (dcpl_para[i] < 0)
                throw sls::RuntimeError(
                    "Could not create file creation properties (parameters) in "
                    "virtual file " +
                    vname);
            if (H5Pset_fill_value(dcpl_para[i],
                                  GetDataTypeinC(parameterDataTypes[i]),
                                  &fill_value) < 0)
                throw sls::RuntimeError("Could not create fill value "
                                        "(parameters) in virtual file " +
                                        vname);
        }

        // hyperslab
        int numMajorHyperslab = numf / maxFramesPerFile;
        if (numf % maxFramesPerFile)
            numMajorHyperslab++;
        uint64_t framesSaved = 0;
        for (int j = 0; j < numMajorHyperslab; j++) {

            uint64_t nDimx = ((numf - framesSaved) > maxFramesPerFile)
                                 ? maxFramesPerFile
                                 : (numf - framesSaved);
            hsize_t offset[3] = {framesSaved, 0, 0};
            hsize_t count[3] = {nDimx, nDimy, nDimz};
            hsize_t offset_para[2] = {framesSaved, 0};
            hsize_t count_para[2] = {nDimx, 1};

            for (int i = 0; i < numDetY * numDetz; ++i) {

                // setect hyperslabs
                if (H5Sselect_hyperslab(vdsDataspace, H5S_SELECT_SET, offset,
                                        nullptr, count, nullptr) < 0) {
                    throw sls::RuntimeError("Could not select hyperslab");
                }
                if (H5Sselect_hyperslab(vdsDataspace_para, H5S_SELECT_SET,
                                        offset_para, nullptr, count_para,
                                        nullptr) < 0) {
                    throw sls::RuntimeError(
                        "Could not select hyperslab for parameters");
                }

                // source file name
                std::ostringstream os;
                os << *filePath << "/" << *fileNamePrefix << "_d"
                   << (*detIndex * (*numUnitsPerDetector) + i) << "_f" << j
                   << '_' << *fileIndex << ".h5";
                std::string srcFileName = os.str();

                LOG(logDEBUG1) << srcFileName;
                // find relative path
                std::string relative_srcFileName = srcFileName;
                {
                    size_t i = srcFileName.rfind('/', srcFileName.length());
                    if (i != std::string::npos)
                        relative_srcFileName = (srcFileName.substr(
                            i + 1, srcFileName.length() - i));
                }

                // source dataset name
                std::ostringstream osfn;
                osfn << "/data";
                if (*numImages > 1)
                    osfn << "_f" << std::setfill('0') << std::setw(12) << j;
                std::string srcDatasetName = osfn.str();

                // source dataspace
                hsize_t srcdims[3] = {nDimx, nDimy, nDimz};
                hsize_t srcdimsmax[3] = {H5S_UNLIMITED, nDimy, nDimz};
                hid_t srcDataspace = H5Screate_simple(3, srcdims, srcdimsmax);
                if (srcDataspace < 0)
                    throw sls::RuntimeError(
                        "Could not create source dataspace in virtual file " +
                        vname);
                hsize_t srcdims_para[1] = {nDimx};
                hsize_t srcdimsmax_para[1] = {H5S_UNLIMITED};
                hid_t srcDataspace_para =
                    H5Screate_simple(1, srcdims_para, srcdimsmax_para);
                if (srcDataspace_para < 0)
                    throw sls::RuntimeError("Could not create source dataspace "
                                            "(parameters) in virtual file " +
                                            vname);

                // mapping
                if (H5Pset_virtual(dcpl, vdsDataspace,
                                   relative_srcFileName.c_str(),
                                   srcDatasetName.c_str(), srcDataspace) < 0) {
                    throw sls::RuntimeError(
                        "Could not set mapping for paramter 1");
                }

                for (unsigned int k = 0; k < parameterNames.size(); ++k) {
                    if (H5Pset_virtual(dcpl_para[k], vdsDataspace_para,
                                       relative_srcFileName.c_str(),
                                       parameterNames[k].c_str(),
                                       srcDataspace_para) < 0) {
                        throw sls::RuntimeError(
                            "Could not set mapping for paramter " +
                            std::to_string(k));
                    }
                }

                // H5Sclose(srcDataspace);
                // H5Sclose(srcDataspace_para);
                offset[2] += nDimz;
                if (offset[2] >= (numDetz * nDimz)) {
                    offset[2] = 0;
                    offset[1] += nDimy;
                }
                offset_para[1]++;
            }
            framesSaved += nDimx;
        }

        // dataset
        std::string virtualDatasetName = "data";
        hid_t vdsdataset = H5Dcreate2(virtualfd, virtualDatasetName.c_str(),
                                      GetDataTypeinC(datatype), vdsDataspace,
                                      H5P_DEFAULT, dcpl, H5P_DEFAULT);
        if (vdsdataset < 0)
            throw sls::RuntimeError(
                "Could not create virutal dataset in virtual file " + vname);

        // virtual parameter dataset
        for (unsigned int i = 0; i < parameterNames.size(); ++i) {
            hid_t vdsdataset_para = H5Dcreate2(
                virtualfd, parameterNames[i].c_str(),
                GetDataTypeinC(parameterDataTypes[i]), vdsDataspace_para,
                H5P_DEFAULT, dcpl_para[i], H5P_DEFAULT);
            if (vdsdataset_para < 0)
                throw sls::RuntimeError("Could not create virutal dataset "
                                        "(parameters) in virtual file " +
                                        vname);
        }

        // close
        H5Fclose(virtualfd);
        virtualfd = 0;

        // link
        LinkVirtualInMaster(vname, virtualDatasetName);
    } catch (const sls::RuntimeError &e) {
        if (virtualfd > 0)
            H5Fclose(virtualfd);
        virtualfd = 0;
    }
}

void HDF5File::LinkVirtualInMaster(std::string fname, std::string dsetname) {

    if (fname == currentFileName) {
        std::lock_guard<std::mutex> lock(HDF5File::hdf5Lib);
    }

    char linkname[100];
    hid_t vfd = 0;

    try {
        hid_t dfal = H5Pcreate(H5P_FILE_ACCESS);
        if (dfal < 0)
            throw sls::RuntimeError(
                "Could not create file access property for link");
        if (H5Pset_fclose_degree(dfal, H5F_CLOSE_STRONG) < 0)
            throw sls::RuntimeError(
                "Could not set strong file close degree for link");

        // open master file
        hid_t mfd = H5Fopen(masterFileName.c_str(), H5F_ACC_RDWR, dfal);
        if (mfd < 0)
            throw sls::RuntimeError("Could not open master file");

        // open virtual file
        vfd = H5Fopen(fname.c_str(), H5F_ACC_RDWR, dfal);
        if (vfd < 0) {
            H5Fclose(mfd);
            mfd = 0;
            throw sls::RuntimeError("Could not open virtual file");
        }

        // find relative path
        std::string relative_virtualfname = fname;
        {
            size_t i = fname.rfind('/', fname.length());
            if (i != std::string::npos)
                relative_virtualfname =
                    (fname.substr(i + 1, fname.length() - i));
        }

        //**data dataset**
        hid_t vdset = H5Dopen2(vfd, dsetname.c_str(), H5P_DEFAULT);
        if (vdset < 0) {
            H5Fclose(mfd);
            throw sls::RuntimeError("Could not open virtual data dataset");
        }
        sprintf(linkname, "/entry/data/%s", dsetname.c_str());
        if (H5Lcreate_external(relative_virtualfname.c_str(), dsetname.c_str(),
                               mfd, linkname, H5P_DEFAULT, H5P_DEFAULT) < 0) {
            H5Fclose(mfd);
            mfd = 0;
            throw sls::RuntimeError("Could not create link to data dataset");
        }
        H5Dclose(vdset);

        //**paramter datasets**
        for (unsigned int i = 0; i < parameterNames.size(); ++i) {
            hid_t vdset_para = H5Dopen2(
                vfd, (std::string(parameterNames[i])).c_str(), H5P_DEFAULT);
            if (vdset_para < 0) {
                H5Fclose(mfd);
                mfd = 0;
                throw sls::RuntimeError(
                    "Could not open virtual parameter dataset to create link");
            }
            sprintf(linkname, "/entry/data/%s",
                    (std::string(parameterNames[i])).c_str());

            if (H5Lcreate_external(relative_virtualfname.c_str(),
                                   parameterNames[i].c_str(), mfd, linkname,
                                   H5P_DEFAULT, H5P_DEFAULT) < 0) {
                H5Fclose(mfd);
                mfd = 0;
                throw sls::RuntimeError(
                    "Could not create link to virtual parameter dataset");
            }
        }

        H5Fclose(mfd);
        mfd = 0;
        H5Fclose(vfd);
        vfd = 0;
    } catch (...) {
        if (vfd > 0)
            H5Fclose(vfd);
        vfd = 0;
    }
}

hid_t HDF5File::GetDataTypeinC(DataType dtype) {
    if (dtype == PredType::STD_U8LE)
        return H5T_STD_U8LE;
    else if (dtype == PredType::STD_U16LE)
        return H5T_STD_U16LE;
    else if (dtype == PredType::STD_U32LE)
        return H5T_STD_U32LE;
    else if (dtype == PredType::STD_U64LE)
        return H5T_STD_U64LE;
    else {
        hid_t s = H5Tcopy(H5T_C_S1);
        H5Tset_size(s, MAX_NUM_PACKETS);
        return s;
    }
}