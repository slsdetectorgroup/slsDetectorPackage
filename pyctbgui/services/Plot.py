from functools import partial
import random
from pathlib import Path

from PyQt5 import QtWidgets, QtGui, uic

import pyqtgraph as pg

from ..utils.defines import Defines
from ..utils.pixelmap import moench04_analog, matterhorn_transceiver


class PlotTab(QtWidgets.QWidget):

    def __init__(self, parent):
        super(PlotTab, self).__init__(parent)
        uic.loadUi(Path(__file__).parent.parent / 'ui' / "plot.ui", parent)
        self.view = parent
        self.mainWindow = None
        self.det = None
        self.signalsTab = None
        self.transceiverTab = None
        self.acquisitionTab = None
        self.adcTab = None

    def setup_ui(self):
        self.signalsTab = self.mainWindow.signalsTab
        self.transceiverTab = self.mainWindow.transceiverTab
        self.acquisitionTab = self.mainWindow.acquisitionTab
        self.adcTab = self.mainWindow.adcTab

        self.initializeColorMaps()

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
        self.view.checkBoxAcquire.stateChanged.connect(self.setPedestal)
        self.view.checkBoxSubtract.stateChanged.connect(self.setPedestal)
        self.view.checkBoxCommonMode.stateChanged.connect(self.setPedestal)
        self.view.pushButtonReset.clicked.connect(self.resetPedestal)
        self.view.checkBoxRaw.stateChanged.connect(self.setRawData)
        self.view.spinBoxRawMin.editingFinished.connect(self.setRawData)
        self.view.spinBoxRawMax.editingFinished.connect(self.setRawData)
        self.view.checkBoxPedestal.stateChanged.connect(self.setPedestalSubtract)
        self.view.spinBoxPedestalMin.editingFinished.connect(self.setPedestalSubtract)
        self.view.spinBoxPedestalMax.editingFinished.connect(self.setPedestalSubtract)
        self.view.spinBoxFit.editingFinished.connect(self.setFitADC)
        self.view.spinBoxPlot.editingFinished.connect(self.setPlotBit)
        self.view.pushButtonReferesh.clicked.connect(self.acquisitionTab.refresh)

        self.mainWindow.plotAnalogImage.scene.sigMouseMoved.connect(
            partial(self.showPlotValues, self.mainWindow.plotAnalogImage))
        self.mainWindow.plotDigitalImage.scene.sigMouseMoved.connect(
            partial(self.showPlotValues, self.mainWindow.plotDigitalImage))
        self.mainWindow.plotTransceiverImage.scene.sigMouseMoved.connect(
            partial(self.showPlotValues, self.mainWindow.plotTransceiverImage))

    def refresh(self):
        self.getZMQHWM()

    def initializeColorMaps(self):
        self.view.comboBoxColorMap.addItems(Defines.Color_map)
        self.view.comboBoxColorMap.setCurrentIndex(Defines.Color_map.index(Defines.Default_Color_Map))
        self.setColorMap()

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
        if rx_zmq_hwm < 25 and rx_zmq_hwm > -1:
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
            self.mainWindow.plotAnalogWaveform.addLegend(colCount=4)
            self.mainWindow.plotDigitalWaveform.setLabel(
                'left', "<span style=\"color:black;font-size:14px\">Digital Bit</span>")
            self.mainWindow.plotDigitalWaveform.setLabel(
                'bottom', "<span style=\"color:black;font-size:14px\">Digital Sample [#]</span>")
            self.mainWindow.plotDigitalWaveform.addLegend(colCount=4)
            self.mainWindow.plotTransceiverWaveform.setLabel(
                'left', "<span style=\"color:black;font-size:14px\">Transceiver Bit</span>")
            self.mainWindow.plotTransceiverWaveform.setLabel(
                'bottom', "<span style=\"color:black;font-size:14px\">Transceiver Sample [#]</span>")
            self.mainWindow.plotTransceiverWaveform.addLegend(colCount=4)

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
            self.mainWindow.pixelMapTransceiver = matterhorn_transceiver()
        elif self.view.comboBoxPlot.currentText() == "Moench04":
            self.mainWindow.nAnalogRows = Defines.Moench04.nRows
            self.mainWindow.nAnalogCols = Defines.Moench04.nCols
            self.mainWindow.pixelMapAnalog = moench04_analog()

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

    def setPedestal(self):
        print("plot options - Not implemented yet")
        # TODO: acquire, subtract, common mode

    def resetPedestal(self):
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
        if 0 <= x < nMaxX and 0 <= y < nMaxY:
            val = frame[int(x), int(y)]
            message = f'[{x:.2f}, {y:.2f}] = {val:.2f}'
            sender.setToolTip(message)
            #print(message)
        else:
            sender.setToolTip('')
