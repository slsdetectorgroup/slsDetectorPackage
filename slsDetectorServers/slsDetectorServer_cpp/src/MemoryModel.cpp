#include "MemoryModel.hpp"
#include <fcntl.h>
#include <memory>
#include <stdexcept>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>

HardwareMemoryModel::HardwareMemoryModel(const uint32_t IPcore_base_address,
                                         const size_t size_memory_space_)
    : IPCore_base_address(IPcore_base_address),
      size_memory_space(size_memory_space_) {}

void HardwareMemoryModel::mapToMemory() {

    int fd = open("/dev/mem", O_RDWR | O_SYNC, 0);

    if (fd == -1) {
        throw std::runtime_error("Can't find /dev/mem");
    }

    mapped_memory_ptr = reinterpret_cast<volatile uint32_t *>(
        mmap(nullptr, size_memory_space, PROT_READ | PROT_WRITE, MAP_SHARED, fd,
             IPCore_base_address));

    if (mapped_memory_ptr ==
        reinterpret_cast<volatile uint32_t *>(MAP_FAILED)) {
        throw std::runtime_error(
            fmt::format("Failed to map base address: {}",
                        IPCore_base_address)); // TODO: needs ToString
    }

    close(fd);
}

volatile uint32_t *HardwareMemoryModel::getMappedMemoryPtr() const {
    return mapped_memory_ptr;
}

void HardwareMemoryModel::unmapMemory() {

    if (mapped_memory_ptr != nullptr) {
        if (munmap(const_cast<uint32_t *>(mapped_memory_ptr),
                   size_memory_space) < 0) {
            throw std::runtime_error(
                fmt::format("Failed to unmap memory for IP core: {}",
                            IPCore_base_address)); // TODO: needs ToString
        }
        mapped_memory_ptr = nullptr;
    }
}

HardwareMemoryModel::~HardwareMemoryModel() { unmapMemory(); }

VirtualMemoryModel::VirtualMemoryModel(const uint32_t IPcore_base_address,
                                       const size_t size_memory_space_)
    : IPCore_base_address(IPcore_base_address),
      size_memory_space(size_memory_space_) {}

void VirtualMemoryModel::mapToMemory() {

    mapped_memory_ptr =
        std::make_unique<uint32_t[]>(size_memory_space / sizeof(uint32_t));
}

uint32_t *VirtualMemoryModel::getMappedMemoryPtr() const {
    return mapped_memory_ptr.get();
}