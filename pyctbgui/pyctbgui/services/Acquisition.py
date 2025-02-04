import json
import typing
from pathlib import Path
import numpy as np
import time
import zmq
from PyQt5 import QtWidgets, uic
import logging

from slsdet import readoutMode, runStatus, detectorType
from pyctbgui.utils.defines import Defines
from pyctbgui.utils.numpyWriter.npy_writer import NumpyFileManager
from pyctbgui.utils.numpyWriter.npz_writer import NpzFileWriter

if typing.TYPE_CHECKING:
    # only used for type hinting. To avoid circular dependencies these
    # won't be imported in runtime
    from pyctbgui.services import SignalsTab, TransceiverTab, AdcTab, PlotTab


class AcquisitionTab(QtWidgets.QWidget):

    def __init__(self, parent):
        self.__isWaveform = None
        super().__init__(parent)
        self.currentMeasurement = None
        self.dsamples = None
        self.stoppedFlag = None
        self.asamples = None
        self.tsamples = None
        uic.loadUi(Path(__file__).parent.parent / 'ui' / "acquisition.ui", parent)
        self.view = parent
        self.mainWindow = None
        self.det = None
        self.signalsTab: SignalsTab = None
        self.transceiverTab: TransceiverTab = None
        self.adcTab: AdcTab = None
        self.plotTab: PlotTab = None
        self.writeNumpy: bool = False
        self.outputDir: Path = Path('/')
        self.outputFileNamePrefix: str = ''
        self.numpyFileManagers: dict[str, NumpyFileManager] = {}

        self.logger = logging.getLogger('AcquisitionTab')

    def setup_ui(self):
        self.signalsTab = self.mainWindow.signalsTab
        self.transceiverTab = self.mainWindow.transceiverTab
        self.adcTab = self.mainWindow.adcTab
        self.plotTab = self.mainWindow.plotTab
        self.toggleStartButton(False)
        if self.det.type == detectorType.XILINX_CHIPTESTBOARD:
            self.view.labelRunF.setDisabled(True)
            self.view.labelADCF.setDisabled(True)
            self.view.labelADCPhase.setDisabled(True)
            self.view.labelADCPipeline.setDisabled(True)
            self.view.labelDBITF.setDisabled(True)
            self.view.labelDBITPhase.setDisabled(True)
            self.view.labelDBITPipeline.setDisabled(True)
            self.view.spinBoxRunF.setDisabled(True)
            self.view.spinBoxADCF.setDisabled(True)
            self.view.spinBoxADCPhase.setDisabled(True)
            self.view.spinBoxADCPipeline.setDisabled(True)
            self.view.spinBoxDBITF.setDisabled(True)
            self.view.spinBoxDBITPhase.setDisabled(True)
            self.view.spinBoxDBITPipeline.setDisabled(True)

    def connect_ui(self):
        # For Acquistions Tab
        self.view.comboBoxROMode.currentIndexChanged.connect(self.setReadOut)
        self.view.spinBoxTransceiver.editingFinished.connect(self.setTransceiver)
        self.view.spinBoxAnalog.editingFinished.connect(self.setAnalog)
        self.view.spinBoxDigital.editingFinished.connect(self.setDigital)
        
        if self.det.type == detectorType.CHIPTESTBOARD:
            self.view.spinBoxRunF.editingFinished.connect(self.setRunFrequency)
            self.view.spinBoxADCF.editingFinished.connect(self.setADCFrequency)
            self.view.spinBoxADCPhase.editingFinished.connect(self.setADCPhase)
            self.view.spinBoxADCPipeline.editingFinished.connect(self.setADCPipeline)
            self.view.spinBoxDBITF.editingFinished.connect(self.setDBITFrequency)
            self.view.spinBoxDBITPhase.editingFinished.connect(self.setDBITPhase)
            self.view.spinBoxDBITPipeline.editingFinished.connect(self.setDBITPipeline)

        self.view.checkBoxFileWriteRaw.stateChanged.connect(self.setFileWrite)
        self.view.checkBoxFileWriteNumpy.stateChanged.connect(self.setFileWriteNumpy)
        self.view.lineEditFileName.editingFinished.connect(self.setFileName)
        self.view.lineEditFilePath.editingFinished.connect(self.setFilePath)
        self.view.pushButtonFilePath.clicked.connect(self.browseFilePath)
        self.view.spinBoxAcquisitionIndex.editingFinished.connect(self.setAccquisitionIndex)
        self.view.spinBoxFrames.editingFinished.connect(self.setFrames)
        self.view.spinBoxPeriod.editingFinished.connect(self.setPeriod)
        self.view.comboBoxPeriod.currentIndexChanged.connect(self.setPeriod)
        self.view.spinBoxTriggers.editingFinished.connect(self.setTriggers)

    def refresh(self):
        self.getReadout()
        self.getTransceiver()
        self.getAnalog()
        self.getDigital()

        if self.det.type == detectorType.CHIPTESTBOARD:
            self.getRunFrequency()
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
        self.view.comboBoxROMode.currentIndexChanged.disconnect()
        self.view.spinBoxAnalog.editingFinished.disconnect()
        self.view.spinBoxDigital.editingFinished.disconnect()
        self.view.spinBoxTransceiver.editingFinished.disconnect()

        self.mainWindow.romode = self.det.romode
        self.view.comboBoxROMode.setCurrentIndex(self.mainWindow.romode.value)
        match self.mainWindow.romode:
            case readoutMode.ANALOG_ONLY:
                self.view.spinBoxAnalog.setEnabled(True)
                self.view.labelAnalog.setEnabled(True)
                self.view.spinBoxDigital.setDisabled(True)
                self.view.labelDigital.setDisabled(True)
                self.view.labelTransceiver.setDisabled(True)
                self.view.spinBoxTransceiver.setDisabled(True)
            case readoutMode.DIGITAL_ONLY:
                self.view.spinBoxAnalog.setDisabled(True)
                self.view.labelAnalog.setDisabled(True)
                self.view.spinBoxDigital.setEnabled(True)
                self.view.labelDigital.setEnabled(True)
                self.view.labelTransceiver.setDisabled(True)
                self.view.spinBoxTransceiver.setDisabled(True)
            case readoutMode.ANALOG_AND_DIGITAL:
                self.view.spinBoxAnalog.setEnabled(True)
                self.view.labelAnalog.setEnabled(True)
                self.view.spinBoxDigital.setEnabled(True)
                self.view.labelDigital.setEnabled(True)
                self.view.labelTransceiver.setDisabled(True)
                self.view.spinBoxTransceiver.setDisabled(True)
            case readoutMode.TRANSCEIVER_ONLY:
                self.view.spinBoxAnalog.setDisabled(True)
                self.view.labelAnalog.setDisabled(True)
                self.view.spinBoxDigital.setDisabled(True)
                self.view.labelDigital.setDisabled(True)
                self.view.labelTransceiver.setEnabled(True)
                self.view.spinBoxTransceiver.setEnabled(True)
            case _:
                self.view.spinBoxAnalog.setDisabled(True)
                self.view.labelAnalog.setDisabled(True)
                self.view.spinBoxDigital.setEnabled(True)
                self.view.labelDigital.setEnabled(True)
                self.view.labelTransceiver.setEnabled(True)
                self.view.spinBoxTransceiver.setEnabled(True)

        self.view.comboBoxROMode.currentIndexChanged.connect(self.setReadOut)
        self.view.spinBoxAnalog.editingFinished.connect(self.setAnalog)
        self.view.spinBoxDigital.editingFinished.connect(self.setDigital)
        self.view.spinBoxTransceiver.editingFinished.connect(self.setTransceiver)
        self.getAnalog()
        self.getDigital()
        self.plotTab.showPlot()

    def plotReferesh(self):
        self.read_zmq()

    def setReadOut(self):
        self.view.comboBoxROMode.currentIndexChanged.disconnect()
        try:
            if self.view.comboBoxROMode.currentIndex() == 0:
                self.det.romode = readoutMode.ANALOG_ONLY
            elif self.view.comboBoxROMode.currentIndex() == 1:
                self.det.romode = readoutMode.DIGITAL_ONLY
            elif self.view.comboBoxROMode.currentIndex() == 2:
                self.det.romode = readoutMode.ANALOG_AND_DIGITAL
            elif self.view.comboBoxROMode.currentIndex() == 3:
                self.det.romode = readoutMode.TRANSCEIVER_ONLY
            else:
                self.det.romode = readoutMode.DIGITAL_AND_TRANSCEIVER
        except Exception as e:
            QtWidgets.QMessageBox.warning(self.mainWindow, "Readout Mode Fail", str(e), QtWidgets.QMessageBox.Ok)
        # TODO: handling double event exceptions
        self.view.comboBoxROMode.currentIndexChanged.connect(self.setReadOut)
        self.getReadout()

    def getRunFrequency(self):
        self.view.spinBoxRunF.editingFinished.disconnect()
        self.view.spinBoxRunF.setValue(self.det.runclk)
        self.view.spinBoxRunF.editingFinished.connect(self.setRunFrequency)

    def setRunFrequency(self):
        self.view.spinBoxRunF.editingFinished.disconnect()
        try:
            self.det.runclk = self.view.spinBoxRunF.value()
        except Exception as e:
            QtWidgets.QMessageBox.warning(self.mainWindow, "Run Frequency Fail", str(e), QtWidgets.QMessageBox.Ok)
        # TODO: handling double event exceptions
        self.view.spinBoxRunF.editingFinished.connect(self.setRunFrequency)
        self.getRunFrequency()

    def getTransceiver(self):
        self.view.spinBoxTransceiver.editingFinished.disconnect()
        self.tsamples = self.det.tsamples
        self.view.spinBoxTransceiver.setValue(self.tsamples)
        self.view.spinBoxTransceiver.editingFinished.connect(self.setTransceiver)

    def setTransceiver(self):
        self.view.spinBoxTransceiver.editingFinished.disconnect()
        try:
            self.det.tsamples = self.view.spinBoxTransceiver.value()
        except Exception as e:
            QtWidgets.QMessageBox.warning(self.mainWindow, "Transceiver Samples Fail", str(e),
                                          QtWidgets.QMessageBox.Ok)
        # TODO: handling double event exceptions
        self.view.spinBoxTransceiver.editingFinished.connect(self.setTransceiver)
        self.getTransceiver()

    def getAnalog(self):
        self.view.spinBoxAnalog.editingFinished.disconnect()
        self.asamples = self.det.asamples
        self.view.spinBoxAnalog.setValue(self.asamples)
        self.view.spinBoxAnalog.editingFinished.connect(self.setAnalog)

    def setAnalog(self):
        self.view.spinBoxAnalog.editingFinished.disconnect()
        try:
            self.det.asamples = self.view.spinBoxAnalog.value()
        except Exception as e:
            QtWidgets.QMessageBox.warning(self.mainWindow, "Digital Samples Fail", str(e), QtWidgets.QMessageBox.Ok)
        # TODO: handling double event exceptions
        self.view.spinBoxAnalog.editingFinished.connect(self.setAnalog)
        self.getAnalog()

    def getDigital(self):
        self.view.spinBoxDigital.editingFinished.disconnect()
        self.dsamples = self.det.dsamples
        self.view.spinBoxDigital.setValue(self.dsamples)
        self.view.spinBoxDigital.editingFinished.connect(self.setDigital)

    def setDigital(self):
        self.view.spinBoxDigital.editingFinished.disconnect()
        try:
            self.det.dsamples = self.view.spinBoxDigital.value()
        except Exception as e:
            QtWidgets.QMessageBox.warning(self.mainWindow, "Digital Samples Fail", str(e), QtWidgets.QMessageBox.Ok)
        # TODO: handling double event exceptions
        self.view.spinBoxDigital.editingFinished.connect(self.setDigital)
        self.getDigital()

    def getADCFrequency(self):
        self.view.spinBoxADCF.editingFinished.disconnect()
        self.view.spinBoxADCF.setValue(self.det.adcclk)
        self.view.spinBoxADCF.editingFinished.connect(self.setADCFrequency)

    def setADCFrequency(self):
        self.view.spinBoxADCF.editingFinished.disconnect()
        try:
            self.det.adcclk = self.view.spinBoxADCF.value()
        except Exception as e:
            QtWidgets.QMessageBox.warning(self.mainWindow, "ADC Frequency Fail", str(e), QtWidgets.QMessageBox.Ok)
        # TODO: handling double event exceptions
        self.view.spinBoxADCF.editingFinished.connect(self.setADCFrequency)
        self.getADCFrequency()

    def getADCPhase(self):
        self.view.spinBoxADCPhase.editingFinished.disconnect()
        self.view.spinBoxADCPhase.setValue(self.det.adcphase)
        self.view.spinBoxADCPhase.editingFinished.connect(self.setADCPhase)

    def setADCPhase(self):
        self.view.spinBoxADCPhase.editingFinished.disconnect()
        try:
            self.det.adcphase = self.view.spinBoxADCPhase.value()
        except Exception as e:
            QtWidgets.QMessageBox.warning(self.mainWindow, "ADC Phase Fail", str(e), QtWidgets.QMessageBox.Ok)
        # TODO: handling double event exceptions
        self.view.spinBoxADCPhase.editingFinished.connect(self.setADCPhase)
        self.getADCPhase()

    def getADCPipeline(self):
        self.view.spinBoxADCPipeline.editingFinished.disconnect()
        self.view.spinBoxADCPipeline.setValue(self.det.adcpipeline)
        self.view.spinBoxADCPipeline.editingFinished.connect(self.setADCPipeline)

    def setADCPipeline(self):
        self.view.spinBoxADCPipeline.editingFinished.disconnect()
        try:
            self.det.adcpipeline = self.view.spinBoxADCPipeline.value()
        except Exception as e:
            QtWidgets.QMessageBox.warning(self.mainWindow, "ADC Pipeline Fail", str(e), QtWidgets.QMessageBox.Ok)
        # TODO: handling double event exceptions
        self.view.spinBoxADCPipeline.editingFinished.connect(self.setADCPipeline)
        self.getADCPipeline()

    def getDBITFrequency(self):
        self.view.spinBoxDBITF.editingFinished.disconnect()
        self.view.spinBoxDBITF.setValue(self.det.dbitclk)
        self.view.spinBoxDBITF.editingFinished.connect(self.setDBITFrequency)

    def setDBITFrequency(self):
        self.view.spinBoxDBITF.editingFinished.disconnect()
        try:
            self.det.dbitclk = self.view.spinBoxDBITF.value()
        except Exception as e:
            QtWidgets.QMessageBox.warning(self.mainWindow, "DBit Frequency Fail", str(e), QtWidgets.QMessageBox.Ok)
        # TODO: handling double event exceptions
        self.view.spinBoxDBITF.editingFinished.connect(self.setDBITFrequency)
        self.getDBITFrequency()

    def getDBITPhase(self):
        self.view.spinBoxDBITPhase.editingFinished.disconnect()
        self.view.spinBoxDBITPhase.setValue(self.det.dbitphase)
        self.view.spinBoxDBITPhase.editingFinished.connect(self.setDBITPhase)

    def setDBITPhase(self):
        self.view.spinBoxDBITPhase.editingFinished.disconnect()
        try:
            self.det.dbitphase = self.view.spinBoxDBITPhase.value()
        except Exception as e:
            QtWidgets.QMessageBox.warning(self.mainWindow, "DBit Phase Fail", str(e), QtWidgets.QMessageBox.Ok)
        # TODO: handling double event exceptions
        self.view.spinBoxDBITPhase.editingFinished.connect(self.setDBITPhase)
        self.getDBITPhase()

    def getDBITPipeline(self):
        self.view.spinBoxDBITPipeline.editingFinished.disconnect()
        self.view.spinBoxDBITPipeline.setValue(self.det.dbitpipeline)
        self.view.spinBoxDBITPipeline.editingFinished.connect(self.setDBITPipeline)

    def setDBITPipeline(self):
        self.view.spinBoxDBITPipeline.editingFinished.disconnect()
        try:
            self.det.dbitpipeline = self.view.spinBoxDBITPipeline.value()
        except Exception as e:
            QtWidgets.QMessageBox.warning(self.mainWindow, "DBit Pipeline Fail", str(e), QtWidgets.QMessageBox.Ok)
        # TODO: handling double event exceptions
        self.view.spinBoxDBITPipeline.editingFinished.connect(self.setDBITPipeline)
        self.getDBITPipeline()

    def getFileWrite(self):
        self.view.checkBoxFileWriteRaw.stateChanged.disconnect()
        self.view.checkBoxFileWriteRaw.setChecked(self.det.fwrite)
        self.view.checkBoxFileWriteRaw.stateChanged.connect(self.setFileWrite)

    def setFileWrite(self):
        self.det.fwrite = self.view.checkBoxFileWriteRaw.isChecked()
        self.getFileWrite()

    def setFileWriteNumpy(self):
        """
        slot for saving the data in numpy (.npy) format
        """
        self.writeNumpy = not self.writeNumpy

    def getFileName(self):
        """
        set the lineEditFilePath input widget to the filename value from the detector
        """

        self.view.lineEditFileName.editingFinished.disconnect()
        fileName = self.det.fname
        self.view.lineEditFileName.setText(fileName)
        self.outputFileNamePrefix = fileName
        self.view.lineEditFileName.editingFinished.connect(self.setFileName)

    def setFileName(self):
        """
        slot for setting the filename from the widget to the detector
        """
        self.det.fname = self.view.lineEditFileName.text()
        self.getFileName()

    def getFilePath(self):
        """
        set the lineEditFilePath input widget to the path value from the detector
        """
        self.view.lineEditFilePath.editingFinished.disconnect()
        path = self.det.fpath
        self.view.lineEditFilePath.setText(str(path))
        self.view.lineEditFilePath.editingFinished.connect(self.setFilePath)
        self.outputDir = path

    def setFilePath(self):
        """
        slot to set the directory of the output for the detector
        """
        self.det.fpath = Path(self.view.lineEditFilePath.text())
        self.getFilePath()

    def saveNumpyFile(self, data: np.ndarray | dict[str, np.ndarray], jsonHeader):
        """
        save the acquisition data (waveform or image) in the specified path
        save waveform in multiple .npy files
        save image as npy file format
        @note: frame number can be up to 100,000 so the data arrays cannot be fully loaded to memory
        """
        if not self.writeNumpy:
            return
        if self.outputDir == Path('/'):
            self.outputDir = Path('./')
        if self.outputFileNamePrefix == '':
            self.outputFileNamePrefix = 'run'

        for device in data:
            if device not in self.numpyFileManagers:
                tmpPath = self.outputDir / f'{self.outputFileNamePrefix}_{device}_{jsonHeader["fileIndex"]}.npy'
                self.numpyFileManagers[device] = NumpyFileManager(tmpPath, 'w', data[device].shape, data[device].dtype)
            self.numpyFileManagers[device].writeOneFrame(data[device])

        if 'progress' in jsonHeader and jsonHeader['progress'] >= 100:
            # close opened files after saving the last frame
            self.closeOpenedNumpyFiles(jsonHeader)

    def closeOpenedNumpyFiles(self, jsonHeader):
        """
        create npz file for waveform plots and close opened numpy files to persist their data
        """
        if not self.writeNumpy:
            return
        if len(self.numpyFileManagers) == 0:
            return
        oneFile: bool = len(self.numpyFileManagers) == 1

        filepaths = [npw.file.name for device, npw in self.numpyFileManagers.items()]
        filenames = list(self.numpyFileManagers.keys())
        ext = 'npy' if oneFile else 'npz'
        newPath = self.outputDir / f'{self.outputFileNamePrefix}_{jsonHeader["fileIndex"]}.{ext}'

        if not oneFile:
            # if there is multiple .npy files group them in an .npz file
            self.numpyFileManagers.clear()
            NpzFileWriter.zipNpyFiles(newPath, filepaths, filenames, deleteOriginals=True, compressed=False)
        else:
            # rename files from "run_ADC0_0.npy" to "run_0.npy" if it is a single .npy file
            oldPath = self.outputDir / f'{self.outputFileNamePrefix}_' \
                                       f'{self.numpyFileManagers.popitem()[0]}_{jsonHeader["fileIndex"]}.{ext}'
            Path.rename(oldPath, newPath)

        self.logger.info(f'Saving numpy data in {newPath} Finished')

    def browseFilePath(self):
        response = QtWidgets.QFileDialog.getExistingDirectory(parent=self.mainWindow,
                                                              caption="Select Path to Save Output File",
                                                              directory=str(Path.cwd()),
                                                              options=(QtWidgets.QFileDialog.ShowDirsOnly
                                                                       | QtWidgets.QFileDialog.DontResolveSymlinks)
                                                              # filter='README (*.md *.ui)'
                                                              )
        if response:
            self.view.lineEditFilePath.setText(response)
            self.setFilePath()

    def getAccquisitionIndex(self):
        self.view.spinBoxAcquisitionIndex.editingFinished.disconnect()
        self.view.spinBoxAcquisitionIndex.setValue(self.det.findex)
        self.view.spinBoxAcquisitionIndex.editingFinished.connect(self.setAccquisitionIndex)

    def setAccquisitionIndex(self):
        self.det.findex = self.view.spinBoxAcquisitionIndex.value()
        self.getAccquisitionIndex()

    def getFrames(self):
        self.view.spinBoxFrames.editingFinished.disconnect()
        self.view.spinBoxFrames.setValue(self.det.frames)
        self.view.spinBoxFrames.editingFinished.connect(self.setFrames)

    def setFrames(self):
        self.det.frames = self.view.spinBoxFrames.value()
        self.getFrames()

    def getPeriod(self):
        self.view.spinBoxPeriod.editingFinished.disconnect()
        self.view.comboBoxPeriod.currentIndexChanged.disconnect()

        # Converting to right time unit for period
        tPeriod = self.det.period
        if tPeriod < 100e-9:
            self.view.comboBoxPeriod.setCurrentIndex(3)
            self.view.spinBoxPeriod.setValue(tPeriod / 1e-9)
        elif tPeriod < 100e-6:
            self.view.comboBoxPeriod.setCurrentIndex(2)
            self.view.spinBoxPeriod.setValue(tPeriod / 1e-6)
        elif tPeriod < 100e-3:
            self.view.comboBoxPeriod.setCurrentIndex(1)
            self.view.spinBoxPeriod.setValue(tPeriod / 1e-3)
        else:
            self.view.comboBoxPeriod.setCurrentIndex(0)
            self.view.spinBoxPeriod.setValue(tPeriod)

        self.view.spinBoxPeriod.editingFinished.connect(self.setPeriod)
        self.view.comboBoxPeriod.currentIndexChanged.connect(self.setPeriod)

    def setPeriod(self):
        if self.view.comboBoxPeriod.currentIndex() == 0:
            self.det.period = self.view.spinBoxPeriod.value()
        elif self.view.comboBoxPeriod.currentIndex() == 1:
            self.det.period = self.view.spinBoxPeriod.value() * (1e-3)
        elif self.view.comboBoxPeriod.currentIndex() == 2:
            self.det.period = self.view.spinBoxPeriod.value() * (1e-6)
        else:
            self.det.period = self.view.spinBoxPeriod.value() * (1e-9)

        self.getPeriod()

    def getTriggers(self):
        self.view.spinBoxTriggers.editingFinished.disconnect()
        self.view.spinBoxTriggers.setValue(self.det.triggers)
        self.view.spinBoxTriggers.editingFinished.connect(self.setTriggers)

    def setTriggers(self):
        self.det.triggers = self.view.spinBoxTriggers.value()
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
            self.plotTab.showPatternViewer(False)
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
        if self.plotTab.view.radioButtonImage.isChecked():
            # matterhorn image
            if self.plotTab.view.comboBoxPlot.currentText() == "Matterhorn":
                if self.mainWindow.romode not in [readoutMode.TRANSCEIVER_ONLY, readoutMode.DIGITAL_AND_TRANSCEIVER]:
                    QtWidgets.QMessageBox.warning(self.mainWindow, "Plot type",
                                                  "To read Matterhorn image, please enable transceiver readout mode",
                                                  QtWidgets.QMessageBox.Ok)
                    return False
                if self.transceiverTab.getTransceiverEnableReg() != Defines.Matterhorn.tranceiverEnable:
                    QtWidgets.QMessageBox.warning(
                        self.mainWindow, "Plot type", "To read Matterhorn image, please set transceiver enable to " +
                        str(Defines.Matterhorn.tranceiverEnable), QtWidgets.QMessageBox.Ok)
                    return False
            # moench04 image
            elif self.plotTab.view.comboBoxPlot.currentText() == "Moench04":
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
        self.signalsTab.getDBitOffset()
        self.adcTab.getADCEnableReg()
        self.signalsTab.updateDigitalBitEnable()
        self.transceiverTab.getTransceiverEnableReg()

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

        numMeasurments = self.view.spinBoxMeasurements.value()
        if measurementDone:

            if self.det.rx_status == runStatus.RUNNING:
                self.det.rx_stop()
            if self.view.checkBoxFileWriteRaw.isChecked() or self.view.checkBoxFileWriteNumpy.isChecked():
                self.view.spinBoxAcquisitionIndex.stepUp()
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
            self.mainWindow.progressBar.setValue(int(jsonHeader['progress']))
            self.updateCurrentFrame(jsonHeader['frameIndex'])

            # waveform
            waveforms = {}
            if self.plotTab.view.radioButtonWaveform.isChecked():
                # analog
                if self.mainWindow.romode.value in [0, 2]:
                    waveforms |= self.adcTab.processWaveformData(data, self.asamples)
                # digital
                if self.mainWindow.romode.value in [1, 2, 4]:
                    waveforms |= self.signalsTab.processWaveformData(data, self.asamples, self.dsamples)
                # transceiver
                if self.mainWindow.romode.value in [3, 4]:
                    waveforms |= self.transceiverTab.processWaveformData(data, self.dsamples)
            # image
            else:
                # analog
                if self.mainWindow.romode.value in [0, 2]:
                    waveforms['analog_image'] = self.adcTab.processImageData(data, self.asamples)
                # transceiver
                if self.mainWindow.romode.value in [3, 4]:
                    waveforms['tx_image'] = self.transceiverTab.processImageData(data, self.dsamples)

            self.saveNumpyFile(waveforms, jsonHeader)
        except zmq.ZMQError:
            pass
        except Exception:
            self.logger.exception("Exception caught")

    def setup_zmq(self):
        self.det.rx_zmqstream = 1
        self.zmqIp = self.det.zmqip
        self.zmqport = self.det.zmqport
        self.zmq_stream = self.det.rx_zmqstream

        self.context = zmq.Context()
        self.socket = self.context.socket(zmq.SUB)
        self.socket.connect(f"tcp://{self.zmqIp}:{self.zmqport}")
        self.socket.subscribe("")

    def saveParameters(self) -> list[str]:
        if self.det.type == detectorType.CHIPTESTBOARD:
            return [
                f'romode {self.view.comboBoxROMode.currentText().lower()}',
                f'runclk {self.view.spinBoxRunF.value()}',
                f'adcclk {self.view.spinBoxADCF.value()}',
                f'adcphase {self.view.spinBoxADCPhase.value()}',
                f'adcpipeline {self.view.spinBoxADCPipeline.value()}',
                f'dbitclk {self.view.spinBoxDBITF.value()}',
                f'dbitphase {self.view.spinBoxDBITPhase.value()}',
                f'dbitpipeline {self.view.spinBoxDBITPipeline.value()}',
                f'fwrite {int(self.view.checkBoxFileWriteRaw.isChecked())}',
                f'fname {self.view.lineEditFileName.text()}',
                f'fpath {self.view.lineEditFilePath.text()}',
                f'findex {self.view.spinBoxAcquisitionIndex.value()}',
                f'frames {self.view.spinBoxFrames.value()}',
                f'triggers {self.view.spinBoxTriggers.value()}',
                f'period {self.view.spinBoxPeriod.value()} {self.view.comboBoxPeriod.currentText().lower()}',
                f'asamples {self.view.spinBoxAnalog.value()}',
                f'dsamples {self.view.spinBoxDigital.value()}',
                f'tsamples {self.view.spinBoxTransceiver.value()}',
            ]
        else:
            return [
                f'romode {self.view.comboBoxROMode.currentText().lower()}',
                f'fwrite {int(self.view.checkBoxFileWriteRaw.isChecked())}',
                f'fname {self.view.lineEditFileName.text()}',
                f'fpath {self.view.lineEditFilePath.text()}',
                f'findex {self.view.spinBoxAcquisitionIndex.value()}',
                f'frames {self.view.spinBoxFrames.value()}',
                f'triggers {self.view.spinBoxTriggers.value()}',
                f'period {self.view.spinBoxPeriod.value()} {self.view.comboBoxPeriod.currentText().lower()}',
                f'asamples {self.view.spinBoxAnalog.value()}',
                f'dsamples {self.view.spinBoxDigital.value()}',
                f'tsamples {self.view.spinBoxTransceiver.value()}',
            ] 

