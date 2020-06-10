/************************************************
 * @file BinaryFile.cpp
 * @short sets/gets properties for the binary file,
 * creates/closes the file and writes data to it
 ***********************************************/

#include "BinaryFile.h"
#include "Fifo.h"
#include "receiver_defs.h"

#include <iomanip>
#include <iostream>
#include <string.h>

FILE *BinaryFile::masterfd = nullptr;

BinaryFile::BinaryFile(int ind, uint32_t *maxf, int *nd, std::string *fname,
                       std::string *fpath, uint64_t *findex, bool *owenable,
                       int *dindex, int *nunits, uint64_t *nf, uint32_t *dr,
                       uint32_t *portno, bool *smode)
    : File(ind, BINARY, maxf, nd, fname, fpath, findex, owenable, dindex,
           nunits, nf, dr, portno, smode),
      filefd(nullptr), numFramesInFile(0), numActualPacketsInFile(0),
      maxMasterFileSize(2000) {
#ifdef VERBOSE
    PrintMembers();
#endif
}

BinaryFile::~BinaryFile() { CloseAllFiles(); }

void BinaryFile::PrintMembers(TLogLevel level) {
    File::PrintMembers(level);
    LOG(logINFO) << "Max Frames Per File: " << *maxFramesPerFile;
    LOG(logINFO) << "Number of Frames in File: " << numFramesInFile;
}

void BinaryFile::CreateFile() {
    numFramesInFile = 0;
    numActualPacketsInFile = 0;

    std::ostringstream os;
    os << *filePath << "/" << *fileNamePrefix << "_d"
       << (*detIndex * (*numUnitsPerDetector) + index) << "_f" << subFileIndex
       << '_' << *fileIndex << ".raw";
    currentFileName = os.str();

    if (!(*overWriteEnable)) {
        if (nullptr ==
            (filefd = fopen((const char *)currentFileName.c_str(), "wx"))) {
            filefd = nullptr;
            throw sls::RuntimeError("Could not create/overwrite file " +
                                    currentFileName);
        }
    } else if (nullptr ==
               (filefd = fopen((const char *)currentFileName.c_str(), "w"))) {
        filefd = nullptr;
        throw sls::RuntimeError("Could not create file " + currentFileName);
    }
    // setting to no file buffering
    setvbuf(filefd, nullptr, _IONBF, 0);

    if (!(*silentMode)) {
        LOG(logINFO) << "[" << *udpPortNumber
                     << "]: Binary File created: " << currentFileName;
    }
}

void BinaryFile::CloseCurrentFile() {
    if (filefd)
        fclose(filefd);
    filefd = nullptr;
}

void BinaryFile::CloseAllFiles() {
    CloseCurrentFile();
    if (master) {
        if (masterfd)
            fclose(masterfd);
        masterfd = nullptr;
    }
}

int BinaryFile::WriteData(char *buf, int bsize) {
    if (!filefd)
        return 0;
    return fwrite(buf, 1, bsize, filefd);
}

void BinaryFile::WriteToFile(char *buffer, int buffersize,
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

    // write to file
    int ret = 0;

    // contiguous bitset
    if (sizeof(sls_bitset) == sizeof(bitset_storage)) {
        ret = WriteData(buffer, buffersize);
    }

    // not contiguous bitset
    else {
        // write detector header
        ret = WriteData(buffer, sizeof(sls_detector_header));

        // get contiguous representation of bit mask
        bitset_storage storage;
        memset(storage, 0, sizeof(bitset_storage));
        sls_bitset bits = *(sls_bitset *)(buffer + sizeof(sls_detector_header));
        for (int i = 0; i < MAX_NUM_PACKETS; ++i)
            storage[i >> 3] |= (bits[i] << (i & 7));
        // write bitmask
        ret += WriteData((char *)storage, sizeof(bitset_storage));

        // write data
        ret += WriteData(buffer + sizeof(sls_detector_header),
                         buffersize - sizeof(sls_receiver_header));
    }

    // if write error
    if (ret != buffersize) {
        throw sls::RuntimeError(std::to_string(index) +
                                " : Write to file failed for image number " +
                                std::to_string(currentFrameNumber));
    }
}

