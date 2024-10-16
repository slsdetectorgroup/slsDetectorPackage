from functools import partial
from pathlib import Path

import numpy as np
from PyQt5 import QtWidgets, uic
import pyqtgraph as pg
from pyqtgraph import LegendItem

from pyctbgui.utils import decoder
from pyctbgui.utils.defines import Defines

from pyctbgui.utils.bit_utils import bit_is_set, manipulate_bit
import pyctbgui.utils.pixelmap as pm
from pyctbgui.utils.recordOrApplyPedestal import recordOrApplyPedestal


class TransceiverTab(QtWidgets.QWidget):

    def __init__(self, parent):
        super().__init__(parent)
        uic.loadUi(Path(__file__).parent.parent / 'ui' / "transceiver.ui", parent)
        self.view = parent
        self.mainWindow = None
        self.det = None
        self.plotTab = None
        self.legend: LegendItem | None = None
        self.acquisitionTab = None

    def setup_ui(self):
        self.plotTab = self.mainWindow.plotTab
        self.acquisitionTab = self.mainWindow.acquisitionTab
        for i in range(Defines.transceiver.count):
            self.setTransceiverButtonColor(i, self.plotTab.getRandomColor())
        self.initializeAllTransceiverPlots()

        self.legend = self.mainWindow.plotTransceiverWaveform.getPlotItem().legend
        self.legend.clear()

        # subscribe to toggle legend
        self.plotTab.subscribeToggleLegend(self.updateLegend)

    def connect_ui(self):
        for i in range(Defines.transceiver.count):
            getattr(self.view, f"checkBoxTransceiver{i}").stateChanged.connect(partial(self.setTransceiverEnable, i))
            getattr(self.view,
                    f"checkBoxTransceiver{i}Plot").stateChanged.connect(partial(self.setTransceiverEnablePlot, i))
            getattr(self.view, f"pushButtonTransceiver{i}").clicked.connect(partial(self.selectTransceiverColor, i))
        self.view.lineEditTransceiverMask.editingFinished.connect(self.setTransceiverEnableReg)

    def refresh(self):
        self.updateTransceiverEnable()

    def getEnabledPlots(self):
        """
        return plots that are shown (checkBoxTransceiver{i}Plot is checked)
        """
        enabledPlots = []
        self.legend.clear()
        for i in range(Defines.transceiver.count):
            if getattr(self.view, f'checkBoxTransceiver{i}Plot').isChecked():
                plotName = getattr(self.view, f"labelTransceiver{i}").text()
                enabledPlots.append((self.mainWindow.transceiverPlots[i], plotName))
        return enabledPlots

    def updateLegend(self):
        """
        update the legend for the transceiver waveform plot
        should be called after checking or unchecking plot checkbox
        """
        if not self.mainWindow.showLegend:
            self.legend.clear()
        else:
            for plot, name in self.getEnabledPlots():
                self.legend.addItem(plot, name)

    @recordOrApplyPedestal
    def _processWaveformData(self, data, dSamples, romode, nDBitEnabled, nTransceiverEnabled):
        """
        model function
        processes raw receiver waveform data
        @param data: raw receiver waveform data
        @param dSamples: digital samples
        @param romode: readout mode value
        @param nDBitEnabled: number of digital bits enabled
        @param nTransceiverEnabled: number of transceivers enabled
        @return: processed transceiver data
        """
        transceiverOffset = 0
        if romode == 4:
            nbitsPerDBit = dSamples
            if dSamples % 8 != 0:
                nbitsPerDBit += (8 - (dSamples % 8))
            transceiverOffset += nDBitEnabled * (nbitsPerDBit // 8)
        trans_array = np.array(np.frombuffer(data, offset=transceiverOffset, dtype=np.uint16))
        return trans_array.reshape(-1, nTransceiverEnabled)

    def processWaveformData(self, data, dSamples):
        """
        plots raw waveform data
        data: raw waveform data
        dsamples: digital samples
        tsamples: transceiver samples
        """
        waveforms = {}
        trans_array = self._processWaveformData(data, dSamples, self.mainWindow.romode.value,
                                                self.mainWindow.nDBitEnabled, self.nTransceiverEnabled)
        idx = 0
        for i in range(Defines.transceiver.count):
            checkBoxPlot = getattr(self.view, f"checkBoxTransceiver{i}Plot")
            checkBoxEn = getattr(self.view, f"checkBoxTransceiver{i}")
            if checkBoxEn.isChecked() and checkBoxPlot.isChecked():
                waveform = trans_array[:, idx]
                idx += 1
                self.mainWindow.transceiverPlots[i].setData(waveform)
                plotName = getattr(self.view, f"labelTransceiver{i}").text()
                waveforms[plotName] = waveform
        return waveforms

    @recordOrApplyPedestal
    def _processImageData(self, data, dSamples, romode, nDBitEnabled):
        """
        processes raw image data
        @param data:
        @param dSamples:
        @param romode:
        @param nDBitEnabled:
        @return:
        """
        transceiverOffset = 0
        if romode == 4:
            nbitsPerDBit = dSamples
            if dSamples % 8 != 0:
                nbitsPerDBit += (8 - (dSamples % 8))
            transceiverOffset += nDBitEnabled * (nbitsPerDBit // 8)
        trans_array = np.array(np.frombuffer(data, offset=transceiverOffset, dtype=np.uint16))
        return decoder.decode(trans_array, pm.matterhorn_transceiver())

    def processImageData(self, data, dSamples):
        """
        view function
        plots transceiver image
        dSamples: digital samples
        data: raw image data
        """
        # get zoom state
        viewBox = self.mainWindow.plotTransceiverImage.getView()
        state = viewBox.getState()
        try:
            self.mainWindow.transceiver_frame = self._processImageData(data, dSamples, self.mainWindow.romode.value,
                                                                       self.mainWindow.nDBitEnabled)
            self.plotTab.ignoreHistogramSignal = True
            self.mainWindow.plotTransceiverImage.setImage(self.mainWindow.transceiver_frame)
        except Exception:
            self.mainWindow.statusbar.setStyleSheet("color:red")
            message = f'Warning: Invalid size for Transceiver Image. Expected' \
                      f' {self.mainWindow.nTransceiverRows * self.mainWindow.nTransceiverCols} size,' \
                      f' got {self.mainWindow.transceiver_frame.size} instead.'
            self.acquisitionTab.updateCurrentFrame('Invalid Image')
            self.mainWindow.statusbar.showMessage(message)
            print(message)

        self.plotTab.setFrameLimits(self.mainWindow.transceiver_frame)

        # keep the zoomed in state (not 1st image)
        if self.mainWindow.firstTransceiverImage:
            self.mainWindow.firstTransceiverImage = False
        else:
            viewBox.setState(state)
        return self.mainWindow.transceiver_frame

    def initializeAllTransceiverPlots(self):
        self.mainWindow.plotTransceiverWaveform = pg.plot()
        self.mainWindow.plotTransceiverWaveform.addLegend(colCount=Defines.colCount)
        self.mainWindow.verticalLayoutPlot.addWidget(self.mainWindow.plotTransceiverWaveform, 5)
        self.mainWindow.transceiverPlots = {}
        waveform = np.zeros(1000)
        for i in range(Defines.transceiver.count):
            pen = pg.mkPen(color=self.getTransceiverButtonColor(i), width=1)
            legendName = getattr(self.view, f"labelTransceiver{i}").text()
            self.mainWindow.transceiverPlots[i] = self.mainWindow.plotTransceiverWaveform.plot(waveform,
                                                                                               pen=pen,
                                                                                               name=legendName)
            self.mainWindow.transceiverPlots[i].hide()

        self.mainWindow.plotTransceiverImage = pg.ImageView()
        self.mainWindow.nTransceiverRows = 0
        self.mainWindow.nTransceiverCols = 0
        self.mainWindow.transceiver_frame = np.zeros(
            (self.mainWindow.nTransceiverRows, self.mainWindow.nTransceiverCols))
        self.mainWindow.plotTransceiverImage.setImage(self.mainWindow.transceiver_frame)
        self.mainWindow.verticalLayoutPlot.addWidget(self.mainWindow.plotTransceiverImage, 6)

        cm = pg.colormap.get('CET-L9')  # prepare a linear color map
        self.mainWindow.plotTransceiverImage.setColorMap(cm)

    def getTransceiverEnableReg(self):
        retval = self.det.transceiverenable
        self.view.lineEditTransceiverMask.editingFinished.disconnect()
        self.view.lineEditTransceiverMask.setText("0x{:08x}".format(retval))
        self.view.lineEditTransceiverMask.editingFinished.connect(self.setTransceiverEnableReg)
        return retval

    def setTransceiverEnableReg(self):
        self.view.lineEditTransceiverMask.editingFinished.disconnect()
        try:
            mask = int(self.view.lineEditTransceiverMask.text(), 16)
            self.det.transceiverenable = mask
        except Exception as e:
            QtWidgets.QMessageBox.warning(self.mainWindow, "Transceiver Enable Fail", str(e), QtWidgets.QMessageBox.Ok)
            pass
        # TODO: handling double event exceptions
        self.view.lineEditTransceiverMask.editingFinished.connect(self.setTransceiverEnableReg)
        self.updateTransceiverEnable()

    def getTransceiverEnable(self, i, mask):
        checkBox = getattr(self.view, f"checkBoxTransceiver{i}")
        checkBox.stateChanged.disconnect()
        checkBox.setChecked(bit_is_set(mask, i))
        checkBox.stateChanged.connect(partial(self.setTransceiverEnable, i))

    def updateTransceiverEnable(self):
        retval = self.getTransceiverEnableReg()
        self.nTransceiverEnabled = bin(retval).count('1')
        for i in range(4):
            self.getTransceiverEnable(i, retval)
            self.getTransceiverEnablePlot(i)
            self.getTransceiverEnableColor(i)
            self.plotTab.addSelectedTransceiverPlots(i)

    def setTransceiverEnable(self, i):
        checkBox = getattr(self.view, f"checkBoxTransceiver{i}")
        try:
            enableMask = manipulate_bit(checkBox.isChecked(), self.det.transceiverenable, i)
            self.det.transceiverenable = enableMask
        except Exception as e:
            QtWidgets.QMessageBox.warning(self.mainWindow, "Transceiver Enable Fail", str(e), QtWidgets.QMessageBox.Ok)
            pass

        self.updateTransceiverEnable()

    def getTransceiverEnablePlot(self, i):
        checkBox = getattr(self.view, f"checkBoxTransceiver{i}")
        checkBoxPlot = getattr(self.view, f"checkBoxTransceiver{i}Plot")
        checkBoxPlot.setEnabled(checkBox.isChecked())

    def setTransceiverEnablePlot(self, i):
        pushButton = getattr(self.view, f"pushButtonTransceiver{i}")
        checkBox = getattr(self.view, f"checkBoxTransceiver{i}Plot")
        pushButton.setEnabled(checkBox.isChecked())
        self.plotTab.addSelectedTransceiverPlots(i)
        self.updateLegend()

    def getTransceiverEnableColor(self, i):
        checkBox = getattr(self.view, f"checkBoxTransceiver{i}Plot")
        pushButton = getattr(self.view, f"pushButtonTransceiver{i}")
        pushButton.setEnabled(checkBox.isEnabled() and checkBox.isChecked())

    def selectTransceiverColor(self, i):
        pushButton = getattr(self.view, f"pushButtonTransceiver{i}")
        self.plotTab.showPalette(pushButton)
        pen = pg.mkPen(color=self.getTransceiverButtonColor(i), width=1)
        self.mainWindow.transceiverPlots[i].setPen(pen)

    def getTransceiverButtonColor(self, i):
        pushButton = getattr(self.view, f"pushButtonTransceiver{i}")
        return self.plotTab.getActiveColor(pushButton)

    def setTransceiverButtonColor(self, i, color):
        pushButton = getattr(self.view, f"pushButtonTransceiver{i}")
        return self.plotTab.setActiveColor(pushButton, color)

    def saveParameters(self):
        return ["transceiverenable {}".format(self.view.lineEditTransceiverMask.text())]
