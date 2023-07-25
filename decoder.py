from defines import *
from _decoder import * #bring in the function from the compiled extension
import numpy as np

def moench04(analog_buffer):
    """
    Python implementation, keep as a reference. Change name and replace
    with C version to swap it out in the GUI
    """
    nAnalogCols = 400 #We know we have a Moench
    nAnalogRows = 400
    adcNumbers = Defines.Moench04.adcNumbers
    nPixelsPerSC = Defines.Moench04.nPixelsPerSuperColumn
    scWidth = Defines.Moench04.superColumnWidth

    analog_frame = np.zeros((nAnalogCols, nAnalogRows), dtype = analog_buffer.dtype)

    for iPixel in range(nPixelsPerSC):
        for iSC, iAdc in enumerate(adcNumbers):
            col = ((iAdc % 16) * scWidth) + (iPixel % scWidth)
            if iSC < 16:
                row = 199 - int(iPixel / scWidth)
            else:
                row = 200 + int(iPixel / scWidth)
            index_min = iPixel * 32 + iSC
            pixel_value = analog_buffer[index_min]
            analog_frame[row, col] = pixel_value

    mask = np.uint16(0x3FFF) #Do we always mask out the top bits?
    np.bitwise_and(analog_frame, mask, out = analog_frame)
    return np.rot90(analog_frame, 3)
