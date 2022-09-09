// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package

#include "sls/ImageFifo.h"
#include <memory>
#include <thread>
#include <cstdlib> //posix_memalign

namespace sls {

//As long as we don't use aligned writes we 
//can also allocate with a normal new
constexpr size_t IO_ALIGNMENT = 4096;

ImageFifo::ImageFifo(size_t fifo_size, size_t image_size)
    : fifo_size_(fifo_size), image_size_(image_size),
      free_slots(static_cast<uint32_t>(fifo_size_)),
      filled_slots(static_cast<uint32_t>(fifo_size_)) {

    posix_memalign(reinterpret_cast<void **>(&data), IO_ALIGNMENT,
                   fifo_size_ * image_size_);

    ImageView v;
    for (size_t i = 0; i < fifo_size_; ++i) {
        v.data = data + i * image_size_;
        free_slots.push(v);
    }
}

ImageFifo::~ImageFifo() { free(data); }
size_t ImageFifo::size() const noexcept { return fifo_size_; }
size_t ImageFifo::image_size() const noexcept { return image_size_; }

size_t ImageFifo::numFilledSlots() const noexcept {
    return filled_slots.sizeGuess();
}

size_t ImageFifo::numFreeSlots() const noexcept {
    return free_slots.sizeGuess();
}

ImageView ImageFifo::pop_free() {
    ImageView v;
    while (!free_slots.pop(v))
        ;
    return v;
}

ImageView ImageFifo::pop_image() {
    ImageView v;
    while (!filled_slots.pop(v))
        ;
    return v;
}

ImageView ImageFifo::pop_image(std::chrono::nanoseconds wait) {
    ImageView v;
    while (!filled_slots.pop(v)) {
        std::this_thread::sleep_for(wait);
    }
    return v;
}

ImageView ImageFifo::pop_image(std::chrono::nanoseconds wait,
                               std::atomic<bool> &stopped) {
    ImageView v;
    while (!filled_slots.pop(v) && !stopped) {
        std::this_thread::sleep_for(wait);
    }
    return v;
}

bool ImageFifo::try_pop_image(ImageView &img) { return filled_slots.pop(img); }

void ImageFifo::push_image(ImageView &v) {
    while (!filled_slots.push(v))
        ;
}

void ImageFifo::push_free(ImageView &v) {
    while (!free_slots.push(v))
        ;
}

} // namespace sls