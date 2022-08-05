// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package

#include "Arping.h"

#include <chrono>
#include <unistd.h>

namespace sls {

// gettid added in glibc 2.30
#if __GLIBC__ == 2 && __GLIBC_MINOR__ < 30
#include <sys/syscall.h>
#define gettid() syscall(SYS_gettid)
#endif

void Arping::SetInterfacesAndIps(const int index, const std::string &interface,
                                 const std::string &ip) {

    if (interface.empty() || ip.empty()) {
        throw RuntimeError("Could not arping. Interface name and ip not "
                           "set up for interface " +
                           std::to_string(index));
    }
    // create commands to arping
    std::ostringstream os;
    os << "arping -c 1 -U -I " << interface << " " << ip;
    // to read error messages
    os << " 2>&1";
    std::string cmd = os.str();
    commands[index] = cmd;
}

pid_t Arping::GetThreadId() const { return threadId; }

bool Arping::IsRunning() const { return runningFlag; }

void Arping::StartThread() {
    TestCommands();
    try {
        t = std::thread(&Arping::ThreadExecution, this);
    } catch (...) {
        throw RuntimeError("Could not start arping thread");
    }
    runningFlag = true;
}

void Arping::StopThread() {
    runningFlag = false;
    t.join();
}

void Arping::ThreadExecution() {
    threadId = gettid();
    LOG(logINFOBLUE) << "Created [ Arping Thread, Tid: " << threadId << " ]";

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
    // atleast one interface must be set up
    if (commands[0].empty()) {
        throw RuntimeError(
            "Could not arping. Interface not set up in apring thread");
    }
    // test if arping commands throw an error
    std::string error = ExecuteCommands();
    if (!error.empty()) {
        throw RuntimeError(error);
    }
}

std::string Arping::ExecuteCommands() {
    for (auto cmd : commands) {

        // empty if 2nd interface not enabled
        if (cmd.empty())
            continue;

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

} // namespace sls
