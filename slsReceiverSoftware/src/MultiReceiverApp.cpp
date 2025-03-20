// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
/* Creates the slsMultiReceiver for running multiple receivers form a single
 * binary */
#include "sls/Receiver.h"
#include "sls/ToString.h"
#include "sls/container_utils.h"
#include "sls/logger.h"
#include "sls/sls_detector_defs.h"
#include "sls/sls_detector_exceptions.h"
#include "sls/versionAPI.h"

#include <csignal> //SIGINT
#include <cstring>
#include <getopt.h>
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

    // if data is modified, eg ROI and size is reduced
    imageSize = 26000;
}

/**
 * Control+C Interrupt Handler
 * to let all the processes know to exit properly
 */
void sigInterruptHandler(int p) { sem_post(&semaphore); }

void GetDeprecatedCommandLineOptions(int argc, char *argv[],
                                     uint16_t &startPort,
                                     uint16_t &numReceivers,
                                     bool &callbackEnabled) {
    std::string deprecatedMessage =
        "Detected deprecated Options. Please update.\n";
    if (argc > 1) {
        try {
            if (argc == 3 || argc == 4) {
                startPort = sls::StringTo<uint16_t>(argv[1]);
                numReceivers = sls::StringTo<uint16_t>(argv[2]);
                if (numReceivers > 1024) {
                    LOG(sls::logWARNING) << deprecatedMessage;
                    LOG(sls::logERROR)
                        << "Did you mix up the order of the arguments?";
                    exit(EXIT_FAILURE);
                }
                if (numReceivers == 0) {
                    LOG(sls::logWARNING) << deprecatedMessage;
                    LOG(sls::logERROR) << "Invalid number of receivers.";
                    exit(EXIT_FAILURE);
                }
                if (argc == 4) {
                    callbackEnabled = sls::StringTo<bool>(argv[3]);
                }
            } else
                throw std::runtime_error("Invalid number of arguments");
        } catch (const std::exception &e) {
            LOG(sls::logWARNING) << deprecatedMessage;
            LOG(sls::logERROR) << e.what();
            exit(EXIT_FAILURE);
        }
    }
}

std::string getHelpMessage() {
    std::string name = "slsMultiReceiver";
    return "\nUsage: " + name + " Options:\n" +
           "\t-v, --version       : Version of " + name + ".\n" +
           "\t-n, --num-receivers : Number of receivers.\n" +
           "\t-p, --port          : TCP port to communicate with client for "
           "configuration. Non-zero and 16 bit.\n" +
           "\t-c, --callback      : Enable dummy callbacks for debugging. "
           "Disabled by default. \n" +
           "\t-u, --uid           : Set effective user id if receiver started "
           "with privileges. \n\n";
}

/**
 * Example of main program using the Receiver class
 *
 * - Defines in file for:
 *  	- Default Number of receivers is 1
 *  	- Default Start TCP port is 1954
 */
