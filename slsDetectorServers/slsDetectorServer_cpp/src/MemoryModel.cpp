#include "MemoryModel.hpp"
#include <fcntl.h>
#include <memory>
#include <stdexcept>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>

namespace sls {

HardwareMemoryModel::HardwareMemoryModel(const uint32_t IPcore_base_address,
                                         const size_t size_memory_space_)
    : IPCore_base_address(IPcore_base_address),
      size_memory_space(size_memory_space_) {}

void HardwareMemoryModel::mapToMemory() {

    int fd = open("/dev/mem", O_RDWR | O_SYNC, 0);

    if (fd == -1) {
        throw RuntimeError("Can't find /dev/mem");
    }

    auto void_mmap_ptr =
        mmap(nullptr, size_memory_space, PROT_READ | PROT_WRITE, MAP_SHARED, fd,
             IPCore_base_address);

    if (void_mmap_ptr == MAP_FAILED) {
        throw RuntimeError(
            fmt::format("Failed to map base address: {}",
                        IPCore_base_address)); // TODO: needs ToString
    }

    mapped_memory_ptr = reinterpret_cast<volatile uint32_t *>(void_mmap_ptr);

    close(fd);
}

volatile uint32_t *HardwareMemoryModel::getMappedMemoryPtr() const {
    return mapped_memory_ptr;
}

void HardwareMemoryModel::unmapMemory() {

    if (mapped_memory_ptr != nullptr) {
        if (munmap(reinterpret_cast<void *>(
                       const_cast<uint32_t *>(mapped_memory_ptr)),
                   size_memory_space) < 0) {
            LOG(logWARNING)
                << fmt::format("Failed to unmap memory for IP core: {}",
                               IPCore_base_address); // TODO: needs ToString
        }
        mapped_memory_ptr = nullptr;
    }
}

HardwareMemoryModel::~HardwareMemoryModel() {
    LOG(logDEBUG1) << "HardwareMemoryModel destructor called, unmapping memory";
    unmapMemory();
}

} // namespace sls