void BinaryFile::CreateMasterFile(bool masterFileWriteEnable,
                                  masterAttributes &masterFileAttributes) {
    // beginning of every acquisition
    numFramesInFile = 0;
    numActualPacketsInFile = 0;

    if (masterFileWriteEnable && master) {

        std::ostringstream os;
        os << *filePath << "/" << *fileNamePrefix << "_master"
           << "_" << *fileIndex << ".raw";
        masterFileName = os.str();
        if (!(*silentMode)) {
            LOG(logINFO) << "Master File: " << masterFileName;
        }
        masterFileAttributes.version = BINARY_WRITER_VERSION;

        // create master file
        if (!(*overWriteEnable)) {
            if (nullptr == (masterfd = fopen(
                                (const char *)masterFileName.c_str(), "wx"))) {
                masterfd = nullptr;
                throw sls::RuntimeError("Could not create binary master file "
                                        "(without overwrite enable) " +
                                        masterFileName);
            }
        } else if (nullptr ==
                   (masterfd =
                        fopen((const char *)masterFileName.c_str(), "w"))) {
            masterfd = nullptr;
            throw sls::RuntimeError("Could not create binary master file "
                                    "(with overwrite enable) " +
                                    masterFileName);
        }
        // create master file data
        time_t t = time(nullptr);
        char message[maxMasterFileSize];
        sprintf(message,
                "Version                    : %.1f\n"
                "Detector Type              : %d\n"
                "Dynamic Range              : %d\n"
                "Ten Giga                   : %d\n"
                "Image Size                 : %d bytes\n"
                "nPixelsX                   : %d pixels\n"
                "nPixelsY                   : %d pixels\n"
                "Max Frames Per File        : %u\n"
                "Total Frames               : %lld\n"
                "Exptime (ns)               : %lld\n"
                "SubExptime (ns)            : %lld\n"
                "SubPeriod(ns)              : %lld\n"
                "Period (ns)                : %lld\n"
                "Quad Enable                : %d\n"
                "Analog Flag                : %d\n"
                "Digital Flag               : %d\n"
                "ADC Mask                   : %d\n"
                "Dbit Offset                : %d\n"
                "Dbit Bitset                : %lld\n"
                "Roi (xmin, xmax)           : %d %d\n"
                "Exptime1 (ns)              : %lld\n"
                "Exptime2 (ns)              : %lld\n"
                "Exptime3 (ns)              : %lld\n"
                "GateDelay1 (ns)            : %lld\n"
                "GateDelay2 (ns)            : %lld\n"
                "GateDelay3 (ns)            : %lld\n"
                "Gates                      : %d\n"
                "Timestamp                  : %s\n\n"

                "#Frame Header\n"
                "Frame Number               : 8 bytes\n"
                "SubFrame Number/ExpLength  : 4 bytes\n"
                "Packet Number              : 4 bytes\n"
                "Bunch ID                   : 8 bytes\n"
                "Timestamp                  : 8 bytes\n"
                "Module Id                  : 2 bytes\n"
                "Row                        : 2 bytes\n"
                "Column                     : 2 bytes\n"
                "Reserved                   : 2 bytes\n"
                "Debug                      : 4 bytes\n"
                "Round Robin Number         : 2 bytes\n"
                "Detector Type              : 1 byte\n"
                "Header Version             : 1 byte\n"
                "Packets Caught Mask        : 64 bytes\n",
                masterFileAttributes.version, masterFileAttributes.detectorType,
                masterFileAttributes.dynamicRange, masterFileAttributes.tenGiga,
                masterFileAttributes.imageSize, masterFileAttributes.nPixelsX,
                masterFileAttributes.nPixelsY,
                masterFileAttributes.maxFramesPerFile,
                (long long int)masterFileAttributes.totalFrames,
                (long long int)masterFileAttributes.exptimeNs,
                (long long int)masterFileAttributes.subExptimeNs,
                (long long int)masterFileAttributes.subPeriodNs,
                (long long int)masterFileAttributes.periodNs,
                masterFileAttributes.quadEnable,
                masterFileAttributes.analogFlag,
                masterFileAttributes.digitalFlag, masterFileAttributes.adcmask,
                masterFileAttributes.dbitoffset,
                (long long int)masterFileAttributes.dbitlist,
                masterFileAttributes.roiXmin, masterFileAttributes.roiXmax,
                (long long int)masterFileAttributes.exptime1Ns,
                (long long int)masterFileAttributes.exptime2Ns,
                (long long int)masterFileAttributes.exptime3Ns,
                (long long int)masterFileAttributes.gateDelay1Ns,
                (long long int)masterFileAttributes.gateDelay2Ns,
                (long long int)masterFileAttributes.gateDelay3Ns,
                masterFileAttributes.gates, ctime(&t));
        if (strlen(message) > maxMasterFileSize) {
            throw sls::RuntimeError("Master File Size " +
                                    std::to_string(strlen(message)) +
                                    " is greater than max str size " +
                                    std::to_string(maxMasterFileSize));
        }
        // write and close file
        if (fwrite((void *)message, 1, strlen(message), masterfd) !=
            strlen(message)) {
            throw sls::RuntimeError(
                "Master binary file incorrect number of bytes written to file");
        }
        if (masterfd)
            fclose(masterfd);
        masterfd = nullptr;
    }
}
