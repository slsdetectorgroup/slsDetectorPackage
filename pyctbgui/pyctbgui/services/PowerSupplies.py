from functools import partial
from pathlib import Path

from PyQt5 import QtWidgets, uic
from pyctbgui.utils.defines import Defines

from slsdet import powerIndex, detectorType


class PowerSuppliesTab(QtWidgets.QWidget):

    def __init__(self, parent):
        super().__init__(parent)
        uic.loadUi(Path(__file__).parent.parent / 'ui' / "powerSupplies.ui", parent)
        self.view = parent

    def refresh(self):
        self.updateVoltageNames()
        if self.det.type == detectorType.CHIPTESTBOARD:
            self.getVChip()
        for i in Defines.powerSupplies:
            self.update(i)

    def update(self, i):
        self.getPowerEnable(i)
        self.getVoltage(i)
        if self.det.type == detectorType.CHIPTESTBOARD:
            self.getMeasuredVoltage(i)
            self.getMeasuredCurrent(i)

    def connect_ui(self):
        for i in Defines.powerSupplies:
            spinBox = getattr(self.view, f"spinBoxV{i}")
            checkBox = getattr(self.view, f"checkBoxV{i}")
            spinBox.editingFinished.connect(partial(self.setVoltage, i))
            checkBox.stateChanged.connect(partial(self.setPowerEnable, i))
        self.view.pushButtonPowerOff.clicked.connect(self.powerOff)

    def setup_ui(self):
        if self.det.type == detectorType.XILINX_CHIPTESTBOARD:
            self.view.labelVChip.setDisabled(True)
            for i in Defines.powerSupplies:
                labelV = getattr(self.view, f"labelV{i}")
                labelV.setDisabled(True)
                labelI = getattr(self.view, f"labelI{i}")
                labelI.setDisabled(True)

    def updateVoltageNames(self):
        for i in Defines.powerSupplies:
            checkBox = getattr(self.view, f"checkBoxV{i}")
            dac = getattr(powerIndex, f"V_POWER_{i}")
            retval = self.det.getPowerName(dac)
            checkBox.setText(retval)

    def getMeasuredVoltage(self, i):
        label = getattr(self.view, f"labelV{i}")
        voltageIndex = getattr(powerIndex, f"V_POWER_{i}")
        retval = self.det.getMeasuredPower(voltageIndex)
        label.setText(f'{str(retval)} mV')
      
    def getMeasuredCurrent(self, i):
        label = getattr(self.view, f"labelI{i}")
        currentIndex = getattr(powerIndex, f"I_POWER_{i}")
        retval = self.det.getMeasuredCurrent(currentIndex)
        label.setText(f'{str(retval)} mA')

    def getVChip(self):
        self.view.labelVChip.setText(f"{str(self.det.getPowerDAC(powerIndex.V_POWER_CHIP))} mV")

    def getVoltage(self, i):
        spinBox = getattr(self.view, f"spinBoxV{i}")
        spinBox.editingFinished.disconnect()
        voltageIndex = getattr(powerIndex, f"V_POWER_{i}")
        spinBox.setValue(self.det.getPowerDAC(voltageIndex))
        spinBox.editingFinished.connect(partial(self.setVoltage, i))

    def setVoltage(self, i):
        spinBox = getattr(self.view, f"spinBoxV{i}")
        spinBox.editingFinished.disconnect()
        voltageIndex = getattr(powerIndex, f"V_POWER_{i}")
        try:
            self.det.setPowerDAC(voltageIndex, spinBox.value())
        except Exception as e:
            QtWidgets.QMessageBox.warning(self.mainWindow, "Voltage Fail", str(e), QtWidgets.QMessageBox.Ok)
            pass
        spinBox.editingFinished.connect(partial(self.setVoltage, i))
        self.update(i)
        if self.det.type == detectorType.CHIPTESTBOARD:
            self.getVChip()

    def getPowerEnable(self, i):
        checkBox = getattr(self.view, f"checkBoxV{i}")
        checkBox.stateChanged.disconnect()
        voltageIndex = getattr(powerIndex, f"V_POWER_{i}")
        retval = self.det.isPowerEnabled(voltageIndex)
        checkBox.setChecked(retval)
        checkBox.stateChanged.connect(partial(self.setPowerEnable, i))

    def setPowerEnable(self, i):
        checkBox = getattr(self.view, f"checkBoxV{i}")
        checkBox.stateChanged.disconnect()
        voltageIndex = getattr(powerIndex, f"V_POWER_{i}")
        try:
            self.det.setPowerEnabled([voltageIndex], checkBox.isChecked())
        except Exception as e:
            QtWidgets.QMessageBox.warning(self.mainWindow, "Voltage Fail", str(e), QtWidgets.QMessageBox.Ok)
            pass
        checkBox.stateChanged.connect(partial(self.setPowerEnable, i))
        self.update(i)
        if self.det.type == detectorType.CHIPTESTBOARD:
            self.getVChip()


    def powerOff(self):
        voltageIndices = [getattr(powerIndex, f"V_POWER_{i}") for i in Defines.powerSupplies]
        try:
            self.det.setPowerEnabled(voltageIndices, False)
        except Exception as e:
            QtWidgets.QMessageBox.warning(self.mainWindow, "Power Off Fail", str(e), QtWidgets.QMessageBox.Ok)
            pass
        finally:
            self.refresh()

    def saveParameters(self) -> list:
        commands = []
        for i in Defines.powerSupplies:
            enabled = getattr(self.view, f"checkBoxV{i}").isChecked()
            commands.append(f"power v_{i.lower()} {enabled}")
            value = getattr(self.view, f"spinBoxV{i}").value()
            commands.append(f"powerdac v_{i.lower()} {value}")
        return commands
