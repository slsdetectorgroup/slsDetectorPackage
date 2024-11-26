def set_bit(value, bit_nr):
    return value | 1 << bit_nr


def remove_bit(value, bit_nr):
    return value & ~(1 << bit_nr)


def bit_is_set(value, bit_nr):
    return (value >> bit_nr) & 1 == 1


def manipulate_bit(is_set, value, bit_nr):
    if is_set:
        return set_bit(value, bit_nr)
    return remove_bit(value, bit_nr)
