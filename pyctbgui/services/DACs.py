from functools import partial
from PyQt5 import QtWidgets
from pyctbgui.utils.defines import Defines

from slsdet import Detector, dacIndex, readoutMode, runStatus


class DacTab:
    def __init__(self, mainWindow):
        self.mainWindow = mainWindow

    def setup_ui(self):
        for i in range(Defines.dac.count):
            dac = getattr(dacIndex, f"DAC_{i}")
            getattr(self.mainWindow, f"spinBoxDAC{i}").setValue(self.mainWindow.det.getDAC(dac)[0])

        if self.mainWindow.det.highvoltage == 0:
            self.mainWindow.spinBoxHighVoltage.setDisabled(True)
            self.mainWindow.checkBoxHighVoltage.setChecked(False)

    def connect_ui(self):
        n_dacs = len(self.mainWindow.det.daclist)
        for i in range(n_dacs):
            getattr(self.mainWindow, f"spinBoxDAC{i}").editingFinished.connect(partial(self.setDAC, i))
            getattr(self.mainWindow, f"checkBoxDAC{i}").stateChanged.connect(partial(self.setDACTristate, i))
            getattr(self.mainWindow, f"checkBoxDAC{i}mV").stateChanged.connect(partial(self.getDAC, i))

        self.mainWindow.comboBoxADCVpp.currentIndexChanged.connect(self.setADCVpp)
        self.mainWindow.spinBoxHighVoltage.editingFinished.connect(self.setHighVoltage)
        self.mainWindow.checkBoxHighVoltage.stateChanged.connect(self.setHighVoltage)

    def refresh(self):
        self.updateDACNames()
        for i in range(Defines.dac.count):
            self.getDACTristate(i)
            self.getDAC(i)

        self.getADCVpp()
        self.getHighVoltage()

    def updateDACNames(self):
        for i, name in enumerate(self.mainWindow.det.getDacNames()):
            getattr(self.mainWindow, f"checkBoxDAC{i}").setText(name)

    def getDACTristate(self, i):
        checkBox = getattr(self.mainWindow, f"checkBoxDAC{i}")
        dac = getattr(dacIndex, f"DAC_{i}")
        checkBox.stateChanged.disconnect()
        if (self.mainWindow.det.getDAC(dac)[0]) == -100:
            checkBox.setChecked(False)
        else:
            checkBox.setChecked(True)
        checkBox.stateChanged.connect(partial(self.setDACTristate, i))

    def setDACTristate(self, i):
        checkBox = getattr(self.mainWindow, f"checkBoxDAC{i}")
        if not checkBox.isChecked():
            self.setDAC(i)
        self.getDAC(i)

    def getDAC(self, i):
        checkBox = getattr(self.mainWindow, f"checkBoxDAC{i}")
        checkBoxmV = getattr(self.mainWindow, f"checkBoxDAC{i}mV")
        spinBox = getattr(self.mainWindow, f"spinBoxDAC{i}")
        label = getattr(self.mainWindow, f"labelDAC{i}")
        dac = getattr(dacIndex, f"DAC_{i}")

        checkBox.stateChanged.disconnect()
        checkBoxmV.stateChanged.disconnect()
        spinBox.editingFinished.disconnect()

        # do not uncheck automatically
        if (self.mainWindow.det.getDAC(dac)[0]) != -100:
            checkBox.setChecked(True)

        if checkBox.isChecked():
            spinBox.setEnabled(True)
            checkBoxmV.setEnabled(True)
        else:
            spinBox.setDisabled(True)
            checkBoxmV.setDisabled(True)

        if checkBoxmV.isChecked():
            label.setText(str(self.mainWindow.det.getDAC(dac, True)[0]))
        else:
            label.setText(str(self.mainWindow.det.getDAC(dac)[0]))

        checkBox.stateChanged.connect(partial(self.setDACTristate, i))
        checkBoxmV.stateChanged.connect(partial(self.getDAC, i))
        spinBox.editingFinished.connect(partial(self.setDAC, i))

    def setDAC(self, i):
        checkBoxDac = getattr(self.mainWindow, f"checkBoxDAC{i}")
        checkBoxmV = getattr(self.mainWindow, f"checkBoxDAC{i}mV")
        spinBox = getattr(self.mainWindow, f"spinBoxDAC{i}")
        dac = getattr(dacIndex, f"DAC_{i}")

        value = -100
        if checkBoxDac.isChecked():
            value = spinBox.value()

        self.mainWindow.det.setDAC(dac, value, checkBoxmV.isChecked())
        self.getDAC(i)

    def getADCVpp(self):
        retval = self.mainWindow.det.adcvpp
        self.mainWindow.labelADCVpp.setText(f'Mode: {str(retval)}')

        self.mainWindow.comboBoxADCVpp.currentIndexChanged.disconnect()
        self.mainWindow.comboBoxADCVpp.setCurrentIndex(retval)
        self.mainWindow.comboBoxADCVpp.currentIndexChanged.connect(self.setADCVpp)

    def setADCVpp(self):
        self.mainWindow.det.adcvpp = self.mainWindow.comboBoxADCVpp.currentIndex()
        self.getADCVpp()

    def getHighVoltage(self):
        retval = self.mainWindow.det.highvoltage
        self.mainWindow.labelHighVoltage.setText(str(retval))

        self.mainWindow.spinBoxHighVoltage.editingFinished.disconnect()
        self.mainWindow.checkBoxHighVoltage.stateChanged.disconnect()

        self.mainWindow.spinBoxHighVoltage.setValue(retval)
        if retval:
            self.mainWindow.checkBoxHighVoltage.setChecked(True)
        if self.mainWindow.checkBoxHighVoltage.isChecked():
            self.mainWindow.spinBoxHighVoltage.setEnabled(True)

        self.mainWindow.spinBoxHighVoltage.editingFinished.connect(self.setHighVoltage)
        self.mainWindow.checkBoxHighVoltage.stateChanged.connect(self.setHighVoltage)

    def setHighVoltage(self):
        value = 0
        if self.mainWindow.checkBoxHighVoltage.isChecked():
            value = self.mainWindow.spinBoxHighVoltage.value()
        try:
            self.mainWindow.det.highvoltage = value
        except Exception as e:
            QtWidgets.QMessageBox.warning(self.mainWindow, "High Voltage Fail", str(e), QtWidgets.QMessageBox.Ok)
            pass
        self.getHighVoltage()