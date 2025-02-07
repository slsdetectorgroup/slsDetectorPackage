// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
/* Creates the slsFrameSynchronizer for running multiple receivers in different
 * threads form a single binary that will spit out zmq streams without
 * reconstructing image. Sample python script for pull socket for this combiner
 * in python/scripts folder. TODO: Not handling empty frames from one socket
 */
#include "sls/Receiver.h"
#include "sls/ToString.h"
#include "sls/container_utils.h"
#include "sls/logger.h"
#include "sls/sls_detector_defs.h"

#include <csignal> //SIGINT
#include <cstdio>
#include <cstring>
#include <iostream>
#include <mutex>
#include <ostream>
#include <semaphore.h>
#include <sys/socket.h>
#include <sys/wait.h> //wait
#include <thread>
#include <unistd.h>

#include <set>
#include <vector>
#include <zmq.h>

std::vector<std::thread> threads;
std::vector<sem_t *> semaphores;
sls::TLogLevel printHeadersLevel = sls::logDEBUG;

/** Define Colors to print data call back in different colors for different
 * recievers */
#define PRINT_IN_COLOR(c, f, ...)                                              \
    printf("\033[%dm" f RESET, 30 + c + 1, ##__VA_ARGS__)

/** Structure handling different threads */
struct Status {
    bool starting = true;
    bool terminate = false;
    unsigned long num_receivers;
    sem_t available;
    std::mutex mtx;
    std::vector<zmq_msg_t *> headers;
    std::map<unsigned int,
             std::map<long unsigned int, std::vector<zmq_msg_t *>>>
        frames;
    std::vector<zmq_msg_t *> ends;

    Status(bool start, bool term, unsigned long num_recv)
        : starting(start), terminate(term), num_receivers(num_recv) {
        sem_init(&available, 0, 0);
    }
};

/**
 * Control+C Interrupt Handler
 * to let all the processes know to exit properly
 */
void sigInterruptHandler(int p) {
    for (size_t i = 0; i != semaphores.size(); ++i) {
        sem_post(semaphores[i]);
    }
}

/**
 * prints usage of this example program
 */
std::string getHelpMessage() {
    std::ostringstream os;
    os << "\nUsage:\n"
          "./slsFrameSynchronizer [start tcp port] [num recevers] [print "
          "callback headers (optional)]\n"
       << "\t - tcp port has to be non-zero and 16 bit\n"
       << "\t - print callback headers option is 0 (disabled) by default\n";
    return os.str();
}

void zmq_free(void *data, void *hint) { free(data); }

void print_frames(
    const std::map<unsigned int,
                   std::map<long unsigned int, std::vector<zmq_msg_t *>>>
        &frames) {
    LOG(sls::logDEBUG) << "Printing frames";
    for (const auto &outer_pair : frames) {
        unsigned int udpPort = outer_pair.first;
        const auto &trigger_map = outer_pair.second;
        LOG(sls::logDEBUG) << "UDP port: " << udpPort;
        for (const auto &inner_pair : trigger_map) {
            long unsigned int acqIndex = inner_pair.first;
            const auto &msg_vector = inner_pair.second;
            LOG(sls::logDEBUG) << "  acq index: " << acqIndex << '['
                               << msg_vector.size() << ']';
        }
    }
}

std::set<long unsigned int>
find_keys(const std::map<unsigned int,
                         std::map<long unsigned int, std::vector<zmq_msg_t *>>>
              &maps) {
    std::set<long unsigned int>
        all_keys; // Set to collect all unique keys across all maps
    std::set<long unsigned int> valid_keys; // Set to store final valid keys

    // If no maps are provided, return empty set
    if (maps.empty()) {
        return valid_keys;
    }

    // Collect all unique keys from all maps
    for (const auto &[port, trigger_map] : maps) {
        for (const auto &[idx, msgs] : trigger_map) {
            all_keys.insert(idx);
        }
    }

    // Now check each key against all maps (udp ports)
    for (const auto &key : all_keys) {
        LOG(sls::logDEBUG) << "Checking key: " << key;
        bool is_valid = true;
        for (const auto &[port, map] : maps) {
            auto it = map.find(key);
            if (it != map.end()) {
                LOG(sls::logDEBUG)
                    << "  Key " << key << " found in map " << port;
            } else {
                // Key is missing, check if the map has a larger key
                LOG(sls::logDEBUG)
                    << "  Key " << key << " missing in map " << port;
                auto upper_it = map.upper_bound(key);
                if (upper_it != map.end()) {
                    LOG(sls::logDEBUG)
                        << ", but found larger key: " << upper_it->first
                        << std::endl;
                } else {
                    LOG(sls::logDEBUG) << ", no larger key found. Key " << key
                                       << " is invalid.\n";
                    is_valid = false;
                    break;
                }
            }
        }

        if (is_valid) {
            LOG(sls::logDEBUG) << "  Key " << key << " is valid.\n";
            valid_keys.insert(key);
        } else {
            LOG(sls::logDEBUG) << "  Key " << key << " is not valid.\n";
        }
    }

    return valid_keys;
}

int zmq_send_multipart(void *socket, const std::vector<zmq_msg_t *> &messages) {
    size_t num_messages = messages.size();

    // Iterate over each message in the vector
    for (size_t i = 0; i < num_messages; ++i) {
        zmq_msg_t *msg = messages[i];

        // Determine flags: ZMQ_SNDMORE for all messages except the last
        int flags = (i == num_messages - 1) ? 0 : ZMQ_SNDMORE;

        // Send the message part
        if (zmq_msg_send(msg, socket, flags) == -1) {
            LOG(sls::logERROR)
                << "Error sending message: " << zmq_strerror(zmq_errno());
            return slsDetectorDefs::FAIL;
        }
    }

    return slsDetectorDefs::OK;
}

void Correlate(Status *stat) {
    void *context = zmq_ctx_new();

    void *socket = zmq_socket(context, ZMQ_PUSH);
    int rc = zmq_bind(socket, "tcp://*:5555");
    if (rc != 0) {
        LOG(sls::logERROR) << "failed to bind";
    }

    while (!stat->terminate) {
        sem_wait(&(stat->available));
        {
            std::lock_guard<std::mutex> lock(stat->mtx);
            if (stat->starting) {
                if (stat->headers.size() == stat->num_receivers) {
                    LOG(sls::logDEBUG) << "Sending all start messages";
                    stat->starting = false;
                    zmq_send_multipart(socket, stat->headers);
                    stat->headers.clear();
                }
            } else {
                //print_frames(stat->frames);
                auto common_keys = find_keys(stat->frames);
                for (const auto &key : common_keys) {
                    std::vector<zmq_msg_t *> parts;
                    for (const auto &[port, trigger_map] : stat->frames) {
                        auto it = trigger_map.find(key);
                        if (it != trigger_map.end()) {
                            parts.insert(parts.end(),
                                         stat->frames[port][key].begin(),
                                         stat->frames[port][key].end());
                            LOG(sls::logDEBUG)
                                << "  Key " << key << " found in map " << port;
                            stat->frames[port].erase(key);
                        }
                    }
                    LOG(sls::logDEBUG) << key << " ";
                    LOG(printHeadersLevel) << "Sending data for index " << key;
                    zmq_send_multipart(socket, parts);
                }
            }
            if (stat->ends.size() == stat->num_receivers) {
                LOG(sls::logDEBUG) << "Sending all end messages";
                // clean up all remaining frames
                zmq_send_multipart(socket, stat->ends);
                stat->ends.clear();
            }
        }
    }
    zmq_close(socket);
    zmq_ctx_destroy(context);
}

int StartAcquisitionCallback(const slsDetectorDefs::startCallbackHeader callbackHeader,
             void *objectPointer) {
    LOG(printHeadersLevel) << "Start Acquisition:"
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
    std::ostringstream oss;
    oss << "{\"htype\":\"header\""
        << ", \"udpPorts\":" << sls::ToString(callbackHeader.udpPort)
        << ", \"bitmode\":" << callbackHeader.dynamicRange
        << ", \"filePath\":\"" << callbackHeader.filePath
        << "\", \"fileName\":\"" << callbackHeader.fileName
        << "\", \"fileIndex\":" << callbackHeader.fileIndex
        << ", \"detshape\":" << sls::ToString(callbackHeader.detectorShape)
        << ", \"size\":" << callbackHeader.imageSize
        << ", \"quad\":" << (callbackHeader.quad ? 1 : 0) << "}\n";
    std::string message = oss.str();
    LOG(sls::logDEBUG) << "Start Acquisition message:" << std::endl << message;
    int length = message.length();
    char *hdata = new char[length];
    memcpy(hdata, message.c_str(), length);

    zmq_msg_t *hmsg = new zmq_msg_t;
    zmq_msg_init_data(hmsg, hdata, length, zmq_free, NULL);
    Status *stat = static_cast<Status *>(objectPointer);
    {
        std::lock_guard<std::mutex> lock(stat->mtx);
        stat->headers.push_back(hmsg);
        stat->starting = true;
        for (int port : callbackHeader.udpPort) {
            // clear cache for udp port
            for (auto &pair : stat->frames[port]) {
                // clearing cache for acq index
                for (auto msg : pair.second) {
                    zmq_msg_close(msg);
                    free(msg);
                }
            }
            stat->frames[port].clear();
        }
    }
    sem_post(&stat->available);
    return slsDetectorDefs::OK; // TODO: change return to void
}

void AcquisitionFinishedCallback(
    const slsDetectorDefs::endCallbackHeader callbackHeader,
    void *objectPointer) {
    LOG(printHeadersLevel) << "Acquisition Finished:"
                           << "\n\t["
                           << "\n\tUDP Port : "
                           << sls::ToString(callbackHeader.udpPort)
                           << "\n\tComplete Frames : "
                           << sls::ToString(callbackHeader.completeFrames)
                           << "\n\tLast Frame Index : "
                           << sls::ToString(callbackHeader.lastFrameIndex)
                           << "\n\t]";
    std::ostringstream oss;
    oss << "{\"htype\":\"series_end\""
        << ", \"udpPorts\":" << sls::ToString(callbackHeader.udpPort)
        << ", \"comleteFrames\":"
        << sls::ToString(callbackHeader.completeFrames)
        << ", \"lastFrameIndex\":"
        << sls::ToString(callbackHeader.lastFrameIndex) << "}\n";
    std::string message = oss.str();
    int length = message.length();
    LOG(sls::logDEBUG) << "Acquisition Finished message:" << std::endl
                       << message;
    char *hdata = new char[length];
    memcpy(hdata, message.c_str(), length);

    zmq_msg_t *hmsg = new zmq_msg_t;
    zmq_msg_init_data(hmsg, hdata, length, zmq_free, NULL);
    Status *stat = static_cast<Status *>(objectPointer);
    {
        std::lock_guard<std::mutex> lock(stat->mtx);
        stat->ends.push_back(hmsg);
    }
    sem_post(&stat->available);
}

void GetDataCallback(slsDetectorDefs::sls_receiver_header &header,
             slsDetectorDefs::dataCallbackHeader callbackHeader,
             char *dataPointer, size_t &imageSize, void *objectPointer) {

    slsDetectorDefs::sls_detector_header detectorHeader = header.detHeader;

    if (printHeadersLevel < sls::logDEBUG) {
        // print in different color for each udp port
        PRINT_IN_COLOR((callbackHeader.udpPort % 10),
                       "Data: "
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
                       callbackHeader.udpPort, callbackHeader.shape.x,
                       callbackHeader.shape.y, callbackHeader.acqIndex,
                       callbackHeader.frameIndex, callbackHeader.progress,
                       sls::ToString(callbackHeader.completeImage).c_str(),
                       sls::ToString(callbackHeader.flipRows).c_str(),
                       sls::ToString(callbackHeader.addJsonHeader).c_str(),
                       detectorHeader.frameNumber, detectorHeader.expLength,
                       detectorHeader.packetNumber, detectorHeader.detSpec1,
                       detectorHeader.timestamp, detectorHeader.modId,
                       detectorHeader.row, detectorHeader.column,
                       detectorHeader.detSpec2, detectorHeader.detSpec3,
                       detectorHeader.detSpec4,
                       sls::ToString(detectorHeader.detType).c_str(),
                       detectorHeader.version,
                       // header->packetsMask.to_string().c_str(),
                       ((uint8_t)(*((uint8_t *)(dataPointer)))), imageSize);
    }
    std::ostringstream oss;
    oss << "{\"htype\":\"module\""
        << ", \"port\":" << callbackHeader.udpPort
        << ", \"shape\":" << sls::ToString(callbackHeader.shape)
        << ", \"acqIndex\":" << callbackHeader.acqIndex
        << ", \"frameIndex\":" << callbackHeader.frameIndex
        << ", \"flipRows\":" << (callbackHeader.flipRows ? 1 : 0)
        << ", \"progress\":" << callbackHeader.progress
        << ", \"completeImage\":" << (callbackHeader.completeImage ? 1 : 0)
        << ", \"frameNumber\":" << detectorHeader.frameNumber
        << ", \"expLength\":" << detectorHeader.expLength
        << ", \"packetNumber\":" << detectorHeader.packetNumber
        << ", \"detSpec1\":" << detectorHeader.detSpec1
        << ", \"timestamp\":" << detectorHeader.timestamp
        << ", \"modId\":" << detectorHeader.modId
        << ", \"row\":" << detectorHeader.row
        << ", \"column\":" << detectorHeader.column
        << ", \"detSpec2\":" << detectorHeader.detSpec2
        << ", \"detSpec3\":" << detectorHeader.detSpec3
        << ", \"detSpec4\":" << detectorHeader.detSpec4
        << ", \"detType\":" << static_cast<int>(detectorHeader.detType)
        << ", \"version\":" << static_cast<int>(detectorHeader.version);

    if (!callbackHeader.addJsonHeader.empty()) {
        oss << ", \"addJsonHeader\": {";
        for (auto it = callbackHeader.addJsonHeader.begin();
             it != callbackHeader.addJsonHeader.end(); ++it) {
            if (it != callbackHeader.addJsonHeader.begin()) {
                oss << ", ";
            }
            oss << "\"" << it->first.c_str() << "\":\"" << it->second.c_str()
                << "\"";
        }
        oss << " } ";
    }
    oss << "}\n";
    std::string message = oss.str();
    LOG(sls::logDEBUG) << "Data message:" << std::endl << message;
    int length = message.length();

    char *hdata = new char[length];
    memcpy(hdata, message.c_str(), length);
    zmq_msg_t *hmsg = new zmq_msg_t;
    zmq_msg_init_data(hmsg, hdata, length, zmq_free, NULL);
    LOG(sls::logDEBUG) << callbackHeader.udpPort << ": created header frame";

    char *data = new char[imageSize];
    zmq_msg_t *msg = new zmq_msg_t;
    zmq_msg_init_data(msg, data, imageSize, zmq_free, NULL);
    LOG(sls::logDEBUG) << callbackHeader.udpPort
                       << ": copied data to data frame";

    Status *stat = static_cast<Status *>(objectPointer);
    {
        std::lock_guard<std::mutex> lock(stat->mtx);
        LOG(sls::logDEBUG) << callbackHeader.udpPort << "put data in cache";
        stat->frames[callbackHeader.udpPort][header.detHeader.frameNumber]
            .push_back(hmsg);
        stat->frames[callbackHeader.udpPort][header.detHeader.frameNumber]
            .push_back(msg);
    }
    sem_post(&stat->available);
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
    uint16_t startTCPPort = DEFAULT_TCP_RX_PORTNO;
    bool printHeaders = false;

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
                    printHeaders = sls::StringTo<bool>(argv[3]);
                    if (printHeaders) {
                        printHeadersLevel = sls::logINFOBLUE;
                    }
                }
            } else
                throw std::runtime_error("Invalid number of arguments");
        } catch (const std::exception &e) {
            cprintf(RED, "Error: %s\n%s\n", e.what(), getHelpMessage().c_str());
            return EXIT_FAILURE;
        }
    }

    cprintf(RESET, "Number of Receivers: %d\n", numReceivers);
    cprintf(RESET, "Start TCP Port: %hu\n", startTCPPort);
    cprintf(RESET, "Print Callback Headers: %s\n\n",
            (printHeaders ? "Enabled" : "Disabled"));

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

    Status stat{true, false, (unsigned long)numReceivers};
    void *user_data = static_cast<void *>(&stat);

    std::thread combinerThread(Correlate, &stat);

    /** - loop over number of receivers */
    for (int i = 0; i != numReceivers; ++i) {
        sem_t *semaphore = new sem_t;
        sem_init(semaphore, 1, 0);
        semaphores.push_back(semaphore);

        uint16_t port = startTCPPort + i;
        threads.emplace_back([semaphore, port, user_data]() {
            sls::Receiver receiver(port);

            receiver.registerCallBackStartAcquisition(StartAcquisitionCallback, user_data);
            receiver.registerCallBackAcquisitionFinished(AcquisitionFinishedCallback,
                                                         user_data);
            receiver.registerCallBackRawDataReady(GetDataCallback, user_data);
            /**	- as long as no Ctrl+C */
            sem_wait(semaphore);
            sem_destroy(semaphore);
        });
    }

    for (auto &thread : threads) {
        thread.join();
    }

    stat.terminate = true;
    sem_post(&stat.available);
    combinerThread.join();
    sem_destroy(&stat.available);

    LOG(sls::logINFOBLUE) << "Goodbye!";
    return EXIT_SUCCESS;
}
