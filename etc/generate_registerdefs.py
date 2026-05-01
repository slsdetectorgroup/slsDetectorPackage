import pandas as pd 
import argparse 

def Ip_core_name_to_enum_type(ip_core_name : str) -> str: 
    """Convert IP core name to enum type IPCore."""
    if pd.isna(ip_core_name): 
        return "IPCore::UNKNOWN"
    
    return f"IPCore::{ip_core_name.upper()}"

def create_bitmask_and_offset(from_bit : int, to_bit : int) -> tuple[int, int]: 
    """Create a bitmask for a register field given the from_bit and to_bit."""
    if from_bit < 0 or to_bit < 0 or from_bit > to_bit or from_bit >= 32 or to_bit >= 32:
        raise ValueError(f"Invalid bit range: from_bit={from_bit}, to_bit={to_bit}")
    
    offset = from_bit
    num_bits = to_bit - from_bit + 1
    adress_space = 0xFFFFFFFF  # Assuming a 32-bit address space for the bitmask
    mask = (adress_space >> (32 - num_bits)) 
    return mask, offset # to get value of field: (register_value >> offset) & mask, to set value of field: register_value = (register_value & ~(mask << offset)) | ((field_value & mask) << offset)

def argument_parser():
    parser = argparse.ArgumentParser(description="Generate register definitions from a CSV file.")
    parser.add_argument("--csv_file", required=True, help="Path to the CSV file containing register definitions.")
    parser.add_argument("--output_header_file", required=True, help="Path to the output header file to write the register definitions to.")
    parser.add_argument(
        "--overwrite",
        action="store_true",
        help="Overwrite the output file instead of appending.",
    )
    return parser.parse_args()

# TODO: should be configurable 
header = r"""
#pragma once
// clang-format off
#include "RegisterHelperStructs.hpp"

namespace sls {



namespace Reg {

/// @brief Enum for IP cores, value are adresses
enum class IPCore : uint32_t {
    MH_RO_SM_AXI = 0xB0010000,
    FHDR_AXI = 0xB0011000,
    AURORA_STATUS = 0xB0014000,
    AURORA_STATUS2 = 0xB0015000,
    PACKETIZERREG = 4,
    UNKNOWN = 0x00000000 // dont know yet
};

constexpr size_t IPCORE_REGISTER_BLOCK_SIZE =
    0x1000; // size of each IP core address space in bytes // TODO: maybe add in
            // other file definitions

// clang-format off

"""

postpend = r""" 
constexpr RegisterField ModuleRow{
     Frame_HDR_ModCoord_LSB_Reg, 0, 0xffff}; 

constexpr RegisterField ModuleCol{
     Frame_HDR_ModCoord_LSB_Reg, 16, 0xffff};

constexpr RegisterField ModuleCoordz{
     Frame_HDR_ModCoord_MSB_Reg, 0, 0xffff};

constexpr RegisterField ModuleIndex{
     Frame_HDR_ModCoord_MSB_Reg, 16, 0xffff};
} // namespace Reg
} // namespace sls
// clang-format on
"""

postpend = r""" 
} // namespace Reg
} // namespace sls
// clang-format on
"""

def main():

    args = argument_parser()

    registers_list = pd.read_csv(args.csv_file)

    registers = registers_list.drop_duplicates(subset=["Reg_name", "Address"])

    file_mode = "w" if args.overwrite else "a"
    header_file = open(args.output_header_file, file_mode)

    if args.overwrite:
        header_file.write(header)
        header_file.write("\n\n")

    header_file.write("// Register definitions") 
    header_file.write("\n")

    for index, row in registers.iterrows():
        local_address_offset_in_bytes = row["Address"]
        register_name = row["Reg_name"]
        ip_core_name = row["Interface"]

        define_register_string = (
            f"constexpr Register {register_name}{{"
            f"{Ip_core_name_to_enum_type(ip_core_name)}, {hex(int(local_address_offset_in_bytes, 16))}}};"
        )

        header_file.write(f"{define_register_string}\n")
        header_file.write("\n") 

    header_file.write("\n") 
    header_file.write("\n")

    header_file.write("// Register fields")
    header_file.write("\n")

    for index, row in registers_list.iterrows():
        register_name = row["Reg_name"]
        field_name = row["Name"]
        from_bit = row["From_bit"]
        to_bit = row["To_bit"]
        mask, offset = create_bitmask_and_offset(from_bit, to_bit)

        define_registerfield_string = (
            f"constexpr RegisterField {field_name}{{\n"
            f"     {register_name}, {offset}, {hex(mask)}}};"
            )

        header_file.write(f"{define_registerfield_string}\n")
        header_file.write("\n")

    header_file.write(postpend) # TODO: have to take care xof it manually when in append mode - ugly  
    header_file.close()


if __name__ == "__main__":
    main()
