// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once
/**
 *@short creates/destroys an ARPing thread to ping the interfaces slsReceiver is
listening to.
 */

#include "sls/logger.h"
#include "sls/sls_detector_defs.h"

#include <atomic>
#include <string>
#include <thread>
#include <utility> // pair, make_pair

class ThreadArping : private virtual slsDetectorDefs {

  private:
    std::atomic<bool> killThread{false};
    std::atomic<bool> runningFlag{false};

    pthread_t threadObject;
    std::vector<std::pair<std::string, std::string>> arpInterfaceIp;
    std::vector<std::string> commands;
    pid_t threadId;

  public:
    ThreadArping();
    virtual ~ThreadArping();
    pid_t GetThreadId() const;
    bool IsRunning() const;
    void StartRunning();
    void StopRunning();
    void ClearIpsAndInterfaces();
    void AddIpsAndInterfaces(std::string interface, std::string ip);

  private:
    /**
     * Thread called:  An infinite while loop that runs arping as long as
     * RunningMask is satisfied Then it exits the thread on its own if
     * killThread is true
     */
    void RunningThread();
};
