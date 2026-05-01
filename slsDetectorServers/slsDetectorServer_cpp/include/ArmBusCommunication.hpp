#pragma once
#include "RegisterHelperStructs.hpp"
#include "fmt/format.h"
#include <cstdint>
#include <fcntl.h>
#include <map>
#include <memory>
#include <stdexcept>
#include <sys/mman.h>
#include <vector>

// TODO: maybe should be templated on address type (e.g. uint32_t or uint64_t)
// for more flexibility?

namespace sls {

template <typename IPCoreEnumType, typename MemoryModel>
struct IpCoreRegisterBlock {

    const std::map<IPCoreEnumType, MemoryModel> &operator()() const {
        return memoryblocks_;
    }

    std::map<IPCoreEnumType, MemoryModel> &operator()() {
        return memoryblocks_;
    }

  private:
    std::map<IPCoreEnumType, MemoryModel> memoryblocks_;
};

template <typename IPCoreEnumType, typename MemoryModel>
class BusCommunication {

  public:
    BusCommunication() = default;

    void mapToMemory();

    uint32_t readRegister(const Register &register_) const;
    void writeRegister(const Register &register_, const uint32_t data) const;

  private:
    /// @brief stores register blocks for each IP core
    IpCoreRegisterBlock<IPCoreEnumType, MemoryModel> ipcoreregisterblocks;

    void bus_w(const uint32_t offset, IPCoreEnumType baseadress,
               const uint32_t data) const;

    uint32_t bus_r(const uint32_t offset, IPCoreEnumType baseadress) const;
};

template <typename IPCoreEnumType, typename MemoryModel>
void BusCommunication<IPCoreEnumType, MemoryModel>::mapToMemory() {

    for (auto &map_elem : ipcoreregisterblocks()) {
        map_elem.second.mapToMemory();
    }
}

template <typename IPCoreEnumType, typename MemoryModel>
uint32_t BusCommunication<IPCoreEnumType, MemoryModel>::readRegister(
    const Register &register_) const {
    return bus_r(register_.offset_in_bytes, register_.ip_core);
}

template <typename IPCoreEnumType, typename MemoryModel>
void BusCommunication<IPCoreEnumType, MemoryModel>::writeRegister(
    const Register &register_, const uint32_t data) const {
    bus_w(register_.offset_in_bytes, register_.ip_core, data);
}

template <typename IPCoreEnumType, typename MemoryModel>
uint32_t BusCommunication<IPCoreEnumType, MemoryModel>::bus_r(
    const uint32_t offset, const IPCoreEnumType baseadress) const {
    auto ptr1 = ipcoreregisterblocks().at(baseadress).getMappedMemoryPtr() +
                offset / (sizeof(uint32_t));
    return *ptr1;
}

template <typename IPCoreEnumType, typename MemoryModel>
void BusCommunication<IPCoreEnumType, MemoryModel>::bus_w(
    const uint32_t offset, const IPCoreEnumType baseadress,
    const uint32_t data) const {
    auto ptr1 = ipcoreregisterblocks().at(baseadress).getMappedMemoryPtr() +
                offset / (sizeof(uint32_t));
    *ptr1 = data;
}

} // namespace sls