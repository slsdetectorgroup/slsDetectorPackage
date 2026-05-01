#include "fmt/format.h"
#include <cstdint>
#include <memory>

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
class VirtualMemoryModel {

  public:
    VirtualMemoryModel(const uint32_t IPcore_base_address,
                       const size_t size_memory_space_);

    ~VirtualMemoryModel() = default;

    VirtualMemoryModel(const VirtualMemoryModel &) = delete;
    VirtualMemoryModel &operator=(const VirtualMemoryModel &) = delete;

    VirtualMemoryModel(VirtualMemoryModel &&) noexcept = default;
    VirtualMemoryModel &
    operator=(VirtualMemoryModel &&) = delete; // const members

    void mapToMemory();

    uint32_t *getMappedMemoryPtr() const;

  private:
    std::unique_ptr<uint32_t[]> mapped_memory_ptr;

    /// @brief offset of the IP core base address in the memory space, used for
    /// mapping
    const size_t IPCore_base_address{0};

    /// @brief size mapped memory region [bytes]
    const size_t size_memory_space{0};
};
