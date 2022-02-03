// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once
/**
 *@short creates/destroys an ARPing thread to arping the interfaces slsReceiver
is listening to.
 */

#include "sls/logger.h"
#include "sls/sls_detector_defs.h"

#include <atomic>
#include <thread>

class Arping : private virtual slsDetectorDefs {

  public:
    void ClearIpsAndInterfaces();
    void AddInterfacesAndIps(const std::string &interface,
                             const std::string &ip);
    pid_t GetThreadId() const;
    bool IsRunning() const;
    void StartThread();
    void StopThread();

  private:
    void TestCommands();
    std::string ExecuteCommands();
    void ThreadExecution();

    std::vector<std::string> commands;
    std::atomic<bool> runningFlag{false};
    std::thread t;
    pid_t threadId{0};
};
