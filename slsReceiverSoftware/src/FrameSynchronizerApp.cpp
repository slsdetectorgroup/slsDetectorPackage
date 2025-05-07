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
#include "sls/versionAPI.h"

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
using ZmqMsgList = std::vector<zmq_msg_t *>;
using FrameMap = std::map<uint64_t, ZmqMsgList>;
using PortFrameMap = std::map<uint16_t, FrameMap>;
struct FrameStatus {
    bool starting = true;
    bool terminate = false;
    int num_receivers;
    sem_t available;
    std::mutex mtx;
    ZmqMsgList headers;
    PortFrameMap frames;
    ZmqMsgList ends;

    FrameStatus(bool start, bool term, int num_recv)
        : starting(start), terminate(term), num_receivers(num_recv) {
        sem_init(&available, 0, 0);
    }
};
FrameStatus *global_frame_status = nullptr;

/**
 * Control+C Interrupt Handler
 * to let all the processes know to exit properly
 */
void sigInterruptHandler(int p) {
    for (size_t i = 0; i != semaphores.size(); ++i) {
        sem_post(semaphores[i]);
    }
}

void cleanup() {
    if (global_frame_status) {
        std::lock_guard<std::mutex> lock(global_frame_status->mtx);
        for (auto &outer_pair : global_frame_status->frames) {
            for (auto &inner_pair : outer_pair.second) {
                for (zmq_msg_t *msg : inner_pair.second) {
                    if (msg) {
                        zmq_msg_close(msg);
                        delete msg;
                    }
                }
                inner_pair.second.clear();
            }
            outer_pair.second.clear();
        }
        global_frame_status->frames.clear();
    }
}

/**
 * prints usage of this example program
 */
std::string getHelpMessage() {
    std::ostringstream os;
    os << "\nUsage:\n"
       << "./slsFrameSynchronizer --version or -v\n"
       << "\t - Gets the slsFrameSynchronizer version\n\n"
       << "./slsFrameSynchronizer [start tcp port] [num recevers] [print "
          "callback headers (optional)]\n"
       << "\t - tcp port has to be non-zero and 16 bit\n"
       << "\t - print callback headers option is 0 (disabled) by default\n";
    return os.str();
}

void zmq_free(void *data, void *hint) { delete[] static_cast<char *>(data); }

void print_frames(const PortFrameMap &frame_port_map) {
    LOG(sls::logDEBUG) << "Printing frames";
    for (const auto &it : frame_port_map) {
        const uint16_t udpPort = it.first;
        const auto &frame_map = it.second;
        LOG(sls::logDEBUG) << "UDP port: " << udpPort;
        for (const auto &frame : frame_map) {
            const uint64_t fnum = frame.first;
            const auto &msg_list = frame.second;
            LOG(sls::logDEBUG)
                << "  acq index: " << fnum << '[' << msg_list.size() << ']';
        }
    }
}

/** Valid frame numbers mean they exist across all ports or
 * has at least a larger fnum in the port with the missing fnum **/
std::set<uint64_t> get_valid_fnums(const PortFrameMap &port_frame_map) {
    // empty list
    std::set<uint64_t> valid_fnums;
    if (port_frame_map.empty()) {
        return valid_fnums;
    }

    // collect all unique frame numbers from all ports
    std::set<uint64_t> unique_fnums;
    for (const auto &it : port_frame_map) {
        const FrameMap &frame_map = it.second;
        for (const auto &frame : frame_map) {
            unique_fnums.insert(frame.first);
        }
    }

    // collect valid frame numbers
    for (auto &fnum : unique_fnums) {
        bool is_valid = true;
        for (const auto &it : port_frame_map) {
            const uint16_t port = it.first;
            const FrameMap &frame_map = it.second;
            auto frame = frame_map.find(fnum);
            // invalid: fnum missing in one port
            if (frame == frame_map.end()) {
                LOG(sls::logDEBUG)
                    << "Fnum " << fnum << " is missing in port " << port;
                auto upper_frame = frame_map.upper_bound(fnum);
                if (upper_frame == frame_map.end()) {
                    LOG(sls::logDEBUG) << "And no larger fnum found. Fnum "
                                       << fnum << " is invalid.\n";
                    is_valid = false;
                    break;
                }
            }
        }
        if (is_valid) {
            valid_fnums.insert(fnum);
        }
    }

    return valid_fnums;
}

