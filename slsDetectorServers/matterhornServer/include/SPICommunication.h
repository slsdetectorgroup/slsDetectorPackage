#include "MemoryModel.hpp"
#include "SPIRegisterDefs.hpp"
#include "SPIRegisterHelperStructs.hpp"
#include "fmt/format.h"
#include "sls/logger.h"
#include <map>
#include <vector>

namespace sls {

/// @brief abstract base class for SPI communication
template <typename DerivedSPIModel> class SPICommunication {

  public:
    SPICommunication() = default;

    ~SPICommunication();

    std::vector<std::byte> SPIread(const SPIRegister &spi_reg,
                                   const uint8_t chip_id) const;

    void SPIwrite(const SPIRegister &spi_reg, const uint8_t chip_id,
                  const std::vector<std::byte> &data);

    void mapToMemory();

    void unmapMemory();
};

template <typename DerivedSPIModel>
SPICommunication<DerivedSPIModel>::~SPICommunication() {
    unmapMemory();
}

template <typename DerivedSPIModel>
void SPICommunication<DerivedSPIModel>::mapToMemory() {
    static_cast<DerivedSPIModel *>(this)->map_to_memory();
}

template <typename DerivedSPIModel>
void SPICommunication<DerivedSPIModel>::unmapMemory() {
    static_cast<DerivedSPIModel *>(this)->unmap_memory();
}

template <typename DerivedSPIModel>
std::vector<std::byte>
SPICommunication<DerivedSPIModel>::SPIread(const SPIRegister &spi_reg,
                                           const uint8_t chip_id) const {
    return static_cast<const DerivedSPIModel *>(this)->spi_read(
        spi_reg.n_bytes, chip_id, spi_reg.spi_register_id);
}

template <typename DerivedSPIModel>
void SPICommunication<DerivedSPIModel>::SPIwrite(
    const SPIRegister &spi_reg, const uint8_t chip_id,
    const std::vector<std::byte> &data) {

    if (data.size() != spi_reg.n_bytes) {
        LOG(logERROR) << fmt::format("Data size {} does not match number of "
                                     "bytes {} for SPI register {}",
                                     data.size(), spi_reg.n_bytes,
                                     spi_reg.spi_register_id);
        throw std::runtime_error(
            fmt::format("Data size {} does not match number of bytes {} for "
                        "SPI register {}",
                        data.size(), spi_reg.n_bytes, spi_reg.spi_register_id));
    }

    static_cast<DerivedSPIModel *>(this)->spi_write(
        chip_id, spi_reg.spi_register_id, data);

    static_cast<DerivedSPIModel *>(this)->spi_write(
        chip_id, SPIRegisters::SPI_REG_ExtraClocks.spi_register_id,
        std::vector<std::byte>{
            std::byte{0x00}}); // extra clock trigger to actually load the
                               // new value into the register
}

/**
 * Non destructive read from SPI register. Read n_bytes by shifting in
 * dummy data while keeping csn 0 after the operation. Shift the read out
 * data back in to restore the register.
 */
class HardwareSPICommunication
    : public SPICommunication<HardwareSPICommunication> {

  public:
    HardwareSPICommunication() = default;

    void map_to_memory();

    void unmap_memory();

    void spi_write(const uint8_t chip_id, const uint8_t register_id,
                   const std::vector<std::byte> &data);

    std::vector<std::byte> spi_read(const size_t n_bytes, const uint8_t chip_id,
                                    const uint8_t register_id) const;

  private:
    int spi_filedescriptor = -1;
};

// template <SPIRegister... SPIRegisters, uint8_t NUM_CHIPS_PER_MODULE> // non
// type template parameters only for c++20
template <typename SPIRegisters> // TODO add a type trait to ensure it stores
                                 // all fields
class VirtualSPICommunication
    : public SPICommunication<VirtualSPICommunication<SPIRegisters>> {

  public:
    VirtualSPICommunication() {
        // TODO should it be in the constructor?
        /*
        (virtual_registers.emplace(
             SPIRegisters.spi_register_id,
             VirtualMemoryModel<std::byte>{SPIRegisters.spi_register_id,
                                           SPIRegisters.n_bytes *
                                               NUM_CHIPS_PER_MODULE}),
         ...);
         */

        LOG(logDEBUG) << fmt::format(
            "Initializing virtual SPI communication with {} registers for {} "
            "chips per module",
            SPIRegisters::spiregisters.size(),
            SPIRegisters::NUM_CHIPS_PER_MODULE);

        for (const auto &reg : SPIRegisters::spiregisters) {
            virtual_registers.emplace(
                reg.spi_register_id,
                VirtualMemoryModel<std::byte>{
                    reg.spi_register_id,
                    reg.n_bytes * SPIRegisters::NUM_CHIPS_PER_MODULE});
        }

        LOG(logDEBUG) << fmt::format(
            "Initialized virtual SPI communication with {} registers for {} "
            "chips per module",
            virtual_registers.size(), SPIRegisters::NUM_CHIPS_PER_MODULE);

        for (const auto &[register_id, register_memory] : virtual_registers) {
            LOG(logDEBUG) << fmt::format(
                "Virtual SPI register with id {} has virtual", register_id);
        }
    }

    void map_to_memory() {

        // resize the virtual register memory to the correct size based on
        // the defined SPI registers
        for (auto &[register_id, register_memory] : virtual_registers) {
            LOG(logDEBUG) << fmt::format("Mapping virtual SPI register with id "
                                         "{} to virtual memory",
                                         register_id);
            register_memory.mapToMemory();
        }
    }

    void unmap_memory() { // do nothing
    }

    std::vector<std::byte> spi_read(const size_t n_bytes, const uint8_t chip_id,
                                    const uint8_t register_id) const {

        auto mapped_register =
            virtual_registers.at(register_id).getMappedMemoryPtr();

        mapped_register +=
            chip_id * n_bytes; // TODO: how to handle different chip ids ->
                               // e.g. broadcast do we want it to be
                               // configurable for different chip ids?

        // TODO: should I emulate the shifting in of dummy data and shifting
        // out of the register data here to be more realistic?
        std::vector<std::byte> output_data(n_bytes);
        std::memcpy(output_data.data(), mapped_register, n_bytes);

        return output_data;
    }

    void spi_write(const uint8_t chip_id, const uint8_t register_id,
                   const std::vector<std::byte> &data) {

        auto mapped_register =
            virtual_registers.at(register_id).getMappedMemoryPtr();

        mapped_register +=
            chip_id * data.size(); // TODO: how to handle different
                                   // chip ids -> e.g. broadcast do

        // TODO: should I emulate the shifting in of dummy data and shifting
        // out of
        std::memcpy(mapped_register, data.data(), data.size());
    }

  private:
    /// @brief map of register id to virtual memory model for each register
    std::map<uint16_t, VirtualMemoryModel<std::byte>> virtual_registers{};
};

} // namespace sls
