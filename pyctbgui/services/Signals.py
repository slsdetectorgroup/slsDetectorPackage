from functools import partial
from PyQt5 import QtWidgets
import pyqtgraph as pg

from ..utils.bit_utils import bit_is_set, manipulate_bit
from ..utils.defines import Defines


class Signals:
    def __init__(self, mainWindow):
        self.mainWindow = mainWindow

    def refresh(self):
        self.updateSignalNames()
        self.updateDigitalBitEnable()
        self.updateIOOut()
        self.getDBitOffset()

    def connect_ui(self):
        for i in range(64):
            getattr(self.mainWindow, f"checkBoxBIT{i}DB").stateChanged.connect(partial(self.setDigitalBitEnable, i))
            getattr(self.mainWindow, f"checkBoxBIT{i}Out").stateChanged.connect(partial(self.setIOOut, i))
            getattr(self.mainWindow, f"checkBoxBIT{i}Plot").stateChanged.connect(partial(self.setEnableBitPlot, i))
            getattr(self.mainWindow, f"pushButtonBIT{i}").clicked.connect(partial(self.selectBitColor, i))
        self.mainWindow.checkBoxBIT0_31DB.stateChanged.connect(partial(self.setDigitalBitEnableRange, 0, 32))
        self.mainWindow.checkBoxBIT32_63DB.stateChanged.connect(partial(self.setDigitalBitEnableRange, 32, 64))
        self.mainWindow.checkBoxBIT0_31Plot.stateChanged.connect(partial(self.setEnableBitPlotRange, 0, 32))
        self.mainWindow.checkBoxBIT32_63Plot.stateChanged.connect(partial(self.setEnableBitPlotRange, 32, 64))
        self.mainWindow.checkBoxBIT0_31Out.stateChanged.connect(partial(self.setIOOutRange, 0, 32))
        self.mainWindow.checkBoxBIT32_63Out.stateChanged.connect(partial(self.setIOOutRange, 32, 64))
        self.mainWindow.lineEditPatIOCtrl.editingFinished.connect(self.setIOOutReg)
        self.mainWindow.spinBoxDBitOffset.editingFinished.connect(self.setDbitOffset)

    def setup_ui(self):
        for i in range(64):
            self.setDBitButtonColor(i, self.mainWindow.plotTab.getRandomColor())

    def updateSignalNames(self):
        for i, name in enumerate(self.mainWindow.det.getSignalNames()):
            getattr(self.mainWindow, f"labelBIT{i}").setText(name)

    def getDigitalBitEnable(self, i, dbitList):
        checkBox = getattr(self.mainWindow, f"checkBoxBIT{i}DB")
        checkBox.stateChanged.disconnect()
        checkBox.setChecked(i in list(dbitList))
        checkBox.stateChanged.connect(partial(self.setDigitalBitEnable, i))

    def updateDigitalBitEnable(self):
        retval = self.mainWindow.det.rx_dbitlist
        self.mainWindow.rx_dbitlist = list(retval)
        self.mainWindow.nDbitEnabled = len(list(retval))
        for i in range(64):
            self.getDigitalBitEnable(i, retval)
            self.getEnableBitPlot(i)
            self.getEnableBitColor(i)
            self.mainWindow.plotTab.addSelectedDigitalPlots(i)
        self.getDigitalBitEnableRange(retval)
        self.getEnableBitPlotRange()

    def setDigitalBitEnable(self, i):
        bitList = self.mainWindow.det.rx_dbitlist
        checkBox = getattr(self.mainWindow, f"checkBoxBIT{i}DB")
        if checkBox.isChecked():
            bitList.append(i)
        else:
            bitList.remove(i)
        self.mainWindow.det.rx_dbitlist = bitList

        self.updateDigitalBitEnable()

    def getDigitalBitEnableRange(self, dbitList):
        self.mainWindow.checkBoxBIT0_31DB.stateChanged.disconnect()
        self.mainWindow.checkBoxBIT32_63DB.stateChanged.disconnect()
        self.mainWindow.checkBoxBIT0_31DB.setChecked(all(x in list(dbitList) for x in range(32)))
        self.mainWindow.checkBoxBIT32_63DB.setChecked(all(x in list(dbitList) for x in range(32, 64)))
        self.mainWindow.checkBoxBIT0_31DB.stateChanged.connect(partial(self.setDigitalBitEnableRange, 0, 32))
        self.mainWindow.checkBoxBIT32_63DB.stateChanged.connect(partial(self.setDigitalBitEnableRange, 32, 64))

    def setDigitalBitEnableRange(self, start_nr, end_nr):
        bitList = self.mainWindow.det.rx_dbitlist
        checkBox = getattr(self.mainWindow, f"checkBoxBIT{start_nr}_{end_nr - 1}DB")
        for i in range(start_nr, end_nr):
            if checkBox.isChecked():
                if i not in list(bitList):
                    bitList.append(i)
            else:
                if i in list(bitList):
                    bitList.remove(i)
        self.mainWindow.det.rx_dbitlist = bitList

        self.updateDigitalBitEnable()

    def getEnableBitPlot(self, i):
        checkBox = getattr(self.mainWindow, f"checkBoxBIT{i}DB")
        checkBoxPlot = getattr(self.mainWindow, f"checkBoxBIT{i}Plot")
        checkBoxPlot.setEnabled(checkBox.isChecked())

    def setEnableBitPlot(self, i):
        pushButton = getattr(self.mainWindow, f"pushButtonBIT{i}")
        checkBox = getattr(self.mainWindow, f"checkBoxBIT{i}Plot")
        pushButton.setEnabled(checkBox.isChecked())

        self.getEnableBitPlotRange()
        self.mainWindow.plotTab.addSelectedDigitalPlots(i)

    def getEnableBitPlotRange(self):
        self.mainWindow.checkBoxBIT0_31Plot.stateChanged.disconnect()
        self.mainWindow.checkBoxBIT32_63Plot.stateChanged.disconnect()
        self.mainWindow.checkBoxBIT0_31Plot.setEnabled(
            all(getattr(self.mainWindow, f"checkBoxBIT{i}Plot").isEnabled() for i in range(32)))
        self.mainWindow.checkBoxBIT32_63Plot.setEnabled(
            all(getattr(self.mainWindow, f"checkBoxBIT{i}Plot").isEnabled() for i in range(32, 64)))
        self.mainWindow.checkBoxBIT0_31Plot.setChecked(
            all(getattr(self.mainWindow, f"checkBoxBIT{i}Plot").isChecked() for i in range(32)))
        self.mainWindow.checkBoxBIT32_63Plot.setChecked(
            all(getattr(self.mainWindow, f"checkBoxBIT{i}Plot").isChecked() for i in range(32, 64)))
        self.mainWindow.checkBoxBIT0_31Plot.stateChanged.connect(partial(self.setEnableBitPlotRange, 0, 32))
        self.mainWindow.checkBoxBIT32_63Plot.stateChanged.connect(partial(self.setEnableBitPlotRange, 32, 64))

    def setEnableBitPlotRange(self, start_nr, end_nr):
        checkBox = getattr(self.mainWindow, f"checkBoxBIT{start_nr}_{end_nr - 1}Plot")
        enable = checkBox.isChecked()
        for i in range(start_nr, end_nr):
            checkBox = getattr(self.mainWindow, f"checkBoxBIT{i}Plot")
            checkBox.setChecked(enable)
        self.mainWindow.plotTab.addAllSelectedDigitalPlots()

    def getEnableBitColor(self, i):
        checkBox = getattr(self.mainWindow, f"checkBoxBIT{i}Plot")
        pushButton = getattr(self.mainWindow, f"pushButtonBIT{i}")
        pushButton.setEnabled(checkBox.isEnabled() and checkBox.isChecked())

    def selectBitColor(self, i):
        pushButton = getattr(self.mainWindow, f"pushButtonBIT{i}")
        self.mainWindow.plotTab.showPalette(pushButton)
        pen = pg.mkPen(color=self.getDBitButtonColor(i), width=1)
        self.mainWindow.digitalPlots[i].setPen(pen)

    def getDBitButtonColor(self, i):
        pushButton = getattr(self.mainWindow, f"pushButtonBIT{i}")
        return self.mainWindow.plotTab.getActiveColor(pushButton)

    def setDBitButtonColor(self, i, color):
        pushButton = getattr(self.mainWindow, f"pushButtonBIT{i}")
        return self.mainWindow.plotTab.setActiveColor(pushButton, color)

    def getIOOutReg(self):
        retval = self.mainWindow.det.patioctrl
        self.mainWindow.lineEditPatIOCtrl.editingFinished.disconnect()
        self.mainWindow.lineEditPatIOCtrl.setText("0x{:016x}".format(retval))
        self.mainWindow.lineEditPatIOCtrl.editingFinished.connect(self.setIOOutReg)
        return retval

    def setIOOutReg(self):
        self.mainWindow.lineEditPatIOCtrl.editingFinished.disconnect()
        try:
            self.mainWindow.det.patioctrl = int(self.mainWindow.lineEditPatIOCtrl.text(), 16)
        except Exception as e:
            QtWidgets.QMessageBox.warning(self.mainWindow, "IO Out Fail", str(e), QtWidgets.QMessageBox.Ok)
            pass
        # TODO: handling double event exceptions
        self.mainWindow.lineEditPatIOCtrl.editingFinished.connect(self.setIOOutReg)
        self.updateIOOut()

    def updateCheckBoxIOOut(self, i, out):
        checkBox = getattr(self.mainWindow, f"checkBoxBIT{i}Out")
        checkBox.stateChanged.disconnect()
        checkBox.setChecked(bit_is_set(out, i))
        checkBox.stateChanged.connect(partial(self.setIOOut, i))

    def updateIOOut(self):
        retval = self.getIOOutReg()
        for i in range(64):
            self.updateCheckBoxIOOut(i, retval)
        self.getIOoutRange(retval)

    def setIOOut(self, i):
        out = self.mainWindow.det.patioctrl
        checkBox = getattr(self.mainWindow, f"checkBoxBIT{i}Out")
        mask = manipulate_bit(checkBox.isChecked(), out, i)
        self.mainWindow.det.patioctrl = mask

        retval = self.getIOOutReg()
        self.updateCheckBoxIOOut(i, retval)
        self.getIOoutRange(retval)

    def getIOoutRange(self, out):
        self.mainWindow.checkBoxBIT0_31Out.stateChanged.disconnect()
        self.mainWindow.checkBoxBIT32_63Out.stateChanged.disconnect()
        self.mainWindow.checkBoxBIT0_31Out.setChecked((out & Defines.BIT0_31_MASK) == Defines.BIT0_31_MASK)
        self.mainWindow.checkBoxBIT32_63Out.setChecked((out & Defines.BIT32_63_MASK) == Defines.BIT32_63_MASK)
        self.mainWindow.checkBoxBIT0_31Out.stateChanged.connect(partial(self.setIOOutRange, 0, 32))
        self.mainWindow.checkBoxBIT32_63Out.stateChanged.connect(partial(self.setIOOutRange, 32, 64))

    def setIOOutRange(self, start_nr, end_nr):
        out = self.mainWindow.det.patioctrl
        checkBox = getattr(self.mainWindow, f"checkBoxBIT{start_nr}_{end_nr - 1}Out")
        mask = getattr(Defines, f"BIT{start_nr}_{end_nr - 1}_MASK")
        if checkBox.isChecked():
            self.mainWindow.det.patioctrl = out | mask
        else:
            self.mainWindow.det.patioctrl = out & ~mask
        self.updateIOOut()

    def getDBitOffset(self):
        self.mainWindow.spinBoxDBitOffset.editingFinished.disconnect()
        self.mainWindow.rx_dbitoffset = self.mainWindow.det.rx_dbitoffset
        self.mainWindow.spinBoxDBitOffset.setValue(self.mainWindow.rx_dbitoffset)
        self.mainWindow.spinBoxDBitOffset.editingFinished.connect(self.setDbitOffset)

    def setDbitOffset(self):
        self.mainWindow.det.rx_dbitoffset = self.mainWindow.spinBoxDBitOffset.value()
