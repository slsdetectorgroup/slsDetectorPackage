from functools import partial
from pathlib import Path

import numpy as np
from PyQt5 import QtWidgets, uic
import pyqtgraph as pg
from pyctbgui.utils.defines import Defines

from ..utils.bit_utils import bit_is_set, manipulate_bit


class TransceiverTab(QtWidgets.QWidget):
    def __init__(self, parent):
        super(TransceiverTab, self).__init__(parent)
        uic.loadUi(Path(__file__).parent.parent / 'ui' / "transceiver.ui", parent)
        self.view = parent
        self.mainWindow = None
        self.det = None
        self.plotTab = None

    def setup_ui(self):
        self.plotTab = self.mainWindow.plotTab

        for i in range(Defines.transceiver.count):
            self.setTransceiverButtonColor(i, self.plotTab.getRandomColor())
        self.initializeAllTransceiverPlots()

    def connect_ui(self):
        for i in range(Defines.transceiver.count):
            getattr(self.view, f"checkBoxTransceiver{i}").stateChanged.connect(
                partial(self.setTransceiverEnable, i))
            getattr(self.view, f"checkBoxTransceiver{i}Plot").stateChanged.connect(
                partial(self.setTransceiverEnablePlot, i))
            getattr(self.view, f"pushButtonTransceiver{i}").clicked.connect(
                partial(self.selectTransceiverColor, i))
        self.view.lineEditTransceiverMask.editingFinished.connect(self.setTransceiverEnableReg)

    def refresh(self):
        self.updateTransceiverEnable()

    def initializeAllTransceiverPlots(self):
        self.mainWindow.plotTransceiverWaveform = pg.plot()
        self.mainWindow.plotTransceiverWaveform.addLegend(colCount=Defines.colCount)
        self.mainWindow.verticalLayoutPlot.addWidget(self.mainWindow.plotTransceiverWaveform, 5)
        self.mainWindow.transceiverPlots = {}
        waveform = np.zeros(1000)
        for i in range(Defines.transceiver.count):
            pen = pg.mkPen(color=self.getTransceiverButtonColor(i), width=1)
            legendName = getattr(self.view, f"labelTransceiver{i}").text()
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
        retval = self.det.transceiverenable
        self.view.lineEditTransceiverMask.editingFinished.disconnect()
        self.view.lineEditTransceiverMask.setText("0x{:08x}".format(retval))
        self.view.lineEditTransceiverMask.editingFinished.connect(self.setTransceiverEnableReg)
        return retval

    def setTransceiverEnableReg(self):
        self.view.lineEditTransceiverMask.editingFinished.disconnect()
        try:
            mask = int(self.view.lineEditTransceiverMask.text(), 16)
            self.det.transceiverenable = mask
        except Exception as e:
            QtWidgets.QMessageBox.warning(self.mainWindow, "Transceiver Enable Fail", str(e), QtWidgets.QMessageBox.Ok)
            pass
        # TODO: handling double event exceptions
        self.view.lineEditTransceiverMask.editingFinished.connect(self.setTransceiverEnableReg)
        self.updateTransceiverEnable()

    def getTransceiverEnable(self, i, mask):
        checkBox = getattr(self.view, f"checkBoxTransceiver{i}")
        checkBox.stateChanged.disconnect()
        checkBox.setChecked(bit_is_set(mask, i))
        checkBox.stateChanged.connect(partial(self.setTransceiverEnable, i))

    def updateTransceiverEnable(self):
        retval = self.getTransceiverEnableReg()
        self.nTransceiverEnabled = bin(retval).count('1')
        for i in range(4):
            self.getTransceiverEnable(i, retval)
            self.getTransceiverEnablePlot(i)
            self.getTransceiverEnableColor(i)
            self.plotTab.addSelectedTransceiverPlots(i)

    def setTransceiverEnable(self, i):
        checkBox = getattr(self.view, f"checkBoxTransceiver{i}")
        try:
            enableMask = manipulate_bit(checkBox.isChecked(), self.det.transceiverenable, i)
            self.det.transceiverenable = enableMask
        except Exception as e:
            QtWidgets.QMessageBox.warning(self.mainWindow, "Transceiver Enable Fail", str(e), QtWidgets.QMessageBox.Ok)
            pass

        self.updateTransceiverEnable()

    def getTransceiverEnablePlot(self, i):
        checkBox = getattr(self.view, f"checkBoxTransceiver{i}")
        checkBoxPlot = getattr(self.view, f"checkBoxTransceiver{i}Plot")
        checkBoxPlot.setEnabled(checkBox.isChecked())

    def setTransceiverEnablePlot(self, i):
        pushButton = getattr(self.view, f"pushButtonTransceiver{i}")
        checkBox = getattr(self.view, f"checkBoxTransceiver{i}Plot")
        pushButton.setEnabled(checkBox.isChecked())
        self.plotTab.addSelectedTransceiverPlots(i)

    def getTransceiverEnableColor(self, i):
        checkBox = getattr(self.view, f"checkBoxTransceiver{i}Plot")
        pushButton = getattr(self.view, f"pushButtonTransceiver{i}")
        pushButton.setEnabled(checkBox.isEnabled() and checkBox.isChecked())

    def selectTransceiverColor(self, i):
        pushButton = getattr(self.view, f"pushButtonTransceiver{i}")
        self.plotTab.showPalette(pushButton)
        pen = pg.mkPen(color=self.getTransceiverButtonColor(i), width=1)
        self.mainWindow.transceiverPlots[i].setPen(pen)

    def getTransceiverButtonColor(self, i):
        pushButton = getattr(self.view, f"pushButtonTransceiver{i}")
        return self.plotTab.getActiveColor(pushButton)

    def setTransceiverButtonColor(self, i, color):
        pushButton = getattr(self.view, f"pushButtonTransceiver{i}")
        return self.plotTab.setActiveColor(pushButton, color)
