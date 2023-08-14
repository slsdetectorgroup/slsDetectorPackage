from functools import partial
from pathlib import Path

from PyQt5 import QtWidgets, uic

from slsdet import Detector, dacIndex, readoutMode, runStatus

from pyctbgui import utils


class PowerSuppliesTab(QtWidgets.QWidget):
    def __init__(self,parent):
        super(PowerSuppliesTab, self).__init__()
        uic.loadUi(Path(__file__).parent.parent / 'ui' / "power_supplies.ui", self)



    def refresh(self):
        self.updateVoltageNames()
        for i in ('A', 'B', 'C', 'D', 'IO'):
            self.getVoltage(i)
            self.getCurrent(i)

    def connect_ui(self):
        for i in ('A', 'B', 'C', 'D', 'IO'):
            spinBox = getattr(self, f"spinBoxV{i}")
            checkBox = getattr(self, f"checkBoxV{i}")
            spinBox.editingFinished.connect(partial(self.setVoltage, i))
            checkBox.stateChanged.connect(partial(self.setVoltage, i))
        self.pushButtonPowerOff.clicked.connect(self.powerOff)

    def setup_ui(self):
        self.mainWindow = utils.mainWindow
        self.det = self.mainWindow.det

        for i in ('A', 'B', 'C', 'D', 'IO'):
            dac = getattr(dacIndex, f"V_POWER_{i}")
            spinBox = getattr(self, f"spinBoxV{i}")
            checkBox = getattr(self, f"checkBoxV{i}")
            retval = self.det.getVoltage(dac)[0]
            spinBox.setValue(retval)
            if retval == 0:
                checkBox.setChecked(False)
                spinBox.setDisabled(True)

    def updateVoltageNames(self):
        retval = self.det.getVoltageNames()
        getattr(self, f"checkBoxVA").setText(retval[0])
        getattr(self, f"checkBoxVB").setText(retval[1])
        getattr(self, f"checkBoxVC").setText(retval[2])
        getattr(self, f"checkBoxVD").setText(retval[3])
        getattr(self, f"checkBoxVIO").setText(retval[4])

    def getVoltage(self, i):
        spinBox = getattr(self, f"spinBoxV{i}")
        checkBox = getattr(self, f"checkBoxV{i}")
        voltageIndex = getattr(dacIndex, f"V_POWER_{i}")
        label = getattr(self, f"labelV{i}")

        spinBox.editingFinished.disconnect()
        checkBox.stateChanged.disconnect()

        retval = self.det.getMeasuredVoltage(voltageIndex)[0]
        # spinBox.setValue(retval)
        if retval > 1:
            checkBox.setChecked(True)
        if checkBox.isChecked():
            spinBox.setEnabled(True)
        else:
            spinBox.setDisabled(True)
        label.setText(f'{str(retval)} mV')

        spinBox.editingFinished.connect(partial(self.setVoltage, i))
        checkBox.stateChanged.connect(partial(self.setVoltage, i))

        self.getVChip()

        # TODO: handle multiple events when pressing enter (twice)

    def setVoltage(self, i):
        checkBox = getattr(self, f"checkBoxV{i}")
        spinBox = getattr(self, f"spinBoxV{i}")
        voltageIndex = getattr(dacIndex, f"V_POWER_{i}")
        spinBox.editingFinished.disconnect()

        value = 0
        if checkBox.isChecked():
            value = spinBox.value()
        try:
            self.det.setVoltage(voltageIndex, value)
        except Exception as e:
            QtWidgets.QMessageBox.warning(self.mainWindow, "Voltage Fail", str(e), QtWidgets.QMessageBox.Ok)
            pass

        # TODO: (properly) disconnecting and connecting to handle multiple events (out of focus and pressing enter).
        spinBox.editingFinished.connect(partial(self.setVoltage, i))
        self.getVoltage(i)
        self.getCurrent(i)

    def getCurrent(self, i):
        label = getattr(self, f"labelI{i}")
        currentIndex = getattr(dacIndex, f"I_POWER_{i}")
        retval = self.det.getMeasuredCurrent(currentIndex)[0]
        label.setText(f'{str(retval)} mA')

    def getVChip(self):
        self.spinBoxVChip.setValue(self.det.getVoltage(dacIndex.V_POWER_CHIP)[0])

    def powerOff(self):
        for i in ('A', 'B', 'C', 'D', 'IO'):
            # set all voltages to 0
            checkBox = getattr(self, f"checkBoxV{i}")
            checkBox.stateChanged.disconnect()
            checkBox.setChecked(False)
            checkBox.stateChanged.connect(partial(self.setVoltage, i))
            self.setVoltage(i)
