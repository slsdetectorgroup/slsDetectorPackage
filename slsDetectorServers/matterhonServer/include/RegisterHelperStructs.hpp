#include <cstdint>
#include <string_view>

enum class IPCore : uint32_t; // forward declaration of IPCore enum class

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

    /// @brief Bitmask for the field
    const uint32_t bitmask{};

    /// @brief Bit position of the least significant bit of the field in the
    /// register
    const uint32_t bit_position{};
};