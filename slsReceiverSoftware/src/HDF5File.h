#pragma once
/************************************************
 * @file HDF5File.h
 * @short sets/gets properties for the HDF5 file,
 * creates/closes the file and writes data to it
 ***********************************************/
/**
 *@short sets/gets properties for the HDF5 file, creates/closes the file and
 *writes data to it
 */

#include "File.h"

#include "H5Cpp.h"
#ifndef H5_NO_NAMESPACE
using namespace H5;
#endif
#include <mutex>

class HDF5File : private virtual slsDetectorDefs, public File {

  public:
    /**
     * Constructor
     * creates the File Writer
     * @param ind self index
     * @param maxf pointer to max frames per file
     * @param nd pointer to number of detectors in each dimension
     * @param fname pointer to file name prefix
     * @param fpath pointer to file path
     * @param findex pointer to file index
     * @param owenable pointer to over write enable
     * @param dindex pointer to detector index
     * @param nunits pointer to number of theads/ units per detector
     * @param nf pointer to number of images in acquisition
     * @param dr pointer to dynamic range
     * @param portno pointer to udp port number for logging
     * @param nx number of pixels in x direction
     * @param ny number of pixels in y direction
     * @param smode pointer to silent mode
     */
    HDF5File(int ind, uint32_t *maxf, int *nd, std::string *fname,
             std::string *fpath, uint64_t *findex, bool *owenable, int *dindex,
             int *nunits, uint64_t *nf, uint32_t *dr, uint32_t *portno,
             uint32_t nx, uint32_t ny, bool *smode);
    ~HDF5File();
    void SetNumberofPixels(uint32_t nx, uint32_t ny);
    void CreateFile();
    void CloseCurrentFile();
    void CloseAllFiles();
    void WriteToFile(char *buffer, int bufferSize, uint64_t currentFrameNumber,
                     uint32_t numPacketsCaught);
    void CreateMasterFile(bool masterFileWriteEnable,
                          masterAttributes &masterFileAttributes);
    void EndofAcquisition(bool anyPacketsCaught, uint64_t numImagesCaught);

  private:
    void CloseFile(H5File *&fd, bool masterFile);
    void WriteDataFile(uint64_t currentFrameNumber, char *buffer);
    void WriteParameterDatasets(uint64_t currentFrameNumber,
                                sls_receiver_header *rheader);
    void ExtendDataset();
    void CreateDataFile();
    void CreateMasterDataFile(masterAttributes &masterFileAttributes);
    void CreateVirtualDataFile(uint32_t maxFramesPerFile, uint64_t numf);
    void LinkVirtualInMaster(std::string fname, std::string dsetname);
    hid_t GetDataTypeinC(DataType dtype);

    static std::mutex hdf5Lib;

    H5File *masterfd;
    /** Virtual File handle ( only file name because
    code in C as H5Pset_virtual doesnt exist yet in C++) */
    hid_t virtualfd;
    H5File *filefd;
    DataSpace *dataspace;
    DataSet *dataset;
    DataType datatype;

    uint32_t nPixelsX;
    uint32_t nPixelsY;
    uint32_t numFramesInFile;
    uint64_t numActualPacketsInFile;
    int numFilesinAcquisition;

    std::vector<std::string> parameterNames;
    std::vector<DataType> parameterDataTypes;
    DataSpace *dataspace_para;
    std::vector<DataSet *> dataset_para;

    uint64_t extNumImages;
};
