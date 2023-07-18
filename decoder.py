from defines import *
import numpy as np
def moench04(analog_buffer):
    nAnalogCols = 400 #We know we have a Moench
    nAnalogRows = 400
    adcNumbers = Defines.Moench04.adcNumbers
    nPixelsPerSC = Defines.Moench04.nPixelsPerSuperColumn
    scWidth = Defines.Moench04.superColumnWidth

    analog_frame = np.zeros((nAnalogCols, nAnalogRows))

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

    return np.rot90(analog_frame)
