// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once

/**
 * @file counting_semaphore.h
 * @short C++17 backport of std::counting_semaphore and std::binary_semaphore
 *
 * Mirrors the C++20 <semaphore> types so that the project can switch by
 * replacing this include with <semaphore> and sls:: with std::. Unnamed POSIX
 * semaphores (sem_init) cannot be used instead as they are not implemented on
 * macOS.
 *
 * Differences to the std types:
 *   - try_acquire_for and try_acquire_until are not implemented
 *   - the constructor is not constexpr
 *   - try_acquire never fails spuriously (std allows it to)
 *   - the preconditions (0 <= desired <= max() and
 *     0 <= update <= max() - counter) are not checked, as in std violating
 *     them is undefined behaviour
 *
 * Built on std::mutex and std::condition_variable, therefore NOT
 * async-signal-safe. Do not call release() from a signal handler.
 */

#include <condition_variable>
#include <cstddef>
#include <limits>
#include <mutex>

namespace sls {

template <std::ptrdiff_t LeastMaxValue =
              std::numeric_limits<std::ptrdiff_t>::max()>
class counting_semaphore {
    static_assert(LeastMaxValue >= 0, "LeastMaxValue must be non-negative");

  public:
    static constexpr std::ptrdiff_t max() noexcept { return LeastMaxValue; }

    explicit counting_semaphore(std::ptrdiff_t desired) : count_(desired) {}

    counting_semaphore(const counting_semaphore &) = delete;
    counting_semaphore &operator=(const counting_semaphore &) = delete;

    void acquire() {
        std::unique_lock<std::mutex> lk(mtx_);
        cv_.wait(lk, [this] { return count_ > 0; });
        --count_;
    }

    bool try_acquire() noexcept {
        std::lock_guard<std::mutex> lk(mtx_);
        if (count_ == 0)
            return false;
        --count_;
        return true;
    }

    void release(std::ptrdiff_t update = 1) {
        // notify while holding the lock: a thread returning from acquire()
        // may destroy the semaphore, which must not happen before we are done
        // with the condition variable
        std::lock_guard<std::mutex> lk(mtx_);
        count_ += update;
        if (update == 1)
            cv_.notify_one();
        else
            cv_.notify_all();
    }

  private:
    std::mutex mtx_;
    std::condition_variable cv_;
    std::ptrdiff_t count_;
};

using binary_semaphore = counting_semaphore<1>;

} // namespace sls
