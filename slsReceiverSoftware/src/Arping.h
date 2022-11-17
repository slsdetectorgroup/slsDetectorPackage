// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once
/**
 *@short creates/destroys an ARPing thread to arping the interfaces slsReceiver
is listening to.
 */

#include "receiver_defs.h"
#include "sls/logger.h"

#include <atomic>
#include <thread>

namespace sls {

class Arping {

  public:
    Arping();
    ~Arping();

    void SetInterfacesAndIps(const int index, const std::string &interface,
                             const std::string &ip);
    pid_t GetThreadId() const;
    bool IsRunning() const;
    void StartThread();
    void StopThread();

  private:
    void TestCommands();
    std::string ExecuteCommands();
    void ThreadExecution();

    std::vector<std::string> commands =
        std::vector<std::string>(MAX_NUMBER_OF_LISTENING_THREADS);
    std::atomic<bool> runningFlag{false};
    std::thread t;
    std::atomic<pid_t> threadId{0};
};

} // namespace sls
