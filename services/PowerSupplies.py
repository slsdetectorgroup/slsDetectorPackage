from functools import partial
from slsdet import Detector, dacIndex, readoutMode, runStatus
from PyQt5 import QtWidgets


class PowerSupplies:
    def __init__(self, mainWindow):
        self.mainWindow = mainWindow

    def refresh(self):
        self.updateVoltageNames()
        for i in ('A', 'B', 'C', 'D', 'IO'):
            self.getVoltage(i)
            self.getCurrent(i)

    def connect_ui(self):
        for i in ('A', 'B', 'C', 'D', 'IO'):
            spinBox = getattr(self.mainWindow, f"spinBoxV{i}")
            checkBox = getattr(self.mainWindow, f"checkBoxV{i}")
            spinBox.editingFinished.connect(partial(self.setVoltage, i))
            checkBox.stateChanged.connect(partial(self.setVoltage, i))
        self.mainWindow.pushButtonPowerOff.clicked.connect(self.powerOff)

    def setup_ui(self):
        for i in ('A', 'B', 'C', 'D', 'IO'):
            dac = getattr(dacIndex, f"V_POWER_{i}")
            spinBox = getattr(self.mainWindow, f"spinBoxV{i}")
            checkBox = getattr(self.mainWindow, f"checkBoxV{i}")
            retval = self.mainWindow.det.getVoltage(dac)[0]
            spinBox.setValue(retval)
            if retval == 0:
                checkBox.setChecked(False)
                spinBox.setDisabled(True)

    def updateVoltageNames(self):
        retval = self.mainWindow.det.getVoltageNames()
        getattr(self.mainWindow, f"checkBoxVA").setText(retval[0])
        getattr(self.mainWindow, f"checkBoxVB").setText(retval[1])
        getattr(self.mainWindow, f"checkBoxVC").setText(retval[2])
        getattr(self.mainWindow, f"checkBoxVD").setText(retval[3])
        getattr(self.mainWindow, f"checkBoxVIO").setText(retval[4])

    def getVoltage(self, i):
        spinBox = getattr(self.mainWindow, f"spinBoxV{i}")
        checkBox = getattr(self.mainWindow, f"checkBoxV{i}")
        voltageIndex = getattr(dacIndex, f"V_POWER_{i}")
        label = getattr(self.mainWindow, f"labelV{i}")

        spinBox.editingFinished.disconnect()
        checkBox.stateChanged.disconnect()

        retval = self.mainWindow.det.getMeasuredVoltage(voltageIndex)[0]
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
        checkBox = getattr(self.mainWindow, f"checkBoxV{i}")
        spinBox = getattr(self.mainWindow, f"spinBoxV{i}")
        voltageIndex = getattr(dacIndex, f"V_POWER_{i}")
        spinBox.editingFinished.disconnect()

        value = 0
        if checkBox.isChecked():
            value = spinBox.value()
        try:
            self.mainWindow.det.setVoltage(voltageIndex, value)
        except Exception as e:
            QtWidgets.QMessageBox.warning(self.mainWindow, "Voltage Fail", str(e), QtWidgets.QMessageBox.Ok)
            pass

        # TODO: (properly) disconnecting and connecting to handle multiple events (out of focus and pressing enter).
        spinBox.editingFinished.connect(partial(self.setVoltage, i))
        self.getVoltage(i)
        self.getCurrent(i)

    def getCurrent(self, i):
        label = getattr(self.mainWindow, f"labelI{i}")
        currentIndex = getattr(dacIndex, f"I_POWER_{i}")
        retval = self.mainWindow.det.getMeasuredCurrent(currentIndex)[0]
        label.setText(f'{str(retval)} mA')

    def getVChip(self):
        self.mainWindow.spinBoxVChip.setValue(self.mainWindow.det.getVoltage(dacIndex.V_POWER_CHIP)[0])

    def powerOff(self):
        for i in ('A', 'B', 'C', 'D', 'IO'):
            # set all voltages to 0
            checkBox = getattr(self.mainWindow, f"checkBoxV{i}")
            checkBox.stateChanged.disconnect()
            checkBox.setChecked(False)
            checkBox.stateChanged.connect(partial(self.setVoltage, i))
            self.setVoltage(i)
