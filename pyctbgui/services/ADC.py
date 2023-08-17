from functools import partial
from pathlib import Path

import numpy as np
from PyQt5 import QtWidgets, uic
import pyqtgraph as pg

from ..utils.bit_utils import bit_is_set, manipulate_bit
from ..utils.defines import Defines


class AdcTab(QtWidgets.QWidget):
    def __init__(self, parent):
        super(AdcTab, self).__init__(parent)
        uic.loadUi(Path(__file__).parent.parent / 'ui' / "adc.ui", parent)
        self.view = parent
        self.mainWindow = None
        self.det = None
        self.plotTab = None

    def setup_ui(self):
        self.plotTab = self.mainWindow.plotTab
        for i in range(Defines.adc.count):
            self.setADCButtonColor(i, self.plotTab.getRandomColor())
        self.initializeAllAnalogPlots()

    def initializeAllAnalogPlots(self):
        self.mainWindow.plotAnalogWaveform = pg.plot()
        self.mainWindow.plotAnalogWaveform.addLegend(colCount=4)
        self.mainWindow.verticalLayoutPlot.addWidget(self.mainWindow.plotAnalogWaveform, 1)
        self.mainWindow.analogPlots = {}
        waveform = np.zeros(1000)
        for i in range(Defines.adc.count):
            pen = pg.mkPen(color=self.getADCButtonColor(i), width=1)
            legendName = getattr(self.view, f"labelADC{i}").text()
            self.mainWindow.analogPlots[i] = self.mainWindow.plotAnalogWaveform.plot(waveform, pen=pen, name=legendName)
            self.mainWindow.analogPlots[i].hide()

        self.mainWindow.plotAnalogImage = pg.ImageView()
        self.mainWindow.nAnalogRows = 0
        self.mainWindow.nAnalogCols = 0
        self.mainWindow.analog_frame = np.zeros((self.mainWindow.nAnalogRows, self.mainWindow.nAnalogCols))
        self.mainWindow.plotAnalogImage.getView().invertY(False)
        self.mainWindow.plotAnalogImage.setImage(self.mainWindow.analog_frame)
        self.mainWindow.verticalLayoutPlot.addWidget(self.mainWindow.plotAnalogImage, 2)

    def connect_ui(self):
        for i in range(Defines.adc.count):
            getattr(self.view, f"checkBoxADC{i}Inv").stateChanged.connect(partial(self.setADCInv, i))
            getattr(self.view, f"checkBoxADC{i}En").stateChanged.connect(partial(self.setADCEnable, i))
            getattr(self.view, f"checkBoxADC{i}Plot").stateChanged.connect(partial(self.setADCEnablePlot, i))
            getattr(self.view, f"pushButtonADC{i}").clicked.connect(partial(self.selectADCColor, i))
        self.view.checkBoxADC0_15En.stateChanged.connect(partial(self.setADCEnableRange, 0, Defines.adc.half))
        self.view.checkBoxADC16_31En.stateChanged.connect(
            partial(self.setADCEnableRange, Defines.adc.half, Defines.adc.count))
        self.view.checkBoxADC0_15Plot.stateChanged.connect(
            partial(self.setADCEnablePlotRange, 0, Defines.adc.half))
        self.view.checkBoxADC16_31Plot.stateChanged.connect(
            partial(self.setADCEnablePlotRange, Defines.adc.half, Defines.adc.count))
        self.view.checkBoxADC0_15Inv.stateChanged.connect(partial(self.setADCInvRange, 0, Defines.adc.half))
        self.view.checkBoxADC16_31Inv.stateChanged.connect(
            partial(self.setADCInvRange, Defines.adc.half, Defines.adc.count))
        self.view.lineEditADCInversion.editingFinished.connect(self.setADCInvReg)
        self.view.lineEditADCEnable.editingFinished.connect(self.setADCEnableReg)

    def refresh(self):
        self.updateADCNames()
        self.updateADCInv()
        self.updateADCEnable()

        # ADCs Tab functions

    def updateADCNames(self):
        for i, adc_name in enumerate(self.det.getAdcNames()):
            getattr(self.view, f"labelADC{i}").setText(adc_name)

    def getADCEnableReg(self):
        retval = self.det.adcenable
        if self.det.tengiga:
            retval = self.det.adcenable10g
        self.view.lineEditADCEnable.editingFinished.disconnect()
        self.view.lineEditADCEnable.setText("0x{:08x}".format(retval))
        self.view.lineEditADCEnable.editingFinished.connect(self.setADCEnableReg)
        return retval

    def setADCEnableReg(self):
        self.view.lineEditADCEnable.editingFinished.disconnect()
        try:
            mask = int(self.mainWindow.lineEditADCEnable.text(), 16)
            if self.det.tengiga:
                self.det.adcenable10g = mask
            else:
                self.det.adcenable = mask
        except Exception as e:
            QtWidgets.QMessageBox.warning(self.mainWindow, "ADC Enable Fail", str(e), QtWidgets.QMessageBox.Ok)
            pass
        # TODO: handling double event exceptions
        self.view.lineEditADCEnable.editingFinished.connect(self.setADCEnableReg)
        self.updateADCEnable()

    def getADCEnable(self, i, mask):
        checkBox = getattr(self.view, f"checkBoxADC{i}En")
        checkBox.stateChanged.disconnect()
        checkBox.setChecked(bit_is_set(mask, i))
        checkBox.stateChanged.connect(partial(self.setADCEnable, i))

    def updateADCEnable(self):
        retval = self.getADCEnableReg()
        self.mainWindow.nADCEnabled = bin(retval).count('1')
        for i in range(Defines.adc.count):
            self.getADCEnable(i, retval)
            self.getADCEnablePlot(i)
            self.getADCEnableColor(i)
            self.plotTab.addSelectedAnalogPlots(i)
        self.getADCEnableRange(retval)
        self.getADCEnablePlotRange()

    def setADCEnable(self, i):
        checkBox = getattr(self.view, f"checkBoxADC{i}En")
        try:
            if self.det.tengiga:
                enableMask = manipulate_bit(checkBox.isChecked(), self.det.adcenable10g, i)
                self.det.adcenable10g = enableMask
            else:
                enableMask = manipulate_bit(checkBox.isChecked(), self.det.adcenable, i)
                self.det.adcenable = enableMask
        except Exception as e:
            QtWidgets.QMessageBox.warning(self.mainWindow, "ADC Enable Fail", str(e), QtWidgets.QMessageBox.Ok)
            pass

        self.updateADCEnable()

    def getADCEnableRange(self, mask):
        self.view.checkBoxADC0_15En.stateChanged.disconnect()
        self.view.checkBoxADC16_31En.stateChanged.disconnect()
        self.view.checkBoxADC0_15En.setChecked((mask & Defines.adc.BIT0_15_MASK) == Defines.adc.BIT0_15_MASK)
        self.view.checkBoxADC16_31En.setChecked((mask & Defines.adc.BIT16_31_MASK) == Defines.adc.BIT16_31_MASK)
        self.view.checkBoxADC0_15En.stateChanged.connect(partial(self.setADCEnableRange, 0, Defines.adc.half))
        self.view.checkBoxADC16_31En.stateChanged.connect(
            partial(self.setADCEnableRange, Defines.adc.half, Defines.adc.count))

    def setADCEnableRange(self, start_nr, end_nr):
        mask = self.getADCEnableReg()
        retval = 0
        checkBox = getattr(self.view, f"checkBoxADC{start_nr}_{end_nr - 1}En")
        for i in range(start_nr, end_nr):
            mask = manipulate_bit(checkBox.isChecked(), mask, i)
        try:
            if self.det.tengiga:
                self.det.adcenable10g = mask
            else:
                self.det.adcenable = mask
        except Exception as e:
            QtWidgets.QMessageBox.warning(self.mainWindow, "ADC Enable Fail", str(e), QtWidgets.QMessageBox.Ok)
            pass
        self.updateADCEnable()

    def getADCEnablePlot(self, i):
        checkBox = getattr(self.view, f"checkBoxADC{i}En")
        checkBoxPlot = getattr(self.view, f"checkBoxADC{i}Plot")
        checkBoxPlot.setEnabled(checkBox.isChecked())

    def setADCEnablePlot(self, i):
        pushButton = getattr(self.view, f"pushButtonADC{i}")
        checkBox = getattr(self.view, f"checkBoxADC{i}Plot")
        pushButton.setEnabled(checkBox.isChecked())

        self.getADCEnablePlotRange()
        self.plotTab.addSelectedAnalogPlots(i)

    def getADCEnablePlotRange(self):
        self.view.checkBoxADC0_15Plot.stateChanged.disconnect()
        self.view.checkBoxADC16_31Plot.stateChanged.disconnect()
        self.view.checkBoxADC0_15Plot.setEnabled(
            all(getattr(self.view, f"checkBoxADC{i}Plot").isEnabled() for i in range(Defines.adc.half)))
        self.view.checkBoxADC16_31Plot.setEnabled(all(
            getattr(self.view, f"checkBoxADC{i}Plot").isEnabled() for i in
            range(Defines.adc.half, Defines.adc.count)))
        self.view.checkBoxADC0_15Plot.setChecked(
            all(getattr(self.view, f"checkBoxADC{i}Plot").isChecked() for i in range(Defines.adc.half)))
        self.view.checkBoxADC16_31Plot.setChecked(all(
            getattr(self.view, f"checkBoxADC{i}Plot").isChecked() for i in
            range(Defines.adc.half, Defines.adc.count)))
        self.view.checkBoxADC0_15Plot.stateChanged.connect(
            partial(self.setADCEnablePlotRange, 0, Defines.adc.half))
        self.view.checkBoxADC16_31Plot.stateChanged.connect(
            partial(self.setADCEnablePlotRange, Defines.adc.half, Defines.adc.count))

    def setADCEnablePlotRange(self, start_nr, end_nr):
        checkBox = getattr(self.view, f"checkBoxADC{start_nr}_{end_nr - 1}Plot")
        enable = checkBox.isChecked()
        for i in range(start_nr, end_nr):
            checkBox = getattr(self.view, f"checkBoxADC{i}Plot")
            checkBox.setChecked(enable)
        self.plotTab.addAllSelectedAnalogPlots()

    def getADCEnableColor(self, i):
        checkBox = getattr(self.view, f"checkBoxADC{i}Plot")
        pushButton = getattr(self.view, f"pushButtonADC{i}")
        pushButton.setEnabled(checkBox.isEnabled() and checkBox.isChecked())

    def selectADCColor(self, i):
        pushButton = getattr(self.view, f"pushButtonADC{i}")
        self.plotTab.showPalette(pushButton)
        pen = pg.mkPen(color=self.getADCButtonColor(i), width=1)
        self.mainWindow.analogPlots[i].setPen(pen)

    def getADCButtonColor(self, i):
        pushButton = getattr(self.view, f"pushButtonADC{i}")
        return self.plotTab.getActiveColor(pushButton)

    def setADCButtonColor(self, i, color):
        pushButton = getattr(self.view, f"pushButtonADC{i}")
        return self.plotTab.setActiveColor(pushButton, color)

    def getADCInvReg(self):
        retval = self.det.adcinvert
        self.view.lineEditADCInversion.editingFinished.disconnect()
        self.view.lineEditADCInversion.setText("0x{:08x}".format(retval))
        self.view.lineEditADCInversion.editingFinished.connect(self.setADCInvReg)
        return retval

    def setADCInvReg(self):
        self.view.lineEditADCInversion.editingFinished.disconnect()
        try:
            self.det.adcinvert = int(self.mainWindow.lineEditADCInversion.text(), 16)
        except Exception as e:
            QtWidgets.QMessageBox.warning(self.mainWindow, "ADC Inversion Fail", str(e), QtWidgets.QMessageBox.Ok)
            pass
        # TODO: handling double event exceptions
        self.view.lineEditADCInversion.editingFinished.connect(self.setADCInvReg)
        self.updateADCInv()

    def getADCInv(self, i, inv):
        checkBox = getattr(self.view, f"checkBoxADC{i}Inv")
        checkBox.stateChanged.disconnect()
        checkBox.setChecked(bit_is_set(inv, i))
        checkBox.stateChanged.connect(partial(self.setADCInv, i))

    def updateADCInv(self):
        retval = self.getADCInvReg()
        for i in range(Defines.adc.count):
            self.getADCInv(i, retval)
        self.getADCInvRange(retval)

    def setADCInv(self, i):
        out = self.det.adcinvert
        checkBox = getattr(self.view, f"checkBoxADC{i}Inv")
        mask = manipulate_bit(checkBox.isChecked(), out, i)
        self.det.adcinvert = mask

        retval = self.getADCInvReg()
        self.getADCInv(i, retval)
        self.getADCInvRange(retval)

    def getADCInvRange(self, inv):
        self.view.checkBoxADC0_15Inv.stateChanged.disconnect()
        self.view.checkBoxADC16_31Inv.stateChanged.disconnect()
        self.view.checkBoxADC0_15Inv.setChecked((inv & Defines.adc.BIT0_15_MASK) == Defines.adc.BIT0_15_MASK)
        self.view.checkBoxADC16_31Inv.setChecked((inv & Defines.adc.BIT16_31_MASK) == Defines.adc.BIT16_31_MASK)
        self.view.checkBoxADC0_15Inv.stateChanged.connect(partial(self.setADCInvRange, 0, Defines.adc.half))
        self.view.checkBoxADC16_31Inv.stateChanged.connect(
            partial(self.setADCInvRange, Defines.adc.half, Defines.adc.count))

    def setADCInvRange(self, start_nr, end_nr):
        out = self.det.adcinvert
        checkBox = getattr(self.view, f"checkBoxADC{start_nr}_{end_nr - 1}Inv")
        mask = getattr(Defines, f"BIT{start_nr}_{end_nr - 1}_MASK")
        if checkBox.isChecked():
            self.det.adcinvert = out | mask
        else:
            self.det.adcinvert = out & ~mask

        self.updateADCInv()
