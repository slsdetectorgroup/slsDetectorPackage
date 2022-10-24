// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
/* slsReceiver */
#include "sls/Receiver.h"
#include "sls/container_utils.h"
#include "sls/logger.h"
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

void sigInterruptHandler(int p) { sem_post(&semaphore); }

int main(int argc, char *argv[]) {

    sem_init(&semaphore, 1, 0);

    LOG(sls::logINFOBLUE) << "Created [ Tid: " << gettid() << " ]";

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

    try {
        sls::Receiver r(argc, argv);
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
