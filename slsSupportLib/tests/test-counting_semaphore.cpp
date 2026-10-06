// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "catch.hpp"
#include "sls/counting_semaphore.h"

#include <atomic>
#include <chrono>
#include <cstddef>
#include <thread>
#include <type_traits>
#include <vector>

namespace sls {

// Only what std::counting_semaphore guarantees is tested, so that the tests
// also pass with the std types once the project moves to C++20.

static_assert(!std::is_copy_constructible<counting_semaphore<>>::value,
              "semaphore must not be copyable");
static_assert(!std::is_copy_assignable<counting_semaphore<>>::value,
              "semaphore must not be copyable");
static_assert(!std::is_move_constructible<counting_semaphore<>>::value,
              "semaphore must not be movable");
static_assert(
    std::is_same<binary_semaphore, counting_semaphore<1>>::value,
    "binary_semaphore is a counting_semaphore with a least max value of 1");

TEST_CASE("max is at least the least max value", "[support][semaphore]") {
    CHECK(binary_semaphore::max() >= 1);
    CHECK(counting_semaphore<5>::max() >= 5);
    CHECK(counting_semaphore<>::max() >= 1);
}

TEST_CASE("try_acquire takes what the semaphore was constructed with",
          "[support][semaphore]") {
    counting_semaphore<> s(2);
    CHECK(s.try_acquire());
    CHECK(s.try_acquire());
    CHECK_FALSE(s.try_acquire());
}

TEST_CASE("try_acquire fails on an empty semaphore", "[support][semaphore]") {
    binary_semaphore s{0};
    CHECK_FALSE(s.try_acquire());
    s.release();
    CHECK(s.try_acquire());
    CHECK_FALSE(s.try_acquire());
}

TEST_CASE("release with an update count adds that many",
          "[support][semaphore]") {
    counting_semaphore<> s(0);
    s.release(3);
    s.release();
    for (int i = 0; i != 4; ++i) {
        CHECK(s.try_acquire());
    }
    CHECK_FALSE(s.try_acquire());
}

TEST_CASE("acquire does not block when the count is positive",
          "[support][semaphore]") {
    counting_semaphore<> s(1);
    s.acquire();
    CHECK_FALSE(s.try_acquire());
}

TEST_CASE("acquire blocks until release", "[support][semaphore]") {
    binary_semaphore s{0};
    std::atomic<bool> acquired{false};
    std::thread t([&] {
        s.acquire();
        acquired = true;
    });

    // cannot have passed acquire, no matter how the thread is scheduled
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    CHECK_FALSE(acquired);

    s.release();
    t.join();
    CHECK(acquired);
    CHECK_FALSE(s.try_acquire());
}

TEST_CASE("release with an update count wakes up that many waiting threads",
          "[support][semaphore]") {
    const int nThreads = 4;
    counting_semaphore<> s(0);
    std::atomic<int> acquired{0};
    std::vector<std::thread> threads;
    for (int i = 0; i != nThreads; ++i) {
        threads.emplace_back([&] {
            s.acquire();
            ++acquired;
        });
    }
    s.release(nThreads);
    for (auto &t : threads) {
        t.join();
    }
    CHECK(acquired == nThreads);
    CHECK_FALSE(s.try_acquire());
}

TEST_CASE("every release is acquired exactly once", "[support][semaphore]") {
    const int nProducers = 3;
    const int nConsumers = 3;
    const int nPerThread = 2000;
    counting_semaphore<> s(0);
    std::atomic<int> acquired{0};
    std::vector<std::thread> threads;
    for (int i = 0; i != nConsumers; ++i) {
        threads.emplace_back([&] {
            for (int j = 0; j != nPerThread; ++j) {
                s.acquire();
                ++acquired;
            }
        });
    }
    for (int i = 0; i != nProducers; ++i) {
        threads.emplace_back([&] {
            for (int j = 0; j != nPerThread; ++j) {
                s.release();
            }
        });
    }
    for (auto &t : threads) {
        t.join();
    }
    CHECK(acquired == nConsumers * nPerThread);
    CHECK_FALSE(s.try_acquire());
}

TEST_CASE("a waiter can destroy the semaphore as soon as it has acquired",
          "[support][semaphore]") {
    for (int i = 0; i != 200; ++i) {
        auto *done = new binary_semaphore(0);
        std::thread worker([done] { done->release(); });
        done->acquire();
        delete done;
        worker.join();
    }
    SUCCEED();
}

} // namespace sls
