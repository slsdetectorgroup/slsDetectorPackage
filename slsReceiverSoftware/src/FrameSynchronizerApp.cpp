// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
/* Creates the slsFrameSynchronizer for running multiple receivers in different threads form a single binary that will spit out zmq streams without reconstructing image */
#include "sls/Receiver.h"
#include "sls/ToString.h"
#include "sls/container_utils.h"
#include "sls/logger.h"
#include "sls/sls_detector_defs.h"

#include <csignal> //SIGINT
#include <cstring>
#include <iostream>
#include <semaphore.h>
#include <sys/wait.h> //wait
#include <unistd.h>
#include <thread>

// gettid added in glibc 2.30
#if __GLIBC__ == 2 && __GLIBC_MINOR__ < 30
#include <sys/syscall.h>
#define gettid() syscall(SYS_gettid)
#endif

/** Define Colors to print data call back in different colors for different
 * recievers */
#define PRINT_IN_COLOR(c, f, ...)                                              \
    printf("\033[%dm" f RESET, 30 + c + 1, ##__VA_ARGS__)

//std::vector<sem_t> semaphores;

/**
 * Control+C Interrupt Handler
 * to let all the processes know to exit properly
 */
void sigInterruptHandler(int p) { 
   // for (size_t i = 0; i != semaphores.size(); ++i)
    //    sem_post(&semaphores[i]); 
}

/**
 * prints usage of this example program
 */
std::string getHelpMessage() {
    return std::string(
        "\n\nUsage:\n"
        "./slsFrameSynchronizer(detReceiver) [start_tcp_port (non-zero and 16 "
        "bit)] [num_receivers] [optional: 1 for call back (print frame header "
        "for debugging), 0 for none (default)]\n\n");
}

/**
 * Start Acquisition Call back (slsFrameSynchronizer writes data if file write
 * enabled) if registerCallBackRawDataReady or
 * registerCallBackRawDataModifyReady registered, users get data
 */
int StartAcq(const slsDetectorDefs::startCallbackHeader callbackHeader,
             void *objectPointer) {
    LOG(sls::logINFOBLUE) << "#### Start Acquisition:"
                          << "\n\t["
                          << "\n\tUDP Port : "
                          << sls::ToString(callbackHeader.udpPort)
                          << "\n\tDynamic Range : "
                          << callbackHeader.dynamicRange
                          << "\n\tDetector Shape : "
                          << sls::ToString(callbackHeader.detectorShape)
                          << "\n\tImage Size : " << callbackHeader.imageSize
                          << "\n\tFile Path : " << callbackHeader.filePath
                          << "\n\tFile Name : " << callbackHeader.fileName
                          << "\n\tFile Index : " << callbackHeader.fileIndex
                          << "\n\tQuad Enable : " << callbackHeader.quad
                          << "\n\t]";
    return 0;
}

/** Acquisition Finished Call back */
void AcquisitionFinished(
    const slsDetectorDefs::endCallbackHeader callbackHeader,
    void *objectPointer) {
    LOG(sls::logINFOBLUE) << "#### AcquisitionFinished:"
                          << "\n\t["
                          << "\n\tUDP Port : "
                          << sls::ToString(callbackHeader.udpPort)
                          << "\n\tComplete Frames : "
                          << sls::ToString(callbackHeader.completeFrames)
                          << "\n\tLast Frame Index : "
                          << sls::ToString(callbackHeader.lastFrameIndex)
                          << "\n\t]";
}

void printDataCallBackHeader(slsDetectorDefs::sls_receiver_header &header,
                             slsDetectorDefs::dataCallbackHeader callbackHeader,
                             char *dataPointer, size_t imageSize,
                             void *objectPointer) {
    slsDetectorDefs::sls_detector_header detectorHeader = header.detHeader;

    PRINT_IN_COLOR(
        (callbackHeader.udpPort % 10),
        "#### GetData: "
        "\n\tCallback Header: "
        "\n\t["
        "\n\tUDP Port: %u"
        "\n\tShape: [%u, %u]"
        "\n\tAcq Index : %lu"
        "\n\tFrame Index :%lu"
        "\n\tProgress : %.2f%%"
        "\n\tCompelte Image :%s"
        "\n\tFlip Rows :%s"
        "\n\tAdditional Json Header : %s"
        "\n\t]"
        "\n\ttReceiver Header: "
        "\n\t["
        "\n\tFrame Number : %lu"
        "\n\tExposure Length :%u"
        "\n\tPackets Caught :%u"
        "\n\tDetector Specific 1: %lu"
        "\n\tTimestamp : %lu"
        "\n\tModule Id :%u"
        "\n\tRow : %u"
        "\n\tColumn :%u"
        "\n\tDetector Specific 2 : %u"
        "\n\tDetector Specific 3 : %u"
        "\n\tDetector Specific 4 : %u"
        "\n\tDetector Type : %s"
        "\n\tVersion: %u"
        "\n\t]"
        "\n\tFirst Byte Data: 0x%x"
        "\n\tImage Size: %zu\n\n",
        callbackHeader.udpPort, callbackHeader.shape.x, callbackHeader.shape.y,
        callbackHeader.acqIndex, callbackHeader.frameIndex,
        callbackHeader.progress,
        sls::ToString(callbackHeader.completeImage).c_str(),
        sls::ToString(callbackHeader.flipRows).c_str(),
        sls::ToString(callbackHeader.addJsonHeader).c_str(),
        detectorHeader.frameNumber, detectorHeader.expLength,
        detectorHeader.packetNumber, detectorHeader.detSpec1,
        detectorHeader.timestamp, detectorHeader.modId, detectorHeader.row,
        detectorHeader.column, detectorHeader.detSpec2, detectorHeader.detSpec3,
        detectorHeader.detSpec4, sls::ToString(detectorHeader.detType).c_str(),
        detectorHeader.version,
        // header->packetsMask.to_string().c_str(),
        ((uint8_t)(*((uint8_t *)(dataPointer)))), imageSize);
}

/**
 * Get Receiver Data Call back
 * Prints in different colors(for each receiver process) the different headers
 * for each image call back.
 */
void GetData(slsDetectorDefs::sls_receiver_header &header,
             slsDetectorDefs::dataCallbackHeader callbackHeader,
             char *dataPointer, size_t imageSize, void *objectPointer) {
    printDataCallBackHeader(header, callbackHeader, dataPointer, imageSize,
                            objectPointer);
}

/**
 * Get Receiver Data Call back (modified)
 * Prints in different colors(for each receiver process) the different headers
 * for each image call back.
 * @param modifiedImageSize new data size in bytes after the callback.
 * This will be the size written/streamed. (only smaller value is allowed).
 */
void GetData(slsDetectorDefs::sls_receiver_header &header,
             slsDetectorDefs::dataCallbackHeader callbackHeader,
             char *dataPointer, size_t &modifiedImageSize,
             void *objectPointer) {
    printDataCallBackHeader(header, callbackHeader, dataPointer,
                            modifiedImageSize, objectPointer);

    // if data is modified, eg ROI and size is reduced
    modifiedImageSize = 26000;
}

/**
 * Example of main program using the Receiver class
 *
 * - Defines in file for:
 *  	- Default Number of receivers is 1
 *  	- Default Start TCP port is 1954
 */
int main(int argc, char *argv[]) {

    /**	- set default values */
    int numReceivers = 1;
    uint16_t startTCPPort = 1954;
    int withCallback = 0;

    /**	- get number of receivers and start tcp port from command line
     * arguments */
    try {
        if (argc == 3 || argc == 4) {
            startTCPPort = sls::StringTo<uint16_t>(argv[1]);
            if (startTCPPort == 0) {
                throw;
            }
            numReceivers = std::stoi(argv[2]);
            if (argc == 4) {
                withCallback = std::stoi(argv[3]);
            }
        } else
            throw;
    } catch (...) {
        throw std::runtime_error(getHelpMessage());
    }

    cprintf(RESET, "Number of Receivers: %d\n", numReceivers);
    cprintf(RESET, "Start TCP Port: %hu\n", startTCPPort);
    cprintf(RESET, "Callback Enable: %d\n", withCallback);

    /** - Catch signal SIGINT to close files and call destructors properly */
    struct sigaction sa;
    sa.sa_flags = 0;                     // no flags
    sa.sa_handler = sigInterruptHandler; // handler function
    sigemptyset(&sa.sa_mask); // dont block additional signals during invocation
                              // of handler
    if (sigaction(SIGINT, &sa, nullptr) == -1) {
        cprintf(RED, "Could not set handler function for SIGINT\n");
    }

    /** - Ignore SIG_PIPE, prevents global signal handler, handle locally,
       instead of a server crashing due to client crash when writing, it just
       gives error */
    struct sigaction asa;
    asa.sa_flags = 0;          // no flags
    asa.sa_handler = SIG_IGN;  // handler function
    sigemptyset(&asa.sa_mask); // dont block additional signals during
                               // invocation of handler
    if (sigaction(SIGPIPE, &asa, nullptr) == -1) {
        cprintf(RED, "Could not set handler function for SIGPIPE\n");
    }

    /** - loop over number of receivers */
    std::vector<std::thread> threads;

    for (int i = 0; i != numReceivers; ++i) {


        threads.emplace_back([i, startTCPPort, withCallback, numReceivers]() {
            //sem_init(&semaphores[i], 1, 0);
            sls::Receiver receiver(startTCPPort + i);

            /**	- register callbacks. remember to set file write enable to 0
        (using the client) if we should not write files and you will write data
        using the callbacks */
            if (withCallback) {

                /** - Call back for start acquisition */
                cprintf(BLUE, "Registering 	StartAcq()\n");
                receiver.registerCallBackStartAcquisition(StartAcq, nullptr);

                /** - Call back for acquisition finished */
                cprintf(BLUE, "Registering 	AcquisitionFinished()\n");
                receiver.registerCallBackAcquisitionFinished(
                    AcquisitionFinished, nullptr);

                /* 	- Call back for raw data */
                cprintf(BLUE, "Registering     GetData() \n");
                if (withCallback == 1)
                    receiver.registerCallBackRawDataReady(GetData, nullptr);
                else if (withCallback == 2)
                    receiver.registerCallBackRawDataModifyReady(GetData,
                                                                    nullptr);
            }


            /** - Print Ready and Instructions how to exit */
            if (i == (numReceivers - 1)) {
                std::cout << "Ready ... \n";
                cprintf(RESET, "\n[ Press \'Ctrl+c\' to exit ]\n");
            }

            cprintf(RED, "Receiver %d\n", i);
            /**	- as long as no Ctrl+C */
            //sem_wait(&semaphores[i]);
            //sem_destroy(&semaphores[i]);
            cprintf(RED, "Receiver %d done\n", i);

        });
    }
   

    std::cout << "Goodbye!\n";
    return 0;
}
