from functools import partial

import numpy as np
from slsdet import Detector, dacIndex, readoutMode, runStatus
from PyQt5 import QtWidgets
import pyqtgraph as pg

from .Plot import PlotService
from utils.bit_utils import bit_is_set, manipulate_bit


class TransceiverService():
    def __init__(self, mainWindow):
        self.mainWindow = mainWindow

        self.plotService = self.mainWindow.plotService

    def setup_ui(self):
        for i in range(4):
            self.setTransceiverButtonColor(i, self.plotService.getRandomColor())
        self.initializeAllTransceiverPlots()

    def connect_ui(self):
        for i in range(4):
            getattr(self.mainWindow, f"checkBoxTransceiver{i}").stateChanged.connect(
                partial(self.setTransceiverEnable, i))
            getattr(self.mainWindow, f"checkBoxTransceiver{i}Plot").stateChanged.connect(
                partial(self.setTransceiverEnablePlot, i))
            getattr(self.mainWindow, f"pushButtonTransceiver{i}").clicked.connect(
                partial(self.selectTransceiverColor, i))
        self.mainWindow.lineEditTransceiverMask.editingFinished.connect(self.setTransceiverEnableReg)

    def refresh(self):
        self.updateTransceiverEnable()

    def initializeAllTransceiverPlots(self):
        self.mainWindow.plotTransceiverWaveform = pg.plot()
        self.mainWindow.verticalLayoutPlot.addWidget(self.mainWindow.plotTransceiverWaveform, 5)
        self.mainWindow.transceiverPlots = {}
        waveform = np.zeros(1000)
        for i in range(4):
            pen = pg.mkPen(color=self.getTransceiverButtonColor(i), width=1)
            legendName = getattr(self.mainWindow, f"labelTransceiver{i}").text()
            self.mainWindow.transceiverPlots[i] = self.mainWindow.plotTransceiverWaveform.plot(waveform, pen=pen,
                                                                                               name=legendName)
            self.mainWindow.transceiverPlots[i].hide()

        self.mainWindow.plotTransceiverImage = pg.ImageView()
        self.mainWindow.nTransceiverRows = 0
        self.mainWindow.nTransceiverCols = 0
        self.mainWindow.transceiver_frame = np.zeros(
            (self.mainWindow.nTransceiverRows, self.mainWindow.nTransceiverCols))
        self.mainWindow.plotTransceiverImage.setImage(self.mainWindow.transceiver_frame)
        self.mainWindow.verticalLayoutPlot.addWidget(self.mainWindow.plotTransceiverImage, 6)

        cm = pg.colormap.get('CET-L9')  # prepare a linear color map
        self.mainWindow.plotTransceiverImage.setColorMap(cm)

    def getTransceiverEnableReg(self):
        retval = self.mainWindow.det.transceiverenable
        self.mainWindow.lineEditTransceiverMask.editingFinished.disconnect()
        self.mainWindow.lineEditTransceiverMask.setText("0x{:08x}".format(retval))
        self.mainWindow.lineEditTransceiverMask.editingFinished.connect(self.setTransceiverEnableReg)
        return retval

    def setTransceiverEnableReg(self):
        self.mainWindow.lineEditTransceiverMask.editingFinished.disconnect()
        try:
            mask = int(self.mainWindow.lineEditTransceiverMask.text(), 16)
            self.mainWindow.det.transceiverenable = mask
        except Exception as e:
            QtWidgets.QMessageBox.warning(self.mainWindow, "Transceiver Enable Fail", str(e), QtWidgets.QMessageBox.Ok)
            pass
        # TODO: handling double event exceptions
        self.mainWindow.lineEditTransceiverMask.editingFinished.connect(self.setTransceiverEnableReg)
        self.updateTransceiverEnable()

    def getTransceiverEnable(self, i, mask):
        checkBox = getattr(self.mainWindow, f"checkBoxTransceiver{i}")
        checkBox.stateChanged.disconnect()
        checkBox.setChecked(bit_is_set(mask, i))
        checkBox.stateChanged.connect(partial(self.setTransceiverEnable, i))

    def updateTransceiverEnable(self):
        retval = self.getTransceiverEnableReg()
        self.mainWindow.nTransceiverEnabled = bin(retval).count('1')
        for i in range(4):
            self.getTransceiverEnable(i, retval)
            self.getTransceiverEnablePlot(i)
            self.getTransceiverEnableColor(i)
            self.plotService.addSelectedTransceiverPlots(i)

    def setTransceiverEnable(self, i):
        checkBox = getattr(self.mainWindow, f"checkBoxTransceiver{i}")
        try:
            enableMask = manipulate_bit(checkBox.isChecked(), self.mainWindow.det.transceiverenable, i)
            self.mainWindow.det.transceiverenable = enableMask
        except Exception as e:
            QtWidgets.QMessageBox.warning(self.mainWindow, "Transceiver Enable Fail", str(e), QtWidgets.QMessageBox.Ok)
            pass

        self.updateTransceiverEnable()

    def getTransceiverEnablePlot(self, i):
        checkBox = getattr(self.mainWindow, f"checkBoxTransceiver{i}")
        checkBoxPlot = getattr(self.mainWindow, f"checkBoxTransceiver{i}Plot")
        checkBoxPlot.setEnabled(checkBox.isChecked())

    def setTransceiverEnablePlot(self, i):
        pushButton = getattr(self.mainWindow, f"pushButtonTransceiver{i}")
        checkBox = getattr(self.mainWindow, f"checkBoxTransceiver{i}Plot")
        pushButton.setEnabled(checkBox.isChecked())
        self.plotService.addSelectedTransceiverPlots(i)

    def getTransceiverEnableColor(self, i):
        checkBox = getattr(self.mainWindow, f"checkBoxTransceiver{i}Plot")
        pushButton = getattr(self.mainWindow, f"pushButtonTransceiver{i}")
        pushButton.setEnabled(checkBox.isEnabled() and checkBox.isChecked())

    def selectTransceiverColor(self, i):
        pushButton = getattr(self.mainWindow, f"pushButtonTransceiver{i}")
        self.plotService.showPalette(pushButton)
        pen = pg.mkPen(color=self.getTransceiverButtonColor(i), width=1)
        self.mainWindow.transceiverPlots[i].setPen(pen)

    def getTransceiverButtonColor(self, i):
        pushButton = getattr(self.mainWindow, f"pushButtonTransceiver{i}")
        return self.plotService.getActiveColor(pushButton)

    def setTransceiverButtonColor(self, i, color):
        pushButton = getattr(self.mainWindow, f"pushButtonTransceiver{i}")
        return self.plotService.setActiveColor(pushButton, color)
