// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
/* slsReceiver */
#include "CommandLineOptions.h"
#include "sls/Receiver.h"
#include "sls/thread_utils.h"
#include "sls/ToString.h"
#include "sls/container_utils.h"
#include "sls/logger.h"
#include "sls/network_utils.h"
#include "sls/sls_detector_defs.h"

#include <csignal> //SIGINT
#include <pthread.h>
#include <unistd.h>

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

    LOG(sls::logINFOBLUE) << "Current Process [ Tid: " << sls::getThreadId()
                          << " ]";

    // Block SIGINT on this thread before any other thread is created so that
    // every thread spawned by Receiver inherits the block. We wait for the
    // signal synchronously with sigwait() further down. This avoids needing a
    // signal handler that posts to a semaphore, which is not portable to
    // macOS (sem_init on unnamed semaphores is unimplemented there).
    sigset_t sigset;
    sigemptyset(&sigset);
    sigaddset(&sigset, SIGINT);
    if (pthread_sigmask(SIG_BLOCK, &sigset, nullptr) != 0) {
        LOG(sls::logERROR) << "Could not block SIGINT";
        return EXIT_FAILURE;
    }

    // handle locally on socket crash
    sls::setupSignalHandler(SIGPIPE, SIG_IGN);

    try {
        sls::Receiver r(o.port);
        LOG(sls::logINFO) << "[ Press \'Ctrl+c\' to exit ]";
        int sig = 0;
        sigwait(&sigset, &sig);
    } catch (...) {
        LOG(sls::logINFOBLUE)
            << "Exiting [ Tid: " << sls::getThreadId() << " ]";
        throw;
    }
    LOG(sls::logINFOBLUE) << "Exiting [ Tid: " << sls::getThreadId() << " ]";
    LOG(sls::logINFO) << "Exiting Receiver";
    return EXIT_SUCCESS;
}
