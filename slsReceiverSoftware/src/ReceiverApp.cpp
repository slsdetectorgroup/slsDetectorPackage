// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
/* slsReceiver */
#include "sls/Receiver.h"
#include "sls/ToString.h"
#include "sls/container_utils.h"
#include "sls/logger.h"
#include "sls/sls_detector_defs.h"

#include <csignal> //SIGINT
#include <getopt.h>
#include <semaphore.h>
#include <unistd.h>

// gettid added in glibc 2.30
#if __GLIBC__ == 2 && __GLIBC_MINOR__ < 30
#include <sys/syscall.h>
#define gettid() syscall(SYS_gettid)
#endif

sem_t semaphore;

void sigInterruptHandler(int p) { sem_post(&semaphore); }

int main(int argc, char *argv[]) {

    uint16_t port = DEFAULT_TCP_RX_PORTNO;
    uid_t userid = -1;

    std::string help_message =
        "\nUsage: " + std::string(argv[0]) + " Options:\n" +
        "\t-v, --version       : Version of " + std::string(argv[0]) + ".\n" +
        "\t-p, --port          : TCP port to communicate with client for "
        "configuration. Non-zero and 16 bit.\n" +
        "\t-u, --uid           : Set effective user id if receiver started "
        "with privileges. \n\n";

    static struct option long_options[] = {
        {"help", no_argument, nullptr, 'h'},
        {"version", no_argument, nullptr, 'v'},
        {"rx_tcpport", required_argument, nullptr, 't'},
        {"port", required_argument, nullptr, 'p'},
        {"uid", required_argument, nullptr, 'u'},
        {nullptr, 0, nullptr, 0}};

    int option_index = 0;
    int opt = 0;
    while (-1 != (opt = getopt_long(argc, argv, "hvt:p:u:", long_options,
                                    &option_index))) {

        switch (opt) {

        case 't':
            LOG(sls::logWARNING)
                << "Deprecated option. Please use 'p' or '--port'.";
            //[[fallthrough]]; TODO: for when we update to c++17
        case 'p':
            try {
                port = sls::StringTo<uint16_t>(optarg);
            } catch (...) {
                throw sls::RuntimeError("Could not scan port number." +
                                        help_message);
            }
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
            throw sls::RuntimeError(help_message);
        }
    }

    LOG(sls::logINFOBLUE) << "Current Process [ Tid: " << gettid() << " ]";
    LOG(sls::logINFO) << "Port: " << port;

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

    // Catch signal SIGINT to close files and call destructors properly
    struct sigaction sa;
    sa.sa_flags = 0;                     // no flags
    sa.sa_handler = sigInterruptHandler; // handler function
    sigemptyset(&sa.sa_mask); // dont block additional signals during invocation
                              // of handler
    if (sigaction(SIGINT, &sa, nullptr) == -1) {
        LOG(sls::logERROR) << "Could not set handler function for SIGINT";
    }

    // if socket crash, ignores SISPIPE, prevents global signal handler
    // subsequent read/write to socket gives error - must handle locally
    struct sigaction asa;
    asa.sa_flags = 0;          // no flags
    asa.sa_handler = SIG_IGN;  // handler function
    sigemptyset(&asa.sa_mask); // dont block additional signals during
                               // invocation of handler
    if (sigaction(SIGPIPE, &asa, nullptr) == -1) {
        LOG(sls::logERROR) << "Could not set handler function for SIGPIPE";
    }

    sem_init(&semaphore, 1, 0);
    try {
        sls::Receiver r(port);
        LOG(sls::logINFO) << "[ Press \'Ctrl+c\' to exit ]";
        sem_wait(&semaphore);
        sem_destroy(&semaphore);
    } catch (...) {
        // pass
    }
    LOG(sls::logINFOBLUE) << "Exiting [ Tid: " << gettid() << " ]";
    LOG(sls::logINFO) << "Exiting Receiver";
    return 0;
}
