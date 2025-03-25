// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "sls/Receiver.h"
#include "ClientInterface.h"
#include "sls/ToString.h"
#include "sls/container_utils.h"
#include "sls/logger.h"
#include "sls/sls_detector_exceptions.h"
#include "sls/versionAPI.h"

#include <cstdlib>
#include <fstream>
#include <getopt.h>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <unistd.h>

namespace sls {

// gettid added in glibc 2.30
#if __GLIBC__ == 2 && __GLIBC_MINOR__ < 30
#include <sys/syscall.h>
#define gettid() syscall(SYS_gettid)
#endif

Receiver::~Receiver() = default;

Receiver::Receiver(int argc, char *argv[]) : tcpipInterface(nullptr) {

    // options
    uint16_t tcpip_port_no = 1954;
    uid_t userid = -1;

    // parse command line for config
    static struct option long_options[] = {
        // These options set a flag.
        //{"verbose", no_argument,       &verbose_flag, 1},
        // These options don’t set a flag. We distinguish them by their indices.
        {"rx_tcpport", required_argument, nullptr,
         't'}, // TODO change or backward compatible to "port, p"?
        {"uid", required_argument, nullptr, 'u'},
        {"version", no_argument, nullptr, 'v'},
        {"help", no_argument, nullptr, 'h'},
        {nullptr, 0, nullptr, 0}};

    // initialize global optind variable (required when instantiating multiple
    // receivers in the same process)
    optind = 1;
    // getopt_long stores the option index here.
    int option_index = 0;
    int c = 0;

    std::string help_message =
        "\nUsage: " + std::string(argv[0]) + " [arguments]\n" +
        "Possible arguments are:\n" +
        "\t-t, --rx_tcpport <port> : TCP Communication Port with "
        "client. Non-zero and 16 bit.\n" +
        "\t-u, --uid <user id>     : Set effective user id if receiver "
        "\n" +
        "\t                          started with privileges. \n\n";

    while (c != -1) {
        c = getopt_long(argc, argv, "hvt:u:", long_options, &option_index);

        // Detect the end of the options.
        if (c == -1)
            break;

        switch (c) {

        case 't':
            try {
                tcpip_port_no = sls::StringTo<uint16_t>(optarg);
                validatePortNumber(tcpip_port_no);
            } catch (...) {
                throw RuntimeError("Could not scan TCP port number." +
                                   help_message);
            }
            break;

        case 'u':
            if (sscanf(optarg, "%u", &userid) != 1) {
                throw RuntimeError("Could not scan uid" + help_message);
            }
            break;

        case 'v':
            std::cout << "slsReceiver Version: " << APIRECEIVER << std::endl;
            LOG(logINFOBLUE) << "Exiting [ Tid: " << gettid() << " ]";
            exit(EXIT_SUCCESS);

        case 'h':
            std::cout << help_message << std::endl;
            exit(EXIT_SUCCESS);
        default:
            throw RuntimeError(help_message);
        }
    }

    // set effective id if provided
    if (userid != static_cast<uid_t>(-1)) {
        if (geteuid() == userid) {
            LOG(logINFO) << "Process already has the same Effective UID "
                         << userid;
        } else {
            if (seteuid(userid) != 0) {
                std::ostringstream oss;
                oss << "Could not set Effective UID to " << userid;
                throw RuntimeError(oss.str());
            }
            if (geteuid() != userid) {
                std::ostringstream oss;
                oss << "Could not set Effective UID to " << userid << ". Got "
                    << geteuid();
                throw RuntimeError(oss.str());
            }
            LOG(logINFO) << "Process Effective UID changed to " << userid;
        }
    }

    // might throw an exception
    tcpipInterface = make_unique<ClientInterface>(tcpip_port_no);
}

Receiver::Receiver(uint16_t tcpip_port_no) {
    // might throw an exception
    tcpipInterface = make_unique<ClientInterface>(tcpip_port_no);
}

std::string Receiver::getReceiverVersion() {
    return tcpipInterface->getReceiverVersion();
}

void Receiver::registerCallBackStartAcquisition(
    int (*func)(const startCallbackHeader, void *), void *arg) {
    tcpipInterface->registerCallBackStartAcquisition(func, arg);
}

void Receiver::registerCallBackAcquisitionFinished(
    void (*func)(const endCallbackHeader, void *), void *arg) {
    tcpipInterface->registerCallBackAcquisitionFinished(func, arg);
}

void Receiver::registerCallBackRawDataReady(
    void (*func)(sls_receiver_header &, const dataCallbackHeader, char *,
                 size_t &, void *),
    void *arg) {
    tcpipInterface->registerCallBackRawDataReady(func, arg);
}

} // namespace sls