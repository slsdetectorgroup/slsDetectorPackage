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
        threads.clear();
        threadIds.clear();
        runningFlag = true;

        // create threadss
        for (auto arp : arpInterfaceIp) {
            try {
                std::thread temp =
                    std::thread(&ThreadArping::RunningThread, this,
                                threads.size(), arp.first, arp.second);
                threads.push_back(temp.native_handle());
                temp.detach();
            } catch (...) {
                StopRunning();
                throw sls::RuntimeError("Could not create arping thread [" +
                                        arp.first + ", " + arp.second + "]");
            }
        }
    }
}

void ThreadArping::StopRunning() {
    int i = 0;
    for (auto t : threads) {
        pthread_cancel(t);
        LOG(logINFOBLUE) << "Killing [ Arping Thread " << i << ": ("
                         << arpInterfaceIp[i].first << ", "
                         << arpInterfaceIp[i].second << ")]";
        ++i;
    }
    threads.clear();
    runningFlag = false;
}

void ThreadArping::ClearIpsAndInterfaces() { arpInterfaceIp.clear(); }

void ThreadArping::AddIpsAndInterfaces(std::string interface, std::string ip) {
    arpInterfaceIp.push_back(std::make_pair(interface, ip));
}

void ThreadArping::RunningThread(int index, std::string interface,
                                 std::string ip) {
    pid_t threadId = syscall(SYS_gettid);
    LOG(logINFOBLUE) << "Created [ Arping Thread " << index << ": ("
                     << interface << ", " << ip << ") Tid: " << threadId << "]";
    {
        std::lock_guard<std::mutex> lock(&mutexIds);
        threadIds.push_back(threadId);
    }

    while (IsRunning()) {
        LOG(logINFOBLUE) << "Going to sleep apring id " << threadId;
        // wait for 60s
        usleep(60 * 1000 * 1000);
    }

    LOG(logINFOBLUE) << "Exiting [ Arping Thread " << index << ": ("
                     << interface << ", " << ip << ") Tid: " << threadId << "]";
}