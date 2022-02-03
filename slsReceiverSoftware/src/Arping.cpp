// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package

#include "Arping.h"

#include <chrono>
#include <sys/syscall.h>
#include <unistd.h>

void Arping::ClearIpsAndInterfaces() { commands.clear(); }

void Arping::AddInterfacesAndIps(const std::string &interface,
                                 const std::string &ip) {
    // create commands to arping
    std::ostringstream os;
    os << "arping -c 1 -U -I " << interface << " " << ip;
    // to read error messages
    os << " 2>&1";
    std::string cmd = os.str();
    commands.push_back(cmd);
}

pid_t Arping::GetThreadId() const { return threadId; }

bool Arping::IsRunning() const { return runningFlag; }

void Arping::StartThread() {
    TestCommands();
    try {
        t = std::thread(&Arping::ThreadExecution, this);
    } catch (...) {
        throw sls::RuntimeError("Could not start arping thread");
    }
    runningFlag = true;
}

void Arping::StopThread() {
    runningFlag = false;
    t.join();
}

void Arping::ThreadExecution() {
    threadId = syscall(SYS_gettid);
    LOG(logINFOBLUE) << "Created [ Arping Thread, Tid: " << threadId << "]";

    while (runningFlag) {
        std::string error = ExecuteCommands();
        // just print (was already tested at thread start)
        if (!error.empty()) {
            LOG(logERROR) << error;
        }

        // wait for 60s as long as thread not killed
        int nsecs = 0;
        while (runningFlag && nsecs != 60) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            ++nsecs;
        }
    }

    LOG(logINFOBLUE) << "Exiting [ Arping Thread, Tid: " << threadId << " ]";
    threadId = 0;
}

void Arping::TestCommands() {
    std::string error = ExecuteCommands();
    if (!error.empty()) {
        throw sls::RuntimeError(error);
    }
}

std::string Arping::ExecuteCommands() {
    for (auto cmd : commands) {
        LOG(logDEBUG) << "Executing Arping Command: " << cmd;

        // execute command
        FILE *sysFile = popen(cmd.c_str(), "r");
        if (sysFile == NULL) {
            std::ostringstream os;
            os << "Could not Arping [" << cmd << " ] : Popen fail";
            return os.str();
        }

        // copy output
        char output[MAX_STR_LENGTH] = {0};
        fgets(output, sizeof(output), sysFile);
        output[sizeof(output) - 1] = '\0';

        // check exit status of command
        if (pclose(sysFile)) {
            std::ostringstream os;
            os << "Could not arping[" << cmd << "] : " << output;
            return os.str();
        } else {
            LOG(logDEBUG) << output;
        }
    }

    return std::string();
}