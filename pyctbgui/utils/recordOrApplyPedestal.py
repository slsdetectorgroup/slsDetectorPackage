import logging
from pathlib import Path

import numpy as np

__frameCount = 0
__pedestalSum = np.array(0, np.float64)
__pedestal = np.array(0, np.float32)
__loadedPedestal = False


def reset(plotTab):
    global __frameCount, __pedestalSum, __pedestal, __loadedPedestal
    __frameCount = 0
    __pedestalSum = np.array(0, np.float64)
    __pedestal = np.array(0, np.float64)
    __loadedPedestal = False

    plotTab.updateLabelPedestalFrames()


def getFramesCount():
    return __frameCount


def getPedestal():
    return __pedestal


def calculatePedestal():
    global __pedestalSum, __pedestal
    if __loadedPedestal:
        return __pedestal
    if __frameCount == 0:
        __pedestal = np.array(0, np.float64)
    else:
        __pedestal = __pedestalSum / __frameCount
    return __pedestal


def savePedestal(path=Path('/tmp/pedestal')):
    pedestal = calculatePedestal()
    np.save(path, pedestal)


def loadPedestal(path: Path):
    global __pedestal, __loadedPedestal
    __loadedPedestal = True
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
        global __frameCount, __pedestal, __pedestalSum

        frame = func(obj, *args, **kwargs)
        if not np.array_equal(0, __pedestalSum) and __pedestalSum.shape != frame.shape:
            # check if __pedestalSum has same different shape as the frame
            __logger.info('pedestal shape mismatch. resetting pedestal...')
            reset(obj.plotTab)

        if obj.plotTab.pedestalRecord:
            if __loadedPedestal:
                # reset loaded pedestal if we acquire in record mode
                __logger.warning('resetting loaded pedestal...')
                reset(obj.plotTab)
            __frameCount += 1

            __pedestalSum = np.add(__pedestalSum, frame, dtype=np.float64)

            obj.plotTab.updateLabelPedestalFrames()
            return frame
        if obj.plotTab.pedestalApply:
            # apply pedestal
            # check if pedestal is calculated
            if __loadedPedestal and frame.shape != __pedestal.shape:
                __logger.warning('pedestal shape mismatch. resetting pedestal...')
                obj.plotTab.mainWindow.statusbar.setStyleSheet("color:red")
                obj.plotTab.mainWindow.statusbar.showMessage('pedestal shape mismatch. resetting pedestal...')
                reset(obj.plotTab)

            return frame - calculatePedestal()

        return frame

    return wrapper
