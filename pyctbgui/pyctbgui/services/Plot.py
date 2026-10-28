import logging
from functools import partial
import random
from pathlib import Path

import numpy as np
from PyQt5 import QtWidgets, QtGui, QtCore, uic

from aare import transform, ReadoutMode 
from aare._aare import Matterhorn10, Matterhorn02, Moench04, Moench05

import pyqtgraph as pg
from pyctbgui.utils import recordOrApplyPedestal 
from pyqtgraph import PlotWidget

from pyctbgui.utils.defines import Defines


class PlotTab(QtWidgets.QWidget):

    def __init__(self, parent):
        super().__init__(parent)
        pg.setConfigOptions(imageAxisOrder="row-major")  
        uic.loadUi(Path(__file__).parent.parent / 'ui' / "plot.ui", parent)
        self.view = parent
        self.mainWindow = None
        self.det = None
        self.signalsTab = None
        self.transceiverTab = None
        self.acquisitionTab = None
        self.adcTab = None

        self.shownCounterImages: list[bool] = [True] * Defines.max_counters # per default show all 4 counter images

        self.showtransceiverImage : bool = False
        self.showdigitalImage : bool = False
        self.showanalogImage : bool = False

        # color range variables 
        self.colorRangeMode: Defines.colorRange = Defines.colorRange.auto

        self.digitalcolorrange : list[float] = [0.0, 0.0]
        self.analogcolorrange : list[float] = [0.0, 0.0]
        self.transceivercolorrange : list[list[float]] =  [[0.0, 0.0] for _ in range(Defines.max_counters)]

        self.digitalcminSpinBoxes : QtWidgets.QSpinBox = None
        self.digitalcmaxSpinBoxes : QtWidgets.QSpinBox = None
        self.analogcminSpinBoxes : QtWidgets.QSpinBox = None
        self.analogcmaxSpinBoxes : QtWidgets.QSpinBox = None
        self.transceivercminSpinBoxes : list[QtWidgets.QSpinBox] = None
        self.transceivercmaxSpinBoxes : list[QtWidgets.QSpinBox] = None

        # list of callback functions to notify tabs when we should hide their legend
        # follows the observer design pattern
        self.hideLegendObservers = []
        self.pedestalRecord: bool = False
        self.pedestalApply: bool = True
        self.__acqFrames = None
        self.logger = logging.getLogger('PlotTab')
        self.plotSplitter = None 

    def setup_ui(self):
        self.signalsTab = self.mainWindow.signalsTab
        self.transceiverTab = self.mainWindow.transceiverTab
        self.acquisitionTab = self.mainWindow.acquisitionTab
        self.adcTab = self.mainWindow.adcTab
        self.initializeColorMaps()

        # TODO use list comprehension 
        self.checkBoxCounters = [self.view.checkBoxCounter0, self.view.checkBoxCounter1, self.view.checkBoxCounter2, self.view.checkBoxCounter3]
        
        for checkBox in self.checkBoxCounters:
            checkBox.setChecked(True) # per default enable all
            checkBox.setEnabled(False)

        self.transceivercminSpinBoxes = [self.view.transceivercminSpinBox0, self.view.transceivercminSpinBox1, self.view.transceivercminSpinBox2, self.view.transceivercminSpinBox3]
        self.transceivercmaxSpinBoxes = [self.view.transceivercmaxSpinBox0, self.view.transceivercmaxSpinBox1, self.view.transceivercmaxSpinBox2, self.view.transceivercmaxSpinBox3]
        self.analogcminSpinBoxes = self.view.analogcminSpinBox
        self.analogcmaxSpinBoxes = self.view.analogcmaxSpinBox
        self.digitalcminSpinBoxes = self.view.digitalcminSpinBox
        self.digitalcmaxSpinBoxes = self.view.digitalcmaxSpinBox
        

    def connect_ui(self):
        self.view.radioButtonNoPlot.clicked.connect(self.plotOptions)
        self.view.radioButtonWaveform.clicked.connect(self.plotOptions)
        self.view.radioButtonDistribution.clicked.connect(self.plotOptions)
        self.view.radioButtonImage.clicked.connect(self.plotOptions)
        self.view.comboBoxPlot.currentIndexChanged.connect(self.setDecoder)
        self.view.comboBoxColorMap.currentIndexChanged.connect(self.setColorMap)
        self.view.comboBoxZMQHWM.currentIndexChanged.connect(self.setZMQHWM)
        self.view.spinBoxSerialOffset.editingFinished.connect(self.setSerialOffset)
        self.view.spinBoxNCount.editingFinished.connect(self.setNCounter)
        self.view.spinBoxDynamicRange.editingFinished.connect(self.setDynamicRange)
        self.view.spinBoxImageX.editingFinished.connect(self.setImageX)
        self.view.spinBoxImageY.editingFinished.connect(self.setImageY)

        self.view.radioButtonPedestalRecord.toggled.connect(self.togglePedestalRecord)
        self.view.radioButtonPedestalApply.toggled.connect(self.togglePedestalApply)
        self.view.pushButtonPedestalReset.clicked.connect(self.resetPedestal)
        self.view.pushButtonSavePedestal.clicked.connect(self.savePedestal)
        self.view.pushButtonLoadPedestal.clicked.connect(self.loadPedestal)

        self.view.checkBoxRaw.stateChanged.connect(self.setRawData)
        self.view.spinBoxRawMin.editingFinished.connect(self.setRawData)
        self.view.spinBoxRawMax.editingFinished.connect(self.setRawData)
        self.view.checkBoxPedestal.stateChanged.connect(self.setPedestalSubtract)
        self.view.spinBoxPedestalMin.editingFinished.connect(self.setPedestalSubtract)
        self.view.spinBoxPedestalMax.editingFinished.connect(self.setPedestalSubtract)
        self.view.spinBoxFit.editingFinished.connect(self.setFitADC)
        self.view.spinBoxPlot.editingFinished.connect(self.setPlotBit)
        self.view.pushButtonReferesh.clicked.connect(self.acquisitionTab.refresh)
        # color range widgets
        self.view.radioButtonAutomatic.clicked.connect(partial(self.setColorRangeMode, Defines.colorRange.auto))
        self.view.radioButtonFixed.clicked.connect(partial(self.setColorRangeMode, Defines.colorRange.fixed))
        self.view.radioButtonCenter.clicked.connect(partial(self.setColorRangeMode, Defines.colorRange.center))

        for index, cminSpinBox in enumerate(self.transceivercminSpinBoxes):
            cminSpinBox.editingFinished.connect(partial(self.setCmin, imagetype = Defines.ImageType.TRANSCEIVER, index=index))

        for index, cmaxSpinBox in enumerate(self.transceivercmaxSpinBoxes):
            cmaxSpinBox.editingFinished.connect(partial(self.setCmax, imagetype = Defines.ImageType.TRANSCEIVER, index=index))

        self.analogcminSpinBoxes.editingFinished.connect(partial(self.setCmin, imagetype = Defines.ImageType.ANALOG, index=0))
        self.analogcmaxSpinBoxes.editingFinished.connect(partial(self.setCmax, imagetype = Defines.ImageType.ANALOG, index=0))
        self.digitalcminSpinBoxes.editingFinished.connect(partial(self.setCmin, imagetype = Defines.ImageType.DIGITAL, index=0))
        self.digitalcmaxSpinBoxes.editingFinished.connect(partial(self.setCmax, imagetype = Defines.ImageType.DIGITAL, index=0))

        for index, checkBox in enumerate(self.checkBoxCounters): 
            checkBox.stateChanged.connect(lambda state, index=index: self.setCounterCheckBox(index, is_checked=(state == QtCore.Qt.Checked)))


        # show image Values for analog image
        nMaxY = lambda : self.mainWindow.nAnalogRows
        nMaxX = lambda : self.mainWindow.nAnalogCols
        frame = lambda : self.mainWindow.analog_frame
        plot = self.mainWindow.plotAnalogImage
        plot.scene.sigMouseMoved.connect(partial(self.showPlotValues, plot, nMaxX, nMaxY, frame))
        plot.getImageItem().sigImageChanged.connect(partial(self.updateColorRange, imagetype = Defines.ImageType.ANALOG, plot=plot))
        plot.getHistogramWidget().item.sigLevelChangeFinished.connect(lambda _histItem, plot=plot: self.handleHistogramChange(plot, imagetype=Defines.ImageType.ANALOG))


        # show image Values for digital image
        nMaxY = lambda : self.mainWindow.nDigitalRows
        nMaxX = lambda : self.mainWindow.nDigitalCols
        frame = lambda : self.mainWindow.digital_frame
        plot = self.mainWindow.plotDigitalImage
        plot.scene.sigMouseMoved.connect(partial(self.showPlotValues, plot, nMaxX, nMaxY, frame))
        plot.getImageItem().sigImageChanged.connect(partial(self.updateColorRange, imagetype = Defines.ImageType.DIGITAL, plot=plot))
        plot.getHistogramWidget().item.sigLevelChangeFinished.connect(lambda _histItem, plot=plot: self.handleHistogramChange(plot, imagetype=Defines.ImageType.DIGITAL))
        

        # show image Values for transceiver image
        nMaxY = lambda : self.transceiverTab.nTransceiverRows
        nMaxX = lambda : self.transceiverTab.nTransceiverCols

        for index, image_view in enumerate(self.transceiverTab.transceiverImageViews):
            frame = lambda image_view=image_view: image_view.getImageItem().image
            image_view.scene.sigMouseMoved.connect(partial(self.showPlotValues, image_view, nMaxX, nMaxY, frame))
            image_view.getImageItem().sigImageChanged.connect(partial(self.updateColorRange, imagetype = Defines.ImageType.TRANSCEIVER, plot=image_view, index=index))

            image_view.getHistogramWidget().item.sigLevelChangeFinished.connect(
                lambda _histItem, image_view=image_view, index=index: self.handleHistogramChange(plot = image_view, imagetype=Defines.ImageType.TRANSCEIVER, index=index)) # sigLevelChangeFinished passes the histogram item as the first argument, we need to ignore it and pass the image_view instead
            
        self.view.checkBoxShowLegend.stateChanged.connect(self.toggleLegend)


    def setCounterCheckBox(self, index : int, is_checked : bool): 
        """
        slot function for the counter checkboxes, enables/disables the cmin/cmax spinboxes for the counter at index, updates image splitter to show counter image
        """
        self.transceivercmaxSpinBoxes[index].setEnabled(is_checked and self.colorRangeMode == Defines.colorRange.fixed)
        self.transceivercminSpinBoxes[index].setEnabled(is_checked and self.colorRangeMode == Defines.colorRange.fixed)
        self.shownCounterImages[index] = is_checked # TODO: maybe I dont need that 
        self.transceiverTab.update_ImageSplitter()
        
    def enableCounterCheckBox(self, index : int, enabled : bool):
        self.checkBoxCounters[index].setEnabled(enabled)
        is_checked = self.checkBoxCounters[index].isChecked()
        self.transceivercmaxSpinBoxes[index].setEnabled(enabled and is_checked and self.colorRangeMode == Defines.colorRange.fixed)
        self.transceivercminSpinBoxes[index].setEnabled(enabled and is_checked and self.colorRangeMode == Defines.colorRange.fixed)

    def showallCounterCheckBox(self, enabled : bool):
        for checkBox in self.checkBoxCounters:
            checkBox.setVisible(enabled)

        self.view.labelCounters.setVisible(enabled)

        for cminSpinBox, cmaxSpinBox in zip(self.transceivercminSpinBoxes, self.transceivercmaxSpinBoxes):
            cminSpinBox.setEnabled(enabled and self.colorRangeMode == Defines.colorRange.fixed and self.checkBoxCounters[self.transceivercminSpinBoxes.index(cminSpinBox)].isChecked())
            cmaxSpinBox.setEnabled(enabled and self.colorRangeMode == Defines.colorRange.fixed and self.checkBoxCounters[self.transceivercmaxSpinBoxes.index(cmaxSpinBox)].isChecked())
        
    def refresh(self):
        self.getZMQHWM()

    def initializeColorMaps(self):
        self.view.comboBoxColorMap.addItems(Defines.Color_map)
        self.view.comboBoxColorMap.setCurrentIndex(Defines.Color_map.index(Defines.Default_Color_Map))
        self.setColorMap()

    def savePedestal(self):
        """
        slot function to save pedestal values
        """
        response = QtWidgets.QFileDialog.getSaveFileName(self.view, "Save Pedestal", str(self.det.fpath))
        recordOrApplyPedestal.savePedestal(Path(response[0]))
        self.logger.info(f'saved Pedestal in {response[0]}')

    def loadPedestal(self):
        """
        slot function to load pedestal values
        """
        response = QtWidgets.QFileDialog.getOpenFileName(self.view, "Load Pedestal", str(self.det.fpath))
        if response[0] == '':
            return
        recordOrApplyPedestal.reset(self)
        try:
            recordOrApplyPedestal.loadPedestal(Path(response[0]))
        except (IsADirectoryError, ValueError, EOFError):
            QtWidgets.QMessageBox.warning(
                self.view,
                "Loading Pedestal Failed",
                "Loading Pedestal failed make sure the file is in the valid .npy format",
                QtWidgets.QMessageBox.Ok,
            )
            self.logger.exception("Exception when loading pedestal")
        else:
            self.logger.info(f'loaded Pedestal from {response[0]}')
            self.updateLabelPedestalFrames(True)

    def togglePedestalRecord(self):
        """
        slot function for pedestal record radio button
        toggle pedestal record variable and disables the frames spinboxes in acquisition tab or plot tab depending on
        the mode
        """
        self.pedestalRecord = not self.pedestalRecord

    def togglePedestalApply(self):
        """
        slot function for pedestal apply radio button
        """
        self.pedestalApply = not self.pedestalApply

    def resetPedestal(self):
        """
        slot function for resetting the pedestal
        """
        recordOrApplyPedestal.reset(self)

    def updateLabelPedestalFrames(self, loadedPedestal=False):
        """
        updates labelPedestalFrames to the length of savedFrames
        """
        if loadedPedestal:
            self.view.labelPedestalFrames.setText('using loaded pedestal file')
        else:
            self.view.labelPedestalFrames.setText(f'recorded frames: {recordOrApplyPedestal.getFramesCount()}')

    def subscribeToggleLegend(self, fn_cbk):
        """
        subscribe to the event of toggling the hide legend checkbox by subscribing
        with a callback function
        """
        self.hideLegendObservers.append(fn_cbk)

    def toggleLegend(self):
        """
        notify subscribers for the showLegend checkbox event by executing their callbacks
        """
        self.mainWindow.showLegend = not self.mainWindow.showLegend
        for notify_function in self.hideLegendObservers:
            notify_function()

    def setCmin(self, imagetype : Defines.ImageType, value = None, index = 0, updateColorBar=True):
        """
        Set the minimum color range value

        Parameters
        ----------
        imagetype : Defines.ImageType
            The type of image (DIGITAL, ANALOG, or TRANSCEIVER).
        value : float, optional
            The value to set for the minimum color range (default is None).
        index : int, optional
            The index of the counter (default is 0).
        updateColorBar : bool, optional
            Whether to update the color bar (default is True).
        """

        if value is None:
            if imagetype == Defines.ImageType.DIGITAL:
                self.digitalcolorrange[0] = self.digitalcminSpinBoxes.value()
            elif imagetype == Defines.ImageType.ANALOG:
                self.analogcolorrange[0] = self.analogcminSpinBoxes.value()
            else:
                self.transceivercolorrange[index][0] = self.transceivercminSpinBoxes[index].value()
        else:
            if imagetype == Defines.ImageType.DIGITAL:
                self.digitalcolorrange[0] = value
                self.digitalcminSpinBoxes.setValue(self.digitalcolorrange[0])
            elif imagetype == Defines.ImageType.ANALOG:
                self.analogcolorrange[0] = value
                self.analogcminSpinBoxes.setValue(self.analogcolorrange[0])
            else:
                self.transceivercolorrange[index][0] = value
                self.transceivercminSpinBoxes[index].setValue(self.transceivercolorrange[index][0])
            
        if updateColorBar:
            self.updateColorBar(imagetype = imagetype, index=index)

    def setCmax(self, imagetype : Defines.ImageType, value=None, index = 0, updateColorBar=True):
        """
        Set the maximum color range value

        Parameters
        ----------
        imagetype : Defines.ImageType
            The type of image (DIGITAL, ANALOG, or TRANSCEIVER).
        value : float, optional
            The value to set for the minimum color range (default is None).
        index : int, optional
            The index of the counter (default is 0).
        updateColorBar : bool, optional
            Whether to update the color bar (default is True).
        """

        if value is None:
            if imagetype == Defines.ImageType.DIGITAL:
                self.digitalcolorrange[1] = self.digitalcmaxSpinBoxes.value()
            elif imagetype == Defines.ImageType.ANALOG:
                self.analogcolorrange[1] = self.analogcmaxSpinBoxes.value()
            else:
                self.transceivercolorrange[index][1] = self.transceivercmaxSpinBoxes[index].value()
        else:
            if imagetype == Defines.ImageType.DIGITAL:
                self.digitalcolorrange[1] = value
                self.digitalcmaxSpinBoxes.setValue(self.digitalcolorrange[1])
            elif imagetype == Defines.ImageType.ANALOG:
                self.analogcolorrange[1] = value
                self.analogcmaxSpinBoxes.setValue(self.analogcolorrange[1])
            else:
                self.transceivercolorrange[index][1] = value
                self.transceivercmaxSpinBoxes[index].setValue(self.transceivercolorrange[index][1])
            
        if updateColorBar:
            self.updateColorBar(imagetype=imagetype, index=index)

    def setColorRangeMode(self, mode : Defines.colorRange):
        """
        slot for setting the color range mode (all,fixed,3-97%)
        """
        self.colorRangeMode = mode

        # disable or enable cmin/cmax spinboxes
        enableSpinBoxes = mode == Defines.colorRange.fixed


        for cminSpinBox, cmaxSpinBox, checkBox in zip(self.transceivercminSpinBoxes, self.transceivercmaxSpinBoxes, self.checkBoxCounters):
            cminSpinBox.setEnabled(enableSpinBoxes and (checkBox.isChecked() and checkBox.isEnabled() and self.showtransceiverImage)) # if the checkbox is disabled, we still want to enable the spinbox
            cmaxSpinBox.setEnabled(enableSpinBoxes and (checkBox.isChecked() and checkBox.isEnabled() and self.showtransceiverImage)) # if the checkbox is disabled, we still want to enable the spinbox

        self.analogcminSpinBoxes.setEnabled(enableSpinBoxes and self.showanalogImage)
        self.analogcmaxSpinBoxes.setEnabled(enableSpinBoxes and self.showanalogImage)

        self.digitalcminSpinBoxes.setEnabled(enableSpinBoxes and self.showdigitalImage)
        self.digitalcmaxSpinBoxes.setEnabled(enableSpinBoxes and self.showdigitalImage)
            
        for index in range(Defines.max_counters): 
            self.updateColorRange(imagetype=Defines.ImageType.TRANSCEIVER, index=index)

        self.updateColorRange(imagetype=Defines.ImageType.ANALOG)

        self.updateColorRange(imagetype=Defines.ImageType.DIGITAL)

    def handleHistogramChange(self, plot, imagetype : Defines.ImageType, index=0):
        """
        Parameters
        ----------
        plot : Plot
            The plot object whose histogram has changed.
        imagetype : Defines.ImageType
            The type of image (DIGITAL, ANALOG, or TRANSCEIVER).
        index : int, optional
            The index of the counter (default is 0).
        """
        cmin, cmax = plot.getHistogramWidget().item.getLevels()
        self.setCmin(imagetype=imagetype, value=cmin, index=index, updateColorBar=False)
        self.setCmax(imagetype=imagetype, value=cmax, index=index, updateColorBar=False)


    def updateColorRange(self, imagetype : Defines.ImageType, plot = None, index=0):
        """
        Updates the color range of the plot at index based on the current color range mode.
        for mode:
        - auto:  sets cmin and cmax to the maximums/minimum values of the frame
        - 3-97%: limits cmax and cmin so that we ignore 3% from each end of the whole range
        - fixed: this function does not change cmin and cmax

        Parameters  
        ----------
        imagetype : Defines.ImageType
            The type of image (DIGITAL, ANALOG, or TRANSCEIVER).
        plot : Plot, optional
            The plot object to update (default is None).
        index : int, optional
            The index of the counter (default is 0).
        """
        if imagetype == Defines.ImageType.TRANSCEIVER: # update transceiver
            defaultPlot = self.transceiverTab.transceiverImageViews[index]
            cmin, cmax = self.transceivercolorrange[index]
        elif imagetype == Defines.ImageType.DIGITAL: # update digital
            defaultPlot = self.mainWindow.plotDigitalImage
            cmin, cmax = self.digitalcolorrange
        elif imagetype == Defines.ImageType.ANALOG: # update analog
            defaultPlot = self.mainWindow.plotAnalogImage
            cmin, cmax = self.analogcolorrange
        if plot is None:
            plot = defaultPlot

        if self.colorRangeMode == Defines.colorRange.auto:
            if plot.getImageItem().image is None or plot.getImageItem().image.size == 0:
                return
            else:
                frame_min = np.min(plot.getImageItem().image)  
                frame_max = np.max(plot.getImageItem().image)
                self.setCmin(imagetype=imagetype, value=frame_min, index=index) 
                self.setCmax(imagetype=imagetype, value=frame_max, index=index)
        elif self.colorRangeMode == Defines.colorRange.center:
            if plot.getImageItem().image is None or plot.getImageItem().image.size == 0:
                return
            
            else:
                frame_min = np.min(plot.getImageItem().image)
                frame_max = np.max(plot.getImageItem().image)
                self.setCmin(imagetype=imagetype, value=frame_min + 0.03 * (frame_max - frame_min), index=index)
                self.setCmax(imagetype=imagetype, value=frame_max - 0.03 * (frame_max - frame_min), index=index)

        elif self.colorRangeMode == Defines.colorRange.fixed:
            self.setCmin(imagetype=imagetype, value=cmin, index=index)
            self.setCmax(imagetype=imagetype, value=cmax, index=index)

    def updateColorBar(self, imagetype : Defines.ImageType, index = 0): 
        """
        updates the color bar of the plot at index
        """

        plot = None
        cmin, cmax = 0.0, 0.0
        if imagetype == Defines.ImageType.TRANSCEIVER: 
            plot = self.transceiverTab.transceiverImageViews[index]
            cmin, cmax = self.transceivercolorrange[index]
        elif imagetype == Defines.ImageType.DIGITAL: # update digital
            plot = self.mainWindow.plotDigitalImage
            cmin, cmax = self.digitalcolorrange
        elif imagetype == Defines.ImageType.ANALOG: # update analog
            plot = self.mainWindow.plotAnalogImage
            cmin, cmax = self.analogcolorrange
        
        if plot is not None:
            histItem = plot.getHistogramWidget().item
            histItem.blockSignals(True) # avoid triggering handleHistogramChange while setting the levels
            histItem.setLevels(min=cmin, max=cmax)
            histItem.blockSignals(False)
            

    def setColorMap(self):
        cm = pg.colormap.getFromMatplotlib(self.view.comboBoxColorMap.currentText())
        # print(f'color map:{self.comboBoxColorMap.currentText()}')
        self.mainWindow.plotAnalogImage.setColorMap(cm)
        self.mainWindow.plotDigitalImage.setColorMap(cm)
        for i, showncounter in enumerate(self.shownCounterImages):
            self.transceiverTab.transceiverImageViews[i].setColorMap(cm)

    def getZMQHWM(self):

        self.view.comboBoxZMQHWM.currentIndexChanged.disconnect()

        rx_zmq_hwm = self.det.getRxZmqHwm()[0]
        # ensure same value in client zmq
        self.det.setClientZmqHwm(rx_zmq_hwm)

        # high readout, low HWM
        if -1 < rx_zmq_hwm < 25:
            self.view.comboBoxZMQHWM.setCurrentIndex(1)
        # low readout, high HWM
        else:
            self.view.comboBoxZMQHWM.setCurrentIndex(0)
        self.view.comboBoxZMQHWM.currentIndexChanged.connect(self.setZMQHWM)

    def setZMQHWM(self):
        val = self.view.comboBoxZMQHWM.currentIndex()
        # low readout, high HWM
        if val == 0:
            self.det.setRxZmqHwm(Defines.Zmq_hwm_low_speed)
            self.det.setClientZmqHwm(Defines.Zmq_hwm_low_speed)
        # high readout, low HWM
        else:
            self.det.setRxZmqHwm(Defines.Zmq_hwm_high_speed)
            self.det.setClientZmqHwm(Defines.Zmq_hwm_high_speed)

        self.getZMQHWM()

    def addSelectedAnalogPlots(self, i):
        enable = getattr(self.adcTab.view, f"checkBoxADC{i}Plot").isChecked()
        if enable:
            self.mainWindow.analogPlots[i].show()
        if not enable:
            self.mainWindow.analogPlots[i].hide()

    def addAllSelectedAnalogPlots(self):
        for i in range(Defines.adc.count):
            self.addSelectedAnalogPlots(i)

    def removeAllAnalogPlots(self):
        for i in range(Defines.adc.count):
            self.mainWindow.analogPlots[i].hide()

        cm = pg.colormap.get('CET-L9')  # prepare a linear color map
        self.mainWindow.plotDigitalImage.setColorMap(cm)

    def addSelectedDigitalPlots(self, i):
        enable = getattr(self.signalsTab.view, f"checkBoxBIT{i}Plot").isChecked()
        if enable:
            self.mainWindow.digitalPlots[i].show()
        if not enable:
            self.mainWindow.digitalPlots[i].hide()

    def addAllSelectedDigitalPlots(self):
        for i in range(Defines.signals.count):
            self.addSelectedDigitalPlots(i)

    def removeAllDigitalPlots(self):
        for i in range(Defines.signals.count):
            self.mainWindow.digitalPlots[i].hide()

    def addSelectedTransceiverPlots(self, i):
        enable = getattr(self.transceiverTab.view, f"checkBoxTransceiver{i}Plot").isChecked()
        if enable:
            self.mainWindow.transceiverPlots[i].show()
        if not enable:
            self.mainWindow.transceiverPlots[i].hide()

    def addAllSelectedTransceiverPlots(self):
        for i in range(Defines.max_counters):
            self.addSelectedTransceiverPlots(i)

    def removeAllTransceiverPlots(self):
        for i in range(Defines.max_counters):
            self.mainWindow.transceiverPlots[i].hide()

    def showPlot(self):
        self.mainWindow.plotAnalogWaveform.hide()
        self.mainWindow.plotDigitalWaveform.hide()
        self.mainWindow.plotTransceiverWaveform.hide()
        self.mainWindow.plotAnalogImage.hide()
        self.mainWindow.plotDigitalImage.hide()
        self.transceiverTab.transceiverImageSplitter.hide()
        self.view.labelDigitalWaveformOption.setDisabled(True)
        self.view.radioButtonOverlay.setDisabled(True)
        self.view.radioButtonStripe.setDisabled(True)

        if self.mainWindow.romode.value in [0, 2]:
            if self.view.radioButtonWaveform.isChecked():
                self.mainWindow.plotAnalogWaveform.show()
            elif self.view.radioButtonImage.isChecked():
                self.showanalogImage = True
                self.mainWindow.plotAnalogImage.show()
        if self.mainWindow.romode.value in [1, 2, 4]:
            if self.view.radioButtonWaveform.isChecked():
                self.mainWindow.plotDigitalWaveform.show()
            elif self.view.radioButtonImage.isChecked():
                self.showdigitalImage = True
                self.mainWindow.plotDigitalImage.show()
            self.view.labelDigitalWaveformOption.setEnabled(True)
            self.view.radioButtonOverlay.setEnabled(True)
            self.view.radioButtonStripe.setEnabled(True)

        if self.mainWindow.romode.value in [3, 4]:
            if self.view.radioButtonWaveform.isChecked():
                self.mainWindow.plotTransceiverWaveform.show()
            elif self.view.radioButtonImage.isChecked():
                self.showtransceiverImage = True
                for i, showncounter in enumerate(self.shownCounterImages):
                    self.transceiverTab.transceiverImageViews[i].setVisible(showncounter)
            
                self.transceiverTab.transceiverImageSplitter.show()

    def plotOptions(self):

        self.mainWindow.framePatternViewer.hide()
        self.showPlot()

        # disable plotting
        self.mainWindow.read_timer.stop()

        if self.view.radioButtonWaveform.isChecked():
            self.mainWindow.plotAnalogWaveform.setLabel(
                'left', "<span style=\"color:black;font-size:14px\">Output [ADC]</span>")
            self.mainWindow.plotAnalogWaveform.setLabel(
                'bottom', "<span style=\"color:black;font-size:14px\">Analog Sample [#]</span>")
            self.mainWindow.plotDigitalWaveform.setLabel(
                'left', "<span style=\"color:black;font-size:14px\">Digital Bit</span>")
            self.mainWindow.plotDigitalWaveform.setLabel(
                'bottom', "<span style=\"color:black;font-size:14px\">Digital Sample [#]</span>")
            self.mainWindow.plotTransceiverWaveform.setLabel(
                'left', "<span style=\"color:black;font-size:14px\">Transceiver Bit</span>")
            self.mainWindow.plotTransceiverWaveform.setLabel(
                'bottom', "<span style=\"color:black;font-size:14px\">Transceiver Sample [#]</span>")

            self.view.stackedWidgetPlotType.setCurrentIndex(0)
            self.showallCounterCheckBox(False) # disable counter checkboxes for waveform

        elif self.view.radioButtonImage.isChecked():
            self.view.stackedWidgetPlotType.setCurrentIndex(2)
            is_transceiver = self.mainWindow.romode.value in [3, 4]
            self.showallCounterCheckBox(is_transceiver) # enable counter checkboxes for matterhorn
            self.setDecoder()

        if self.view.radioButtonNoPlot.isChecked():
            self.view.labelPlotOptions.hide()
            self.view.stackedWidgetPlotType.hide()
            self.showallCounterCheckBox(False) # disable counter checkboxes when no plot is selected
        # enable plotting
        else:
            self.view.labelPlotOptions.show()
            self.view.stackedWidgetPlotType.show()
            self.mainWindow.read_timer.start(Defines.Time_Plot_Refresh_ms)

    def setDecoder(self):
        # TODO: really dont like to set attributes on the fly - hard to understand whats going on
        if self.view.comboBoxPlot.currentText() == "Matterhorn02":
            print("Initializing decoder for Matterhorn02")
            self.transceiverTab.nTransceiverRows = Matterhorn02.nRows
            self.transceiverTab.nTransceiverCols = Matterhorn02.nCols
            self.transceiverTab.update_numCounters(1)
            self.mainWindow.decoder = transform.Matterhorn02TransceiverTransform() 
        elif self.view.comboBoxPlot.currentText() == "Matterhorn1_16bit_1_counter":
            print("Initializing decoder for Matterhorn1 with 1 counter 16 bit dynamic range")
            self.transceiverTab.nTransceiverRows = Matterhorn10.nRows
            self.transceiverTab.nTransceiverCols = Matterhorn10.nCols
            self.transceiverTab.update_numCounters(1)
            self.mainWindow.decoder = transform.Matterhorn10Transform(16, 1)
        elif self.view.comboBoxPlot.currentText() == "Matterhorn1_16bit_4_counters":
            print("Initializing decoder for Matterhorn1 with 4 counters 16 bit dynamic range")
            self.transceiverTab.nTransceiverRows = Matterhorn10.nRows
            self.transceiverTab.nTransceiverCols = Matterhorn10.nCols
            self.transceiverTab.update_numCounters(4) 
            self.mainWindow.decoder = transform.Matterhorn10Transform(16, 4)
        elif self.view.comboBoxPlot.currentText() == "Matterhorn1_8bit_1_counter":
            print("Initializing decoder for Matterhorn1 with 1 counter 8 bit dynamic range")
            self.transceiverTab.nTransceiverRows = Matterhorn10.nRows
            self.transceiverTab.nTransceiverCols = Matterhorn10.nCols
            self.transceiverTab.update_numCounters(1)
            self.mainWindow.decoder = transform.Matterhorn10Transform(8, 1)
        elif self.view.comboBoxPlot.currentText() == "Matterhorn1_8bit_4_counters":
            print("Initializing decoder for Matterhorn1 with 4 counters 8 bit dynamic range")
            self.transceiverTab.nTransceiverRows = Matterhorn10.nRows
            self.transceiverTab.nTransceiverCols = Matterhorn10.nCols
            self.transceiverTab.update_numCounters(4)
            self.mainWindow.decoder = transform.Matterhorn10Transform(8, 4)
        elif self.view.comboBoxPlot.currentText() == "Matterhorn1_4bit_4_counters":
            print("Initializing decoder for Matterhorn1 with 4 counters 4 bit dynamic range")
            self.transceiverTab.nTransceiverRows = Matterhorn10.nRows
            self.transceiverTab.nTransceiverCols = Matterhorn10.nCols
            self.transceiverTab.update_numCounters(4)
            self.mainWindow.decoder = transform.Matterhorn10Transform(4, 4)
        elif self.view.comboBoxPlot.currentText() == "Matterhorn1_4bit_1_counter":
            print("Initializing decoder for Matterhorn1 with 1 counter 4 bit dynamic range")
            self.transceiverTab.nTransceiverRows = Matterhorn10.nRows
            self.transceiverTab.nTransceiverCols = Matterhorn10.nCols
            self.transceiverTab.update_numCounters(1)
            self.mainWindow.decoder = transform.Matterhorn10Transform(4, 1)
        elif self.view.comboBoxPlot.currentText() == "Moench04":
            self.mainWindow.nAnalogRows = Moench04.nRows
            self.mainWindow.nAnalogCols = Moench04.nCols
            self.mainWindow.decoder = transform.Moench04AnalogTransform() 
        elif self.view.comboBoxPlot.currentText() == "Moench05":
            self.mainWindow.nAnalogRows = Moench05.nRows
            self.mainWindow.nAnalogCols = Moench05.nCols 
            self.mainWindow.decoder = transform.Moench05Transform()

        try: 
            if hasattr(self.mainWindow.decoder, "compatibility") and callable(getattr(self.mainWindow.decoder, "compatibility")):
                self.mainWindow.decoder.compatibility(ReadoutMode(self.mainWindow.romode.value))
        except Exception as e:
            self.mainWindow.statusbar.setStyleSheet("color:red")
            self.mainWindow.statusbar.showMessage(str(e))
            print("Error: ", str(e))
    

    def showPatternViewer(self, enable):
        if enable:
            self.mainWindow.framePatternViewer.show()
            self.mainWindow.framePlot.hide()
        elif self.mainWindow.framePatternViewer.isVisible():
            self.mainWindow.framePatternViewer.hide()
            self.mainWindow.framePlot.show()
            self.showPlot()

    def setSerialOffset(self):
        print("plot options - Not implemented yet")
        # TODO:

    def setNCounter(self):
        print("plot options - Not implemented yet")
        # TODO:

    def setDynamicRange(self):
        print("plot options - Not implemented yet")
        # TODO:

    def setImageX(self):
        print("plot options - Not implemented yet")
        # TODO:

    def setImageY(self):
        print("plot options - Not implemented yet")
        # TODO:

    def setRawData(self):
        print("plot options - Not implemented yet")
        # TODO: raw data, min, max

    def setPedestalSubtract(self):
        print("plot options - Not implemented yet")
        # TODO: pedestal, min, max

    def setFitADC(self):
        print("plot options - Not implemented yet")
        # TODO:

    def setPlotBit(self):
        print("plot options - Not implemented yet")
        # TODO:

    def getRandomColor(self):
        '''
        Returns a random color range (except white) in format string eg. "#aabbcc"
        '''
        randomColor = random.randrange(0, 0xffffaa, 0xaa)
        return "#{:06x}".format(randomColor)

    def getActiveColor(self, button):
        return button.palette().color(QtGui.QPalette.Window)

    def setActiveColor(self, button, str_color):
        button.setStyleSheet(":enabled {background-color: %s" % str_color + "} :disabled {background-color: grey}")

    def showPalette(self, button):
        color = QtWidgets.QColorDialog.getColor()
        if color.isValid():
            self.setActiveColor(button, color.name())
            # get the RGB Values
            # print(color.getRgb())

    def showPlotValues(self, sender, get_nMaxX, get_nMaxY, get_frame, pos):  
        x = sender.getImageItem().mapFromScene(pos).x()
        y = sender.getImageItem().mapFromScene(pos).y()
        val = 0
        nMaxX = get_nMaxX()
        nMaxY = get_nMaxY()
        frame = get_frame()
        if 0 <= x < nMaxX and 0 <= y < nMaxY and not np.array_equal(frame, []):
            val = frame[int(y), int(x)]
            message = f'[row, col]: [{y:.2f}, {x:.2f}] = {val:.2f}'
            sender.setToolTip(message)
            # print(message)
        else:
            sender.setToolTip('')

    def saveParameters(self):
        commands = []
        if self.view.comboBoxZMQHWM.currentIndex() == 0:
            commands.append(f"zmqhwm {Defines.Zmq_hwm_low_speed}")
        else:
            commands.append(f"zmqhwm {Defines.Zmq_hwm_high_speed}")
        return commands
