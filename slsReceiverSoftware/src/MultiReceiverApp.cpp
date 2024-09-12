// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
/* Creates the slsMultiReceiver for running multiple receivers form a single
 * binary */
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

// gettid added in glibc 2.30
#if __GLIBC__ == 2 && __GLIBC_MINOR__ < 30
#include <sys/syscall.h>
#define gettid() syscall(SYS_gettid)
#endif

/** Define Colors to print data call back in different colors for different
 * recievers */
#define PRINT_IN_COLOR(c, f, ...)                                              \
    printf("\033[%dm" f RESET, 30 + c + 1, ##__VA_ARGS__)

sem_t semaphore;

/**
 * Control+C Interrupt Handler
 * to let all the processes know to exit properly
 */
void sigInterruptHandler(int p) { sem_post(&semaphore); }

/**
 * prints usage of this example program
 */
std::string getHelpMessage() {
    return std::string(
        "\n\nUsage:\n"
        "./slsMultiReceiver(detReceiver) [start_tcp_port (non-zero and 16 "
        "bit)] [num_receivers] [optional: 1 for call back (print frame header "
        "for debugging), 0 for none (default)]\n\n");
}

/**
 * Start Acquisition Call back (slsMultiReceiver writes data if file write
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

/**
 * Get Receiver Data Call back
 * Prints in different colors(for each receiver process) the different headers
 * for each image call back.
 */
void GetData(slsDetectorDefs::sls_receiver_header &header,
             slsDetectorDefs::dataCallbackHeader callbackHeader,
             char *dataPointer, size_t &imageSize, void *objectPointer) {

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

    // if data is modified, eg ROI and size is reduced
    imageSize = 26000;
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
    sem_init(&semaphore, 1, 0);

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

    cprintf(BLUE, "Parent Process Created [ Tid: %ld ]\n", (long)gettid());
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
    for (int i = 0; i < numReceivers; ++i) {

        /**	- fork process to create child process */
        pid_t pid = fork();

        /**	- if fork failed, raise SIGINT and properly destroy all child
         * processes */
        if (pid < 0) {
            cprintf(RED, "fork() failed. Killing all the receiver objects\n");
            raise(SIGINT);
        }

        /**	- if child process */
        else if (pid == 0) {
            cprintf(BLUE, "Child process %d [ Tid: %ld ]\n", i, (long)gettid());

            std::unique_ptr<sls::Receiver> receiver = nullptr;
            try {
                receiver = sls::make_unique<sls::Receiver>(startTCPPort + i);
            } catch (...) {
                LOG(sls::logINFOBLUE)
                    << "Exiting Child Process [ Tid: " << gettid() << " ]";
                throw;
            }
            /**	- register callbacks. remember to set file write enable to 0
             * (using the client) if we should not write files and you will
             * write data using the callbacks */
            if (withCallback) {

                /** - Call back for start acquisition */
                cprintf(BLUE, "Registering StartAcq()\n");
                receiver->registerCallBackStartAcquisition(StartAcq, nullptr);

                /** - Call back for acquisition finished */
                cprintf(BLUE, "Registering AcquisitionFinished()\n");
                receiver->registerCallBackAcquisitionFinished(
                    AcquisitionFinished, nullptr);

                /* 	- Call back for raw data */
                cprintf(BLUE, "Registering GetData() \n");
                receiver->registerCallBackRawDataReady(GetData, nullptr);
            }

            /**	- as long as no Ctrl+C */
            sem_wait(&semaphore);
            sem_destroy(&semaphore);
            cprintf(BLUE, "Exiting Child Process [ Tid: %ld ]\n",
                    (long)gettid());
            exit(EXIT_SUCCESS);
            break;
        }
    }

    /** - Parent process ignores SIGINT (exits only when all child process
     * exits) */
    sa.sa_flags = 0;          // no flags
    sa.sa_handler = SIG_IGN;  // handler function
    sigemptyset(&sa.sa_mask); // dont block additional signals during invocation
                              // of handler
    if (sigaction(SIGINT, &sa, nullptr) == -1) {
        cprintf(RED, "Could not set handler function for SIGINT\n");
    }

    /** - Print Ready and Instructions how to exit */
    std::cout << "Ready ... \n";
    cprintf(RESET, "\n[ Press \'Ctrl+c\' to exit ]\n");

    /** - Parent process waits for all child processes to exit */
    for (;;) {
        pid_t childPid = waitpid(-1, nullptr, 0);

        // no child closed
        if (childPid == -1) {
            if (errno == ECHILD) {
                cprintf(GREEN, "All Child Processes have been closed\n");
                break;
            } else {
                cprintf(RED, "Unexpected error from waitpid(): (%s)\n",
                        strerror(errno));
                break;
            }
        }

        // child closed
        cprintf(BLUE, "Exiting Child Process [ Tid: %ld ]\n",
                (long int)childPid);
    }

    std::cout << "Goodbye!\n";
    return 0;
}
