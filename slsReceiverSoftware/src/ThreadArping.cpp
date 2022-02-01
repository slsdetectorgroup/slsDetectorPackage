// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package

#include "ThreadArping.h"
#include "sls/container_utils.h"
#include <iostream>
#include <sys/syscall.h>
#include <unistd.h>

ThreadArping::ThreadArping() {}

ThreadArping::~ThreadArping() { StopRunning(); }

bool ThreadArping::IsRunning() const { return runningFlag; }

void ThreadArping::StartRunning() {
    if (!runningFlag) {
        if (arpInterfaceIp.size() == 0) {
            throw sls::RuntimeError("No Interface added to Arping");
        }
        runningFlag = true;

        // create thread
        try {
            std::thread temp = std::thread(&ThreadArping::RunningThread, this);
            threadObject = temp.native_handle();
            temp.detach();
        } catch (...) {
            throw sls::RuntimeError("Could not create arping thread");
        }
    }
}

void ThreadArping::StopRunning() {
    pthread_cancel(threadObject);
    LOG(logINFOBLUE) << "Killing [ Arping Thread, Tid: " << threadId << "]";
}
runningFlag = false;
}

void ThreadArping::ClearIpsAndInterfaces() { arpInterfaceIp.clear(); }

void ThreadArping::AddIpsAndInterfaces(std::string interface, std::string ip) {
    arpInterfaceIp.push_back(std::make_pair(interface, ip));
}

void ThreadArping::RunningThread() {

    threadId = syscall(SYS_gettid);
    LOG(logINFOBLUE) << "Created [ Arping Thread, Tid: " << threadId << "]";

    while (IsRunning()) {
        LOG(logINFOBLUE) << "Going to sleep";

        // wait for 60s
        usleep(60 * 1000 * 1000);
    }

    LOG(logINFOBLUE) << "Exiting [ Arping Thread, Tid: " << threadId << "]";
}