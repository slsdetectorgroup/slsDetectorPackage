// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package

#include "Arping.h"

#include <iostream>

const std::string Arping::ThreadType = "Arping";

Arping::Arping(nt ind) : ThreadObject(ind, ThreadType) {}

Arping::~Arping() = default;

void Arping::ClearIpsAndInterfaces() {
    arpInterfaceIp.clear();
    commands.clear();
}

void Arping::AddInterfacesAndIps(std::string interface, std::string ip) {
    // create commands to arping
    std::ostringstream os;
    os << "arping -c 1 -U -I " << interface << " " << ip;
    // to read error messages
    os << " 2>&1";
    std::string cmd = os.str();
    arpingCommands.push_back(cmd);
}

void Arping::ThreadExecution() {
    // arping

    // wait for 60s
    usleep(60 * 1000 * 1000);
}

LOG(logINFOBLUE) << "Exiting [ Arping Thread, Tid: " << threadId << " ]";
}

void Arping::ExecuteCommands() {
    for (auto cmd : commands) {
        LOG(logDEBUG) << "Executing Arping Command: " << cmd;

        // execute command
        FILE *sysFile = popen(cmd.c_str(), "r");
        if (sysFile == NULL) {
            LOG(logERROR) << "Executing cmd [" cmd << " ] Fail:"
                          << "\n\t Popen fail";
            continue;
        }

        // check for errors
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
}