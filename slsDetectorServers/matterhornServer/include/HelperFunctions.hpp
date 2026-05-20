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
uint32_t convertCounterMaskToSPICounterMask(const uint32_t counter_mask);

/**
 * @brief converts the actual counter mask read from the SPI register to the
 * counter mask to be sent to the client e.g. bit set to 1 if counter enabled
 * @return counter mask to be sent to the client
 */
uint32_t convertSPICounterMaskToCounterMask(const uint32_t spi_counter_mask);