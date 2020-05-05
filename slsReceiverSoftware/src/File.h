#pragma once
/************************************************
 * @file File.h
 * @short sets/gets properties for the file,
 * creates/closes the file and writes data to it
 ***********************************************/
/**
 *@short sets/gets properties for the file, creates/closes the file and writes
 *data to it
 */

#include "logger.h"
#include "receiver_defs.h"
#include "sls_detector_defs.h"

#include <string>

class File : private virtual slsDetectorDefs {

  public:
    /**
     * Constructor
     * creates the File Writer
     * @param ind self index
     * @param type file format type
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
     * @param smode pointer to silent mode
     */
    File(int ind, slsDetectorDefs::fileFormat type, uint32_t *maxf, int *nd,
         std::string *fname, std::string *fpath, uint64_t *findex,
         bool *owenable, int *dindex, int *nunits, uint64_t *nf, uint32_t *dr,
         uint32_t *portno, bool *smode);

    virtual ~File();
    fileFormat GetFileType();
    std::string GetCurrentFileName();
    void resetSubFileIndex();
    virtual void PrintMembers(TLogLevel level = logDEBUG1);

    /**
     * Get Member Pointer Values before the object is destroyed
     * @param nd pointer to number of detectors in each dimension
     * @param maxf pointer to max frames per file
     * @param fname pointer to file name prefix
     * @param fpath pointer to file path
     * @param findex pointer to file index
     * @param owenable pointer to over write enable
     * @param dindex pointer to detector index
     * @param nunits pointer to number of theads/ units per detector
     * @param nf pointer to number of images in acquisition
     * @param dr pointer to dynamic range
     * @param portno pointer to dynamic range
     */
    void GetMemberPointerValues(int *nd, uint32_t *&maxf, std::string *&fname,
                                std::string *&fpath, uint64_t *&findex,
                                bool *&owenable, int *&dindex, int *&nunits,
                                uint64_t *&nf, uint32_t *&dr,
                                uint32_t *&portno);

    virtual void CreateFile() = 0;
    virtual void CloseCurrentFile() = 0;
    virtual void CloseAllFiles() = 0;

    /**
     * Write data to file
     * @param buffer buffer to write from
     * @param fnum current image number
     * @param nump number of packets caught
     */
    virtual void WriteToFile(char *buffer, int buffersize, uint64_t fnum,
                             uint32_t nump) = 0;

    /**
     * Create master file
     * @param mfwenable master file write enable
     * @param attr master file attributes
     */
    virtual void CreateMasterFile(bool mfwenable, masterAttributes &attr) = 0;

    // HDf5 specific
    /**
     * Set Number of pixels
     * @param nx number of pixels in x direction
     * @param ny number of pixels in y direction
     */
    virtual void SetNumberofPixels(uint32_t nx, uint32_t ny) {
        LOG(logERROR) << "This is a generic function SetNumberofPixels that "
                         "should be overloaded by a derived class";
    }

    /**
     * End of Acquisition
     * @param anyPacketsCaught true if any packets are caught, else false
     * @param numf number of images caught
     */
    virtual void EndofAcquisition(bool anyPacketsCaught, uint64_t numf) {
        LOG(logERROR) << "This is a generic function EndofAcquisition that "
                         "should be overloaded by a derived class";
    }

  protected:
    bool master;
    int index;
    slsDetectorDefs::fileFormat formatType;
    uint32_t *maxFramesPerFile;
    std::string masterFileName;
    std::string currentFileName;
    int numDetX;
    int numDetY;
    std::string *fileNamePrefix;
    std::string *filePath;
    uint64_t *fileIndex;
    uint64_t subFileIndex{0};
    bool *overWriteEnable;
    int *detIndex;
    int *numUnitsPerDetector;
    uint64_t *numImages;
    uint32_t *dynamicRange;
    uint32_t *udpPortNumber;
    bool *silentMode;
};