int zmq_send_multipart(void *socket, const ZmqMsgList &messages) {
    size_t num_messages = messages.size();
    for (size_t i = 0; i != num_messages; ++i) {
        zmq_msg_t *msg = messages[i];
        // determine flags: ZMQ_SNDMORE for all messages except the last
        int flags = (i == num_messages - 1) ? 0 : ZMQ_SNDMORE;
        if (zmq_msg_send(msg, socket, flags) == -1) {
            LOG(sls::logERROR)
                << "Error sending message: " << zmq_strerror(zmq_errno());
            return slsDetectorDefs::FAIL;
        }
    }
    return slsDetectorDefs::OK;
}

void Correlate(FrameStatus *stat) {
    void *context = zmq_ctx_new();

    void *socket = zmq_socket(context, ZMQ_PUSH);
    int rc = zmq_bind(socket, "tcp://*:5555");
    if (rc != 0) {
        LOG(sls::logERROR) << "failed to bind";
    }

    while (true) {
        sem_wait(&(stat->available));
        {
            std::lock_guard<std::mutex> lock(stat->mtx);

            if (stat->terminate) {
                break;
            }

            if (stat->starting) {
                // sending all start packets
                if (static_cast<int>(stat->headers.size()) ==
                    stat->num_receivers) {
                    stat->starting = false;
                    // clean up
                    zmq_send_multipart(socket, stat->headers);
                    for (zmq_msg_t *msg : stat->headers) {
                        if (msg) {
                            zmq_msg_close(msg);
                            delete msg;
                        }
                    }
                    stat->headers.clear();
                }
            } else {
                // print_frames(stat->frames);
                auto valid_fnums = get_valid_fnums(stat->frames);
                // sending all valid fnum data packets
                for (const auto &fnum : valid_fnums) {
                    ZmqMsgList msg_list;
                    for (const auto &it : stat->frames) {
                        const uint16_t port = it.first;
                        const FrameMap &frame_map = it.second;
                        auto frame = frame_map.find(fnum);
                        if (frame != frame_map.end()) {
                            msg_list.insert(msg_list.end(),
                                            stat->frames[port][fnum].begin(),
                                            stat->frames[port][fnum].end());
                        }
                    }
                    LOG(printHeadersLevel)
                        << "Sending data packets for fnum " << fnum;
                    zmq_send_multipart(socket, msg_list);
                    // clean up
                    for (const auto &it : stat->frames) {
                        const uint16_t port = it.first;
                        const FrameMap &frame_map = it.second;
                        auto frame = frame_map.find(fnum);
                        if (frame != frame_map.end()) {
                            for (zmq_msg_t *msg : frame->second) {
                                if (msg) {
                                    zmq_msg_close(msg);
                                    delete msg;
                                }
                            }
                            stat->frames[port].erase(fnum);
                        }
                    }
                }
            }
            // sending all end packets
            if (static_cast<int>(stat->ends.size()) == stat->num_receivers) {
                zmq_send_multipart(socket, stat->ends);
                // clean up
                for (zmq_msg_t *msg : stat->ends) {
                    if (msg) {
                        zmq_msg_close(msg);
                        delete msg;
                    }
                }
                stat->ends.clear();
                // clean up old frames
                for (auto &it : stat->frames) {
                    FrameMap &frame_map = it.second;
                    for (auto &frame : frame_map) {
                        for (zmq_msg_t *msg : frame.second) {
                            if (msg) {
                                zmq_msg_close(msg);
                                delete msg;
                            }
                        }
                        frame.second.clear();
                    }
                    frame_map.clear();
                }
                stat->frames.clear();
            }
        }
    }
    zmq_close(socket);
    zmq_ctx_destroy(context);
}

