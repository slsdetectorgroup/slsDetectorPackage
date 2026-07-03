#pragma once
#include "fmt/format.h"
#include "sls/logger.h"
#include "sls/sls_detector_exceptions.h"
#include <cstdint>
#include <vector>

namespace sls {

/// @brief class to handle memory mapping and access for hardware IP cores
class HardwareMemoryModel {

  public:
    HardwareMemoryModel(const uint32_t IPcore_base_address,
                        const size_t size_memory_space_);

    ~HardwareMemoryModel();

    void mapToMemory();

    void unmapMemory();

    volatile uint32_t *getMappedMemoryPtr() const;

  private:
    volatile uint32_t *mapped_memory_ptr{nullptr};

    /// @brief offset of the IP core base address in the memory space, used for
    /// mapping
    const size_t IPCore_base_address{0};

    /// @brief size mapped memory region [bytes]
    const size_t size_memory_space{0};
};

/// @brief class to handle memory mapping and access for virtual IP cores (e.g.
/// use software implementation of memory)
template <typename DataType> class VirtualMemoryModel {

  public:
    // IPcore_base_address is not used for virtual memory model but kept for
    // compatibility with HardwareMemoryModel interface
    VirtualMemoryModel([[maybe_unused]] const uint32_t IPcore_base_address,
                       const size_t size_memory_space_)
        : size_memory_space(size_memory_space_) {}

    ~VirtualMemoryModel() = default;

    void mapToMemory() {
        mapped_memory.resize(
            size_memory_space /
            sizeof(DataType)); // TODO: should it be zero initialized?
    }

    DataType *getMappedMemoryPtr() { return mapped_memory.data(); }

    const DataType *getMappedMemoryPtr() const { return mapped_memory.data(); }

  private:
    std::vector<DataType> mapped_memory{};

    /// @brief size mapped memory region [bytes]
    const size_t size_memory_space{0};
};

} // namespace sls