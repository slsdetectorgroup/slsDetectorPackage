// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package

#include "Arping.h"

#include <chrono>
#include <signal.h>
#include <sys/wait.h>
#include <thread>
#include <unistd.h>

namespace sls {

// gettid added in glibc 2.30
#if __GLIBC__ == 2 && __GLIBC_MINOR__ < 30
#include <sys/syscall.h>
#define gettid() syscall(SYS_gettid)
#endif

void func(int signum) { wait(NULL); }

Arping::Arping() {}

Arping::~Arping() {
    if (IsRunning()) {
        StopProcess();
    }
}

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
    std::string cmd = os.str();
    commands[index] = cmd;
}

pid_t Arping::GetProcessId() const { return childPid; }

bool Arping::IsRunning() const { return runningFlag; }

void Arping::StartProcess() {
    // to prevent zombies from child processes being killed
    signal(SIGCHLD, func);

    // test once to throw exception if arping failed
    TestForErrors();

    // Needs to be a fork and udp socket deleted after Listening threads
    // done running to prevent udp socket cannot bind because of popen
    // that forks
    childPid = fork();
    // child process
    if (childPid == 0) {
        LOG(logINFOBLUE) << "Created [ Arping Process, Tid: " << gettid()
                         << " ]";
        ProcessExecution();
    }
    // parent process
    else if (childPid > 0) {
        runningFlag = true;
    }
    // error
    else {
        throw RuntimeError("Could not start arping Process");
    }
}

void Arping::StopProcess() {
    LOG(logINFOBLUE) << "Exiting [ Arping Process ]";

    if (kill(childPid, SIGTERM)) {
        throw RuntimeError("Could not kill the arping Process");
    }
    runningFlag = false;
}

void Arping::ProcessExecution() {
    while (true) {
        std::string error = ExecuteCommands();
        // just print (was already tested at Process start)
        if (!error.empty()) {
            LOG(logERROR) << error;
        }
        const auto interval = std::chrono::seconds(60);
        std::this_thread::sleep_for(interval);
    }
}

void Arping::TestForErrors() {
    // atleast one interface must be set up
    if (commands[0].empty()) {
        throw RuntimeError(
            "Could not arping. Interface not set up in arping Process");
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

        LOG(logDEBUG1) << "Executing Arping Command: " << cmd;

        // execute command
        FILE *sysFile = popen(cmd.c_str(), "r");
        if (sysFile == NULL) {
            std::ostringstream os;
            os << "Could not Arping (" << cmd << " ) : Popen fail ("
               << strerror(errno) << ')';
            return os.str();
        }

        // copy output
        char output[MAX_STR_LENGTH] = {0};
        fgets(output, sizeof(output), sysFile);
        output[sizeof(output) - 1] = '\0';

        // check exit status of command
        if (pclose(sysFile)) {
            std::ostringstream os;
            os << "Could not arping (" << cmd << ") : " << strerror(errno);
            return os.str();
        } else {
            LOG(logDEBUG) << output;
        }
    }

    return std::string();
}

} // namespace sls
