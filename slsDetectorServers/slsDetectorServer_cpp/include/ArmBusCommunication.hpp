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

// TODO: maybe should be templated on address type (e.g. uint32_t register or
// uint64_t register) for more flexibility?

namespace sls {

template <typename IPCores, typename MemoryModel> class BusCommunication {

    using IPCoreEnumType = typename IPCores::ipcore_enum_type;

  public:
    BusCommunication();

    void mapToMemory();

    uint32_t readRegister(const Register &register_) const;
    void writeRegister(const Register &register_, const uint32_t data);

  private:
    void bus_w(const uint32_t offset, IPCoreEnumType baseadress,
               const uint32_t data);

    uint32_t bus_r(const uint32_t offset, IPCoreEnumType baseadress) const;

    /// @brief map from id of IP Core to memory model for the register block of
    /// the IP core
    std::map<IPCoreEnumType, MemoryModel> ipcoreregisterblocks{};
};

template <typename IPCores, typename MemoryModel>
BusCommunication<IPCores, MemoryModel>::BusCommunication() {

    for (const auto &ip_core : IPCores::ipcores) {
        ipcoreregisterblocks.emplace(ip_core,
                                     MemoryModel{static_cast<uint32_t>(ip_core),
                                                 IPCores::ip_core_block_size});
    }
}

template <typename IPCores, typename MemoryModel>
void BusCommunication<IPCores, MemoryModel>::mapToMemory() {

    for (auto &map_elem : ipcoreregisterblocks) {
        map_elem.second.mapToMemory();
    }
}

template <typename IPCores, typename MemoryModel>
uint32_t BusCommunication<IPCores, MemoryModel>::readRegister(
    const Register &register_) const {
    return bus_r(register_.offset_in_bytes, register_.ip_core);
}

template <typename IPCores, typename MemoryModel>
void BusCommunication<IPCores, MemoryModel>::writeRegister(
    const Register &register_, const uint32_t data) {
    bus_w(register_.offset_in_bytes, register_.ip_core, data);
}

template <typename IPCores, typename MemoryModel>
uint32_t BusCommunication<IPCores, MemoryModel>::bus_r(
    const uint32_t offset, const IPCoreEnumType baseadress) const {
    auto ptr1 = ipcoreregisterblocks.at(baseadress).getMappedMemoryPtr() +
                offset / (sizeof(uint32_t));
    return *ptr1;
}

template <typename IPCores, typename MemoryModel>
void BusCommunication<IPCores, MemoryModel>::bus_w(
    const uint32_t offset, const IPCoreEnumType baseadress,
    const uint32_t data) {
    auto ptr1 = ipcoreregisterblocks.at(baseadress).getMappedMemoryPtr() +
                offset / (sizeof(uint32_t));
    *ptr1 = data;
}

} // namespace sls