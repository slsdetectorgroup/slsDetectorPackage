from functools import partial
from pathlib import Path

from PyQt5 import QtWidgets, uic
from pyctbgui.utils.defines import Defines

from slsdet import dacIndex, detectorType


class DacTab(QtWidgets.QWidget):

    def __init__(self, parent):
        super().__init__(parent)
        uic.loadUi(Path(__file__).parent.parent / 'ui' / "Dacs.ui", parent)
        self.view = parent

    def setup_ui(self):
        for i in range(Defines.dac.count):
            dac = getattr(dacIndex, f"DAC_{i}")
            getattr(self.view, f"spinBoxDAC{i}").setValue(self.det.getDAC(dac)[0])

        if self.det.type == detectorType.XILINX_CHIPTESTBOARD:
            self.view.checkBoxHighVoltage.setDisabled(True)
            self.view.spinBoxHighVoltage.setDisabled(True)
            self.view.labelHighVoltage.setDisabled(True)
            self.view.labelADCVppDacName.setDisabled(True)
            self.view.labelADCVpp.setDisabled(True)
            self.view.comboBoxADCVpp.setDisabled(True)
        elif self.det.highvoltage == 0:
            self.view.spinBoxHighVoltage.setDisabled(True)
            self.view.checkBoxHighVoltage.setChecked(False)

    def connect_ui(self):
        n_dacs = len(self.det.daclist)
        for i in range(n_dacs):
            getattr(self.view, f"spinBoxDAC{i}").editingFinished.connect(partial(self.setDAC, i))
            getattr(self.view, f"checkBoxDAC{i}").stateChanged.connect(partial(self.setDACTristate, i))
            getattr(self.view, f"checkBoxDAC{i}mV").stateChanged.connect(partial(self.getDAC, i))

        if self.view.comboBoxADCVpp.isEnabled():
            self.view.comboBoxADCVpp.currentIndexChanged.connect(self.setADCVpp)
        if self.view.checkBoxHighVoltage.isEnabled():
            self.view.spinBoxHighVoltage.editingFinished.connect(self.setHighVoltage)
            self.view.checkBoxHighVoltage.stateChanged.connect(self.setHighVoltage)

    def refresh(self):
        self.updateDACNames()
        for i in range(Defines.dac.count):
            self.getDACTristate(i)
            self.getDAC(i)

        if self.view.comboBoxADCVpp.isEnabled():
            self.getADCVpp()
        if self.view.checkBoxHighVoltage.isEnabled():
            self.getHighVoltage()

    def updateDACNames(self):
        for i, name in enumerate(self.det.getDacNames()):
            getattr(self.view, f"checkBoxDAC{i}").setText(name)

    def getDACTristate(self, i):
        checkBox = getattr(self.view, f"checkBoxDAC{i}")
        dac = getattr(dacIndex, f"DAC_{i}")
        checkBox.stateChanged.disconnect()
        if (self.det.getDAC(dac)[0]) == -100:
            checkBox.setChecked(False)
        else:
            checkBox.setChecked(True)
        checkBox.stateChanged.connect(partial(self.setDACTristate, i))

    def setDACTristate(self, i):
        checkBox = getattr(self.view, f"checkBoxDAC{i}")
        if not checkBox.isChecked():
            self.setDAC(i)
        self.getDAC(i)

    def getDAC(self, i):
        checkBox = getattr(self.view, f"checkBoxDAC{i}")
        checkBoxmV = getattr(self.view, f"checkBoxDAC{i}mV")
        spinBox = getattr(self.view, f"spinBoxDAC{i}")
        label = getattr(self.view, f"labelDAC{i}")
        dac = getattr(dacIndex, f"DAC_{i}")

        checkBox.stateChanged.disconnect()
        checkBoxmV.stateChanged.disconnect()
        spinBox.editingFinished.disconnect()

        # do not uncheck automatically
        if self.det.getDAC(dac)[0] != -100:
            checkBox.setChecked(True)

        if checkBox.isChecked():
            spinBox.setEnabled(True)
            checkBoxmV.setEnabled(True)
        else:
            spinBox.setDisabled(True)
            checkBoxmV.setDisabled(True)

        in_mv = checkBoxmV.isChecked() and checkBox.isChecked()
        dacValue = self.det.getDAC(dac, in_mv)[0]
        unit = "mV" if in_mv else ""
        label.setText(f"{dacValue} {unit}")
        spinBox.setValue(dacValue)

        checkBox.stateChanged.connect(partial(self.setDACTristate, i))
        checkBoxmV.stateChanged.connect(partial(self.getDAC, i))
        spinBox.editingFinished.connect(partial(self.setDAC, i))

    def setDAC(self, i):
        checkBoxDac = getattr(self.view, f"checkBoxDAC{i}")
        checkBoxmV = getattr(self.view, f"checkBoxDAC{i}mV")
        spinBox = getattr(self.view, f"spinBoxDAC{i}")
        dac = getattr(dacIndex, f"DAC_{i}")

        value = -100
        if checkBoxDac.isChecked():
            value = spinBox.value()
        in_mV = checkBoxDac.isChecked() and checkBoxmV.isChecked()
        self.det.setDAC(dac, value, in_mV)
        self.getDAC(i)

    def getADCVpp(self):
        retval = self.det.adcvpp
        self.view.labelADCVpp.setText(f'Mode: {str(retval)}')

        self.view.comboBoxADCVpp.currentIndexChanged.disconnect()
        self.view.comboBoxADCVpp.setCurrentIndex(retval)
        self.view.comboBoxADCVpp.currentIndexChanged.connect(self.setADCVpp)

    def setADCVpp(self):
        self.det.adcvpp = self.view.comboBoxADCVpp.currentIndex()
        self.getADCVpp()

    def getHighVoltage(self):
        retval = self.det.highvoltage
        self.view.labelHighVoltage.setText(str(retval))

        self.view.spinBoxHighVoltage.editingFinished.disconnect()
        self.view.checkBoxHighVoltage.stateChanged.disconnect()

        self.view.spinBoxHighVoltage.setValue(retval)
        if retval:
            self.view.checkBoxHighVoltage.setChecked(True)
        if self.view.checkBoxHighVoltage.isChecked():
            self.view.spinBoxHighVoltage.setEnabled(True)

        self.view.spinBoxHighVoltage.editingFinished.connect(self.setHighVoltage)
        self.view.checkBoxHighVoltage.stateChanged.connect(self.setHighVoltage)

    def setHighVoltage(self):
        value = 0
        if self.view.checkBoxHighVoltage.isChecked():
            value = self.view.spinBoxHighVoltage.value()
        try:
            self.det.highvoltage = value
        except Exception as e:
            QtWidgets.QMessageBox.warning(self.mainWindow, "High Voltage Fail", str(e), QtWidgets.QMessageBox.Ok)
            pass
        self.getHighVoltage()

    def saveParameters(self) -> list:
        """
        save parameters for the current tab
        @return: list of commands
        """
        commands = []
        for i in range(Defines.dac.count):
            # if checkbox disabled put -100 in dac units
            enabled = getattr(self.view, f"checkBoxDAC{i}").isChecked()
            if not enabled:
                commands.append(f"dac {i} -100")
            # else put the value in dac or mV units
            else:
                value = getattr(self.view, f"spinBoxDAC{i}").value()
                inMV = getattr(self.view, f"checkBoxDAC{i}mV").isChecked()
                unit = " mV" if inMV else ""
                commands.append(f"dac {i} {value}{unit}")

        if self.view.comboBoxADCVpp.isEnabled():
            commands.append(f"adcvpp {self.view.comboBoxADCVpp.currentText()} mV")
        if self.view.checkBoxHighVoltage.isEnabled():
            commands.append(f"highvoltage {self.view.spinBoxHighVoltage.value()}")
        return commands
