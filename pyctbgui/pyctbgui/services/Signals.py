from functools import partial
from pathlib import Path

import numpy as np
from PyQt5 import QtWidgets, uic
import pyqtgraph as pg
from pyqtgraph import LegendItem

from pyctbgui.utils.bit_utils import bit_is_set, manipulate_bit
from pyctbgui.utils.defines import Defines
from pyctbgui.utils.recordOrApplyPedestal import recordOrApplyPedestal


class SignalsTab(QtWidgets.QWidget):

    def __init__(self, parent):
        super().__init__(parent)
        uic.loadUi(Path(__file__).parent.parent / 'ui' / "signals.ui", parent)
        self.view = parent
        self.mainWindow = None
        self.det = None
        self.plotTab = None
        self.legend: LegendItem | None = None
        self.rx_dbitoffset = None
        self.rx_dbitlist = None

    def refresh(self):
        self.updateSignalNames()
        self.updateDigitalBitEnable()
        self.updateIOOut()
        self.getDBitOffset()

    def connect_ui(self):
        for i in range(Defines.signals.count):
            getattr(self.view, f"checkBoxBIT{i}DB").stateChanged.connect(partial(self.setDigitalBitEnable, i))
            getattr(self.view, f"checkBoxBIT{i}Out").stateChanged.connect(partial(self.setIOOut, i))
            getattr(self.view, f"checkBoxBIT{i}Plot").stateChanged.connect(partial(self.setEnableBitPlot, i))
            getattr(self.view, f"pushButtonBIT{i}").clicked.connect(partial(self.selectBitColor, i))
        self.view.checkBoxBIT0_31DB.stateChanged.connect(
            partial(self.setDigitalBitEnableRange, 0, Defines.signals.half))
        self.view.checkBoxBIT32_63DB.stateChanged.connect(
            partial(self.setDigitalBitEnableRange, Defines.signals.half, Defines.signals.count))
        self.view.checkBoxBIT0_31Plot.stateChanged.connect(partial(self.setEnableBitPlotRange, 0,
                                                                   Defines.signals.half))
        self.view.checkBoxBIT32_63Plot.stateChanged.connect(
            partial(self.setEnableBitPlotRange, Defines.signals.half, Defines.signals.count))
        self.view.checkBoxBIT0_31Out.stateChanged.connect(partial(self.setIOOutRange, 0, Defines.signals.half))
        self.view.checkBoxBIT32_63Out.stateChanged.connect(
            partial(self.setIOOutRange, Defines.signals.half, Defines.signals.count))
        self.view.lineEditPatIOCtrl.editingFinished.connect(self.setIOOutReg)
        self.view.spinBoxDBitOffset.editingFinished.connect(self.setDbitOffset)

    def setup_ui(self):
        self.plotTab = self.mainWindow.plotTab

        for i in range(Defines.signals.count):
            self.setDBitButtonColor(i, self.plotTab.getRandomColor())

        self.initializeAllDigitalPlots()

        self.legend = self.mainWindow.plotDigitalWaveform.getPlotItem().legend
        self.legend.clear()
        # subscribe to toggle legend
        self.plotTab.subscribeToggleLegend(self.updateLegend)

    def getEnabledPlots(self):
        """
        return plots that are shown (checkBoxTransceiver{i}Plot is checked)
        """
        enabledPlots = []
        self.legend.clear()
        for i in range(Defines.signals.count):
            if getattr(self.view, f'checkBoxBIT{i}Plot').isChecked():
                plotName = getattr(self.view, f"labelBIT{i}").text()
                enabledPlots.append((self.mainWindow.digitalPlots[i], plotName))
        return enabledPlots

    def updateLegend(self):
        """
        update the legend for the signals waveform plot
        should be called after checking or unchecking plot checkbox
        """
        if not self.mainWindow.showLegend:
            self.legend.clear()
        else:
            for plot, name in self.getEnabledPlots():
                self.legend.addItem(plot, name)

    @recordOrApplyPedestal
    def _processWaveformData(self, data, aSamples, dSamples, rx_dbitlist, isPlottedArray, rx_dbitoffset, romode,
                             nADCEnabled):
        """
        transform raw waveform data into a processed numpy array
        @param data:  raw waveform data
        """
        dbitoffset = rx_dbitoffset
        if romode == 2:
            dbitoffset += nADCEnabled * 2 * aSamples
        digital_array = np.array(np.frombuffer(data, offset=dbitoffset, dtype=np.uint8))
        nbitsPerDBit = dSamples
        if nbitsPerDBit % 8 != 0:
            nbitsPerDBit += (8 - (dSamples % 8))
        offset = 0
        arr = []
        for i in rx_dbitlist:
            # where numbits * numsamples is not a multiple of 8
            if offset % 8 != 0:
                offset += (8 - (offset % 8))
            if not isPlottedArray[i]:
                offset += nbitsPerDBit
                return None
            waveform = np.zeros(dSamples)
            for iSample in range(dSamples):
                # all samples for digital bit together from slsReceiver
                index = int(offset / 8)
                iBit = offset % 8
                bit = (digital_array[index] >> iBit) & 1
                waveform[iSample] = bit
                offset += 1
            arr.append(waveform)

        return np.array(arr)

    def processWaveformData(self, data, aSamples, dSamples):
        """
        view function
        plots processed waveform data
        data: raw waveform data
        dsamples: digital samples
        asamples: analog samples
        """
        waveforms = {}
        isPlottedArray = {i: getattr(self.view, f"checkBoxBIT{i}Plot").isChecked() for i in self.rx_dbitlist}

        digital_array = self._processWaveformData(data, aSamples, dSamples, self.rx_dbitlist, isPlottedArray,
                                                  self.rx_dbitoffset, self.mainWindow.romode.value,
                                                  self.mainWindow.nADCEnabled)

        irow = 0
        for idx, i in enumerate(self.rx_dbitlist):
            # bits enabled but not plotting
            waveform = digital_array[idx]
            if waveform is None:
                continue
            self.mainWindow.digitalPlots[i].setData(waveform)
            plotName = getattr(self.view, f"labelBIT{i}").text()
            waveforms[plotName] = waveform
            # TODO: left axis does not show 0 to 1, but keeps increasing
            if self.plotTab.view.radioButtonStripe.isChecked():
                self.mainWindow.digitalPlots[i].setY(irow * 2)
                irow += 1
            else:
                self.mainWindow.digitalPlots[i].setY(0)
        return waveforms

    def initializeAllDigitalPlots(self):
        self.mainWindow.plotDigitalWaveform = pg.plot()
        self.mainWindow.plotDigitalWaveform.addLegend(colCount=Defines.colCount)
        self.mainWindow.verticalLayoutPlot.addWidget(self.mainWindow.plotDigitalWaveform, 3)
        self.mainWindow.digitalPlots = {}
        waveform = np.zeros(1000)
        for i in range(Defines.signals.count):
            pen = pg.mkPen(color=self.getDBitButtonColor(i), width=1)
            legendName = getattr(self.view, f"labelBIT{i}").text()
            self.mainWindow.digitalPlots[i] = self.mainWindow.plotDigitalWaveform.plot(waveform,
                                                                                       pen=pen,
                                                                                       name=legendName,
                                                                                       stepMode="left")
            self.mainWindow.digitalPlots[i].hide()

        self.mainWindow.plotDigitalImage = pg.ImageView()
        self.mainWindow.nDigitalRows = 0
        self.mainWindow.nDigitalCols = 0
        self.mainWindow.digital_frame = np.zeros((self.mainWindow.nDigitalRows, self.mainWindow.nDigitalCols))
        self.mainWindow.plotDigitalImage.setImage(self.mainWindow.digital_frame)
        self.mainWindow.verticalLayoutPlot.addWidget(self.mainWindow.plotDigitalImage, 4)

    def updateSignalNames(self):
        for i, name in enumerate(self.det.getSignalNames()):
            getattr(self.view, f"labelBIT{i}").setText(name)

    def getDigitalBitEnable(self, i, dbitList):
        checkBox = getattr(self.view, f"checkBoxBIT{i}DB")
        checkBox.stateChanged.disconnect()
        checkBox.setChecked(i in list(dbitList))
        checkBox.stateChanged.connect(partial(self.setDigitalBitEnable, i))

    def updateDigitalBitEnable(self):
        retval = self.det.rx_dbitlist
        self.rx_dbitlist = list(retval)
        self.mainWindow.nDBitEnabled = len(list(retval))
        for i in range(Defines.signals.count):
            self.getDigitalBitEnable(i, retval)
            self.getEnableBitPlot(i)
            self.getEnableBitColor(i)
            self.plotTab.addSelectedDigitalPlots(i)
        self.getDigitalBitEnableRange(retval)
        self.getEnableBitPlotRange()

    def setDigitalBitEnable(self, i):
        bitList = self.det.rx_dbitlist
        checkBox = getattr(self.view, f"checkBoxBIT{i}DB")
        if checkBox.isChecked():
            bitList.append(i)
        else:
            bitList.remove(i)
        self.det.rx_dbitlist = bitList

        self.updateDigitalBitEnable()

    def getDigitalBitEnableRange(self, dbitList):
        self.view.checkBoxBIT0_31DB.stateChanged.disconnect()
        self.view.checkBoxBIT32_63DB.stateChanged.disconnect()
        self.view.checkBoxBIT0_31DB.setChecked(all(x in list(dbitList) for x in range(Defines.signals.half)))
        self.view.checkBoxBIT32_63DB.setChecked(
            all(x in list(dbitList) for x in range(Defines.signals.half, Defines.signals.count)))
        self.view.checkBoxBIT0_31DB.stateChanged.connect(
            partial(self.setDigitalBitEnableRange, 0, Defines.signals.half))
        self.view.checkBoxBIT32_63DB.stateChanged.connect(
            partial(self.setDigitalBitEnableRange, Defines.signals.half, Defines.signals.count))

    def setDigitalBitEnableRange(self, start_nr, end_nr):
        bitList = self.det.rx_dbitlist
        checkBox = getattr(self.view, f"checkBoxBIT{start_nr}_{end_nr - 1}DB")
        for i in range(start_nr, end_nr):
            if checkBox.isChecked():
                if i not in list(bitList):
                    bitList.append(i)
            else:
                if i in list(bitList):
                    bitList.remove(i)
        self.det.rx_dbitlist = bitList

        self.updateDigitalBitEnable()

    def getEnableBitPlot(self, i):
        checkBox = getattr(self.view, f"checkBoxBIT{i}DB")
        checkBoxPlot = getattr(self.view, f"checkBoxBIT{i}Plot")
        checkBoxPlot.setEnabled(checkBox.isChecked())

    def setEnableBitPlot(self, i):
        pushButton = getattr(self.view, f"pushButtonBIT{i}")
        checkBox = getattr(self.view, f"checkBoxBIT{i}Plot")
        pushButton.setEnabled(checkBox.isChecked())

        self.getEnableBitPlotRange()
        self.plotTab.addSelectedDigitalPlots(i)
        self.updateLegend()

    def getEnableBitPlotRange(self):
        self.view.checkBoxBIT0_31Plot.stateChanged.disconnect()
        self.view.checkBoxBIT32_63Plot.stateChanged.disconnect()
        self.view.checkBoxBIT0_31Plot.setEnabled(
            all(getattr(self.view, f"checkBoxBIT{i}Plot").isEnabled() for i in range(Defines.signals.half)))
        self.view.checkBoxBIT32_63Plot.setEnabled(
            all(
                getattr(self.view, f"checkBoxBIT{i}Plot").isEnabled()
                for i in range(Defines.signals.half, Defines.signals.count)))
        self.view.checkBoxBIT0_31Plot.setChecked(
            all(getattr(self.view, f"checkBoxBIT{i}Plot").isChecked() for i in range(Defines.signals.half)))
        self.view.checkBoxBIT32_63Plot.setChecked(
            all(
                getattr(self.view, f"checkBoxBIT{i}Plot").isChecked()
                for i in range(Defines.signals.half, Defines.signals.count)))
        self.view.checkBoxBIT0_31Plot.stateChanged.connect(partial(self.setEnableBitPlotRange, 0,
                                                                   Defines.signals.half))
        self.view.checkBoxBIT32_63Plot.stateChanged.connect(
            partial(self.setEnableBitPlotRange, Defines.signals.half, Defines.signals.count))

    def setEnableBitPlotRange(self, start_nr, end_nr):
        checkBox = getattr(self.view, f"checkBoxBIT{start_nr}_{end_nr - 1}Plot")
        enable = checkBox.isChecked()
        for i in range(start_nr, end_nr):
            checkBox = getattr(self.view, f"checkBoxBIT{i}Plot")
            checkBox.setChecked(enable)
        self.plotTab.addAllSelectedDigitalPlots()

    def getEnableBitColor(self, i):
        checkBox = getattr(self.view, f"checkBoxBIT{i}Plot")
        pushButton = getattr(self.view, f"pushButtonBIT{i}")
        pushButton.setEnabled(checkBox.isEnabled() and checkBox.isChecked())

    def selectBitColor(self, i):
        pushButton = getattr(self.view, f"pushButtonBIT{i}")
        self.plotTab.showPalette(pushButton)
        pen = pg.mkPen(color=self.getDBitButtonColor(i), width=1)
        self.mainWindow.digitalPlots[i].setPen(pen)

    def getDBitButtonColor(self, i):
        pushButton = getattr(self.view, f"pushButtonBIT{i}")
        return self.plotTab.getActiveColor(pushButton)

    def setDBitButtonColor(self, i, color):
        pushButton = getattr(self.view, f"pushButtonBIT{i}")
        return self.plotTab.setActiveColor(pushButton, color)

    def getIOOutReg(self):
        retval = self.det.patioctrl
        self.view.lineEditPatIOCtrl.editingFinished.disconnect()
        self.view.lineEditPatIOCtrl.setText("0x{:016x}".format(retval))
        self.view.lineEditPatIOCtrl.editingFinished.connect(self.setIOOutReg)
        return retval

    def setIOOutReg(self):
        self.view.lineEditPatIOCtrl.editingFinished.disconnect()
        try:
            self.det.patioctrl = int(self.view.lineEditPatIOCtrl.text(), 16)
        except Exception as e:
            QtWidgets.QMessageBox.warning(self.mainWindow, "IO Out Fail", str(e), QtWidgets.QMessageBox.Ok)
            pass
        # TODO: handling double event exceptions
        self.view.lineEditPatIOCtrl.editingFinished.connect(self.setIOOutReg)
        self.updateIOOut()

    def updateCheckBoxIOOut(self, i, out):
        checkBox = getattr(self.view, f"checkBoxBIT{i}Out")
        checkBox.stateChanged.disconnect()
        checkBox.setChecked(bit_is_set(out, i))
        checkBox.stateChanged.connect(partial(self.setIOOut, i))

    def updateIOOut(self):
        retval = self.getIOOutReg()
        for i in range(Defines.signals.count):
            self.updateCheckBoxIOOut(i, retval)
        self.getIOoutRange(retval)

    def setIOOut(self, i):
        out = self.det.patioctrl
        checkBox = getattr(self.view, f"checkBoxBIT{i}Out")
        mask = manipulate_bit(checkBox.isChecked(), out, i)
        self.det.patioctrl = mask

        retval = self.getIOOutReg()
        self.updateCheckBoxIOOut(i, retval)
        self.getIOoutRange(retval)

    def getIOoutRange(self, out):
        self.view.checkBoxBIT0_31Out.stateChanged.disconnect()
        self.view.checkBoxBIT32_63Out.stateChanged.disconnect()
        self.view.checkBoxBIT0_31Out.setChecked((out & Defines.signals.BIT0_31_MASK) == Defines.signals.BIT0_31_MASK)
        self.view.checkBoxBIT32_63Out.setChecked((out
                                                  & Defines.signals.BIT32_63_MASK) == Defines.signals.BIT32_63_MASK)
        self.view.checkBoxBIT0_31Out.stateChanged.connect(partial(self.setIOOutRange, 0, Defines.signals.half))
        self.view.checkBoxBIT32_63Out.stateChanged.connect(
            partial(self.setIOOutRange, Defines.signals.half, Defines.signals.count))

    def setIOOutRange(self, start_nr, end_nr):
        out = self.det.patioctrl
        checkBox = getattr(self.view, f"checkBoxBIT{start_nr}_{end_nr - 1}Out")
        mask = getattr(Defines.signals, f"BIT{start_nr}_{end_nr - 1}_MASK")
        if checkBox.isChecked():
            self.det.patioctrl = out | mask
        else:
            self.det.patioctrl = out & ~mask
        self.updateIOOut()

    def getDBitOffset(self):
        self.view.spinBoxDBitOffset.editingFinished.disconnect()
        self.rx_dbitoffset = self.det.rx_dbitoffset
        self.view.spinBoxDBitOffset.setValue(self.rx_dbitoffset)
        self.view.spinBoxDBitOffset.editingFinished.connect(self.setDbitOffset)

    def setDbitOffset(self):
        self.det.rx_dbitoffset = self.view.spinBoxDBitOffset.value()

    def saveParameters(self) -> list:
        commands = []
        dblist = [str(i) for i in range(Defines.signals.count) if getattr(self.view, f"checkBoxBIT{i}DB").isChecked()]
        if len(dblist) > 0:
            commands.append(f"rx_dbitlist {', '.join(dblist)}")
        commands.append(f"rx_dbitoffset {self.view.spinBoxDBitOffset.value()}")
        commands.append(f"patioctrl {self.view.lineEditPatIOCtrl.text()}")
        return commands
