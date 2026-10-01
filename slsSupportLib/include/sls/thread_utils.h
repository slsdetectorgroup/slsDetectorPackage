// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once

/**
 * @file thread_utils.h
 * @short id of the calling thread
 */

#include <sys/types.h> // pid_t

#if defined(__APPLE__)
#include <cstdint>
#include <pthread.h>
#elif defined(__linux__)
#include <sys/syscall.h>
#include <unistd.h>
#else
#error "getThreadId is not implemented for this platform"
#endif

namespace sls {

/** Kernel thread id of the calling thread. On macOS the 64-bit id is
 * truncated to pid_t. */
inline pid_t getThreadId() noexcept {
#if defined(__APPLE__)
    std::uint64_t tid = 0;
    pthread_threadid_np(nullptr, &tid);
    return static_cast<pid_t>(tid);
#else
    return static_cast<pid_t>(::syscall(SYS_gettid));
#endif
}

} // namespace sls
