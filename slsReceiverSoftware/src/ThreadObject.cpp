// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
/************************************************
 * @file ThreadObject.cpp
 * @short creates/destroys a thread
 ***********************************************/

#include "ThreadObject.h"
#include "sls/container_utils.h"
#include <iostream>
#include <unistd.h>

namespace sls {

// gettid added in glibc 2.30
#if __GLIBC__ == 2 && __GLIBC_MINOR__ < 30
#include <sys/syscall.h>
#define gettid() syscall(SYS_gettid)
#endif

ThreadObject::ThreadObject(int index, std::string type)
    : index(index), type(type) {
    LOG(logDEBUG) << type << " thread created: " << index;
    sem_init(&semaphore, 1, 0);
    try {
        threadObject = std::thread(&ThreadObject::RunningThread, this);
    } catch (...) {
        throw RuntimeError("Could not create " + type + " thread with index " +
                           std::to_string(index));
    }
}

ThreadObject::~ThreadObject() {
    killThread = true;
    sem_post(&semaphore);
    threadObject.join();
    sem_destroy(&semaphore);
}

pid_t ThreadObject::GetThreadId() const { return threadId; }

bool ThreadObject::IsRunning() const { return runningFlag; }

void ThreadObject::StartRunning() { runningFlag = true; }

void ThreadObject::StopRunning() { runningFlag = false; }

void ThreadObject::RunningThread() {
    threadId = gettid();
    LOG(logINFOBLUE) << "Created [ " << type << "Thread " << index
                     << ", Tid: " << threadId << "]";
    while (!killThread) {
        while (IsRunning()) {
            ThreadExecution();
        }
        // wait till the next acquisition
        sem_wait(&semaphore);
    }
    LOG(logINFOBLUE) << "Exiting [ " << type << " Thread " << index
                     << ", Tid: " << threadId << "]";
    threadId = 0;
}

void ThreadObject::Continue() { sem_post(&semaphore); }

void ThreadObject::SetThreadPriority(int priority) {
    struct sched_param param;
    param.sched_priority = priority;
    if (pthread_setschedparam(threadObject.native_handle(), SCHED_FIFO,
                              &param) == EPERM) {
        if (index == 0) {
            LOG(logWARNING) << "Could not prioritize " << type
                            << " thread. "
                               "(No Root Privileges?)";
        }
    } else {
        LOG(logINFO) << "Priorities set - " << type << ": " << priority;
    }
}

} // namespace sls