int main(int argc, char *argv[]) {
    uint16_t startPort = DEFAULT_TCP_RX_PORTNO;
    uint16_t numReceivers = 1;
    bool callbackEnabled = false;
    uid_t userid = -1;

    std::string help_message = getHelpMessage();

    static struct option long_options[] = {
        {"version", no_argument, nullptr, 'v'},
        {"num-receivers", required_argument, nullptr, 'n'},
        {"rx_tcpport", required_argument, nullptr, 't'},
        {"port", required_argument, nullptr, 'p'},
        {"callback", no_argument, nullptr, 'c'},
        {"uid", required_argument, nullptr, 'u'},
        {"help", no_argument, nullptr, 'h'},
        {nullptr, 0, nullptr, 0}};

    int option_index = 0;
    int opt = 0;
    while (-1 != (opt = getopt_long(argc, argv, "vn:t:p:cu:h", long_options,
                                    &option_index))) {

        switch (opt) {

        case 'v':
            std::cout << argv[0] << " Version: " << APIRECEIVER << std::endl;
            exit(EXIT_SUCCESS);

        case 'n':
            try {
                numReceivers = sls::StringTo<uint16_t>(optarg);
                if (numReceivers == 0 || numReceivers > 100) {
                    throw std::runtime_error("Invalid argument.");
                }
            } catch (...) {
                throw sls::RuntimeError("Invalid number of receivers." +
                                        help_message);
            }
            break;

        case 't':
            LOG(sls::logWARNING)
                << "Deprecated option. Please use 'p' or '--port'.";
            //[[fallthrough]]; TODO: for when we update to c++17
        case 'p':
            try {
                startPort = sls::StringTo<uint16_t>(optarg);
            } catch (...) {
                throw sls::RuntimeError("Could not scan port number." +
                                        help_message);
            }
            break;

        case 'c':
            callbackEnabled = true;
            break;

        case 'u':
            try {
                userid = sls::StringTo<uint32_t>(optarg);
            } catch (...) {
                throw sls::RuntimeError("Invalid uid." + help_message);
            }
            break;

        case 'h':
            std::cout << help_message << std::endl;
            exit(EXIT_SUCCESS);
        default:
            // maintain backward compatibility of [startport] [num-receivers]
            // [callback]
            GetDeprecatedCommandLineOptions(argc, argv, startPort, numReceivers,
                                            callbackEnabled);
            throw sls::RuntimeError(help_message);
        }
    }

    LOG(sls::logINFOBLUE) << "Current Process [ Tid: " << gettid() << ']';
    LOG(sls::logINFO) << "Number of Receivers: " << numReceivers;
    LOG(sls::logINFO) << "Start TCP Port: " << startPort;
    LOG(sls::logINFO) << "Callback Enable: " << callbackEnabled;

    // set effective id if provided
    if (userid != static_cast<uid_t>(-1)) {
        if (geteuid() == userid) {
            LOG(sls::logINFO)
                << "Process already has the same Effective UID " << userid;
        } else {
            if (seteuid(userid) != 0) {
                std::ostringstream oss;
                oss << "Could not set Effective UID to " << userid;
                throw sls::RuntimeError(oss.str());
            }
            if (geteuid() != userid) {
                std::ostringstream oss;
                oss << "Could not set Effective UID to " << userid << ". Got "
                    << geteuid();
                throw sls::RuntimeError(oss.str());
            }
            LOG(sls::logINFO) << "Process Effective UID changed to " << userid;
        }
    }

    /** - Catch signal SIGINT to close files and call destructors properly */
    struct sigaction sa;
    sa.sa_flags = 0;                     // no flags
    sa.sa_handler = sigInterruptHandler; // handler function
    sigemptyset(&sa.sa_mask); // dont block additional signals during invocation
                              // of handler
    if (sigaction(SIGINT, &sa, nullptr) == -1) {
        LOG(sls::logERROR) << "Could not set handler function for SIGINT";
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
        LOG(sls::logERROR) << "Could not set handler function for SIGPIPE";
    }

    /** - loop over number of receivers */
    sem_init(&semaphore, 1, 0);
    for (int i = 0; i < numReceivers; ++i) {

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
                uint16_t port = startPort + i;
                sls::Receiver receiver(port);

                /**	- register callbacks. remember to set file write enable
                 * to 0 (using the client) if we should not write files and you
                 * will write data using the callbacks */
                if (callbackEnabled) {

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
                sem_wait(&semaphore);
                sem_destroy(&semaphore);

            } catch (...) {
                LOG(sls::logINFOBLUE)
                    << "Exiting Child Process [ Tid: " << gettid() << " ]";
                throw;
            }

            LOG(sls::logINFOBLUE)
                << "Exiting Child Process [ Tid: " << gettid() << ']';
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
        LOG(sls::logERROR) << "Could not set handler function for SIGINT";
    }

    /** - Print Ready and Instructions how to exit */
    std::cout << "Ready ... \n";
    LOG(sls::logINFO) << "\n[ Press \'Ctrl+c\' to exit ]";

    /** - Parent process waits for all child processes to exit */
    for (;;) {
        pid_t childPid = waitpid(-1, nullptr, 0);

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

        // child closed
        LOG(sls::logINFOBLUE)
            << "Exiting Child Process [ Tid: " << childPid << ']';
    }

    std::cout << "Goodbye!\n";
    return 0;
}
