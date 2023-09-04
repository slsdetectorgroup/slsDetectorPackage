import logging
from pathlib import Path

import numpy as np

__savedFrames = []
__pedestal = 0


def reset(plotTab):
    global __savedFrames, __pedestal
    __savedFrames = []
    __pedestal = 0
    plotTab.updateLabelPedestalFrames()


def getFramesCount():
    return len(__savedFrames)


def getPedestal():
    return __pedestal


def calculatePedestal():
    global __pedestal
    if len(__savedFrames) == 0:
        __pedestal = 0
    else:
        __pedestal = np.mean(__savedFrames, axis=0)


def savePedestal(path=Path('/tmp/pedestal')):
    calculatePedestal()
    np.save(path, getPedestal())


def loadPedestal(path: Path):
    global __pedestal
    __pedestal = np.load(path)


__logger = logging.getLogger('recordOrApplyPedestal')


def recordOrApplyPedestal(func):
    """
    decorator function used to apply pedestal functionalities
    @param func: processing function that needs to be wrapped
    @return: wrapper function to be called
    """

    def wrapper(obj, *args, **kwargs):
        """
        wrapeer that calls func (a raw data _processing function) and calculates or applies a pedestal to it
        @param obj: reference to func's class instance (self of its class)
        @return: if record mode: return frame untouched, if apply mode: return frame - pedestal
        """
        global __savedFrames, __pedestal

        frame = func(obj, *args, **kwargs)
        if not np.array_equal(__pedestal, 0) and __pedestal.shape != frame.shape:
            __logger.warning('pedestal shape mismatch. resetting pedestal...')
            reset(obj.plotTab)

        if obj.plotTab.pedestalRecord:
            # check if savedFrames has frames with different shapes
            __savedFrames.append(frame)
            obj.plotTab.updateLabelPedestalFrames()
            calculatePedestal()
            return frame
        if obj.plotTab.pedestalApply:
            # apply pedestal
            # check if pedestal is calculated
            return frame - __pedestal

        return frame

    return wrapper
