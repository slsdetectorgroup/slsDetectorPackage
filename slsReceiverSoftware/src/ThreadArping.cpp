// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package

#include "ThreadArping.h"
#include "sls/container_utils.h"
#include <iostream>
#include <sys/syscall.h>
#include <unistd.h>

ThreadArping::ThreadArping() {}

ThreadArping::~ThreadArping() { StopRunning(); }

pid_t ThreadArping::GetThreadId() const { return threadId; }

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
    LOG(logINFOBLUE) << "Killing [ Arping Thread, Tid: " << threadId << " ]";
    runningFlag = false;
}

void ThreadArping::ClearIpsAndInterfaces() { arpInterfaceIp.clear(); }

void ThreadArping::AddIpsAndInterfaces(std::string interface, std::string ip) {
    arpInterfaceIp.push_back(std::make_pair(interface, ip));
}

void ThreadArping::RunningThread() {

    threadId = syscall(SYS_gettid);
    {
        std::ostringstream os;
        os << "Created [ Arping Thread, Tid: " << threadId << " ] for ";
        for (auto ethip : arpInterfaceIp) {
            os << "\n\t[ " << ethip.first << ", " << ethip.second << " ]";
        }
        LOG(logINFOBLUE) << os.str();
    }

    // create the commands to ping necessary interfaces
    std::vector<std::string> commands;
    for (auto ethip : arpInterfaceIp) {
        std::ostringstream os;
        os << "arping -c 1 -U -I " << ethip.first << " " << ethip.second;
        // to read error messages
        os << " 2>&1";
        std::string cmd = os.str();
        commands.push_back(cmd);
    }

    while (IsRunning()) {

        // arping
        for (auto cmd : commands) {
            LOG(logDEBUG) << "Executing Arping Command: " << cmd;

            // execute command and check for errors
            FILE *sysFile = popen(cmd.c_str(), "r");
            char output[MAX_STR_LENGTH] = {0};
            fgets(output, sizeof(output), sysFile);
            output[sizeof(output) - 1] = '\0';
            if (pclose(sysFile)) {
                LOG(logERROR) << "Executing cmd[" << cmd
                              << "]\n\tError Message : " << output;
            } else {
                LOG(logDEBUG) << output;
            }
        }

        // wait for 60s
        usleep(60 * 1000 * 1000);
    }

    LOG(logINFOBLUE) << "Exiting [ Arping Thread, Tid: " << threadId << " ]";
}