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

std::mutex HDF5File::hdf5Lib;

HDF5File::HDF5File(int ind, uint32_t *maxf, int *nd, std::string *fname,
                   std::string *fpath, uint64_t *findex, bool *owenable,
                   int *dindex, int *nunits, uint64_t *nf, uint32_t *dr,
                   uint32_t *portno, uint32_t nx, uint32_t ny, bool *smode)
    :

      File(ind, HDF5, maxf, nd, fname, fpath, findex, owenable, dindex, nunits,
           nf, dr, portno, smode),
      masterfd(nullptr), virtualfd(0), filefd(nullptr), dataspace(nullptr),
      dataset(nullptr), datatype(PredType::STD_U16LE), nPixelsX(nx),
      nPixelsY(ny), numFramesInFile(0), numActualPacketsInFile(0),
      numFilesinAcquisition(0), dataspace_para(nullptr), extNumImages(0) {
    PrintMembers();
    dataset_para.clear();
    parameterNames.clear();
    parameterDataTypes.clear();

    parameterNames = std::vector<std::string>{
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
    parameterDataTypes = std::vector<DataType>{
        PredType::STD_U64LE, PredType::STD_U32LE, PredType::STD_U32LE,
        PredType::STD_U64LE, PredType::STD_U64LE, PredType::STD_U16LE,
        PredType::STD_U16LE, PredType::STD_U16LE, PredType::STD_U16LE,
        PredType::STD_U32LE, PredType::STD_U16LE, PredType::STD_U8LE,
        PredType::STD_U8LE,  strdatatype};
}

HDF5File::~HDF5File() { CloseAllFiles(); }

void HDF5File::SetNumberofPixels(uint32_t nx, uint32_t ny) {
    nPixelsX = nx;
    nPixelsY = ny;
}

void HDF5File::CreateFile() {
    numFilesinAcquisition++;
    numFramesInFile = 0;
    numActualPacketsInFile = 0;

    // first time
    if (subFileIndex == 0u) {
        switch (*dynamicRange) {
        case 16:
            datatype = PredType::STD_U16LE;
            break;
        case 32:
            datatype = PredType::STD_U32LE;
            break;
        default:
            datatype = PredType::STD_U8LE;
            break;
        }
    }
    CreateDataFile();
}

void HDF5File::CloseCurrentFile() {
    CloseFile(filefd, false);
    for (unsigned int i = 0; i < dataset_para.size(); ++i)
        delete dataset_para[i];
    dataset_para.clear();
    if (dataspace_para) {
        delete dataspace_para;
        dataspace_para = nullptr;
    }
    if (dataset) {
        delete dataset;
        dataset = nullptr;
    }
    if (dataspace) {
        delete dataspace;
        dataspace = nullptr;
    }
}

void HDF5File::CloseAllFiles() {
    numFilesinAcquisition = 0;
    {
        CloseFile(filefd, false);
        if (master) {
            CloseFile(masterfd, true);
            // close virtual file
            // c code due to only c implementation of H5Pset_virtual available
            if (virtualfd != 0) {
                if (H5Fclose(virtualfd) < 0) {
                    LOG(logERROR) << "Could not close virtual HDF5 handles";
                }
                virtualfd = 0;
            }
        }
    }
    for (unsigned int i = 0; i < dataset_para.size(); ++i)
        delete dataset_para[i];
    dataset_para.clear();
    delete dataspace_para;
    delete dataset;
    delete dataspace;
}

void HDF5File::WriteToFile(char *buffer, int bufferSize,
                           uint64_t currentFrameNumber,
                           uint32_t numPacketsCaught) {

    // check if maxframesperfile = 0 for infinite
    if ((*maxFramesPerFile) && (numFramesInFile >= (*maxFramesPerFile))) {
        CloseCurrentFile();
        ++subFileIndex;
        CreateFile();
    }
    numFramesInFile++;
    numActualPacketsInFile += numPacketsCaught;

    // extend dataset (when receiver start followed by many status starts
    // (jungfrau)))
    if (currentFrameNumber >= extNumImages) {
        ExtendDataset();
    }

    WriteDataFile(currentFrameNumber, buffer + sizeof(sls_receiver_header));
    WriteParameterDatasets(currentFrameNumber, (sls_receiver_header *)(buffer));
}

void HDF5File::CreateMasterFile(bool masterFileWriteEnable,
                                MasterAttributes &attr) {

    // beginning of every acquisition
    numFramesInFile = 0;
    numActualPacketsInFile = 0;
    extNumImages = *numImages;

    if (masterFileWriteEnable && master) {
        virtualfd = 0;
        CreateMasterDataFile(attr);
    }
}

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

void HDF5File::CloseFile(H5File *&fd, bool masterFile) {
    std::lock_guard<std::mutex> lock(HDF5File::hdf5Lib);
    try {
        Exception::dontPrint(); // to handle errors
        if (fd) {
            fd->close();
            delete fd;
            fd = nullptr;
        }
    } catch (const Exception &error) {
        LOG(logERROR) << "Could not close " << (masterFile ? "master" : "data")
                      << " HDF5 handles of index " << index;
        error.printErrorStack();
    }
}

void HDF5File::WriteDataFile(uint64_t currentFrameNumber, char *buffer) {
    std::lock_guard<std::mutex> lock(HDF5File::hdf5Lib);

    uint64_t nDimx =
        ((*maxFramesPerFile == 0) ? currentFrameNumber
                                  : currentFrameNumber % (*maxFramesPerFile));
    uint32_t nDimy = nPixelsY;
    uint32_t nDimz = ((*dynamicRange == 4) ? (nPixelsX / 2) : nPixelsX);

    hsize_t count[3] = {1, nDimy, nDimz};
    hsize_t start[3] = {nDimx, 0, 0};
    hsize_t dims2[2] = {nDimy, nDimz};
    try {
        Exception::dontPrint(); // to handle errors

        dataspace->selectHyperslab(H5S_SELECT_SET, count, start);
        DataSpace memspace(2, dims2);
        dataset->write(buffer, datatype, memspace, *dataspace);
        memspace.close();
    } catch (const Exception &error) {
        LOG(logERROR) << "Could not write to file in object " << index;
        error.printErrorStack();
        throw sls::RuntimeError("Could not write to file in object " +
                                std::to_string(index));
    }
}

void HDF5File::WriteParameterDatasets(uint64_t currentFrameNumber,
                                      sls_receiver_header *rheader) {
    std::lock_guard<std::mutex> lock(HDF5File::hdf5Lib);

    uint64_t fnum =
        ((*maxFramesPerFile == 0) ? currentFrameNumber
                                  : currentFrameNumber % (*maxFramesPerFile));

    sls_detector_header header = rheader->detHeader;
    hsize_t count[1] = {1};
    hsize_t start[1] = {fnum};
    int i = 0;
    try {
        Exception::dontPrint(); // to handle errors
        dataspace_para->selectHyperslab(H5S_SELECT_SET, count, start);
        DataSpace memspace(H5S_SCALAR);
        dataset_para[0]->write(&header.frameNumber, parameterDataTypes[0],
                               memspace, *dataspace_para);
        i = 1;
        dataset_para[1]->write(&header.expLength, parameterDataTypes[1],
                               memspace, *dataspace_para);
        i = 2;
        dataset_para[2]->write(&header.packetNumber, parameterDataTypes[2],
                               memspace, *dataspace_para);
        i = 3;
        dataset_para[3]->write(&header.bunchId, parameterDataTypes[3], memspace,
                               *dataspace_para);
        i = 4;
        dataset_para[4]->write(&header.timestamp, parameterDataTypes[4],
                               memspace, *dataspace_para);
        i = 5;
        dataset_para[5]->write(&header.modId, parameterDataTypes[5], memspace,
                               *dataspace_para);
        i = 6;
        dataset_para[6]->write(&header.row, parameterDataTypes[6], memspace,
                               *dataspace_para);
        i = 7;
        dataset_para[7]->write(&header.column, parameterDataTypes[7], memspace,
                               *dataspace_para);
        i = 8;
        dataset_para[8]->write(&header.reserved, parameterDataTypes[8],
                               memspace, *dataspace_para);
        i = 9;
        dataset_para[9]->write(&header.debug, parameterDataTypes[9], memspace,
                               *dataspace_para);
        i = 10;
        dataset_para[10]->write(&header.roundRNumber, parameterDataTypes[10],
                                memspace, *dataspace_para);
        i = 11;
        dataset_para[11]->write(&header.detType, parameterDataTypes[11],
                                memspace, *dataspace_para);
        i = 12;
        dataset_para[12]->write(&header.version, parameterDataTypes[12],
                                memspace, *dataspace_para);
        i = 13;

        // contiguous bitset
        if (sizeof(sls_bitset) == sizeof(bitset_storage)) {
            dataset_para[13]->write((char *)&(rheader->packetsMask),
                                    parameterDataTypes[13], memspace,
                                    *dataspace_para);
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
            dataset_para[13]->write((char *)storage, parameterDataTypes[13],
                                    memspace, *dataspace_para);
        }
        i = 14;
    } catch (const Exception &error) {
        error.printErrorStack();
        throw sls::RuntimeError(
            "Could not write parameters (index:" + std::to_string(i) +
            ") to file in object " + std::to_string(index));
    }
}

void HDF5File::ExtendDataset() {
    std::lock_guard<std::mutex> lock(HDF5File::hdf5Lib);

    try {
        Exception::dontPrint(); // to handle errors

        hsize_t dims[3];
        dataspace->getSimpleExtentDims(dims);
        dims[0] += *numImages;

        dataset->extend(dims);
        delete dataspace;
        dataspace = nullptr;
        dataspace = new DataSpace(dataset->getSpace());

        hsize_t dims_para[1] = {dims[0]};
        for (unsigned int i = 0; i < dataset_para.size(); ++i)
            dataset_para[i]->extend(dims_para);
        delete dataspace_para;
        dataspace_para = nullptr;
        dataspace_para = new DataSpace(dataset_para[0]->getSpace());

    } catch (const Exception &error) {
        error.printErrorStack();
        throw sls::RuntimeError("Could not extend dataset in object " +
                                std::to_string(index));
    }
    if (!(*silentMode)) {
        LOG(logINFO) << index << " Extending HDF5 dataset by " << extNumImages
                     << ", Total x Dimension: " << (extNumImages + *numImages);
    }
    extNumImages += *numImages;
}

void HDF5File::CreateDataFile() {

    std::ostringstream os;
    os << *filePath << "/" << *fileNamePrefix << "_d"
       << (*detIndex * (*numUnitsPerDetector) + index) << "_f" << subFileIndex
       << '_' << *fileIndex << ".h5";
    currentFileName = os.str();

    std::lock_guard<std::mutex> lock(HDF5File::hdf5Lib);

    uint64_t framestosave =
        ((*maxFramesPerFile == 0) ? *numImages : // infinite images
             (((extNumImages - subFileIndex) > (*maxFramesPerFile))
                  ? // save up to maximum at a time
                  (*maxFramesPerFile)
                  : (extNumImages - subFileIndex)));

    uint64_t nDimx = framestosave;
    uint32_t nDimy = nPixelsY;
    uint32_t nDimz = ((*dynamicRange == 4) ? (nPixelsX / 2) : nPixelsX);

    try {
        Exception::dontPrint(); // to handle errors

        // file
        FileAccPropList fapl;
        fapl.setFcloseDegree(H5F_CLOSE_STRONG);
        filefd = nullptr;
        if (!(*overWriteEnable))
            filefd = new H5File(currentFileName.c_str(), H5F_ACC_EXCL,
                                FileCreatPropList::DEFAULT, fapl);
        else
            filefd = new H5File(currentFileName.c_str(), H5F_ACC_TRUNC,
                                FileCreatPropList::DEFAULT, fapl);

        // attributes - version
        double dValue = HDF5_WRITER_VERSION;
        DataSpace dataspace_attr = DataSpace(H5S_SCALAR);
        Attribute attribute = filefd->createAttribute(
            "version", PredType::NATIVE_DOUBLE, dataspace_attr);
        attribute.write(PredType::NATIVE_DOUBLE, &dValue);

        // dataspace
        hsize_t srcdims[3] = {nDimx, nDimy, nDimz};
        hsize_t srcdimsmax[3] = {H5S_UNLIMITED, nDimy, nDimz};
        dataspace = nullptr;
        dataspace = new DataSpace(3, srcdims, srcdimsmax);

        // dataset name
        std::ostringstream osfn;
        osfn << "/data";
        if (*numImages > 1)
            osfn << "_f" << std::setfill('0') << std::setw(12) << subFileIndex;
        std::string dsetname = osfn.str();

        // dataset
        // fill value
        DSetCreatPropList plist;
        int fill_value = -1;
        plist.setFillValue(datatype, &fill_value);
        // always create chunked dataset as unlimited is only
        // supported with chunked layout
        hsize_t chunk_dims[3] = {MAX_CHUNKED_IMAGES, nDimy, nDimz};
        plist.setChunk(3, chunk_dims);
        dataset = nullptr;
        dataset = new DataSet(filefd->createDataSet(dsetname.c_str(), datatype,
                                                    *dataspace, plist));

        // create parameter datasets
        hsize_t dims[1] = {nDimx};
        hsize_t dimsmax[1] = {H5S_UNLIMITED};
        dataspace_para = nullptr;
        dataspace_para = new DataSpace(1, dims, dimsmax);

        // always create chunked dataset as unlimited is only
        // supported with chunked layout
        DSetCreatPropList paralist;
        hsize_t chunkpara_dims[3] = {MAX_CHUNKED_IMAGES};
        paralist.setChunk(1, chunkpara_dims);

        for (unsigned int i = 0; i < parameterNames.size(); ++i) {
            DataSet *ds = new DataSet(filefd->createDataSet(
                parameterNames[i].c_str(), parameterDataTypes[i],
                *dataspace_para, paralist));
            dataset_para.push_back(ds);
        }
    } catch (const Exception &error) {
        error.printErrorStack();
        if (filefd) {
            filefd->close();
        }
        throw sls::RuntimeError("Could not create HDF5 handles in object " +
                                index);
    }
    if (!(*silentMode)) {
        LOG(logINFO) << *udpPortNumber
                     << ": HDF5 File created: " << currentFileName;
    }
}

void HDF5File::CreateMasterDataFile(MasterAttributes &attr) {

    std::ostringstream os;
    os << *filePath << "/" << *fileNamePrefix << "_master"
       << "_" << *fileIndex << ".h5";
    masterFileName = os.str();

    if (!(*silentMode)) {
        LOG(logINFO) << "Master File: " << masterFileName;
    }
    masterFileAttributes.version = HDF5_WRITER_VERSION;

    std::lock_guard<std::mutex> lock(HDF5File::hdf5Lib);

    try {
        Exception::dontPrint(); // to handle errors

        FileAccPropList flist;
        flist.setFcloseDegree(H5F_CLOSE_STRONG);
        masterfd = nullptr;
        if (!(*overWriteEnable))
            masterfd = new H5File(masterFileName.c_str(), H5F_ACC_EXCL,
                                  FileCreatPropList::DEFAULT, flist);
        else
            masterfd = new H5File(masterFileName.c_str(), H5F_ACC_TRUNC,
                                  FileCreatPropList::DEFAULT, flist);

        // create attributes
        // version
        {
            double dValue = masterFileAttributes.version;
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            Attribute attribute = masterfd->createAttribute(
                "version", PredType::NATIVE_DOUBLE, dataspace);
            attribute.write(PredType::NATIVE_DOUBLE, &dValue);
        }
        // Create a group in the file
        Group group1(masterfd->createGroup("entry"));
        Group group2(group1.createGroup("data"));
        Group group3(group1.createGroup("instrument"));
        Group group4(group3.createGroup("beam"));
        Group group5(group3.createGroup("detector"));
        Group group6(group1.createGroup("sample"));

        // Dynamic Range
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            DataSet dataset = group5.createDataSet(
                "dynamic range", PredType::NATIVE_INT, dataspace);
            dataset.write(&(masterFileAttributes.dynamicRange),
                          PredType::NATIVE_INT);
            DataSpace dataspaceAttr = DataSpace(H5S_SCALAR);
            StrType strdatatype(PredType::C_S1, 256);
            Attribute attribute =
                dataset.createAttribute("unit", strdatatype, dataspaceAttr);
            attribute.write(strdatatype, std::string("bits"));
        }

        // Ten Giga
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            int iValue = masterFileAttributes.tenGiga;
            DataSet dataset = group5.createDataSet(
                "ten giga enable", PredType::NATIVE_INT, dataspace);
            dataset.write(&iValue, PredType::NATIVE_INT);
        }

        // Image Size
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            DataSet dataset = group5.createDataSet(
                "image size", PredType::NATIVE_INT, dataspace);
            dataset.write(&(masterFileAttributes.imageSize),
                          PredType::NATIVE_INT);
            DataSpace dataspaceAttr = DataSpace(H5S_SCALAR);
            StrType strdatatype(PredType::C_S1, 256);
            Attribute attribute =
                dataset.createAttribute("unit", strdatatype, dataspaceAttr);
            attribute.write(strdatatype, std::string("bytes"));
        }

        // x
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            DataSet dataset = group5.createDataSet(
                "number of pixels in x axis", PredType::NATIVE_INT, dataspace);
            dataset.write(&(masterFileAttributes.nPixelsX),
                          PredType::NATIVE_INT);
        }

        // y
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            DataSet dataset = group5.createDataSet(
                "number of pixels in y axis", PredType::NATIVE_INT, dataspace);
            dataset.write(&(masterFileAttributes.nPixelsY),
                          PredType::NATIVE_INT);
        }

        // Maximum frames per file
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            DataSet dataset = group5.createDataSet(
                "maximum frames per file", PredType::NATIVE_INT, dataspace);
            dataset.write(&(masterFileAttributes.maxFramesPerFile),
                          PredType::NATIVE_INT);
        }

        // Total Frames
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            DataSet dataset = group5.createDataSet(
                "total frames", PredType::STD_U64LE, dataspace);
            dataset.write(&(masterFileAttributes.totalFrames),
                          PredType::STD_U64LE);
        }

        // Exptime
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            DataSet dataset = group5.createDataSet(
                "exposure time", PredType::STD_U64LE, dataspace);
            dataset.write(&(masterFileAttributes.exptimeNs),
                          PredType::STD_U64LE);
            DataSpace dataspaceAttr = DataSpace(H5S_SCALAR);
            StrType strdatatype(PredType::C_S1, 256);
            Attribute attribute =
                dataset.createAttribute("unit", strdatatype, dataspaceAttr);
            attribute.write(strdatatype, std::string("ns"));
        }

        // SubExptime
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            DataSet dataset = group5.createDataSet(
                "sub exposure time", PredType::STD_U64LE, dataspace);
            dataset.write(&(masterFileAttributes.subExptimeNs),
                          PredType::STD_U64LE);
            DataSpace dataspaceAttr = DataSpace(H5S_SCALAR);
            StrType strdatatype(PredType::C_S1, 256);
            Attribute attribute =
                dataset.createAttribute("unit", strdatatype, dataspaceAttr);
            attribute.write(strdatatype, std::string("ns"));
        }

        // SubPeriod
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            DataSet dataset = group5.createDataSet(
                "sub period", PredType::STD_U64LE, dataspace);
            dataset.write(&(masterFileAttributes.subPeriodNs),
                          PredType::STD_U64LE);
            DataSpace dataspaceAttr = DataSpace(H5S_SCALAR);
            StrType strdatatype(PredType::C_S1, 256);
            Attribute attribute =
                dataset.createAttribute("unit", strdatatype, dataspaceAttr);
            attribute.write(strdatatype, std::string("ns"));
        }

        // Period
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            DataSet dataset = group5.createDataSet(
                "acquisition period", PredType::STD_U64LE, dataspace);
            dataset.write(&(masterFileAttributes.periodNs),
                          PredType::STD_U64LE);
            DataSpace dataspaceAttr = DataSpace(H5S_SCALAR);
            StrType strdatatype(PredType::C_S1, 256);
            Attribute attribute =
                dataset.createAttribute("unit", strdatatype, dataspaceAttr);
            attribute.write(strdatatype, std::string("ns"));
        }

        // Quad Enable
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            DataSet dataset = group5.createDataSet(
                "quad enable", PredType::NATIVE_INT, dataspace);
            dataset.write(&(masterFileAttributes.quadEnable),
                          PredType::NATIVE_INT);
        }

        // Analog Flag
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            DataSet dataset = group5.createDataSet(
                "analog flag", PredType::NATIVE_INT, dataspace);
            dataset.write(&(masterFileAttributes.analogFlag),
                          PredType::NATIVE_INT);
        }

        // Digital Flag
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            DataSet dataset = group5.createDataSet(
                "digital flag", PredType::NATIVE_INT, dataspace);
            dataset.write(&(masterFileAttributes.digitalFlag),
                          PredType::NATIVE_INT);
        }

        // ADC Mask
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            DataSet dataset = group5.createDataSet(
                "adc mask", PredType::NATIVE_INT, dataspace);
            dataset.write(&(masterFileAttributes.adcmask),
                          PredType::NATIVE_INT);
        }

        // Dbit Offset
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            DataSet dataset = group5.createDataSet(
                "dbit offset", PredType::NATIVE_INT, dataspace);
            dataset.write(&(masterFileAttributes.dbitoffset),
                          PredType::NATIVE_INT);
        }

        // Dbit List
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            DataSet dataset = group5.createDataSet(
                "dbit bitset list", PredType::STD_U64LE, dataspace);
            dataset.write(&(masterFileAttributes.dbitlist),
                          PredType::STD_U64LE);
        }

        // Roi xmin
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            DataSet dataset = group5.createDataSet(
                "roi xmin", PredType::NATIVE_INT, dataspace);
            dataset.write(&(masterFileAttributes.roiXmin),
                          PredType::NATIVE_INT);
        }

        // Roi xmax
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            DataSet dataset = group5.createDataSet(
                "roi xmax", PredType::NATIVE_INT, dataspace);
            dataset.write(&(masterFileAttributes.roiXmax),
                          PredType::NATIVE_INT);
        }

        // Exptime1
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            DataSet dataset = group5.createDataSet(
                "exposure time1", PredType::STD_U64LE, dataspace);
            dataset.write(&(masterFileAttributes.exptime1Ns),
                          PredType::STD_U64LE);
            DataSpace dataspaceAttr = DataSpace(H5S_SCALAR);
            StrType strdatatype(PredType::C_S1, 256);
            Attribute attribute =
                dataset.createAttribute("unit", strdatatype, dataspaceAttr);
            attribute.write(strdatatype, std::string("ns"));
        }

        // Exptime2
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            DataSet dataset = group5.createDataSet(
                "exposure time2", PredType::STD_U64LE, dataspace);
            dataset.write(&(masterFileAttributes.exptime2Ns),
                          PredType::STD_U64LE);
            DataSpace dataspaceAttr = DataSpace(H5S_SCALAR);
            StrType strdatatype(PredType::C_S1, 256);
            Attribute attribute =
                dataset.createAttribute("unit", strdatatype, dataspaceAttr);
            attribute.write(strdatatype, std::string("ns"));
        }

        // Exptime3
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            DataSet dataset = group5.createDataSet(
                "exposure time3", PredType::STD_U64LE, dataspace);
            dataset.write(&(masterFileAttributes.exptime3Ns),
                          PredType::STD_U64LE);
            DataSpace dataspaceAttr = DataSpace(H5S_SCALAR);
            StrType strdatatype(PredType::C_S1, 256);
            Attribute attribute =
                dataset.createAttribute("unit", strdatatype, dataspaceAttr);
            attribute.write(strdatatype, std::string("ns"));
        }

        // GateDelay1
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            DataSet dataset = group5.createDataSet(
                "gate delay1", PredType::STD_U64LE, dataspace);
            dataset.write(&(masterFileAttributes.gateDelay1Ns),
                          PredType::STD_U64LE);
            DataSpace dataspaceAttr = DataSpace(H5S_SCALAR);
            StrType strdatatype(PredType::C_S1, 256);
            Attribute attribute =
                dataset.createAttribute("unit", strdatatype, dataspaceAttr);
            attribute.write(strdatatype, std::string("ns"));
        }

        // GateDelay2
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            DataSet dataset = group5.createDataSet(
                "gate delay2", PredType::STD_U64LE, dataspace);
            dataset.write(&(masterFileAttributes.gateDelay2Ns),
                          PredType::STD_U64LE);
            DataSpace dataspaceAttr = DataSpace(H5S_SCALAR);
            StrType strdatatype(PredType::C_S1, 256);
            Attribute attribute =
                dataset.createAttribute("unit", strdatatype, dataspaceAttr);
            attribute.write(strdatatype, std::string("ns"));
        }

        // GateDelay3
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            DataSet dataset = group5.createDataSet(
                "gate delay3", PredType::STD_U64LE, dataspace);
            dataset.write(&(masterFileAttributes.gateDelay3Ns),
                          PredType::STD_U64LE);
            DataSpace dataspaceAttr = DataSpace(H5S_SCALAR);
            StrType strdatatype(PredType::C_S1, 256);
            Attribute attribute =
                dataset.createAttribute("unit", strdatatype, dataspaceAttr);
            attribute.write(strdatatype, std::string("ns"));
        }

        // Dbit Offset
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            DataSet dataset =
                group5.createDataSet("gates", PredType::NATIVE_INT, dataspace);
            dataset.write(&(masterFileAttributes.gates), PredType::NATIVE_INT);
        }

        // Timestamp
        {
            time_t t = time(nullptr);
            StrType strdatatype(PredType::C_S1, 256);
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            DataSet dataset =
                group5.createDataSet("timestamp", strdatatype, dataspace);
            dataset.write(std::string(ctime(&t)), strdatatype);
        }

        masterfd->close();

    } catch (const Exception &error) {
        error.printErrorStack();
        if (masterfd) {
            masterfd->close();
        }
        throw sls::RuntimeError("Could not create master HDF5 handles");
    }
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
        hid_t dcpl_para[parameterNames.size()];
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