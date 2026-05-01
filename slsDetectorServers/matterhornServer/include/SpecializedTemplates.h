#pragma once
#include "ArmBusCommunication.hpp"
#include "RegisterDefs.hpp"

/**
 * @file SpecializedTemplates.h
 * @short contains specializations of the template classes for the Matterhorn
 * server implementation
 */

namespace sls {

template <typename MemoryModel>
struct IpCoreRegisterBlock<IPCore, MemoryModel> {

    std::map<IPCore, MemoryModel> &operator()() {
        memoryblocks_.try_emplace(
            IPCore::MH_RO_SM_AXI,
            MemoryModel{static_cast<uint32_t>(IPCore::MH_RO_SM_AXI),
                        IPCORE_REGISTER_BLOCK_SIZE});
        memoryblocks_.try_emplace(
            IPCore::FHDR_AXI,
            MemoryModel{static_cast<uint32_t>(IPCore::FHDR_AXI),
                        IPCORE_REGISTER_BLOCK_SIZE});
        memoryblocks_.try_emplace(
            IPCore::AURORA_STATUS,
            MemoryModel{static_cast<uint32_t>(IPCore::AURORA_STATUS),
                        IPCORE_REGISTER_BLOCK_SIZE});
        memoryblocks_.try_emplace(
            IPCore::AURORA_STATUS2,
            MemoryModel{static_cast<uint32_t>(IPCore::AURORA_STATUS2),
                        IPCORE_REGISTER_BLOCK_SIZE});
        memoryblocks_.try_emplace(
            IPCore::PACKETIZERREG,
            MemoryModel{static_cast<uint32_t>(IPCore::PACKETIZERREG),
                        IPCORE_REGISTER_BLOCK_SIZE});
        memoryblocks_.try_emplace(
            IPCore::UNKNOWN, MemoryModel{static_cast<uint32_t>(IPCore::UNKNOWN),
                                         IPCORE_REGISTER_BLOCK_SIZE});

        return memoryblocks_;
    }

    const std::map<IPCore, MemoryModel> &operator()() const {
        return memoryblocks_;
    }

  private:
    std::map<IPCore, MemoryModel> memoryblocks_;
};

} // namespace sls