int StartAcquisitionCallback(
    const slsDetectorDefs::startCallbackHeader callbackHeader,
    void *objectPointer) {
    LOG(printHeadersLevel)
        << "Start Acquisition:"
        << "\n\t["
        << "\n\tUDP Port : " << sls::ToString(callbackHeader.udpPort)
        << "\n\tDynamic Range : " << callbackHeader.dynamicRange
        << "\n\tDetector Shape : "
        << sls::ToString(callbackHeader.detectorShape)
        << "\n\tImage Size : " << callbackHeader.imageSize
        << "\n\tFile Path : " << callbackHeader.filePath
        << "\n\tFile Name : " << callbackHeader.fileName
        << "\n\tFile Index : " << callbackHeader.fileIndex
        << "\n\tQuad Enable : " << callbackHeader.quad
        << "\n\tAdditional Json Header : "
        << sls::ToString(callbackHeader.addJsonHeader).c_str() << "\n\t]";
    std::ostringstream oss;
    oss << "{\"htype\":\"header\""
        << ", \"udpPorts\":" << sls::ToString(callbackHeader.udpPort)
        << ", \"bitmode\":" << callbackHeader.dynamicRange
        << ", \"filePath\":\"" << callbackHeader.filePath
        << "\", \"fileName\":\"" << callbackHeader.fileName
        << "\", \"fileIndex\":" << callbackHeader.fileIndex
        << ", \"detshape\":" << sls::ToString(callbackHeader.detectorShape)
        << ", \"size\":" << callbackHeader.imageSize
        << ", \"quad\":" << (callbackHeader.quad ? 1 : 0);

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
    LOG(sls::logDEBUG) << "Start Acquisition message:" << std::endl << message;

    int length = message.length();
    char *hdata = new char[length];
    memcpy(hdata, message.c_str(), length);
    zmq_msg_t *hmsg = new zmq_msg_t;
    zmq_msg_init_data(hmsg, hdata, length, zmq_free, NULL);

    // push zmq msg into stat to be processed
    FrameStatus *stat = static_cast<FrameStatus *>(objectPointer);
    {
        std::lock_guard<std::mutex> lock(stat->mtx);
        stat->headers.push_back(hmsg);
        stat->starting = true;
        // clean up old frames
        for (int port : callbackHeader.udpPort) {
            for (auto &frame_map : stat->frames[port]) {
                for (zmq_msg_t *msg : frame_map.second) {
                    if (msg) {
                        zmq_msg_close(msg);
                        delete msg;
                    }
                }
                frame_map.second.clear();
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

    // push zmq msg into stat to be processed
    FrameStatus *stat = static_cast<FrameStatus *>(objectPointer);
    {
        std::lock_guard<std::mutex> lock(stat->mtx);
        stat->ends.push_back(hmsg);
    }
    sem_post(&stat->available);
}

void GetDataCallback(slsDetectorDefs::sls_receiver_header &header,
                     slsDetectorDefs::dataCallbackHeader callbackHeader,
                     char *dataPointer, size_t &imageSize,
                     void *objectPointer) {

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
                       "\n\tComplete Image :%s"
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
        << ", \"imageSize\":" << imageSize
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

    // creating header part of data packet
    int length = message.length();
    char *hdata = new char[length];
    memcpy(hdata, message.c_str(), length);
    zmq_msg_t *hmsg = new zmq_msg_t;
    zmq_msg_init_data(hmsg, hdata, length, zmq_free, NULL);

    // created data part of data packet
    char *data = new char[imageSize];
    zmq_msg_t *msg = new zmq_msg_t;
    zmq_msg_init_data(msg, data, imageSize, zmq_free, NULL);

    // push both parts into stat to be processed
    FrameStatus *stat = static_cast<FrameStatus *>(objectPointer);
    {
        std::lock_guard<std::mutex> lock(stat->mtx);
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

    // version
    if (argc == 2) {
        std::string sargv1 = std::string(argv[1]);
        if (sargv1 == "--version" || sargv1 == "-v") {
            std::cout << "slsFrameSynchronizer Version: " << APIRECEIVER
                      << std::endl;
            exit(EXIT_SUCCESS);
        }
    }

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

    FrameStatus stat{true, false, numReceivers};
    // store pointer for signal handler
    global_frame_status = &stat;

    // thread synchronizing all packets
    void *user_data = static_cast<void *>(&stat);
    std::thread combinerThread(Correlate, &stat);

    for (int i = 0; i != numReceivers; ++i) {
        sem_t *semaphore = new sem_t;
        sem_init(semaphore, 1, 0);
        semaphores.push_back(semaphore);

        uint16_t port = startTCPPort + i;
        threads.emplace_back([i, semaphore, port, user_data]() {
            sls::Receiver receiver(port);
            receiver.registerCallBackStartAcquisition(StartAcquisitionCallback,
                                                      user_data);
            receiver.registerCallBackAcquisitionFinished(
                AcquisitionFinishedCallback, user_data);
            receiver.registerCallBackRawDataReady(GetDataCallback, user_data);
            /**	- as long as no Ctrl+C */
            sem_wait(semaphore);
            sem_destroy(semaphore);
            delete semaphore;

            // clean up frames
            if (i == 0)
                cleanup();
        });
    }

    for (auto &thread : threads) {
        thread.join();
    }

    {
        std::lock_guard<std::mutex> lock(stat.mtx);
        stat.terminate = true;
        sem_post(&stat.available);
    }
    combinerThread.join();
    sem_destroy(&stat.available);

    LOG(sls::logINFOBLUE) << "Goodbye!";
    return EXIT_SUCCESS;
}
