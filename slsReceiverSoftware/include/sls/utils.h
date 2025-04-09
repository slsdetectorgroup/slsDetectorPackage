/**
 * @file utils.cpp
 * @short utility objects for Receiver
 */

#include <cstdint>
#include <memory>

namespace sls {

/*
 * AlignedData
 * Aligns data to a given type T with proper alignment
 * @param data: pointer to data
 * @param size: size of data to align in bytes
 */
template <typename T> struct AlignedData {
    T *aligned_ptr; // aligned data pointer

    AlignedData(char *data, size_t size) {
        if (reinterpret_cast<uintptr_t>(data) % alignof(uint64_t) == 0) {
            // If aligned directly cast to pointer
            aligned_ptr = reinterpret_cast<T *>(data);

        } else {

            auto alignedbuffer = std::aligned_alloc(alignof(T), size);
            std::memcpy(alignedbuffer, data, size);
            aligned_ptr = reinterpret_cast<T *>(alignedbuffer);
        }
    }

    ~AlignedData() { std::free(aligned_ptr); }
};

} // namespace sls