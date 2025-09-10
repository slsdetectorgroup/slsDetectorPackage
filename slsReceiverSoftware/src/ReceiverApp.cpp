// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
/* slsReceiver */
#include "CommandLineOptions.h"
#include "sls/Receiver.h"
#include "sls/ToString.h"
#include "sls/container_utils.h"
#include "sls/logger.h"
#include "sls/network_utils.h"
#include "sls/sls_detector_defs.h"

#include <csignal> //SIGINT
#include <semaphore.h>
#include <unistd.h>

// gettid added in glibc 2.30
#if __GLIBC__ == 2 && __GLIBC_MINOR__ < 30
#include <sys/syscall.h>
#define gettid() syscall(SYS_gettid)
#endif

sem_t semaphore;

/**
 * Control+C Interrupt Handler
 * to let all the other process know to exit properly
 */
void sigInterruptHandler(int signal) {
    (void)signal; // suppress unused warning if needed
    sem_post(&semaphore);
}

int main(int argc, char *argv[]) {

    CommandLineOptions cli(AppType::SingleReceiver);
    ParsedOptions opts;
    try {
        opts = cli.parse(argc, argv);
    } catch (sls::RuntimeError &e) {
        return EXIT_FAILURE;
    }
    auto &o = std::get<CommonOptions>(opts);
    if (o.versionRequested || o.helpRequested) {
        return EXIT_SUCCESS;
    }

    LOG(sls::logINFOBLUE) << "Current Process [ Tid: " << gettid() << " ]";

    // close files on ctrl+c
    sls::setupSignalHandler(SIGINT, sigInterruptHandler);
    // handle locally on socket crash
    sls::setupSignalHandler(SIGPIPE, SIG_IGN);

    sem_init(&semaphore, 1, 0);

    try {
        sls::Receiver r(o.port);
        LOG(sls::logINFO) << "[ Press \'Ctrl+c\' to exit ]";
        sem_wait(&semaphore);
        sem_destroy(&semaphore);
    } catch (...) {
        sem_destroy(&semaphore);
        LOG(sls::logINFOBLUE) << "Exiting [ Tid: " << gettid() << " ]";
        throw;
    }
    LOG(sls::logINFOBLUE) << "Exiting [ Tid: " << gettid() << " ]";
    LOG(sls::logINFO) << "Exiting Receiver";
    return EXIT_SUCCESS;
}
