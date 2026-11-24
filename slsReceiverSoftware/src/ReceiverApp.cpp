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
#include "sls/thread_utils.h"

#include <csignal> //SIGINT
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

    // close files on ctrl+c: SIGINT is blocked (before the receiver threads
    // are created, so they inherit the mask) and waited for synchronously
    sls::blockSignal(SIGINT);
    // handle locally on socket crash
    sls::setupSignalHandler(SIGPIPE, SIG_IGN);

    try {
        sls::Receiver r(o.port);
        LOG(sls::logINFO) << "[ Press \'Ctrl+c\' to exit ]";
        sls::waitForSignal(SIGINT);
    } catch (...) {
        LOG(sls::logINFOBLUE)
            << "Exiting [ Tid: " << sls::getThreadId() << " ]";
        throw;
    }
    LOG(sls::logINFOBLUE) << "Exiting [ Tid: " << sls::getThreadId() << " ]";
    LOG(sls::logINFO) << "Exiting Receiver";
    return EXIT_SUCCESS;
}
