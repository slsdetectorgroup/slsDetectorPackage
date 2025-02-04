from functools import partial
from pathlib import Path

from PyQt5 import uic, QtWidgets

from pyctbgui.utils.defines import Defines
from slsdet import dacIndex, detectorType


class SlowAdcTab(QtWidgets.QWidget):

    def __init__(self, parent):
        super().__init__(parent)
        uic.loadUi(Path(__file__).parent.parent / 'ui' / "slowAdcs.ui", parent)
        self.view = parent
        self.mainWindow = None
        self.det = None

    def setup_ui(self):
        if self.det.type == detectorType.XILINX_CHIPTESTBOARD:
            self.view.pushButtonTemp.setDisabled(True)      

    def connect_ui(self):
        for i in range(Defines.slowAdc.count):
            getattr(self.view, f"pushButtonSlowAdc{i}").clicked.connect(partial(self.updateSlowAdc, i))
        self.view.pushButtonTemp.clicked.connect(self.updateTemperature)

    def refresh(self):
        self.updateSlowAdcNames()
        for i in range(Defines.slowAdc.count):
            self.updateSlowAdc(i)
        if self.det.type == detectorType.CHIPTESTBOARD:
            self.updateTemperature()

    def updateSlowAdcNames(self):
        for i, name in enumerate(self.mainWindow.det.getSlowADCNames()):
            getattr(self.view, f"labelSlowAdc{i}").setText(name)

    def updateSlowAdc(self, i):
        slowADCIndex = getattr(dacIndex, f"SLOW_ADC{i}")
        label = getattr(self.view, f"labelSlowAdcValue{i}")
        slowadc = (self.det.getSlowADC(slowADCIndex))[0] / 1000
        label.setText(f'{slowadc:.2f} mV')

    def updateTemperature(self):
        slowadc = self.det.getTemperature(dacIndex.SLOW_ADC_TEMP)
        self.view.labelTempValue.setText(f'{str(slowadc[0])} °C')
