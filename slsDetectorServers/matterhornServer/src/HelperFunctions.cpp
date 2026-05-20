#include "HelperFunctions.hpp"
#include <stdexcept>

uint32_t convertCounterMaskToSPICounterMask(const uint32_t counter_mask) {
    uint32_t spi_counter_mask = 0;

    switch (counter_mask) {
    case 0b1:
        spi_counter_mask = 0b0000; // counter 0 enabled
        break;
    case 0b10:
        spi_counter_mask = 0b0001; // counter 1 enabled
        break;
    case 0b100:
        spi_counter_mask = 0b0010; // counter 2 enabled
        break;
    case 0b1000:
        spi_counter_mask = 0b0011; // counter 3 enabled
        break;
    case 0b11:
        spi_counter_mask = 0b0100; // counter 0 and 1 enabled
        break;
    case 0b110:
        spi_counter_mask = 0b0101; // counter 1 and 2 enabled
        break;
    case 0b1100:
        spi_counter_mask = 0b0110; // counter 2 and 3 enabled
        break;
    case 0b1001:
        spi_counter_mask = 0b0111; // counter 0 and 3 enabled
        break;
    case 0b111:
        spi_counter_mask = 0b1000; // counter 0, 1 and 2 enabled
        break;
    case 0b1110:
        spi_counter_mask = 0b1001; // counter 1, 2 and 3 enabled
        break;
    case 0b1101:
        spi_counter_mask = 0b1010; // counter 0, 2 and 3 enabled
        break;
    case 0b1011:
        spi_counter_mask = 0b1011; // counter 0, 1 and 3 enabled
        break;
    case 0b1111:
        spi_counter_mask = 0b1100; // counter 0, 1, 2 and 3 enabled
        break;
    default:
        throw std::invalid_argument(
            "Invalid counter mask: Only contiguous counters are enabled "
            "(including wrap around)");
    }

    return spi_counter_mask;
}

uint32_t convertSPICounterMaskToCounterMask(const uint32_t spi_counter_mask) {

    uint32_t counter_mask = 0;

    uint8_t start_counter = spi_counter_mask & 0b11; // extract starting counter
    uint8_t num_counters =
        (spi_counter_mask >> 2) & 0b11; // extract number of counters

    constexpr uint8_t max_counters = 4;

    for (uint8_t i = 0; i < num_counters; ++i) {
        counter_mask |= (1 << ((start_counter + i) % max_counters));
    }

    return counter_mask;
}