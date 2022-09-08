def set_bit(value, bit_nr):
    return  value | 1 << bit_nr

def remove_bit(value, bit_nr):
    return value & ~(1 << bit_nr)