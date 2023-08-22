import numpy as np
# generate pixelmaps for various CTB detectors


def moench03():
    out = np.zeros((400, 400), dtype=np.uint32)
    adc_numbers = np.array(
        (12, 13, 14, 15, 12, 13, 14, 15, 8, 9, 10, 11, 8, 9, 10, 11, 4, 5, 6, 7, 4, 5, 6, 7, 0, 1, 2, 3, 0, 1, 2, 3),
        dtype=np.int_)
    for n_pixel in range(5000):
        for i_sc in range(32):
            adc_nr = adc_numbers[i_sc]
            col = ((adc_nr * 25) + (n_pixel % 25))
            row = 0
            if (i_sc // 4 % 2 == 0):
                row = 199 - (n_pixel // 25)
            else:
                row = 200 + (n_pixel // 25)

            i_analog = n_pixel * 32 + i_sc
            out[row, col] = i_analog

    return out


def moench04_analog():
    out = np.zeros((400, 400), dtype=np.uint32)
    adc_numbers = np.array((9, 8, 11, 10, 13, 12, 15, 14, 1, 0, 3, 2, 5, 4, 7, 6, 23, 22, 21, 20, 19, 18, 17, 16, 31,
                            30, 29, 28, 27, 26, 25, 24),
                           dtype=np.int_)

    for n_pixel in range(5000):
        for i_sc in range(32):
            adc_nr = adc_numbers[i_sc]
            col = ((adc_nr % 16) * 25) + (n_pixel % 25)
            row = 0
            if i_sc < 16:
                row = 199 - (n_pixel // 25)
            else:
                row = 200 + (n_pixel // 25)

            i_analog = n_pixel * 32 + i_sc
            out[row, col] = i_analog

    return out


def matterhorn_transceiver():
    out = np.zeros((48, 48), dtype=np.uint32)

    offset = 0
    nSamples = 4
    for row in range(48):
        for col in range(24):
            for iTrans in range(2):
                out[iTrans * 24 + col, row] = offset + nSamples * iTrans
            offset += 1
            if (col + 1) % nSamples == 0:
                offset += nSamples

    return out
