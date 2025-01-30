import logging
import typing
from functools import partial
from pathlib import Path

import numpy as np
from PyQt5 import QtWidgets, uic
import pyqtgraph as pg
from pyqtgraph import LegendItem

from pyctbgui.utils import decoder
from pyctbgui.utils.bit_utils import bit_is_set, manipulate_bit
from pyctbgui.utils.defines import Defines
import pyctbgui.utils.pixelmap as pm
from pyctbgui.utils.recordOrApplyPedestal import recordOrApplyPedestal

from slsdet import detectorType

if typing.TYPE_CHECKING:
    from pyctbgui.services import AcquisitionTab, PlotTab


class AdcTab(QtWidgets.QWidget):

    def __init__(self, parent, *args, **kwargs):
        super().__init__(parent, *args, **kwargs)
        uic.loadUi(Path(__file__).parent.parent / 'ui' / "adc.ui", parent)
        self.view = parent
        self.mainWindow = None
        self.det = None
        self.plotTab: PlotTab | None = None
        self.acquisitionTab: AcquisitionTab | None = None
        self.legend: LegendItem | None = None
        self.logger = logging.getLogger('AdcTab')
        self.tengiga = True

    def setup_ui(self):
        self.plotTab = self.mainWindow.plotTab
        self.acquisitionTab = self.mainWindow.acquisitionTab
        for i in range(Defines.adc.count):
            self.setADCButtonColor(i, self.plotTab.getRandomColor())
        self.initializeAllAnalogPlots()

        self.legend = self.mainWindow.plotAnalogWaveform.getPlotItem().legend
        self.legend.clear()
        # subscribe to toggle legend
        self.plotTab.subscribeToggleLegend(self.updateLegend)
        
        if self.det.type == detectorType.XILINX_CHIPTESTBOARD:
            self.view.checkBoxADC0_15Inv.setDisabled(True)
            self.view.checkBoxADC16_31Inv.setDisabled(True)
            self.view.lineEditADCInversion.setDisabled(True)
            self.view.labelADCInversion.setDisabled(True)

    def initializeAllAnalogPlots(self):
        self.mainWindow.plotAnalogWaveform = pg.plot()
        self.mainWindow.plotAnalogWaveform.addLegend(colCount=Defines.colCount)
        self.mainWindow.verticalLayoutPlot.addWidget(self.mainWindow.plotAnalogWaveform, 1)
        self.mainWindow.analogPlots = {}
        waveform = np.zeros(1000)
        for i in range(Defines.adc.count):
            pen = pg.mkPen(color=self.getADCButtonColor(i), width=1)
            legendName = getattr(self.view, f"labelADC{i}").text()
            self.mainWindow.analogPlots[i] = self.mainWindow.plotAnalogWaveform.plot(waveform,
                                                                                     pen=pen,
                                                                                     name=legendName)
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
            if self.det.type == detectorType.CHIPTESTBOARD:
                getattr(self.view, f"checkBoxADC{i}Inv").stateChanged.connect(partial(self.setADCInv, i))
            getattr(self.view, f"checkBoxADC{i}En").stateChanged.connect(partial(self.setADCEnable, i))
            getattr(self.view, f"checkBoxADC{i}Plot").stateChanged.connect(partial(self.setADCEnablePlot, i))
            getattr(self.view, f"pushButtonADC{i}").clicked.connect(partial(self.selectADCColor, i))
        self.view.checkBoxADC0_15En.stateChanged.connect(partial(self.setADCEnableRange, 0, Defines.adc.half))
        self.view.checkBoxADC16_31En.stateChanged.connect(
            partial(self.setADCEnableRange, Defines.adc.half, Defines.adc.count))
        self.view.checkBoxADC0_15Plot.stateChanged.connect(partial(self.setADCEnablePlotRange, 0, Defines.adc.half))
        self.view.checkBoxADC16_31Plot.stateChanged.connect(
            partial(self.setADCEnablePlotRange, Defines.adc.half, Defines.adc.count))
        self.view.lineEditADCEnable.editingFinished.connect(self.setADCEnableReg)
        if self.det.type == detectorType.CHIPTESTBOARD:
            self.view.checkBoxADC0_15Inv.stateChanged.connect(partial(self.setADCInvRange, 0, Defines.adc.half))
            self.view.checkBoxADC16_31Inv.stateChanged.connect(
            partial(self.setADCInvRange, Defines.adc.half, Defines.adc.count))
            self.view.lineEditADCInversion.editingFinished.connect(self.setADCInvReg)

    def refresh(self):
        self.updateADCNames()
        if self.det.type == detectorType.CHIPTESTBOARD:
            self.updateADCInv()
        self.updateADCEnable()

        # ADCs Tab functions

    def getEnabledPlots(self):
        """
        return plots that are shown (checkBoxADC#Plot is checked)
        """
        enabledPlots = []
        self.legend.clear()
        for i in range(Defines.adc.count):
            if getattr(self.view, f'checkBoxADC{i}Plot').isChecked():
                plotName = getattr(self.view, f"labelADC{i}").text()
                enabledPlots.append((self.mainWindow.analogPlots[i], plotName))
        return enabledPlots

    def updateLegend(self):
        """
        update the legend for the ADC waveform plot
        should be called after checking or unchecking plot checkbox
        """
        if not self.mainWindow.showLegend:
            self.legend.clear()
        else:
            for plot, name in self.getEnabledPlots():
                self.legend.addItem(plot, name)

    def updateADCNames(self):
        """
        get adc names from detector and update them in the UI
        """
        for i, adc_name in enumerate(self.det.getAdcNames()):
            getattr(self.view, f"labelADC{i}").setText(adc_name)

    def processWaveformData(self, data: bytes, aSamples: int) -> dict[str, np.ndarray]:
        """
        view function
        plots processed waveform data
        @param data: raw waveform data
        @param aSamples: analog samples
        @return: waveform dict returned to handle it for saving the output
        """

        waveforms = {}
        analog_array = self._processWaveformData(data, aSamples, self.mainWindow.nADCEnabled)
        idx = 0
        for i in range(Defines.adc.count):
            checkBoxPlot = getattr(self.view, f"checkBoxADC{i}Plot")
            checkBoxEn = getattr(self.view, f"checkBoxADC{i}En")

            if checkBoxEn.isChecked() and checkBoxPlot.isChecked():
                waveform = analog_array[:, idx]
                idx += 1
                self.mainWindow.analogPlots[i].setData(waveform)
                plotName = getattr(self.view, f"labelADC{i}").text()
                waveforms[plotName] = waveform
        return waveforms

    @recordOrApplyPedestal
    def _processWaveformData(self, data: bytes, aSamples: int, nADCEnabled: int) -> np.ndarray:
        """
        model function
        processes raw waveform data
        @param data: raw waveform data
        @param aSamples: analog samples
        @param nADCEnabled: number of enabled ADCs
        @return: processed waveform data
        """
        analog_array = np.array(np.frombuffer(data, dtype=np.uint16, count=nADCEnabled * aSamples))
        return analog_array.reshape(-1, nADCEnabled)

    def processImageData(self, data, aSamples):
        """
        process the raw receiver data for analog image
        data: raw analog image
        aSamples: analog samples
        """
        # get zoom state
        viewBox = self.mainWindow.plotAnalogImage.getView()
        state = viewBox.getState()
        try:
            self.mainWindow.analog_frame = self._processImageData(data, aSamples, self.mainWindow.nADCEnabled)
            self.plotTab.ignoreHistogramSignal = True
            self.mainWindow.plotAnalogImage.setImage(self.mainWindow.analog_frame.T)
        except Exception:
            self.logger.exception('Exception Caught')
            self.mainWindow.statusbar.setStyleSheet("color:red")
            message = f'Warning: Invalid size for Analog Image. Expected' \
                      f' {self.mainWindow.nAnalogRows * self.mainWindow.nAnalogCols} ' \
                      f'size, got {self.mainWindow.analog_frame.size} instead.'
            self.acquisitionTab.updateCurrentFrame('Invalid Image')

            self.mainWindow.statusbar.showMessage(message)
            print(message)

        self.plotTab.setFrameLimits(self.mainWindow.analog_frame)

        # keep the zoomed in state (not 1st image)
        if self.mainWindow.firstAnalogImage:
            self.mainWindow.firstAnalogImage = False
        else:
            viewBox.setState(state)
        return self.mainWindow.analog_frame.T

    @recordOrApplyPedestal
    def _processImageData(self, data, aSamples, nADCEnabled):
        analog_array = np.array(np.frombuffer(data, dtype=np.uint16, count=nADCEnabled * aSamples))
        return decoder.decode(analog_array, pm.moench04_analog())

    def getADCEnableReg(self):
        if self.det.type == detectorType.CHIPTESTBOARD:
            self.tengiga = self.det.tengiga
        retval = self.det.adcenable10g
        if not self.tengiga:
            retval = self.det.adcenable
        self.view.lineEditADCEnable.editingFinished.disconnect()
        self.view.lineEditADCEnable.setText("0x{:08x}".format(retval))
        self.view.lineEditADCEnable.editingFinished.connect(self.setADCEnableReg)
        return retval

    def setADCEnableReg(self):
        self.view.lineEditADCEnable.editingFinished.disconnect()
        try:
            mask = int(self.view.lineEditADCEnable.text(), 16)
            if self.tengiga:
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
            if self.tengiga:
                enableMask = manipulate_bit(checkBox.isChecked(), self.det.adcenable10g, i)
                self.det.adcenable10g = enableMask
            else:
                enableMask = manipulate_bit(checkBox.isChecked(), self.det.adcenable, i)
                self.det.adcenable = enableMask
        except Exception as e:
            QtWidgets.QMessageBox.warning(self.mainWindow, "ADC Enable Fail", str(e), QtWidgets.QMessageBox.Ok)

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
        checkBox = getattr(self.view, f"checkBoxADC{start_nr}_{end_nr - 1}En")
        for i in range(start_nr, end_nr):
            mask = manipulate_bit(checkBox.isChecked(), mask, i)
        try:
            if self.tengiga:
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
        self.updateLegend()

    def getADCEnablePlotRange(self):
        self.view.checkBoxADC0_15Plot.stateChanged.disconnect()
        self.view.checkBoxADC16_31Plot.stateChanged.disconnect()
        self.view.checkBoxADC0_15Plot.setEnabled(
            all(getattr(self.view, f"checkBoxADC{i}Plot").isEnabled() for i in range(Defines.adc.half)))
        self.view.checkBoxADC16_31Plot.setEnabled(
            all(
                getattr(self.view, f"checkBoxADC{i}Plot").isEnabled()
                for i in range(Defines.adc.half, Defines.adc.count)))
        self.view.checkBoxADC0_15Plot.setChecked(
            all(getattr(self.view, f"checkBoxADC{i}Plot").isChecked() for i in range(Defines.adc.half)))
        self.view.checkBoxADC16_31Plot.setChecked(
            all(
                getattr(self.view, f"checkBoxADC{i}Plot").isChecked()
                for i in range(Defines.adc.half, Defines.adc.count)))
        self.view.checkBoxADC0_15Plot.stateChanged.connect(partial(self.setADCEnablePlotRange, 0, Defines.adc.half))
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
            self.det.adcinvert = int(self.view.lineEditADCInversion.text(), 16)
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
        mask = getattr(Defines.adc, f"BIT{start_nr}_{end_nr - 1}_MASK")
        if checkBox.isChecked():
            self.det.adcinvert = out | mask
        else:
            self.det.adcinvert = out & ~mask

        self.updateADCInv()

    def saveParameters(self) -> list[str]:
        if self.det.type == detectorType.CHIPTESTBOARD:
            return [
                f"adcenable {self.view.lineEditADCEnable.text()}",
                f"adcinvert {self.view.lineEditADCInversion.text()}",
            ]
        else:
            return [
                f"adcenable {self.view.lineEditADCEnable.text()}"
            ]     
