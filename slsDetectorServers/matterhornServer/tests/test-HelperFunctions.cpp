#include "HelperFunctions.hpp"
#include "catch.hpp"
#include <iostream>

namespace sls {

auto get_test_parameters_countermaskspiconversion() {
    return GENERATE(std::make_tuple(uint32_t{0x1}, uint32_t{0b0000}),
                    std::make_tuple(uint32_t{0x2}, uint32_t{0b0001}),
                    std::make_tuple(uint32_t{0x4}, uint32_t{0b0010}),
                    std::make_tuple(uint32_t{0x3}, uint32_t{0b0100}),
                    std::make_tuple(uint32_t{0x6}, uint32_t{0b0101}),
                    std::make_tuple(uint32_t{0x7}, uint32_t{0b1000}),
                    std::make_tuple(uint32_t{0x8}, uint32_t{0b0011}),
                    std::make_tuple(uint32_t{0x9}, uint32_t{0b0111}),
                    std::make_tuple(uint32_t{0xB}, uint32_t{0b1011}),
                    std::make_tuple(uint32_t{0xC}, uint32_t{0b0110}),
                    std::make_tuple(uint32_t{0xD}, uint32_t{0b1010}),
                    std::make_tuple(uint32_t{0xE}, uint32_t{0b1001}),
                    std::make_tuple(uint32_t{0xF}, uint32_t{0b1100}),
                    std::make_tuple(uint32_t{0xB}, uint32_t{0b1011}));
}

TEST_CASE("Convert Counter Mask to SPI Counter Mask",
          "[MatterHorn][HelperFunctions]") {
    auto [counter_mask, expected_spi_counter_mask] =
        get_test_parameters_countermaskspiconversion();

    REQUIRE(convertCounterMaskToSPICounterMask(counter_mask) ==
            expected_spi_counter_mask);

    REQUIRE_THROWS(
        convertCounterMaskToSPICounterMask(0xA)); // invalid counter mask

    REQUIRE_THROWS(
        convertCounterMaskToSPICounterMask(0x5)); // invalid counter mask
}

TEST_CASE("Convert SPI Counter Mask to Counter Mask",
          "[MatterHorn][HelperFunctions]") {
    auto [counter_mask, spi_counter_mask] =
        get_test_parameters_countermaskspiconversion();

    REQUIRE(convertSPICounterMaskToCounterMask(spi_counter_mask) ==
            counter_mask);
}

} // namespace sls