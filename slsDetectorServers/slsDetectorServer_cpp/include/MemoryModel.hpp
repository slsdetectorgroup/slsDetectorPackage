#pragma once
#include "fmt/format.h"
#include "sls/logger.h"
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
    VirtualMemoryModel(const uint32_t IPcore_base_address,
                       const size_t size_memory_space_)
        : IPCore_base_address(IPcore_base_address),
          size_memory_space(size_memory_space_) {}

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

    /// @brief offset of the IP core base address in the memory space, used for
    /// mapping
    const size_t IPCore_base_address{0};

    /// @brief size mapped memory region [bytes]
    const size_t size_memory_space{0};
};

} // namespace sls