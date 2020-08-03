#pragma once
/************************************************
 * @file BinaryFile.h
 * @short sets/gets properties for the binary file,
 * creates/closes the file and writes data to it
 ***********************************************/
/**
 *@short sets/gets properties for the binary file, creates/closes the file and
 *writes data to it
 */

#include "File.h"

#include <string>

class BinaryFile : private virtual slsDetectorDefs, public File {

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
     * @param smode pointer to silent mode
     */
    BinaryFile(int ind, uint32_t *maxf, int *nd, std::string *fname,
               std::string *fpath, uint64_t *findex, bool *owenable,
               int *dindex, int *nunits, uint64_t *nf, uint32_t *dr,
               uint32_t *portno, bool *smode);
    ~BinaryFile();

    void PrintMembers(TLogLevel level = logDEBUG1) override;
    void CreateFile() override;
    void CreateMasterFile(bool masterFileWriteEnable,
                          MasterAttributes *attr) override;
    void CloseCurrentFile() override;
    void CloseAllFiles() override;
    void WriteToFile(char *buffer, int buffersize, uint64_t currentFrameNumber,
                     uint32_t numPacketsCaught) override;

  private:
    int WriteData(char *buf, int bsize);

    FILE *filefd = nullptr;
    static FILE *masterfd;
    uint32_t numFramesInFile = 0;
    uint64_t numActualPacketsInFile = 0;
};