from functools import partial
from slsdet import Detector, dacIndex, readoutMode, runStatus
from PyQt5 import QtWidgets
import pyqtgraph as pg

from bit_utils import bit_is_set, manipulate_bit


class Transceiver:
    def __init__(self, mainWindow):
        self.mainWindow = mainWindow

    def setup_ui(self):
        for i in range(4):
            self.setTransceiverButtonColor(i, self.mainWindow.getRandomColor())

    def connect_ui(self):
        for i in range(4):
            getattr(self.mainWindow, f"checkBoxTransceiver{i}").stateChanged.connect(partial(self.setTransceiverEnable, i))
            getattr(self.mainWindow, f"checkBoxTransceiver{i}Plot").stateChanged.connect(partial(self.setTransceiverEnablePlot, i))
            getattr(self.mainWindow, f"pushButtonTransceiver{i}").clicked.connect(partial(self.selectTransceiverColor, i))
        self.mainWindow.lineEditTransceiverMask.editingFinished.connect(self.setTransceiverEnableReg)

    def refresh(self):
        self.updateTransceiverEnable()

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
            self.mainWindow.addSelectedTransceiverPlots(i)

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
        self.mainWindow.addSelectedTransceiverPlots(i)

    def getTransceiverEnableColor(self, i):
        checkBox = getattr(self.mainWindow, f"checkBoxTransceiver{i}Plot")
        pushButton = getattr(self.mainWindow, f"pushButtonTransceiver{i}")
        pushButton.setEnabled(checkBox.isEnabled() and checkBox.isChecked())

    def selectTransceiverColor(self, i):
        pushButton = getattr(self.mainWindow, f"pushButtonTransceiver{i}")
        self.mainWindow.showPalette(pushButton)
        pen = pg.mkPen(color=self.getTransceiverButtonColor(i), width=1)
        self.mainWindow.transceiverPlots[i].setPen(pen)

    def getTransceiverButtonColor(self, i):
        pushButton = getattr(self.mainWindow, f"pushButtonTransceiver{i}")
        return self.mainWindow.getActiveColor(pushButton)

    def setTransceiverButtonColor(self, i, color):
        pushButton = getattr(self.mainWindow, f"pushButtonTransceiver{i}")
        return self.mainWindow.setActiveColor(pushButton, color)
