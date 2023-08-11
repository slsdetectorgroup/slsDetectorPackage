import json
import os
from pathlib import Path
from utils import decoder

import numpy as np
from slsdet import readoutMode, runStatus
from PyQt5 import QtWidgets

from utils.SingletonMeta import SingletonMeta
from utils.defines import Defines

import time
import zmq


class AcquisitionService(metaclass=SingletonMeta):
    def __init__(self, mainWindow):
        self.mainWindow = mainWindow
        self.det = self.mainWindow.det

    def setup_ui(self):
        self.toggleStartButton(False)

    def connect_ui(self):
        # For Acquistions Tab
        self.mainWindow.comboBoxROMode.currentIndexChanged.connect(self.setReadOut)
        self.mainWindow.spinBoxRunF.editingFinished.connect(self.setRunFrequency)
        self.mainWindow.spinBoxTransceiver.editingFinished.connect(self.setTransceiver)
        self.mainWindow.spinBoxAnalog.editingFinished.connect(self.setAnalog)
        self.mainWindow.spinBoxDigital.editingFinished.connect(self.setDigital)
        self.mainWindow.spinBoxADCF.editingFinished.connect(self.setADCFrequency)
        self.mainWindow.spinBoxADCPhase.editingFinished.connect(self.setADCPhase)
        self.mainWindow.spinBoxADCPipeline.editingFinished.connect(self.setADCPipeline)
        self.mainWindow.spinBoxDBITF.editingFinished.connect(self.setDBITFrequency)
        self.mainWindow.spinBoxDBITPhase.editingFinished.connect(self.setDBITPhase)
        self.mainWindow.spinBoxDBITPipeline.editingFinished.connect(self.setDBITPipeline)

        self.mainWindow.radioButtonNoPlot.clicked.connect(self.mainWindow.plotService.plotOptions)
        self.mainWindow.radioButtonWaveform.clicked.connect(self.mainWindow.plotService.plotOptions)
        self.mainWindow.radioButtonDistribution.clicked.connect(self.mainWindow.plotService.plotOptions)
        self.mainWindow.radioButtonImage.clicked.connect(self.mainWindow.plotService.plotOptions)
        self.mainWindow.comboBoxPlot.currentIndexChanged.connect(self.mainWindow.plotService.setPixelMap)
        self.mainWindow.comboBoxColorMap.currentIndexChanged.connect(self.mainWindow.plotService.setColorMap)
        self.mainWindow.comboBoxZMQHWM.currentIndexChanged.connect(self.mainWindow.plotService.setZMQHWM)
        self.mainWindow.spinBoxSerialOffset.editingFinished.connect(self.mainWindow.plotService.setSerialOffset)
        self.mainWindow.spinBoxNCount.editingFinished.connect(self.mainWindow.plotService.setNCounter)
        self.mainWindow.spinBoxDynamicRange.editingFinished.connect(self.mainWindow.plotService.setDynamicRange)
        self.mainWindow.spinBoxImageX.editingFinished.connect(self.mainWindow.plotService.setImageX)
        self.mainWindow.spinBoxImageY.editingFinished.connect(self.mainWindow.plotService.setImageY)
        self.mainWindow.checkBoxAcquire.stateChanged.connect(self.mainWindow.plotService.setPedestal)
        self.mainWindow.checkBoxSubtract.stateChanged.connect(self.mainWindow.plotService.setPedestal)
        self.mainWindow.checkBoxCommonMode.stateChanged.connect(self.mainWindow.plotService.setPedestal)
        self.mainWindow.pushButtonReset.clicked.connect(self.mainWindow.plotService.resetPedestal)
        self.mainWindow.checkBoxRaw.stateChanged.connect(self.mainWindow.plotService.setRawData)
        self.mainWindow.spinBoxRawMin.editingFinished.connect(self.mainWindow.plotService.setRawData)
        self.mainWindow.spinBoxRawMax.editingFinished.connect(self.mainWindow.plotService.setRawData)
        self.mainWindow.checkBoxPedestal.stateChanged.connect(self.mainWindow.plotService.setPedestalSubtract)
        self.mainWindow.spinBoxPedestalMin.editingFinished.connect(self.mainWindow.plotService.setPedestalSubtract)
        self.mainWindow.spinBoxPedestalMax.editingFinished.connect(self.mainWindow.plotService.setPedestalSubtract)
        self.mainWindow.spinBoxFit.editingFinished.connect(self.mainWindow.plotService.setFitADC)
        self.mainWindow.spinBoxPlot.editingFinished.connect(self.mainWindow.plotService.setPlotBit)
        self.mainWindow.pushButtonReferesh.clicked.connect(self.mainWindow.plotService.plotReferesh)

        self.mainWindow.checkBoxFileWrite.stateChanged.connect(self.setFileWrite)
        self.mainWindow.lineEditFileName.editingFinished.connect(self.setFileName)
        self.mainWindow.lineEditFilePath.editingFinished.connect(self.setFilePath)
        self.mainWindow.pushButtonFilePath.clicked.connect(self.browseFilePath)
        self.mainWindow.spinBoxAcquisitionIndex.editingFinished.connect(self.setAccquisitionIndex)
        self.mainWindow.spinBoxFrames.editingFinished.connect(self.setFrames)
        self.mainWindow.spinBoxPeriod.editingFinished.connect(self.setPeriod)
        self.mainWindow.comboBoxPeriod.currentIndexChanged.connect(self.setPeriod)
        self.mainWindow.spinBoxTriggers.editingFinished.connect(self.setTriggers)
        self.mainWindow.pushButtonStart.clicked.connect(self.toggleAcquire)

    def refresh(self):
        self.getReadout()
        self.getRunFrequency()
        self.getTransceiver()
        self.getAnalog()
        self.getDigital()
        self.getADCFrequency()
        self.getADCPhase()
        self.getADCPipeline()
        self.getDBITFrequency()
        self.getDBITPhase()
        self.getDBITPipeline()
        self.getFileWrite()
        self.getFileName()
        self.getFilePath()
        self.getAccquisitionIndex()
        self.getFrames()
        self.getTriggers()
        self.getPeriod()
        self.updateDetectorStatus(self.det.status)

    # Acquisition Tab functions

    def getReadout(self):
        self.mainWindow.comboBoxROMode.currentIndexChanged.disconnect()
        self.mainWindow.spinBoxAnalog.editingFinished.disconnect()
        self.mainWindow.spinBoxDigital.editingFinished.disconnect()
        self.mainWindow.spinBoxTransceiver.editingFinished.disconnect()

        self.mainWindow.romode = self.det.romode
        self.mainWindow.comboBoxROMode.setCurrentIndex(self.mainWindow.romode.value)
        match self.mainWindow.romode:
            case readoutMode.ANALOG_ONLY:
                self.mainWindow.spinBoxAnalog.setEnabled(True)
                self.mainWindow.labelAnalog.setEnabled(True)
                self.mainWindow.spinBoxDigital.setDisabled(True)
                self.mainWindow.labelDigital.setDisabled(True)
                self.mainWindow.labelTransceiver.setDisabled(True)
                self.mainWindow.spinBoxTransceiver.setDisabled(True)
            case readoutMode.DIGITAL_ONLY:
                self.mainWindow.spinBoxAnalog.setDisabled(True)
                self.mainWindow.labelAnalog.setDisabled(True)
                self.mainWindow.spinBoxDigital.setEnabled(True)
                self.mainWindow.labelDigital.setEnabled(True)
                self.mainWindow.labelTransceiver.setDisabled(True)
                self.mainWindow.spinBoxTransceiver.setDisabled(True)
            case readoutMode.ANALOG_AND_DIGITAL:
                self.mainWindow.spinBoxAnalog.setEnabled(True)
                self.mainWindow.labelAnalog.setEnabled(True)
                self.mainWindow.spinBoxDigital.setEnabled(True)
                self.mainWindow.labelDigital.setEnabled(True)
                self.mainWindow.labelTransceiver.setDisabled(True)
                self.mainWindow.spinBoxTransceiver.setDisabled(True)
            case readoutMode.TRANSCEIVER_ONLY:
                self.mainWindow.spinBoxAnalog.setDisabled(True)
                self.mainWindow.labelAnalog.setDisabled(True)
                self.mainWindow.spinBoxDigital.setDisabled(True)
                self.mainWindow.labelDigital.setDisabled(True)
                self.mainWindow.labelTransceiver.setEnabled(True)
                self.mainWindow.spinBoxTransceiver.setEnabled(True)
            case _:
                self.mainWindow.spinBoxAnalog.setDisabled(True)
                self.mainWindow.labelAnalog.setDisabled(True)
                self.mainWindow.spinBoxDigital.setEnabled(True)
                self.mainWindow.labelDigital.setEnabled(True)
                self.mainWindow.labelTransceiver.setEnabled(True)
                self.mainWindow.spinBoxTransceiver.setEnabled(True)

        self.mainWindow.comboBoxROMode.currentIndexChanged.connect(self.setReadOut)
        self.mainWindow.spinBoxAnalog.editingFinished.connect(self.setAnalog)
        self.mainWindow.spinBoxDigital.editingFinished.connect(self.setDigital)
        self.mainWindow.spinBoxTransceiver.editingFinished.connect(self.setTransceiver)
        self.getAnalog()
        self.getDigital()
        self.mainWindow.plotService.showPlot()

    def setReadOut(self):
        self.mainWindow.comboBoxROMode.currentIndexChanged.disconnect()
        try:
            if self.mainWindow.comboBoxROMode.currentIndex() == 0:
                self.det.romode = readoutMode.ANALOG_ONLY
            elif self.mainWindow.comboBoxROMode.currentIndex() == 1:
                self.det.romode = readoutMode.DIGITAL_ONLY
            elif self.mainWindow.comboBoxROMode.currentIndex() == 2:
                self.det.romode = readoutMode.ANALOG_AND_DIGITAL
            elif self.mainWindow.comboBoxROMode.currentIndex() == 3:
                self.det.romode = readoutMode.TRANSCEIVER_ONLY
            else:
                self.det.romode = readoutMode.DIGITAL_AND_TRANSCEIVER
        except Exception as e:
            QtWidgets.QMessageBox.warning(self.mainWindow, "Readout Mode Fail", str(e), QtWidgets.QMessageBox.Ok)
            pass
        # TODO: handling double event exceptions
        self.mainWindow.comboBoxROMode.currentIndexChanged.connect(self.setReadOut)
        self.getReadout()

    def getRunFrequency(self):
        self.mainWindow.spinBoxRunF.editingFinished.disconnect()
        self.mainWindow.spinBoxRunF.setValue(self.det.runclk)
        self.mainWindow.spinBoxRunF.editingFinished.connect(self.setRunFrequency)

    def setRunFrequency(self):
        self.mainWindow.spinBoxRunF.editingFinished.disconnect()
        try:
            self.det.runclk = self.mainWindow.spinBoxRunF.value()
        except Exception as e:
            QtWidgets.QMessageBox.warning(self.mainWindow, "Run Frequency Fail", str(e), QtWidgets.QMessageBox.Ok)
            pass
        # TODO: handling double event exceptions
        self.mainWindow.spinBoxRunF.editingFinished.connect(self.setRunFrequency)
        self.getRunFrequency()

    def getTransceiver(self):
        self.mainWindow.spinBoxTransceiver.editingFinished.disconnect()
        self.tsamples = self.det.tsamples
        self.mainWindow.spinBoxTransceiver.setValue(self.tsamples)
        self.mainWindow.spinBoxTransceiver.editingFinished.connect(self.setTransceiver)

    def setTransceiver(self):
        self.mainWindow.spinBoxTransceiver.editingFinished.disconnect()
        try:
            self.det.tsamples = self.mainWindow.spinBoxTransceiver.value()
        except Exception as e:
            QtWidgets.QMessageBox.warning(self.mainWindow, "Transceiver Samples Fail", str(e), QtWidgets.QMessageBox.Ok)
            pass
        # TODO: handling double event exceptions
        self.mainWindow.spinBoxTransceiver.editingFinished.connect(self.setTransceiver)
        self.getTransceiver()

    def getAnalog(self):
        self.mainWindow.spinBoxAnalog.editingFinished.disconnect()
        self.asamples = self.det.asamples
        self.mainWindow.spinBoxAnalog.setValue(self.asamples)
        self.mainWindow.spinBoxAnalog.editingFinished.connect(self.setAnalog)

    def setAnalog(self):
        self.mainWindow.spinBoxAnalog.editingFinished.disconnect()
        try:
            self.det.asamples = self.mainWindow.spinBoxAnalog.value()
        except Exception as e:
            QtWidgets.QMessageBox.warning(self.mainWindow, "Digital Samples Fail", str(e), QtWidgets.QMessageBox.Ok)
            pass
        # TODO: handling double event exceptions
        self.mainWindow.spinBoxAnalog.editingFinished.connect(self.setAnalog)
        self.getAnalog()

    def getDigital(self):
        self.mainWindow.spinBoxDigital.editingFinished.disconnect()
        self.dsamples = self.det.dsamples
        self.mainWindow.spinBoxDigital.setValue(self.dsamples)
        self.mainWindow.spinBoxDigital.editingFinished.connect(self.setDigital)

    def setDigital(self):
        self.mainWindow.spinBoxDigital.editingFinished.disconnect()
        try:
            self.det.dsamples = self.mainWindow.spinBoxDigital.value()
        except Exception as e:
            QtWidgets.QMessageBox.warning(self.mainWindow, "Digital Samples Fail", str(e), QtWidgets.QMessageBox.Ok)
            pass
        # TODO: handling double event exceptions
        self.mainWindow.spinBoxDigital.editingFinished.connect(self.setDigital)
        self.getDigital()

    def getADCFrequency(self):
        self.mainWindow.spinBoxADCF.editingFinished.disconnect()
        self.mainWindow.spinBoxADCF.setValue(self.det.adcclk)
        self.mainWindow.spinBoxADCF.editingFinished.connect(self.setADCFrequency)

    def setADCFrequency(self):
        self.mainWindow.spinBoxADCF.editingFinished.disconnect()
        try:
            self.det.adcclk = self.mainWindow.spinBoxADCF.value()
        except Exception as e:
            QtWidgets.QMessageBox.warning(self.mainWindow, "ADC Frequency Fail", str(e), QtWidgets.QMessageBox.Ok)
            pass
        # TODO: handling double event exceptions
        self.mainWindow.spinBoxADCF.editingFinished.connect(self.setADCFrequency)
        self.getADCFrequency()

    def getADCPhase(self):
        self.mainWindow.spinBoxADCPhase.editingFinished.disconnect()
        self.mainWindow.spinBoxADCPhase.setValue(self.det.adcphase)
        self.mainWindow.spinBoxADCPhase.editingFinished.connect(self.setADCPhase)

    def setADCPhase(self):
        self.mainWindow.spinBoxADCPhase.editingFinished.disconnect()
        try:
            self.det.adcphase = self.mainWindow.spinBoxADCPhase.value()
        except Exception as e:
            QtWidgets.QMessageBox.warning(self.mainWindow, "ADC Phase Fail", str(e), QtWidgets.QMessageBox.Ok)
            pass
        # TODO: handling double event exceptions
        self.mainWindow.spinBoxADCPhase.editingFinished.connect(self.setADCPhase)
        self.getADCPhase()

    def getADCPipeline(self):
        self.mainWindow.spinBoxADCPipeline.editingFinished.disconnect()
        self.mainWindow.spinBoxADCPipeline.setValue(self.det.adcpipeline)
        self.mainWindow.spinBoxADCPipeline.editingFinished.connect(self.setADCPipeline)

    def setADCPipeline(self):
        self.mainWindow.spinBoxADCPipeline.editingFinished.disconnect()
        try:
            self.det.adcpipeline = self.mainWindow.spinBoxADCPipeline.value()
        except Exception as e:
            QtWidgets.QMessageBox.warning(self.mainWindow, "ADC Pipeline Fail", str(e), QtWidgets.QMessageBox.Ok)
            pass
        # TODO: handling double event exceptions
        self.mainWindow.spinBoxADCPipeline.editingFinished.connect(self.setADCPipeline)
        self.getADCPipeline()

    def getDBITFrequency(self):
        self.mainWindow.spinBoxDBITF.editingFinished.disconnect()
        self.mainWindow.spinBoxDBITF.setValue(self.det.dbitclk)
        self.mainWindow.spinBoxDBITF.editingFinished.connect(self.setDBITFrequency)

    def setDBITFrequency(self):
        self.mainWindow.spinBoxDBITF.editingFinished.disconnect()
        try:
            self.det.dbitclk = self.mainWindow.spinBoxDBITF.value()
        except Exception as e:
            QtWidgets.QMessageBox.warning(self.mainWindow, "DBit Frequency Fail", str(e), QtWidgets.QMessageBox.Ok)
            pass
        # TODO: handling double event exceptions
        self.mainWindow.spinBoxDBITF.editingFinished.connect(self.setDBITFrequency)
        self.getDBITFrequency()

    def getDBITPhase(self):
        self.mainWindow.spinBoxDBITPhase.editingFinished.disconnect()
        self.mainWindow.spinBoxDBITPhase.setValue(self.det.dbitphase)
        self.mainWindow.spinBoxDBITPhase.editingFinished.connect(self.setDBITPhase)

    def setDBITPhase(self):
        self.mainWindow.spinBoxDBITPhase.editingFinished.disconnect()
        try:
            self.det.dbitphase = self.mainWindow.spinBoxDBITPhase.value()
        except Exception as e:
            QtWidgets.QMessageBox.warning(self.mainWindow, "DBit Phase Fail", str(e), QtWidgets.QMessageBox.Ok)
            pass
        # TODO: handling double event exceptions
        self.mainWindow.spinBoxDBITPhase.editingFinished.connect(self.setDBITPhase)
        self.getDBITPhase()

    def getDBITPipeline(self):
        self.mainWindow.spinBoxDBITPipeline.editingFinished.disconnect()
        self.mainWindow.spinBoxDBITPipeline.setValue(self.det.dbitpipeline)
        self.mainWindow.spinBoxDBITPipeline.editingFinished.connect(self.setDBITPipeline)

    def setDBITPipeline(self):
        self.mainWindow.spinBoxDBITPipeline.editingFinished.disconnect()
        try:
            self.det.dbitpipeline = self.mainWindow.spinBoxDBITPipeline.value()
        except Exception as e:
            QtWidgets.QMessageBox.warning(self.mainWindow, "DBit Pipeline Fail", str(e), QtWidgets.QMessageBox.Ok)
            pass
        # TODO: handling double event exceptions
        self.mainWindow.spinBoxDBITPipeline.editingFinished.connect(self.setDBITPipeline)
        self.getDBITPipeline()

    def getFileWrite(self):
        self.mainWindow.checkBoxFileWrite.stateChanged.disconnect()
        self.mainWindow.checkBoxFileWrite.setChecked(self.det.fwrite)
        self.mainWindow.checkBoxFileWrite.stateChanged.connect(self.setFileWrite)

    def setFileWrite(self):
        self.det.fwrite = self.mainWindow.checkBoxFileWrite.isChecked()
        self.getFileWrite()

    def getFileName(self):
        self.mainWindow.lineEditFileName.editingFinished.disconnect()
        self.mainWindow.lineEditFileName.setText(self.det.fname)
        self.mainWindow.lineEditFileName.editingFinished.connect(self.setFileName)

    def setFileName(self):
        self.det.fname = self.mainWindow.lineEditFileName.text()
        self.getFileName()

    def getFilePath(self):
        self.mainWindow.lineEditFilePath.editingFinished.disconnect()
        self.mainWindow.lineEditFilePath.setText(str(self.det.fpath))
        self.mainWindow.lineEditFilePath.editingFinished.connect(self.setFilePath)

    def setFilePath(self):
        self.det.fpath = Path(self.mainWindow.lineEditFilePath.text())
        self.getFilePath()

    def browseFilePath(self):
        response = QtWidgets.QFileDialog.getExistingDirectory(
            parent=self.mainWindow,
            caption="Select Path to Save Output File",
            directory=os.getcwd(),
            options=(QtWidgets.QFileDialog.ShowDirsOnly | QtWidgets.QFileDialog.DontResolveSymlinks)
            # filter='README (*.md *.ui)'
        )
        if response:
            self.mainWindow.lineEditFilePath.setText(response)
            self.setFilePath()

    def getAccquisitionIndex(self):
        self.mainWindow.spinBoxAcquisitionIndex.editingFinished.disconnect()
        self.mainWindow.spinBoxAcquisitionIndex.setValue(self.det.findex)
        self.mainWindow.spinBoxAcquisitionIndex.editingFinished.connect(self.setAccquisitionIndex)

    def setAccquisitionIndex(self):
        self.det.findex = self.mainWindow.spinBoxAcquisitionIndex.value()
        self.getAccquisitionIndex()

    def getFrames(self):
        self.mainWindow.spinBoxFrames.editingFinished.disconnect()
        self.mainWindow.spinBoxFrames.setValue(self.det.frames)
        self.mainWindow.spinBoxFrames.editingFinished.connect(self.setFrames)

    def setFrames(self):
        self.det.frames = self.mainWindow.spinBoxFrames.value()
        self.getFrames()

    def getPeriod(self):
        self.mainWindow.spinBoxPeriod.editingFinished.disconnect()
        self.mainWindow.comboBoxPeriod.currentIndexChanged.disconnect()

        # Converting to right time unit for period
        tPeriod = self.det.period
        if tPeriod < 100e-9:
            self.mainWindow.comboBoxPeriod.setCurrentIndex(3)
            self.mainWindow.spinBoxPeriod.setValue(tPeriod / 1e-9)
        elif tPeriod < 100e-6:
            self.mainWindow.comboBoxPeriod.setCurrentIndex(2)
            self.mainWindow.spinBoxPeriod.setValue(tPeriod / 1e-6)
        elif tPeriod < 100e-3:
            self.mainWindow.comboBoxPeriod.setCurrentIndex(1)
            self.mainWindow.spinBoxPeriod.setValue(tPeriod / 1e-3)
        else:
            self.mainWindow.comboBoxPeriod.setCurrentIndex(0)
            self.mainWindow.spinBoxPeriod.setValue(tPeriod)

        self.mainWindow.spinBoxPeriod.editingFinished.connect(self.setPeriod)
        self.mainWindow.comboBoxPeriod.currentIndexChanged.connect(self.setPeriod)

    def setPeriod(self):
        if self.mainWindow.comboBoxPeriod.currentIndex() == 0:
            self.det.period = self.mainWindow.spinBoxPeriod.value()
        elif self.mainWindow.comboBoxPeriod.currentIndex() == 1:
            self.det.period = self.mainWindow.spinBoxPeriod.value() * (1e-3)
        elif self.mainWindow.comboBoxPeriod.currentIndex() == 2:
            self.det.period = self.mainWindow.spinBoxPeriod.value() * (1e-6)
        else:
            self.det.period = self.mainWindow.spinBoxPeriod.value() * (1e-9)

        self.getPeriod()

    def getTriggers(self):
        self.mainWindow.spinBoxTriggers.editingFinished.disconnect()
        self.mainWindow.spinBoxTriggers.setValue(self.det.triggers)
        self.mainWindow.spinBoxTriggers.editingFinished.connect(self.setTriggers)

    def setTriggers(self):
        self.det.triggers = self.mainWindow.spinBoxTriggers.value()
        self.getTriggers()

    def updateDetectorStatus(self, status):
        self.mainWindow.labelDetectorStatus.setText(status.name)

    def updateCurrentMeasurement(self):
        self.mainWindow.labelCurrentMeasurement.setText(str(self.currentMeasurement))
        # print(f"Meausrement {self.currentMeasurement}")

    def updateCurrentFrame(self, val):
        self.mainWindow.labelFrameNumber.setText(str(val))

    def updateAcquiredFrames(self, val):
        self.mainWindow.labelAcquiredFrames.setText(str(val))

    def toggleAcquire(self):
        if self.mainWindow.pushButtonStart.isChecked():
            self.mainWindow.plotService.showPatternViewer(False)
            self.acquire()
        else:
            self.stopAcquisition()

    def toggleStartButton(self, started):
        if started:
            self.mainWindow.pushButtonStart.setChecked(True)
            self.mainWindow.pushButtonStart.setText('Stop')
        else:
            self.mainWindow.pushButtonStart.setChecked(False)
            self.mainWindow.pushButtonStart.setText('Start')

    def stopAcquisition(self):
        self.det.stop()
        self.stoppedFlag = True

    def checkBeforeAcquire(self):
        if self.mainWindow.radioButtonImage.isChecked():
            # matterhorn image
            if self.mainWindow.comboBoxPlot.currentText() == "Matterhorn":
                if self.mainWindow.romode not in [readoutMode.TRANSCEIVER_ONLY, readoutMode.DIGITAL_AND_TRANSCEIVER]:
                    QtWidgets.QMessageBox.warning(self.mainWindow, "Plot type",
                                                  "To read Matterhorn image, please enable transceiver readout mode",
                                                  QtWidgets.QMessageBox.Ok)
                    return False
                if self.mainWindow.transceiverService.getTransceiverEnableReg() != Defines.Matterhorn.tranceiverEnable:
                    QtWidgets.QMessageBox.warning(self.mainWindow, "Plot type",
                                                  "To read Matterhorn image, please set transceiver enable to " + str(
                                                      Defines.Matterhorn.tranceiverEnable), QtWidgets.QMessageBox.Ok)
                    return False
            # moench04 image
            elif self.mainWindow.comboBoxPlot.currentText() == "Moench04":
                if self.mainWindow.romode not in [readoutMode.ANALOG_ONLY, readoutMode.ANALOG_AND_DIGITAL]:
                    QtWidgets.QMessageBox.warning(self.mainWindow, "Plot type",
                                                  "To read Moench 04 image, please enable analog readout mode",
                                                  QtWidgets.QMessageBox.Ok)
                    return False
                if self.mainWindow.nADCEnabled != 32:
                    QtWidgets.QMessageBox.warning(self.mainWindow, "Plot type",
                                                  "To read Moench 04 image, please enable all 32 adcs",
                                                  QtWidgets.QMessageBox.Ok)
                    return False
        return True

    def acquire(self):
        if not self.checkBeforeAcquire():
            self.toggleStartButton(False)
            return

        self.stoppedFlag = False
        self.toggleStartButton(True)
        self.currentMeasurement = 0

        # ensure zmq streaming is enabled
        if self.det.rx_zmqstream == 0:
            self.det.rx_zmqstream = 1

        # some functions that must be updated for local values
        self.getTransceiver()
        self.getAnalog()
        self.getDigital()
        self.getReadout()
        self.mainWindow.signalsService.getDBitOffset()
        self.mainWindow.adcService.getADCEnableReg()
        self.mainWindow.signalsService.updateDigitalBitEnable()
        self.mainWindow.transceiverService.getTransceiverEnableReg()

        self.startMeasurement()

    def startMeasurement(self):
        try:
            self.updateCurrentMeasurement()
            self.updateCurrentFrame(0)
            self.updateAcquiredFrames(0)
            self.mainWindow.progressBar.setValue(0)

            self.det.rx_start()
            self.det.start()
            time.sleep(Defines.Time_Wait_For_Packets_ms)
            self.checkEndofAcquisition()
        except Exception as e:
            QtWidgets.QMessageBox.warning(self.mainWindow, "Acquire Fail", str(e), QtWidgets.QMessageBox.Ok)
            self.checkEndofAcquisition()

    def checkEndofAcquisition(self):
        caught = self.det.rx_framescaught[0]
        self.updateAcquiredFrames(caught)
        status = self.det.getDetectorStatus()[0]
        self.updateDetectorStatus(status)
        measurementDone = False
        # print(f'status:{status}')
        match status:
            case runStatus.RUNNING:
                pass
            case runStatus.WAITING:
                pass
            case runStatus.TRANSMITTING:
                pass
            case _:
                measurementDone = True

        # check for 500ms for no packets
        # needs more time for 1g streaming out done
        if measurementDone:
            time.sleep(Defines.Time_Wait_For_Packets_ms)
            if self.det.rx_framescaught[0] != caught:
                measurementDone = False

        numMeasurments = self.mainWindow.spinBoxMeasurements.value()
        if measurementDone:
            if self.det.rx_status == runStatus.RUNNING:
                self.det.rx_stop()
            if self.mainWindow.checkBoxFileWrite.isChecked():
                self.mainWindow.spinBoxAcquisitionIndex.stepUp()
                self.setAccquisitionIndex()
            # next measurement
            self.currentMeasurement += 1
            if self.currentMeasurement < numMeasurments and not self.stoppedFlag:
                self.startMeasurement()
            else:
                self.mainWindow.statusTimer.stop()
                self.toggleStartButton(False)
        else:
            self.mainWindow.statusTimer.start(Defines.Time_Status_Refresh_ms)

    # For other functios
    # Reading data from zmq and decoding it
    def read_zmq(self):
        # print("in readzmq")
        try:
            msg = self.socket.recv_multipart(flags=zmq.NOBLOCK)
            if len(msg) != 2:
                if len(msg) != 1:
                    print(f'len(msg) = {len(msg)}')
                return
            header, data = msg
            jsonHeader = json.loads(header)
            # print(jsonHeader)
            self.mainWindow.progressBar.setValue(int(jsonHeader['progress']))
            self.updateCurrentFrame(jsonHeader['frameIndex'])
            # print(f"image size:{int(jsonHeader['size'])}")
            # print(f'Data size: {len(data)}')

            # waveform
            if self.mainWindow.radioButtonWaveform.isChecked():
                # analog
                if self.mainWindow.romode.value in [0, 2]:
                    analog_array = np.array(
                        np.frombuffer(data, dtype=np.uint16, count=self.mainWindow.nADCEnabled * self.asamples))
                    for i in range(32):
                        checkBox = getattr(self.mainWindow, f"checkBoxADC{i}Plot")
                        if checkBox.isChecked():
                            waveform = np.zeros(self.asamples)
                            for iSample in range(self.asamples):
                                # all adc for 1 sample together
                                waveform[iSample] = analog_array[iSample * self.mainWindow.nADCEnabled + i]
                            self.mainWindow.analogPlots[i].setData(waveform)

                # digital
                if self.mainWindow.romode.value in [1, 2, 4]:
                    dbitoffset = self.mainWindow.rx_dbitoffset
                    if self.mainWindow.romode.value == 2:
                        dbitoffset += self.mainWindow.nADCEnabled * 2 * self.asamples
                    digital_array = np.array(np.frombuffer(data, offset=dbitoffset, dtype=np.uint8))
                    nbitsPerDBit = self.dsamples
                    if nbitsPerDBit % 8 != 0:
                        nbitsPerDBit += (8 - (self.dsamples % 8))
                    offset = 0
                    irow = 0
                    for i in self.mainWindow.rx_dbitlist:
                        # where numbits * numsamples is not a multiple of 8
                        if offset % 8 != 0:
                            offset += (8 - (offset % 8))

                        checkBox = getattr(self.mainWindow, f"checkBoxBIT{i}Plot")
                        # bits enabled but not plotting
                        if not checkBox.isChecked():
                            offset += nbitsPerDBit
                            continue
                        # to plot
                        if checkBox.isChecked():
                            waveform = np.zeros(self.dsamples)
                            for iSample in range(self.dsamples):
                                # all samples for digital bit together from slsReceiver
                                index = (int)(offset / 8)
                                iBit = offset % 8
                                bit = (digital_array[index] >> iBit) & 1
                                waveform[iSample] = bit
                                offset += 1
                            self.mainWindow.digitalPlots[i].setData(waveform)
                            # TODO: left axis does not show 0 to 1, but keeps increasing
                            if self.mainWindow.radioButtonStripe.isChecked():
                                self.mainWindow.digitalPlots[i].setY(irow * 2)
                                irow += 1
                            else:
                                self.mainWindow.digitalPlots[i].setY(0)

                # transceiver
                if self.mainWindow.romode.value in [3, 4]:
                    transceiverOffset = 0
                    if self.mainWindow.romode.value == 4:
                        nbitsPerDBit = self.dsamples
                        if self.mainWindow.dsamples % 8 != 0:
                            nbitsPerDBit += (8 - (self.dsamples % 8))
                        transceiverOffset += self.mainWindow.nDbitEnabled * (nbitsPerDBit // 8)
                    # print(f'transceiverOffset:{transceiverOffset}')
                    trans_array = np.array(np.frombuffer(data, offset=transceiverOffset, dtype=np.uint16))
                    for i in range(4):
                        checkBox = getattr(self.mainWindow, f"checkBoxTransceiver{i}Plot")
                        if checkBox.isChecked():
                            waveform = np.zeros(self.tsamples * 4)
                            for iSample in range(self.tsamples * 4):
                                waveform[iSample] = trans_array[iSample * self.mainWindow.nTransceiverEnabled + i]
                            self.mainWindow.transceiverPlots[i].setData(waveform)


            # image
            else:

                # analog
                if self.mainWindow.romode.value in [0, 2]:
                    # get zoom state
                    viewBox = self.mainWindow.plotAnalogImage.getView()
                    state = viewBox.getState()

                    # get histogram (colorbar) levels and histogram zoom range
                    levels = self.mainWindow.plotAnalogImage.getHistogramWidget().item.getLevels()
                    histRange = self.mainWindow.plotAnalogImage.getHistogramWidget().item.getHistogramRange()

                    analog_array = np.array(
                        np.frombuffer(data, dtype=np.uint16, count=self.mainWindow.nADCEnabled * self.asamples))

                    try:
                        self.mainWindow.analog_frame = decoder.decode(analog_array, self.mainWindow.pixelMapAnalog)
                        self.mainWindow.plotAnalogImage.setImage(self.mainWindow.analog_frame.T)
                    except Exception as e:
                        self.mainWindow.statusbar.setStyleSheet("color:red")
                        message = f'Warning: Invalid size for Analog Image. Expected {self.mainWindow.nAnalogRows * self.mainWindow.nAnalogCols} size, got {analog_array.size} instead.'
                        self.updateCurrentFrame('Invalid Image')
                        self.mainWindow.statusbar.showMessage(message)
                        print(message)
                        pass

                    # keep the zoomed in state (not 1st image)
                    if self.mainWindow.firstAnalogImage:
                        self.mainWindow.firstAnalogImage = False
                    else:
                        viewBox.setState(state)
                        self.mainWindow.plotAnalogImage.getHistogramWidget().item.setLevels(min=levels[0],
                                                                                            max=levels[1])
                        self.mainWindow.plotAnalogImage.getHistogramWidget().item.setHistogramRange(*histRange,
                                                                                                    padding=0)

                # transceiver
                if self.mainWindow.romode.value in [3, 4]:
                    # get zoom state
                    viewBox = self.mainWindow.plotTransceiverImage.getView()
                    state = viewBox.getState()
                    # get histogram (colorbar) levels and histogram zoom range
                    levels = self.mainWindow.plotTransceiverImage.getHistogramWidget().item.getLevels()
                    histRange = self.mainWindow.plotTransceiverImage.getHistogramWidget().item.getHistogramRange()

                    transceiverOffset = 0
                    if self.mainWindow.romode.value == 4:
                        nbitsPerDBit = self.dsamples
                        if self.dsamples % 8 != 0:
                            nbitsPerDBit += (8 - (self.dsamples % 8))
                        transceiverOffset += self.mainWindow.nDbitEnabled * (nbitsPerDBit // 8)
                    # print(f'transceiverOffset:{transceiverOffset}')
                    trans_array = np.array(np.frombuffer(data, offset=transceiverOffset, dtype=np.uint16))

                    try:
                        self.mainWindow.transceiver_frame = decoder.decode(trans_array,
                                                                           self.mainWindow.pixelMapTransceiver)
                        # print(f"type of image:{type(self.mainWindows.transceiver_frame)}")
                        self.mainWindow.plotTransceiverImage.setImage(self.mainWindow.transceiver_frame)
                    except Exception as e:
                        self.mainWindow.statusbar.setStyleSheet("color:red")
                        message = f'Warning: Invalid size for Transceiver Image. Expected {self.mainWindow.nTransceiverRows * self.mainWindow.nTransceiverCols} size, got {trans_array.size} instead.'
                        self.updateCurrentFrame('Invalid Image')
                        self.mainWindow.statusbar.showMessage(message)
                        print(message)
                        pass

                    # keep the zoomed in state (not 1st image)
                    if self.mainWindow.firstTransceiverImage:
                        self.mainWindow.firstTransceiverImage = False
                    else:
                        viewBox.setState(state)
                        self.mainWindow.plotTransceiverImage.getHistogramWidget().item.setLevels(min=levels[0],
                                                                                                 max=levels[1])
                        self.mainWindow.plotTransceiverImage.getHistogramWidget().item.setHistogramRange(*histRange,
                                                                                                         padding=0)

        except zmq.ZMQError as e:
            pass
        except Exception as e:
            print(f'Caught exception: {str(e)}')

    def setup_zmq(self):
        self.det.rx_zmqstream = 1
        self.zmqIp = self.det.rx_zmqip
        self.zmqport = self.det.rx_zmqport
        self.zmq_stream = self.det.rx_zmqstream

        self.context = zmq.Context()
        self.socket = self.context.socket(zmq.SUB)
        self.socket.connect(f"tcp://{self.zmqIp}:{self.zmqport}")
        self.socket.subscribe("")
