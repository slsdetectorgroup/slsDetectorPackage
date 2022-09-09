// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package

#pragma once
#include "sls/ImageView.h"
#include "sls/SimpleQueue.h"
#include <chrono>

namespace sls {
class ImageFifo {
    size_t fifo_size_;
    size_t image_size_;
    SimpleQueue<ImageView> free_slots;
    SimpleQueue<ImageView> filled_slots;
    char *data;

  public:
    ImageFifo(size_t fifo_size, size_t image_size);
    ~ImageFifo();

    size_t size() const noexcept;
    size_t image_size() const noexcept;
    size_t numFilledSlots() const noexcept;
    size_t numFreeSlots() const noexcept;

    ImageView pop_free();
    ImageView pop_image();
    ImageView pop_image(std::chrono::nanoseconds wait);
    ImageView pop_image(std::chrono::nanoseconds wait,
                        std::atomic<bool> &stopped);
    bool try_pop_image(ImageView &img);
    void push_image(ImageView &v);
    void push_free(ImageView &v);
};
} // namespace sls