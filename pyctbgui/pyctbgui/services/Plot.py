import logging
from functools import partial
import random
from pathlib import Path

import numpy as np
from PyQt5 import QtWidgets, QtGui, uic

import pyqtgraph as pg
from pyctbgui.utils import recordOrApplyPedestal
from pyqtgraph import PlotWidget

from pyctbgui.utils.defines import Defines


class PlotTab(QtWidgets.QWidget):

    def __init__(self, parent):
        super().__init__(parent)
        self.frame_min: float = 0.0
        self.frame_max: float = 0.0
        uic.loadUi(Path(__file__).parent.parent / 'ui' / "plot.ui", parent)
        self.view = parent
        self.mainWindow = None
        self.det = None
        self.signalsTab = None
        self.transceiverTab = None
        self.acquisitionTab = None
        self.adcTab = None
        self.cmin: float = 0.0
        self.cmax: float = 0.0
        self.colorRangeMode: Defines.colorRange = Defines.colorRange.all
        self.ignoreHistogramSignal: bool = False
        self.imagePlots: list[PlotWidget] = []
        # list of callback functions to notify tabs when we should hide their legend
        # follows the observer design pattern
        self.hideLegendObservers = []
        self.pedestalRecord: bool = False
        self.pedestalApply: bool = True
        self.__acqFrames = None
        self.logger = logging.getLogger('PlotTab')

    def setup_ui(self):
        self.signalsTab = self.mainWindow.signalsTab
        self.transceiverTab = self.mainWindow.transceiverTab
        self.acquisitionTab = self.mainWindow.acquisitionTab
        self.adcTab = self.mainWindow.adcTab
        self.initializeColorMaps()

        self.imagePlots = (
            self.mainWindow.plotAnalogImage,
            self.mainWindow.plotDigitalImage,
            self.mainWindow.plotTransceiverImage,
        )

    def connect_ui(self):
        self.view.radioButtonNoPlot.clicked.connect(self.plotOptions)
        self.view.radioButtonWaveform.clicked.connect(self.plotOptions)
        self.view.radioButtonDistribution.clicked.connect(self.plotOptions)
        self.view.radioButtonImage.clicked.connect(self.plotOptions)
        self.view.comboBoxPlot.currentIndexChanged.connect(self.setPixelMap)
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
        self.view.cminSpinBox.editingFinished.connect(self.setCmin)
        self.view.cmaxSpinBox.editingFinished.connect(self.setCmax)
        self.view.radioButtonAutomatic.clicked.connect(partial(self.setColorRangeMode, Defines.colorRange.all))
        self.view.radioButtonFixed.clicked.connect(partial(self.setColorRangeMode, Defines.colorRange.fixed))
        self.view.radioButtonCenter.clicked.connect(partial(self.setColorRangeMode, Defines.colorRange.center))

        for plot in self.imagePlots:
            plot.scene.sigMouseMoved.connect(partial(self.showPlotValues, plot))
            plot.getHistogramWidget().item.sigLevelChangeFinished.connect(partial(self.handleHistogramChange, plot))

        self.view.checkBoxShowLegend.stateChanged.connect(self.toggleLegend)

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
        toggle pedestal record variable and disables the frames spinboxes in acquisition tab or plot tab depenging on
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

    def setCmin(self, value=None):
        """
        slot for setting cmin from cminSpinBox
        also used as a normal function
        """
        if value is None:
            self.cmin = self.view.cminSpinBox.value()
        else:
            self.cmin = value
        self.updateColorRangeUI()

    def setCmax(self, value=None):
        """
        slot for setting cmax from cmaxSpinBox
        also used as a normal function
        """
        if value is None:
            self.cmax = self.view.cmaxSpinBox.value()
        else:
            self.cmax = value
        self.updateColorRangeUI()

    def setColorRangeMode(self, mode):
        """
        slot for setting the color range mode (all,fixed,3-97%)
        """
        self.colorRangeMode = mode

        # disable or enable cmin/cmax spinboxes
        enableSpinBoxes = mode == Defines.colorRange.fixed
        self.view.cminSpinBox.setEnabled(enableSpinBoxes)
        self.view.cmaxSpinBox.setEnabled(enableSpinBoxes)
        self.updateColorRange()
        self.updateColorRangeUI()

    def handleHistogramChange(self, plot):
        """
        slot called after changing the color bar
        This is called even when pyqtgraph sets the image without any user intervention
        the class attribute ignore_histogram_signal is set to False before setting the image
        so that we can distinguish between the signal originates from pyqt or from the user
        """
        if not self.ignoreHistogramSignal:
            self.cmin, self.cmax = plot.getHistogramWidget().item.getLevels()
            self.setCmin(self.cmin)
            self.setCmax(self.cmax)

        self.ignoreHistogramSignal = False
        # self.setColorRangeMode(Defines.colorRange.fixed)

    def setFrameLimits(self, frame):
        """
        function called from AcquisitionTab::read_zmq to get the maximum and minimum
        values of the decoded frame and save them in frame_min/frame_max
        """
        self.frame_min = np.min(frame)
        self.frame_max = np.max(frame)
        self.updateColorRange()

    def updateColorRange(self):
        """
        for mode:
        - all:   sets cmin and cmax to the maximums/minimum values of the frame
        - 3-97%: limits cmax and cmin so that we ignore 3% from each end of the whole range
        - fixed: this function does not change cmin and cmax
        """

        if self.colorRangeMode == Defines.colorRange.all:
            self.cmin = self.frame_min
            self.cmax = self.frame_max
        elif self.colorRangeMode == Defines.colorRange.center:
            self.cmin = self.frame_min + 0.03 * (self.frame_max - self.frame_min)
            self.cmax = self.frame_max - 0.03 * (self.frame_max - self.frame_min)

        self.updateColorRangeUI()

    def updateColorRangeUI(self):
        """
        updates UI views should be called after every change to cmin or cmax
        """
        for plot in self.imagePlots:
            plot.getHistogramWidget().item.setLevels(min=self.cmin, max=self.cmax)
        self.view.cminSpinBox.setValue(self.cmin)
        self.view.cmaxSpinBox.setValue(self.cmax)

    def setColorMap(self):
        cm = pg.colormap.getFromMatplotlib(self.view.comboBoxColorMap.currentText())
        # print(f'color map:{self.comboBoxColorMap.currentText()}')
        self.mainWindow.plotAnalogImage.setColorMap(cm)
        self.mainWindow.plotDigitalImage.setColorMap(cm)
        self.mainWindow.plotTransceiverImage.setColorMap(cm)

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
        for i in range(Defines.transceiver.count):
            self.addSelectedTransceiverPlots(i)

    def removeAllTransceiverPlots(self):
        for i in range(Defines.transceiver.count):
            self.mainWindow.transceiverPlots[i].hide()

    def showPlot(self):
        self.mainWindow.plotAnalogWaveform.hide()
        self.mainWindow.plotDigitalWaveform.hide()
        self.mainWindow.plotTransceiverWaveform.hide()
        self.mainWindow.plotAnalogImage.hide()
        self.mainWindow.plotDigitalImage.hide()
        self.mainWindow.plotTransceiverImage.hide()
        self.view.labelDigitalWaveformOption.setDisabled(True)
        self.view.radioButtonOverlay.setDisabled(True)
        self.view.radioButtonStripe.setDisabled(True)

        if self.mainWindow.romode.value in [0, 2]:
            if self.view.radioButtonWaveform.isChecked():
                self.mainWindow.plotAnalogWaveform.show()
            elif self.view.radioButtonImage.isChecked():
                self.mainWindow.plotAnalogImage.show()
        if self.mainWindow.romode.value in [1, 2, 4]:
            if self.view.radioButtonWaveform.isChecked():
                self.mainWindow.plotDigitalWaveform.show()
            elif self.view.radioButtonImage.isChecked():
                self.mainWindow.plotDigitalImage.show()
            self.view.labelDigitalWaveformOption.setEnabled(True)
            self.view.radioButtonOverlay.setEnabled(True)
            self.view.radioButtonStripe.setEnabled(True)

        if self.mainWindow.romode.value in [3, 4]:
            if self.view.radioButtonWaveform.isChecked():
                self.mainWindow.plotTransceiverWaveform.show()
            elif self.view.radioButtonImage.isChecked():
                self.mainWindow.plotTransceiverImage.show()

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

        elif self.view.radioButtonImage.isChecked():
            self.view.stackedWidgetPlotType.setCurrentIndex(2)
            self.setPixelMap()

        if self.view.radioButtonNoPlot.isChecked():
            self.view.labelPlotOptions.hide()
            self.view.stackedWidgetPlotType.hide()
        # enable plotting
        else:
            self.view.labelPlotOptions.show()
            self.view.stackedWidgetPlotType.show()
            self.mainWindow.read_timer.start(Defines.Time_Plot_Refresh_ms)

    def setPixelMap(self):
        if self.view.comboBoxPlot.currentText() == "Matterhorn":
            self.mainWindow.nTransceiverRows = Defines.Matterhorn.nRows
            self.mainWindow.nTransceiverCols = Defines.Matterhorn.nCols
        elif self.view.comboBoxPlot.currentText() == "Moench04":
            self.mainWindow.nAnalogRows = Defines.Moench04.nRows
            self.mainWindow.nAnalogCols = Defines.Moench04.nCols

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

    def showPlotValues(self, sender, pos):
        x = sender.getImageItem().mapFromScene(pos).x()
        y = sender.getImageItem().mapFromScene(pos).y()
        val = 0
        nMaxY = self.mainWindow.nAnalogRows
        nMaxX = self.mainWindow.nAnalogCols
        frame = self.mainWindow.analog_frame
        if sender == self.mainWindow.plotDigitalImage:
            nMaxY = self.mainWindow.nDigitalRows
            nMaxX = self.mainWindow.nDigitalCols
            frame = self.mainWindow.digital_frame
        elif sender == self.mainWindow.plotTransceiverImage:
            nMaxY = self.mainWindow.nTransceiverRows
            nMaxX = self.mainWindow.nTransceiverCols
            frame = self.mainWindow.transceiver_frame
        if 0 <= x < nMaxX and 0 <= y < nMaxY and not np.array_equal(frame, []):
            val = frame[int(x), int(y)]
            message = f'[{x:.2f}, {y:.2f}] = {val:.2f}'
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
