#pragma once
#include <cstdint>
#include <exception>
#include <fmt/format.h>
#include <string_view>

namespace sls {

enum class IPCore : uint32_t; // forward declaration of IPCore enum class

/// @brief struct representing 32 bit register
struct Register {
    /// @brief IP core address space
    const IPCore ip_core{}; // TODO replace by enum type

    /// @brief Offset of the register in bytes from the base address of the IP
    /// core
    const uint32_t offset_in_bytes{};
};

struct RegisterField {
    /// @brief Register to which the field belongs
    const Register register_{};

    /// @brief Bit position of the least significant bit of the field in the
    /// register
    const uint32_t bit_position{};

    /// @brief Bitmask for the field
    const uint32_t bitmask{};
};

// TODO: maybe static member function of RegisterField?
template <typename T>
void setRegisterField(uint32_t &registervalue, const RegisterField &reg_field,
                      T field_value) {

    if (field_value > static_cast<T>(reg_field.bitmask)) {
        throw std::invalid_argument(
            fmt::format("Value {} cannot fit in field with bitmask {}",
                        field_value, reg_field.bitmask));
    }
    // Clear the bits corresponding to the field
    registervalue &= ~(reg_field.bitmask << reg_field.bit_position);
    // Set the new value for the field
    registervalue |= (static_cast<uint32_t>(field_value) & reg_field.bitmask)
                     << reg_field.bit_position;
}

template <typename T>
T getRegisterField(const uint32_t &registervalue,
                   const RegisterField &reg_field) {
    // Extract the bits corresponding to the field and shift them to get the
    // value

    auto field_value =
        (registervalue >> reg_field.bit_position) & reg_field.bitmask;

    return static_cast<T>(field_value);
}

} // namespace sls