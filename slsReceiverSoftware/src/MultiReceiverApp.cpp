// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
/* Creates the slsMultiReceiver for running multiple receivers form a single
 * binary */
#include "sls/Receiver.h"
#include "sls/ToString.h"
#include "sls/container_utils.h"
#include "sls/logger.h"
#include "sls/sls_detector_defs.h"
#include "sls/versionAPI.h"

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
    std::ostringstream os;
    os << "\nUsage:\n\n"
       << "./slsMultiReceiver --version or -v\n"
       << "\t - Gets the slsMultiReceiver version\n\n"
       << "./slsMultiReceiver [start tcp port] [num recevers] [call back "
          "option (optional)]\n"
       << "\t - tcp port has to be non-zero and 16 bit\n"
       << "\t - call back option is 0 (disabled) by default, 1 prints frame "
          "header for debugging\n";
    return os.str();
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
                          << "\n\tAdditional Json Header : "
                          << sls::ToString(callbackHeader.addJsonHeader)
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

    // // example of how to use roi or modify data that is later written to file
    // slsDetectorDefs::ROI roi{0, 10, 0, 20};
    // int width = roi.xmax - roi.xmin;
    // int height = roi.ymax - roi.ymin;
    // uint8_t *destPtr = (uint8_t *)dataPointer;
    // for (int irow = roi.ymin; irow < roi.ymax; ++irow) {
    //     memcpy(destPtr,
    //            ((uint8_t *)(dataPointer + irow * callbackHeader.shape.x +
    //                         roi.xmin)),
    //            width);
    //     destPtr += width;
    // }
    // memcpy((uint8_t*)dataPointer, (uint8_t*)dataPointer
    // // setting roi for eg. changes size
    // imageSize = width * height;
}

/**
 * Example of main program using the Receiver class
 *
 * - Defines in file for:
 *  	- Default Number of receivers is 1
 *  	- Default Start TCP port is 1954
 */
int main(int argc, char *argv[]) {

    // version
    if (argc == 2) {
        std::string sargv1 = std::string(argv[1]);
        if (sargv1 == "--version" || sargv1 == "-v") {
            std::cout << "slsMultiReceiver Version: " << APIRECEIVER
                      << std::endl;
            exit(EXIT_SUCCESS);
        }
    }

    /**	- set default values */
    int numReceivers = 1;
    uint16_t startTCPPort = DEFAULT_TCP_RX_PORTNO;
    int withCallback = 0;
    sem_init(&semaphore, 1, 0);

    /**	- get number of receivers and start tcp port from command line
     * arguments */
    if (argc > 1) {
        try {
            if (argc == 3 || argc == 4) {
                startTCPPort = sls::StringTo<uint16_t>(argv[1]);
                if (startTCPPort == 0) {
                    throw std::runtime_error("Invalid start tcp port");
                }
                numReceivers = std::stoi(argv[2]);
                if (numReceivers > 1024) {
                    cprintf(RED,
                            "Did you mix up the order of the arguments?\n%s\n",
                            getHelpMessage().c_str());
                    return EXIT_FAILURE;
                }
                if (numReceivers == 0) {
                    cprintf(RED, "Invalid number of receivers.\n%s\n",
                            getHelpMessage().c_str());
                    return EXIT_FAILURE;
                }
                if (argc == 4) {
                    withCallback = std::stoi(argv[3]);
                }
            } else
                throw std::runtime_error("Invalid number of arguments");
        } catch (const std::exception &e) {
            cprintf(RED, "Error: %s\n%s\n", e.what(), getHelpMessage().c_str());
            return EXIT_FAILURE;
        }
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

            try {
                uint16_t port = startTCPPort + i;
                sls::Receiver receiver(port);

                /**	- register callbacks. remember to set file write enable
                 * to 0 (using the client) if we should not write files and you
                 * will write data using the callbacks */
                if (withCallback) {

                    /** - Call back for start acquisition */
                    cprintf(BLUE, "Registering StartAcq()\n");
                    receiver.registerCallBackStartAcquisition(StartAcq,
                                                              nullptr);

                    /** - Call back for acquisition finished */
                    cprintf(BLUE, "Registering AcquisitionFinished()\n");
                    receiver.registerCallBackAcquisitionFinished(
                        AcquisitionFinished, nullptr);

                    /* 	- Call back for raw data */
                    cprintf(BLUE, "Registering GetData() \n");
                    receiver.registerCallBackRawDataReady(GetData, nullptr);
                }

                /**	- as long as no Ctrl+C */
                sem_wait(&semaphore);
                sem_destroy(&semaphore);

            } catch (...) {
                LOG(sls::logINFOBLUE)
                    << "Exiting Child Process [ Tid: " << gettid() << " ]";
                throw;
            }

            cprintf(BLUE, "Exiting Child Process [ Tid: %ld ]\n",
                    (long)gettid());
            exit(EXIT_SUCCESS);
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
