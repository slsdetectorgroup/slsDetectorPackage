from functools import partial

import numpy as np
import random

from PyQt5 import QtWidgets,QtGui
from ..utils.pixelmap import moench04_analog, matterhorn_transceiver

import pyqtgraph as pg

from ..utils.defines import Defines


class Plot:
    def __init__(self, mainWindow):
        self.mainWindow = mainWindow
        self.det = self.mainWindow.det

    def setup_ui(self):
        self.initializeAllAnalogPlots()
        self.initializeAllDigitalPlots()
        self.initializeAllTransceiverPlots()
        self.initializeColorMaps()

    def connect_ui(self):
        self.mainWindow.plotAnalogImage.scene.sigMouseMoved.connect(partial(self.showPlotValues, self.mainWindow.plotAnalogImage))
        self.mainWindow.plotDigitalImage.scene.sigMouseMoved.connect(partial(self.showPlotValues, self.mainWindow.plotDigitalImage))
        self.mainWindow.plotTransceiverImage.scene.sigMouseMoved.connect(partial(self.showPlotValues, self.mainWindow.plotTransceiverImage))

    def refresh(self):
        self.getZMQHWM()

    def initializeColorMaps(self):
        self.mainWindow.comboBoxColorMap.addItems(Defines.Color_map)
        self.mainWindow.comboBoxColorMap.setCurrentIndex(Defines.Color_map.index(Defines.Default_Color_Map))
        self.setColorMap()

    def setColorMap(self):
        cm = pg.colormap.getFromMatplotlib(self.mainWindow.comboBoxColorMap.currentText())
        # print(f'color map:{self.comboBoxColorMap.currentText()}')
        self.mainWindow.plotAnalogImage.setColorMap(cm)
        self.mainWindow.plotDigitalImage.setColorMap(cm)
        self.mainWindow.plotTransceiverImage.setColorMap(cm)

    def getZMQHWM(self):

        self.mainWindow.comboBoxZMQHWM.currentIndexChanged.disconnect()

        rx_zmq_hwm = self.det.getRxZmqHwm()[0]
        # ensure same value in client zmq
        self.det.setClientZmqHwm(rx_zmq_hwm)

        # high readout, low HWM
        if rx_zmq_hwm < 25 and rx_zmq_hwm > -1:
            self.mainWindow.comboBoxZMQHWM.setCurrentIndex(1)
        # low readout, high HWM
        else:
            self.mainWindow.comboBoxZMQHWM.setCurrentIndex(0)
        self.mainWindow.comboBoxZMQHWM.currentIndexChanged.connect(self.setZMQHWM)

    def setZMQHWM(self):
        val = self.mainWindow.comboBoxZMQHWM.currentIndex()
        # low readout, high HWM
        if val == 0:
            self.det.setRxZmqHwm(Defines.Zmq_hwm_low_speed)
            self.det.setClientZmqHwm(Defines.Zmq_hwm_low_speed)
        # high readout, low HWM
        else:
            self.det.setRxZmqHwm(Defines.Zmq_hwm_high_speed)
            self.det.setClientZmqHwm(Defines.Zmq_hwm_high_speed)

        self.getZMQHWM()

    def initializeAllAnalogPlots(self):

        self.mainWindow.plotAnalogWaveform = pg.plot()
        self.mainWindow.verticalLayoutPlot.addWidget(self.mainWindow.plotAnalogWaveform, 1)
        self.mainWindow.analogPlots = {}
        waveform = np.zeros(1000)
        for i in range(32):
            pen = pg.mkPen(color=self.mainWindow.adcTab.getADCButtonColor(i), width=1)
            legendName = getattr(self.mainWindow, f"labelADC{i}").text()
            self.mainWindow.analogPlots[i] = self.mainWindow.plotAnalogWaveform.plot(waveform, pen=pen, name=legendName)
            self.mainWindow.analogPlots[i].hide()

        self.mainWindow.plotAnalogImage = pg.ImageView()
        self.mainWindow.nAnalogRows = 0
        self.mainWindow.nAnalogCols = 0
        self.mainWindow.analog_frame = np.zeros((self.mainWindow.nAnalogRows, self.mainWindow.nAnalogCols))
        self.mainWindow.plotAnalogImage.getView().invertY(False)
        self.mainWindow.plotAnalogImage.setImage(self.mainWindow.analog_frame)
        self.mainWindow.verticalLayoutPlot.addWidget(self.mainWindow.plotAnalogImage, 2)

    def addSelectedAnalogPlots(self, i):
        enable = getattr(self.mainWindow, f"checkBoxADC{i}Plot").isChecked()
        if enable:
            self.mainWindow.analogPlots[i].show()
        if not enable:
            self.mainWindow.analogPlots[i].hide()

    def addAllSelectedAnalogPlots(self):
        for i in range(32):
            self.addSelectedAnalogPlots(i)

    def removeAllAnalogPlots(self):
        for i in range(32):
            self.mainWindow.analogPlots[i].hide()

    def initializeAllDigitalPlots(self):
        self.mainWindow.plotDigitalWaveform = pg.plot()
        self.mainWindow.verticalLayoutPlot.addWidget(self.mainWindow.plotDigitalWaveform, 3)
        self.mainWindow.digitalPlots = {}
        waveform = np.zeros(1000)
        for i in range(64):
            pen = pg.mkPen(color=self.mainWindow.signalTab.getDBitButtonColor(i), width=1)
            legendName = getattr(self.mainWindow, f"labelBIT{i}").text()
            self.mainWindow.digitalPlots[i] = self.mainWindow.plotDigitalWaveform.plot(waveform, pen=pen, name=legendName, stepMode="left")
            self.mainWindow.digitalPlots[i].hide()

        self.mainWindow.plotDigitalImage = pg.ImageView()
        self.mainWindow.nDigitalRows = 0
        self.mainWindow.nDigitalCols = 0
        self.mainWindow.digital_frame = np.zeros((self.mainWindow.nDigitalRows, self.mainWindow.nDigitalCols))
        self.mainWindow.plotDigitalImage.setImage(self.mainWindow.digital_frame)
        self.mainWindow.verticalLayoutPlot.addWidget(self.mainWindow.plotDigitalImage, 4)

        cm = pg.colormap.get('CET-L9')  # prepare a linear color map
        self.mainWindow.plotDigitalImage.setColorMap(cm)

    def addSelectedDigitalPlots(self, i):
        enable = getattr(self.mainWindow, f"checkBoxBIT{i}Plot").isChecked()
        if enable:
            self.mainWindow.digitalPlots[i].show()
        if not enable:
            self.mainWindow.digitalPlots[i].hide()

    def addAllSelectedDigitalPlots(self):
        for i in range(64):
            self.addSelectedDigitalPlots(i)

    def removeAllDigitalPlots(self):
        for i in range(64):
            self.mainWindow.digitalPlots[i].hide()

    def initializeAllTransceiverPlots(self):
        self.mainWindow.plotTransceiverWaveform = pg.plot()
        self.mainWindow.verticalLayoutPlot.addWidget(self.mainWindow.plotTransceiverWaveform, 5)
        self.mainWindow.transceiverPlots = {}
        waveform = np.zeros(1000)
        for i in range(4):
            pen = pg.mkPen(color=self.mainWindow.transceiverTab.getTransceiverButtonColor(i), width=1)
            legendName = getattr(self.mainWindow, f"labelTransceiver{i}").text()
            self.mainWindow.transceiverPlots[i] = self.mainWindow.plotTransceiverWaveform.plot(waveform, pen=pen, name=legendName)
            self.mainWindow.transceiverPlots[i].hide()

        self.mainWindow.plotTransceiverImage = pg.ImageView()
        self.mainWindow.nTransceiverRows = 0
        self.mainWindow.nTransceiverCols = 0
        self.mainWindow.transceiver_frame = np.zeros((self.mainWindow.nTransceiverRows, self.mainWindow.nTransceiverCols))
        self.mainWindow.plotTransceiverImage.setImage(self.mainWindow.transceiver_frame)
        self.mainWindow.verticalLayoutPlot.addWidget(self.mainWindow.plotTransceiverImage, 6)

        cm = pg.colormap.get('CET-L9')  # prepare a linear color map
        self.mainWindow.plotTransceiverImage.setColorMap(cm)

    def addSelectedTransceiverPlots(self, i):
        enable = getattr(self.mainWindow, f"checkBoxTransceiver{i}Plot").isChecked()
        if enable:
            self.mainWindow.transceiverPlots[i].show()
        if not enable:
            self.mainWindow.transceiverPlots[i].hide()

    def addAllSelectedTransceiverPlots(self):
        for i in range(4):
            self.addSelectedTransceiverPlots(i)

    def removeAllTransceiverPlots(self):
        for i in range(4):
            self.mainWindow.transceiverPlots[i].hide()

    def showPlot(self):
        self.mainWindow.plotAnalogWaveform.hide()
        self.mainWindow.plotDigitalWaveform.hide()
        self.mainWindow.plotTransceiverWaveform.hide()
        self.mainWindow.plotAnalogImage.hide()
        self.mainWindow.plotDigitalImage.hide()
        self.mainWindow.plotTransceiverImage.hide()
        self.mainWindow.labelDigitalWaveformOption.setDisabled(True)
        self.mainWindow.radioButtonOverlay.setDisabled(True)
        self.mainWindow.radioButtonStripe.setDisabled(True)

        if self.mainWindow.romode.value in [0, 2]:
            if self.mainWindow.radioButtonWaveform.isChecked():
                self.mainWindow.plotAnalogWaveform.show()
            elif self.mainWindow.radioButtonImage.isChecked():
                self.mainWindow.plotAnalogImage.show()
        if self.mainWindow.romode.value in [1, 2, 4]:
            if self.mainWindow.radioButtonWaveform.isChecked():
                self.mainWindow.plotDigitalWaveform.show()
            elif self.mainWindow.radioButtonImage.isChecked():
                self.mainWindow.plotDigitalImage.show()
            self.mainWindow.labelDigitalWaveformOption.setEnabled(True)
            self.mainWindow.radioButtonOverlay.setEnabled(True)
            self.mainWindow.radioButtonStripe.setEnabled(True)

        if self.mainWindow.romode.value in [3, 4]:
            if self.mainWindow.radioButtonWaveform.isChecked():
                self.mainWindow.plotTransceiverWaveform.show()
            elif self.mainWindow.radioButtonImage.isChecked():
                self.mainWindow.plotTransceiverImage.show()

    def plotOptions(self):

        self.mainWindow.framePatternViewer.hide()
        self.showPlot()

        # disable plotting
        self.mainWindow.read_timer.stop()

        if self.mainWindow.radioButtonWaveform.isChecked():
            self.mainWindow.plotAnalogWaveform.setLabel('left', "<span style=\"color:black;font-size:14px\">Output [ADC]</span>")
            self.mainWindow.plotAnalogWaveform.setLabel('bottom',
                                             "<span style=\"color:black;font-size:14px\">Analog Sample [#]</span>")
            self.mainWindow.plotAnalogWaveform.addLegend(colCount=4)
            self.mainWindow.plotDigitalWaveform.setLabel('left', "<span style=\"color:black;font-size:14px\">Digital Bit</span>")
            self.mainWindow.plotDigitalWaveform.setLabel('bottom',
                                              "<span style=\"color:black;font-size:14px\">Digital Sample [#]</span>")
            self.mainWindow.plotDigitalWaveform.addLegend(colCount=4)
            self.mainWindow.plotTransceiverWaveform.setLabel('left',
                                                  "<span style=\"color:black;font-size:14px\">Transceiver Bit</span>")
            self.mainWindow.plotTransceiverWaveform.setLabel('bottom',
                                                  "<span style=\"color:black;font-size:14px\">Transceiver Sample [#]</span>")
            self.mainWindow.plotTransceiverWaveform.addLegend(colCount=4)

            self.mainWindow.stackedWidgetPlotType.setCurrentIndex(0)

        elif self.mainWindow.radioButtonImage.isChecked():
            self.mainWindow.stackedWidgetPlotType.setCurrentIndex(2)
            self.setPixelMap()

        if self.mainWindow.radioButtonNoPlot.isChecked():
            self.mainWindow.labelPlotOptions.hide()
            self.mainWindow.stackedWidgetPlotType.hide()
        # enable plotting
        else:
            self.mainWindow.labelPlotOptions.show()
            self.mainWindow.stackedWidgetPlotType.show()
            self.mainWindow.read_timer.start(Defines.Time_Plot_Refresh_ms)

    def setPixelMap(self):
        if self.mainWindow.comboBoxPlot.currentText() == "Matterhorn":
            self.mainWindow.nTransceiverRows = Defines.Matterhorn.nRows
            self.mainWindow.nTransceiverCols = Defines.Matterhorn.nCols
            self.mainWindow.pixelMapTransceiver = matterhorn_transceiver()
        elif self.mainWindow.comboBoxPlot.currentText() == "Moench04":
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

    def plotReferesh(self):
        self.mainWindow.acquisitionTab.read_zmq()

    def getRandomColor(self):
        '''
        Returns a random color range (except white) in format string eg. "#aabbcc"
        '''
        randomColor = random.randrange(0, 0xffffaa, 0xaa)
        return "#{:06x}".format(randomColor)

    def getActiveColor(self, button):
        return button.palette().color(QtGui.QPalette.Window)

    def setActiveColor(self, button, str_color):
        button.setStyleSheet(":enabled {background-color: %s" % str_color
                             + "} :disabled {background-color: grey}")

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
        if x >= 0 and x < nMaxX and y >= 0 and y < nMaxY:
            val = frame[int(x), int(y)]
            message = f'[{x:.2f}, {y:.2f}] = {val:.2f}'
            sender.setToolTip(message)
            #print(message)
        else:
            sender.setToolTip('')