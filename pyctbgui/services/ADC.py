from functools import partial
from PyQt5 import QtWidgets
import pyqtgraph as pg

from ..utils.bit_utils import bit_is_set, manipulate_bit
from ..utils.defines import Defines


class ADC:
    def __init__(self, mainWindow):
        self.mainWindow = mainWindow
        self.det = self.mainWindow.det

    def setup_ui(self):
        for i in range(32):
            self.setADCButtonColor(i, self.mainWindow.plotTab.getRandomColor())

    def connect_ui(self):
        for i in range(32):
            getattr(self.mainWindow, f"checkBoxADC{i}Inv").stateChanged.connect(partial(self.setADCInv, i))
            getattr(self.mainWindow, f"checkBoxADC{i}En").stateChanged.connect(partial(self.setADCEnable, i))
            getattr(self.mainWindow, f"checkBoxADC{i}Plot").stateChanged.connect(partial(self.setADCEnablePlot, i))
            getattr(self.mainWindow, f"pushButtonADC{i}").clicked.connect(partial(self.selectADCColor, i))
        self.mainWindow.checkBoxADC0_15En.stateChanged.connect(partial(self.setADCEnableRange, 0, 16))
        self.mainWindow.checkBoxADC16_31En.stateChanged.connect(partial(self.setADCEnableRange, 16, 32))
        self.mainWindow.checkBoxADC0_15Plot.stateChanged.connect(partial(self.setADCEnablePlotRange, 0, 16))
        self.mainWindow.checkBoxADC16_31Plot.stateChanged.connect(partial(self.setADCEnablePlotRange, 16, 32))
        self.mainWindow.checkBoxADC0_15Inv.stateChanged.connect(partial(self.setADCInvRange, 0, 16))
        self.mainWindow.checkBoxADC16_31Inv.stateChanged.connect(partial(self.setADCInvRange, 16, 32))
        self.mainWindow.lineEditADCInversion.editingFinished.connect(self.setADCInvReg)
        self.mainWindow.lineEditADCEnable.editingFinished.connect(self.setADCEnableReg)

    def refresh(self):
        self.updateADCNames()
        self.updateADCInv()
        self.updateADCEnable()

        # ADCs Tab functions

    def updateADCNames(self):
        for i, adc_name in enumerate(self.det.getAdcNames()):
            getattr(self.mainWindow, f"labelADC{i}").setText(adc_name)

    def getADCEnableReg(self):
        retval = self.det.adcenable
        if self.det.tengiga:
            retval = self.det.adcenable10g
        self.mainWindow.lineEditADCEnable.editingFinished.disconnect()
        self.mainWindow.lineEditADCEnable.setText("0x{:08x}".format(retval))
        self.mainWindow.lineEditADCEnable.editingFinished.connect(self.setADCEnableReg)
        return retval

    def setADCEnableReg(self):
        self.mainWindow.lineEditADCEnable.editingFinished.disconnect()
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
        self.mainWindow.lineEditADCEnable.editingFinished.connect(self.setADCEnableReg)
        self.updateADCEnable()

    def getADCEnable(self, i, mask):
        checkBox = getattr(self.mainWindow, f"checkBoxADC{i}En")
        checkBox.stateChanged.disconnect()
        checkBox.setChecked(bit_is_set(mask, i))
        checkBox.stateChanged.connect(partial(self.setADCEnable, i))

    def updateADCEnable(self):
        retval = self.getADCEnableReg()
        self.mainWindow.nADCEnabled = bin(retval).count('1')
        for i in range(32):
            self.getADCEnable(i, retval)
            self.getADCEnablePlot(i)
            self.getADCEnableColor(i)
            self.mainWindow.plotTab.addSelectedAnalogPlots(i)
        self.getADCEnableRange(retval)
        self.getADCEnablePlotRange()

    def setADCEnable(self, i):
        checkBox = getattr(self.mainWindow, f"checkBoxADC{i}En")
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
        self.mainWindow.checkBoxADC0_15En.stateChanged.disconnect()
        self.mainWindow.checkBoxADC16_31En.stateChanged.disconnect()
        self.mainWindow.checkBoxADC0_15En.setChecked((mask & Defines.BIT0_15_MASK) == Defines.BIT0_15_MASK)
        self.mainWindow.checkBoxADC16_31En.setChecked((mask & Defines.BIT16_31_MASK) == Defines.BIT16_31_MASK)
        self.mainWindow.checkBoxADC0_15En.stateChanged.connect(partial(self.setADCEnableRange, 0, 16))
        self.mainWindow.checkBoxADC16_31En.stateChanged.connect(partial(self.setADCEnableRange, 16, 32))

    def setADCEnableRange(self, start_nr, end_nr):
        mask = self.getADCEnableReg()
        retval = 0
        checkBox = getattr(self.mainWindow, f"checkBoxADC{start_nr}_{end_nr - 1}En")
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
        checkBox = getattr(self.mainWindow, f"checkBoxADC{i}En")
        checkBoxPlot = getattr(self.mainWindow, f"checkBoxADC{i}Plot")
        checkBoxPlot.setEnabled(checkBox.isChecked())

    def setADCEnablePlot(self, i):
        pushButton = getattr(self.mainWindow, f"pushButtonADC{i}")
        checkBox = getattr(self.mainWindow, f"checkBoxADC{i}Plot")
        pushButton.setEnabled(checkBox.isChecked())

        self.getADCEnablePlotRange()
        self.mainWindow.plotTab.addSelectedAnalogPlots(i)

    def getADCEnablePlotRange(self):
        self.mainWindow.checkBoxADC0_15Plot.stateChanged.disconnect()
        self.mainWindow.checkBoxADC16_31Plot.stateChanged.disconnect()
        self.mainWindow.checkBoxADC0_15Plot.setEnabled(all(getattr(self.mainWindow, f"checkBoxADC{i}Plot").isEnabled() for i in range(16)))
        self.mainWindow.checkBoxADC16_31Plot.setEnabled(all(getattr(self.mainWindow, f"checkBoxADC{i}Plot").isEnabled() for i in range(16, 32)))
        self.mainWindow.checkBoxADC0_15Plot.setChecked(all(getattr(self.mainWindow, f"checkBoxADC{i}Plot").isChecked() for i in range(16)))
        self.mainWindow.checkBoxADC16_31Plot.setChecked(all(getattr(self.mainWindow, f"checkBoxADC{i}Plot").isChecked() for i in range(16, 32)))
        self.mainWindow.checkBoxADC0_15Plot.stateChanged.connect(partial(self.setADCEnablePlotRange, 0, 16))
        self.mainWindow.checkBoxADC16_31Plot.stateChanged.connect(partial(self.setADCEnablePlotRange, 16, 32))

    def setADCEnablePlotRange(self, start_nr, end_nr):
        checkBox = getattr(self.mainWindow, f"checkBoxADC{start_nr}_{end_nr - 1}Plot")
        enable = checkBox.isChecked()
        for i in range(start_nr, end_nr):
            checkBox = getattr(self.mainWindow, f"checkBoxADC{i}Plot")
            checkBox.setChecked(enable)
        self.mainWindow.plotTab.addAllSelectedAnalogPlots()

    def getADCEnableColor(self, i):
        checkBox = getattr(self.mainWindow, f"checkBoxADC{i}Plot")
        pushButton = getattr(self.mainWindow, f"pushButtonADC{i}")
        pushButton.setEnabled(checkBox.isEnabled() and checkBox.isChecked())

    def selectADCColor(self, i):
        pushButton = getattr(self.mainWindow, f"pushButtonADC{i}")
        self.mainWindow.plotTab.showPalette(pushButton)
        pen = pg.mkPen(color=self.getADCButtonColor(i), width=1)
        self.mainWindow.analogPlots[i].setPen(pen)

    def getADCButtonColor(self, i):
        pushButton = getattr(self.mainWindow, f"pushButtonADC{i}")
        return self.mainWindow.plotTab.getActiveColor(pushButton)

    def setADCButtonColor(self, i, color):
        pushButton = getattr(self.mainWindow, f"pushButtonADC{i}")
        return self.mainWindow.plotTab.setActiveColor(pushButton, color)

    def getADCInvReg(self):
        retval = self.det.adcinvert
        self.mainWindow.lineEditADCInversion.editingFinished.disconnect()
        self.mainWindow.lineEditADCInversion.setText("0x{:08x}".format(retval))
        self.mainWindow.lineEditADCInversion.editingFinished.connect(self.setADCInvReg)
        return retval

    def setADCInvReg(self):
        self.mainWindow.lineEditADCInversion.editingFinished.disconnect()
        try:
            self.det.adcinvert = int(self.mainWindow.lineEditADCInversion.text(), 16)
        except Exception as e:
            QtWidgets.QMessageBox.warning(self.mainWindow, "ADC Inversion Fail", str(e), QtWidgets.QMessageBox.Ok)
            pass
        # TODO: handling double event exceptions
        self.mainWindow.lineEditADCInversion.editingFinished.connect(self.setADCInvReg)
        self.updateADCInv()

    def getADCInv(self, i, inv):
        checkBox = getattr(self.mainWindow, f"checkBoxADC{i}Inv")
        checkBox.stateChanged.disconnect()
        checkBox.setChecked(bit_is_set(inv, i))
        checkBox.stateChanged.connect(partial(self.setADCInv, i))

    def updateADCInv(self):
        retval = self.getADCInvReg()
        for i in range(32):
            self.getADCInv(i, retval)
        self.getADCInvRange(retval)

    def setADCInv(self, i):
        out = self.det.adcinvert
        checkBox = getattr(self.mainWindow, f"checkBoxADC{i}Inv")
        mask = manipulate_bit(checkBox.isChecked(), out, i)
        self.det.adcinvert = mask

        retval = self.getADCInvReg()
        self.getADCInv(i, retval)
        self.getADCInvRange(retval)

    def getADCInvRange(self, inv):
        self.mainWindow.checkBoxADC0_15Inv.stateChanged.disconnect()
        self.mainWindow.checkBoxADC16_31Inv.stateChanged.disconnect()
        self.mainWindow.checkBoxADC0_15Inv.setChecked((inv & Defines.BIT0_15_MASK) == Defines.BIT0_15_MASK)
        self.mainWindow.checkBoxADC16_31Inv.setChecked((inv & Defines.BIT16_31_MASK) == Defines.BIT16_31_MASK)
        self.mainWindow.checkBoxADC0_15Inv.stateChanged.connect(partial(self.setADCInvRange, 0, 16))
        self.mainWindow.checkBoxADC16_31Inv.stateChanged.connect(partial(self.setADCInvRange, 16, 32))

    def setADCInvRange(self, start_nr, end_nr):
        out = self.det.adcinvert
        checkBox = getattr(self.mainWindow, f"checkBoxADC{start_nr}_{end_nr - 1}Inv")
        mask = getattr(Defines, f"BIT{start_nr}_{end_nr - 1}_MASK")
        if checkBox.isChecked():
            self.det.adcinvert = out | mask
        else:
            self.det.adcinvert = out & ~mask

        self.updateADCInv()
