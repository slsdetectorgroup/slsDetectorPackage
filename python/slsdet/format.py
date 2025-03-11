def hexFormat_nox(val, fill):
    v = hex(val)  # hexadecimal value
    v = (v.lstrip("-0x")).rstrip("L")  # remove leading 0x and - if there and trailing L
    v = v.zfill(fill)  # inserts zeros at the beginning
    return v


def hexFormat(val, fill):
    v = hex(val)  # hexadecimal value
    v = (v.lstrip("-0x")).rstrip("L")  # remove leading 0x and - if there and trailing L
    v = v.zfill(fill)  # inserts zeros at the beginning
    v = "0x" + v  # puts back 0x
    return v


def to_hex(value, width = None):
    # check if value can be formatted in said width
    s = f"{value:x}"
    if width is not None:
        if len(s) > width:
            raise ValueError(f"value: {value} needs a field with of {len(s)}")
    # format
    return f'0x{s:>0{width}}'


def binFormat_nob(val, fill):
    v = bin(val)  # binary value
    v = (v.lstrip("-0b")).rstrip("L")  # remove leading 0x and - if there and trailing L
    v = v.zfill(fill)  # inserts zeros at the beginning
    return v


def binFormat(val, fill):
    v = bin(val)  # binary value
    v = (v.lstrip("-0b")).rstrip("L")  # remove leading 0x and - if there and trailing L
    v = v.zfill(fill)  # inserts zeros at the beginning
    v = "0b" + v  # puts back 0b
    return v


def decFormat(val, fill):
    v = str(val)  # decimal value
    v = v.zfill(fill)  # inserts zeros at the beginning
    return v