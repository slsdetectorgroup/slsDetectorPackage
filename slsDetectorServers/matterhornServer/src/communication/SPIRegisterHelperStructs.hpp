#pragma once
#include "fmt/format.h"
#include <cstdint>
#include <vector>

namespace sls {

struct SPIRegister {
    /// @brief SPI register ID (0-15)
    uint16_t spi_register_id{};

    /// @brief number of bytes in the register
    uint64_t n_bytes{};
};

struct SPIRegisterField {
    /// @brief SPI register to which teh field belongs
    SPIRegister register_{};

    /// @brief least significant bit position of the field in the register
    uint64_t lsb_position{};

    /// @brief number of bits in the field
    /// TODO: can it be larger?
    uint32_t num_bits{};
};

// TODO: maybe change uint32_t but max field size is 32 bits so should be fine
// for now
void inline setSPIRegisterField(std::vector<std::byte> &register_value,
                                const SPIRegisterField &field,
                                uint32_t field_value) {

    // check that the field value can fit in the bitmask
    if ((field_value >> field.num_bits) != 0) {
        throw std::invalid_argument(fmt::format(
            "Value {} cannot fit in field {}", field_value, field.num_bits));
    }

    constexpr uint8_t bits_per_byte = 8;

    // TODO: mmh doesnt feel very modern - maybe better to cast to uint32_t,
    // alignment issues?
    for (std::size_t i = 0; i < field.num_bits; ++i) {
        std::size_t offset = field.lsb_position + i;
        std::size_t byte_index = offset / bits_per_byte;
        std::size_t bit_index = offset % bits_per_byte;

        std::byte mask = std::byte(1) << bit_index;

        const bool bit = (field_value >> i) & 0x1;

        if (bit) {
            register_value[byte_index] |=
                mask; // set the bit in the register value
        } else {
            register_value[byte_index] &=
                ~mask; // clear the bit in the register value
        }
    }
}

uint32_t inline getSPIRegisterField(
    const std::vector<std::byte> &register_value,
    const SPIRegisterField &field) {

    uint32_t field_value = 0;
    constexpr uint8_t bits_per_byte = 8;

    for (std::size_t i = 0; i < field.num_bits; ++i) {
        std::size_t offset = field.lsb_position + i;
        std::size_t byte_index = offset / bits_per_byte;
        std::size_t bit_index = offset % bits_per_byte;

        std::byte mask = std::byte(1) << bit_index;

        field_value |=
            (static_cast<uint32_t>((register_value[byte_index] & mask) >>
                                   bit_index)
             << i); // extract the field value bit from the register value and
                    // set it in the correct position in the field value
    }

    return field_value;
}

} // namespace sls