from functools import partial
from pathlib import Path

from PyQt5 import QtWidgets, uic
from pyctbgui.utils.defines import Defines

from slsdet import dacIndex, detectorType


class PowerSuppliesTab(QtWidgets.QWidget):

    def __init__(self, parent):
        super().__init__(parent)
        uic.loadUi(Path(__file__).parent.parent / 'ui' / "powerSupplies.ui", parent)
        self.view = parent

    def refresh(self):
        self.updateVoltageNames()
        for i in Defines.powerSupplies:
            self.getVoltage(i)
            if self.det.type == detectorType.CHIPTESTBOARD:
                self.getCurrent(i)

    def connect_ui(self):
        for i in Defines.powerSupplies:
            spinBox = getattr(self.view, f"spinBoxV{i}")
            checkBox = getattr(self.view, f"checkBoxV{i}")
            spinBox.editingFinished.connect(partial(self.setVoltage, i))
            checkBox.stateChanged.connect(partial(self.setVoltage, i))
        self.view.pushButtonPowerOff.clicked.connect(self.powerOff)

    def setup_ui(self):
        for i in Defines.powerSupplies:
            dac = getattr(dacIndex, f"V_POWER_{i}")
            spinBox = getattr(self.view, f"spinBoxV{i}")
            checkBox = getattr(self.view, f"checkBoxV{i}")
            retval = self.det.getPower(dac)[0]
            spinBox.setValue(retval)
            if retval == 0:
                checkBox.setChecked(False)
                spinBox.setDisabled(True)
            if self.det.type == detectorType.XILINX_CHIPTESTBOARD:
                label = getattr(self.view, f"labelI{i}")
                label.setDisabled(True)
        if self.det.type == detectorType.XILINX_CHIPTESTBOARD:
            self.view.spinBoxVChip.setDisabled(True)


    def updateVoltageNames(self):
        retval = self.det.getPowerNames()
        getattr(self.view, "checkBoxVA").setText(retval[0])
        getattr(self.view, "checkBoxVB").setText(retval[1])
        getattr(self.view, "checkBoxVC").setText(retval[2])
        getattr(self.view, "checkBoxVD").setText(retval[3])
        getattr(self.view, "checkBoxVIO").setText(retval[4])

    def getVoltage(self, i):
        spinBox = getattr(self.view, f"spinBoxV{i}")
        checkBox = getattr(self.view, f"checkBoxV{i}")
        voltageIndex = getattr(dacIndex, f"V_POWER_{i}")
        label = getattr(self.view, f"labelV{i}")

        spinBox.editingFinished.disconnect()
        checkBox.stateChanged.disconnect()

        if self.det.type == detectorType.XILINX_CHIPTESTBOARD:
            retval = self.det.getPower(voltageIndex)[0]
        else:
            retval = self.det.getMeasuredPower(voltageIndex)[0]
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
        
        if self.det.type == detectorType.CHIPTESTBOARD:
            self.getVChip()

        # TODO: handle multiple events when pressing enter (twice)

    def setVoltage(self, i):
        checkBox = getattr(self.view, f"checkBoxV{i}")
        spinBox = getattr(self.view, f"spinBoxV{i}")
        voltageIndex = getattr(dacIndex, f"V_POWER_{i}")
        spinBox.editingFinished.disconnect()

        value = 0
        if checkBox.isChecked():
            value = spinBox.value()
        try:
            self.det.setPower(voltageIndex, value)
        except Exception as e:
            QtWidgets.QMessageBox.warning(self.mainWindow, "Voltage Fail", str(e), QtWidgets.QMessageBox.Ok)
            pass

        # TODO: (properly) disconnecting and connecting to handle multiple events (out of focus and pressing enter).
        spinBox.editingFinished.connect(partial(self.setVoltage, i))
        self.getVoltage(i)
        if self.det.type == detectorType.CHIPTESTBOARD:
            self.getCurrent(i)

    def getCurrent(self, i):
        label = getattr(self.view, f"labelI{i}")
        currentIndex = getattr(dacIndex, f"I_POWER_{i}")
        retval = self.det.getMeasuredCurrent(currentIndex)[0]
        label.setText(f'{str(retval)} mA')

    def getVChip(self):
        self.view.spinBoxVChip.setValue(self.det.getPower(dacIndex.V_POWER_CHIP)[0])

    def powerOff(self):
        for i in Defines.powerSupplies:
            # set all voltages to 0
            checkBox = getattr(self.view, f"checkBoxV{i}")
            checkBox.stateChanged.disconnect()
            checkBox.setChecked(False)
            checkBox.stateChanged.connect(partial(self.setVoltage, i))
            self.setVoltage(i)

    def saveParameters(self) -> list:
        commands = []
        for i in Defines.powerSupplies:
            enabled = getattr(self.view, f"checkBoxV{i}").isChecked()
            if enabled:
                value = getattr(self.view, f"spinBoxV{i}").value()
                commands.append(f"v_{i.lower()} {value}")
            else:
                commands.append(f"v_{i.lower()} 0")
        return commands
