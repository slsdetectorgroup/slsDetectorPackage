/**
 * @file HelperFunctions.hpp
 * @short contains helper functions for the Matterhorn server implementation
 * e.g. for processing of specific commands
 */
#pragma once
#include <cstdint>

/**
 * @brief converts the counter mask received from the client to the actual
 * counter mask to be written to the SPI register based on the starting counter
 * and number of counters to read
 * @return actual counter mask to be written to the SPI register
 */
uint32_t convertCounterMaskToSPICounterMask(const uint32_t counter_mask) {
    uint32_t spi_counter_mask = 0;

    // start

    /*
    for(uint8_t i = 0; i < 4; ++i) {
        if (counter_mask & (1 << i) && (spi_counter_mask == 0)) {
            spi_counter_mask |= (i % 4); // set the starting counter based on
                                         // the counter mask
        }
        else if ((counter_mask & (1 << i))) {
            spi_counter_mask |= (1 << (2 + (i % 4))); // set the number of
    counters
                                                     // to read based on the
                                                     // counter mask and the
                                                     // starting counter
        }
    }
    */

    // case distinction easiest!!!
}

/**
 * @brief converts the actual counter mask read from the SPI register to the
 * counter mask to be sent to the client e.g. bit set to 1 if counter enabled
 * @return counter mask to be sent to the client
 */
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