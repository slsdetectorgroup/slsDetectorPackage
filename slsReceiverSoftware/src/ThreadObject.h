// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once
/************************************************
 * @file ThreadObject.h
 * @short creates/destroys a thread
 ***********************************************/
/**
 *@short creates/destroys a thread
 */

#include "sls/logger.h"
#include "sls/sls_detector_defs.h"

#include <atomic>
#include <semaphore.h>
#include <string>
#include <thread>

namespace sls {

class ThreadObject : private virtual slsDetectorDefs {
  protected:
    const int index{0};

  public:
    ThreadObject(int index, std::string type);
    virtual ~ThreadObject();
    pid_t GetThreadId() const;
    bool IsRunning() const;
    void StartRunning();
    void StopRunning();
    void Continue();
    void SetThreadPriority(int priority);

  private:
    virtual void ThreadExecution() = 0;
    /**
     * Thread called:  An infinite while loop in which,
     * semaphore starts executing its contents as long RunningMask is satisfied
     * Then it exits the thread on its own if killThread is true
     */
    void RunningThread();

    std::atomic<bool> killThread{false};
    std::atomic<bool> runningFlag{false};
    std::thread threadObject;
    sem_t semaphore;
    const std::string type;
    std::atomic<pid_t> threadId{0};
};

} // namespace sls
