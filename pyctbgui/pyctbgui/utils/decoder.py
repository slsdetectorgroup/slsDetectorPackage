from pyctbgui.utils.defines import Defines
from pyctbgui._decoder import *  #bring in the function from the compiled extension
import numpy as np
"""
Python implementation, keep as a reference. Change name and replace
with C version to swap it out in the GUI
"""


def moench04(analog_buffer):
    nAnalogCols = Defines.Moench04.nCols
    nAnalogRows = Defines.Moench04.nRows
    adcNumbers = Defines.Moench04.adcNumbers
    nPixelsPerSC = Defines.Moench04.nPixelsPerSuperColumn
    scWidth = Defines.Moench04.superColumnWidth

    analog_frame = np.zeros((nAnalogCols, nAnalogRows), dtype=analog_buffer.dtype)

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

    return analog_frame


def matterhorn(trans_buffer):
    nTransceiverRows = Defines.Matterhorn.nRows
    nTransceiverCols = Defines.Matterhorn.nCols

    transceiver_frame = np.zeros((nTransceiverCols, nTransceiverRows), dtype=trans_buffer.dtype)

    offset = 0
    nSamples = Defines.Matterhorn.nPixelsPerTransceiver
    for row in range(Defines.Matterhorn.nRows):
        for col in range(Defines.Matterhorn.nHalfCols):
            #print(f'row:{row} col:{col} offset: {offset}')
            for iTrans in range(Defines.Matterhorn.nTransceivers):
                transceiver_frame[iTrans * Defines.Matterhorn.nHalfCols + col,
                                  row] = trans_buffer[offset + nSamples * iTrans]
            offset += 1
            if (col + 1) % nSamples == 0:
                offset += nSamples

    return transceiver_frame
