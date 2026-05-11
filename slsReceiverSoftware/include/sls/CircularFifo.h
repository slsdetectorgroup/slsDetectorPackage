// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once
/* CircularFifo.h
 * Code & platform dependent issues with it was originally
 * published at http://www.kjellkod.cc/threadsafecircularqueue
 * 2009-11-02
 * @author Kjell Hedstr�m, hedstrom@kjellkod.cc
 * modified by the sls detector group
 * */

#include "sls/thread_utils.h"

#include <atomic>
#include <cstddef>
#include <vector>

namespace sls {

/** Circular Fifo (a.k.a. Circular Buffer)
 * Thread safe for one reader, and one writer */
template <typename Element> class CircularFifo {
  private:
    size_t tail{0};
    size_t head{0};
    size_t capacity;
    std::vector<Element *> data;
    counting_semaphore data_sem; // # of items available to read
    counting_semaphore free_sem; // # of slots available to write
    std::atomic<int> count_{0};  // current number of items, for diagnostics

    size_t increment(size_t i) const { return (i + 1) % capacity; }

  public:
    explicit CircularFifo(size_t size)
        : capacity(size + 1), data(capacity), data_sem(0),
          free_sem(static_cast<int>(size)) {}

    CircularFifo(const CircularFifo &) = delete;
    CircularFifo(CircularFifo &&) = delete;

    virtual ~CircularFifo() = default;

    bool push(Element *&item, bool no_block = false);
    bool pop(Element *&item, bool no_block = false);

    bool isEmpty() const;
    bool isFull() const;

    int getDataValue() const;
    int getFreeValue() const;
};

template <typename Element> int CircularFifo<Element>::getDataValue() const {
    return count_.load(std::memory_order_relaxed);
}

template <typename Element> int CircularFifo<Element>::getFreeValue() const {
    return static_cast<int>(capacity - 1) - getDataValue();
}

/** Producer only: Adds item to the circular queue.
 * If queue is full at 'push' operation no update/overwrite
 * will happen, it is up to the caller to handle this case
 *
 * \param item copy by reference the input item
 * \param no_block if true, return immediately if fifo is full
 * \return whether operation was successful or not */
template <typename Element>
bool CircularFifo<Element>::push(Element *&item, bool no_block) {
    if (no_block) {
        if (!free_sem.try_acquire())
            return false;
    } else {
        free_sem.acquire();
    }
    data[tail] = item;
    tail = increment(tail);
    count_.fetch_add(1, std::memory_order_relaxed);
    data_sem.release();
    return true;
}

/** Consumer only: Removes and returns item from the queue
 * If queue is empty at 'pop' operation no retrieve will happen
 * It is up to the caller to handle this case
 *
 * \param item return by reference the wanted item
 * \param no_block if true, return immediately if fifo is full
 * \return whether operation was successful or not */
template <typename Element>
bool CircularFifo<Element>::pop(Element *&item, bool no_block) {
    if (no_block) {
        if (!data_sem.try_acquire())
            return false;
    } else {
        data_sem.acquire();
    }
    item = data[head];
    head = increment(head);
    count_.fetch_sub(1, std::memory_order_relaxed);
    free_sem.release();
    return true;
}

/** Useful for testing and Consumer check of status
 * Remember that the 'empty' status can change quickly
 * as the Producer adds more items.
 *
 * \return true if circular buffer is empty */
template <typename Element> bool CircularFifo<Element>::isEmpty() const {
    return getDataValue() == 0;
}

/** Useful for testing and Producer check of status
 * Remember that the 'full' status can change quickly
 * as the Consumer catches up.
 *
 * \return true if circular buffer is full.  */
template <typename Element> bool CircularFifo<Element>::isFull() const {
    return getFreeValue() == 0;
}

} // namespace sls
