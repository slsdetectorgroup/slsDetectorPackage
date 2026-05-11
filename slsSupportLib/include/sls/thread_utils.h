// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once

/**
 * @file thread_utils.h
 * @short Portable per-thread identifier for diagnostics/logging.
 *
 * Returns a unique-within-process thread id of the calling thread:
 *   - Linux, glibc >= 2.30: the system gettid() (pid_t).
 *   - Linux, glibc <  2.30: syscall(SYS_gettid) (long, cast to pid_t).
 *   - macOS:                pthread_threadid_np() (uint64_t, truncated to pid_t).
 *
 * The macOS kernel thread id is 64-bit. To stay drop-in compatible with the
 * existing call sites that store the result in pid_t (e.g. ThreadObject::
 * threadId, ClientInterface::parentThreadId / tcpThreadId), we truncate to
 * pid_t. The value is used for diagnostics only and is still unique enough
 * within a single process.
 */

#include <condition_variable>
#include <mutex>
#include <sys/types.h> // pid_t

#if defined(__APPLE__)
#include <cstdint>
#include <pthread.h>
#elif defined(__linux__)
#include <unistd.h>
#if !defined(__GLIBC__) ||                                                     \
    !(__GLIBC__ > 2 || (__GLIBC__ == 2 && __GLIBC_MINOR__ >= 30))
#include <sys/syscall.h>
#endif
#endif

namespace sls {

inline pid_t getThreadId() noexcept {
#if defined(__APPLE__)
    std::uint64_t tid = 0;
    pthread_threadid_np(nullptr, &tid);
    return static_cast<pid_t>(tid);
#elif defined(__GLIBC__) &&                                                    \
    (__GLIBC__ > 2 || (__GLIBC__ == 2 && __GLIBC_MINOR__ >= 30))
    return ::gettid();
#else
    return static_cast<pid_t>(::syscall(SYS_gettid));
#endif
}

/**
 * Minimal C++17 backport of the subset of std::binary_semaphore used in this
 * project. API matches std::binary_semaphore so call sites can switch to
 * <semaphore> verbatim once the project moves to C++20. Built on
 * std::mutex + std::condition_variable; therefore NOT async-signal-safe, do
 * not call release() from a signal handler.
 */
class binary_semaphore {
  public:
    explicit binary_semaphore(int desired) : count_(desired) {}

    binary_semaphore(const binary_semaphore &) = delete;
    binary_semaphore &operator=(const binary_semaphore &) = delete;

    void acquire() {
        std::unique_lock<std::mutex> lk(mtx_);
        cv_.wait(lk, [this] { return count_ > 0; });
        --count_;
    }

    void release() {
        {
            std::lock_guard<std::mutex> lk(mtx_);
            ++count_;
        }
        cv_.notify_one();
    }

  private:
    std::mutex mtx_;
    std::condition_variable cv_;
    int count_;
};

} // namespace sls
