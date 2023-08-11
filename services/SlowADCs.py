from functools import partial
from slsdet import Detector, dacIndex, readoutMode, runStatus

from utils.SingletonMeta import SingletonMeta


class SlowAdcService(metaclass=SingletonMeta):
    def __init__(self, mainWindow):
        self.mainWindow = mainWindow

    def setup_ui(self):
        pass

    def connect_ui(self):
        for i in range(8):
            getattr(self.mainWindow, f"pushButtonSlowAdc{i}").clicked.connect(partial(self.updateSlowAdc, i))
        self.mainWindow.pushButtonTemp.clicked.connect(self.updateTemperature)

    def refresh(self):
        self.updateSlowAdcNames()
        for i in range(8):
            self.updateSlowAdc(i)
        self.updateTemperature()

    def updateSlowAdcNames(self):
        for i, name in enumerate(self.mainWindow.det.getSlowADCNames()):
            getattr(self.mainWindow, f"labelSlowAdc{i}").setText(name)

    def updateSlowAdc(self, i):
        slowADCIndex = getattr(dacIndex, f"SLOW_ADC{i}")
        label = getattr(self.mainWindow, f"labelSlowAdcValue{i}")
        slowadc = (self.mainWindow.det.getSlowADC(slowADCIndex))[0] / 1000
        label.setText(f'{slowadc:.2f} mV')

    def updateTemperature(self):
        slowadc = self.mainWindow.det.getTemperature(dacIndex.SLOW_ADC_TEMP)
        self.mainWindow.labelTempValue.setText(f'{str(slowadc[0])} °C')
