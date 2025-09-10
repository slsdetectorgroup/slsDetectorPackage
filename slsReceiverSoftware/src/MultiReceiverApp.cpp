// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
/* Creates the slsMultiReceiver for running multiple receivers form a single
 * binary */
#include "CommandLineOptions.h"
#include "sls/Receiver.h"
#include "sls/ToString.h"
#include "sls/container_utils.h"
#include "sls/logger.h"
#include "sls/network_utils.h"
#include "sls/sls_detector_defs.h"

#include <csignal> //SIGINT
#include <cstring>
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

/**
 * Start Acquisition Call back (slsMultiReceiver writes data if file write
 * enabled) if registerCallBackRawDataReady or
 * registerCallBackRawDataModifyReady registered, users get data
 */
void StartAcq(const slsDetectorDefs::startCallbackHeader callbackHeader,
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

    // if data is modified, can affect size
    // only reduction in size allowed, not increase
    // imageSize = 26000;
}

sem_t semaphore;

/**
 * Control+C Interrupt Handler
 * to let all the processes know to exit properly
 * All child processes will call the handler (parent process set to ignore)
 */
void sigInterruptHandler(int signal) {
    (void)signal; // suppress unused warning if needed
    sem_post(&semaphore);
}

int main(int argc, char *argv[]) {

    CommandLineOptions cli(AppType::MultiReceiver);
    ParsedOptions opts;
    try {
        opts = cli.parse(argc, argv);
    } catch (sls::RuntimeError &e) {
        return EXIT_FAILURE;
    }
    auto &m = std::get<MultiReceiverOptions>(opts);
    if (m.versionRequested || m.helpRequested) {
        return EXIT_SUCCESS;
    }

    LOG(sls::logINFOBLUE) << "Current Process [ Tid: " << gettid() << ']';

    // close files on ctrl+c
    sls::setupSignalHandler(SIGINT, sigInterruptHandler);
    // handle locally on socket crash
    sls::setupSignalHandler(SIGPIPE, SIG_IGN);

    sem_init(&semaphore, 1, 0);

    /** - loop over receivers */
    for (int i = 0; i < m.numReceivers; ++i) {

        /**	- fork process to create child process */
        pid_t pid = fork();

        /**	- if fork failed, raise SIGINT and properly destroy all child
         * processes */
        if (pid < 0) {
            LOG(sls::logERROR)
                << "fork() failed. Killing all the receiver objects";
            raise(SIGINT);
        }

        /**	- if child process */
        else if (pid == 0) {
            LOG(sls::logINFOBLUE)
                << "Child process " << i << " [ Tid: " << gettid() << ']';

            try {
                uint16_t port = m.port + i;
                sls::Receiver receiver(port);

                /**	- register callbacks. remember to set file write enable
                 * to 0 (using the client) if we should not write files and you
                 * will write data using the callbacks */
                if (m.callbackEnabled) {

                    /** - Call back for start acquisition */
                    LOG(sls::logINFOBLUE) << "Registering StartAcq()";
                    receiver.registerCallBackStartAcquisition(StartAcq,
                                                              nullptr);

                    /** - Call back for acquisition finished */
                    LOG(sls::logINFOBLUE)
                        << "Registering AcquisitionFinished()";
                    receiver.registerCallBackAcquisitionFinished(
                        AcquisitionFinished, nullptr);

                    /* 	- Call back for raw data */
                    LOG(sls::logINFOBLUE) << "Registering GetData()";
                    receiver.registerCallBackRawDataReady(GetData, nullptr);
                }

                /**	- as long as no Ctrl+C */
                // each child process gets a copy of the semaphore
                sem_wait(&semaphore);
                sem_destroy(&semaphore);
                LOG(sls::logINFOBLUE)
                    << "Exiting Child Process [ Tid: " << gettid() << ']';
                exit(EXIT_SUCCESS);
            } catch (...) {
                sem_destroy(&semaphore);
                LOG(sls::logINFOBLUE)
                    << "Exiting Child Process [ Tid: " << gettid() << " ]";
                exit(EXIT_FAILURE);
            }
        }
    }

    /** - Parent process ignores SIGINT and waits for all the child processes to
     * handle the signal */
    sls::setupSignalHandler(SIGINT, SIG_IGN);

    /** - Print Ready and Instructions how to exit */
    std::cout << "Ready ... \n";
    LOG(sls::logINFO) << "\n[ Press \'Ctrl+c\' to exit ]";

    /** - Parent process waits for all child processes to exit */
    for (;;) {
        int status;
        pid_t childPid = waitpid(-1, &status, 0);

        // no child closed
        if (childPid == -1) {
            if (errno == ECHILD) {
                LOG(sls::logINFOGREEN)
                    << "All Child Processes have been closed";
                break;
            } else {
                LOG(sls::logERROR)
                    << "Unexpected error from waitpid(): " << strerror(errno);
                break;
            }
        }

        if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
            std::cerr << "Child " << childPid << " failed\n";
            kill(0, SIGINT); // signal other children to exit
        }
    }

    std::cout << "Goodbye!\n";
    return EXIT_SUCCESS;
}
