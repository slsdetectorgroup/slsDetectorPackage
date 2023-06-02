import time
from PyQt5 import QtWidgets, QtCore, QtGui, uic
import sys, os
import pyqtgraph as pg
from pyqtgraph import PlotWidget
import multiprocessing as mp
from threading import Thread
from PIL import Image as im
#import matplotlib.pyplot as plt
import json
import zmq
import numpy as np
import posixpath
from pathlib import Path

from functools import partial
from slsdet import Detector, dacIndex, readoutMode, runStatus
from bit_utils import set_bit, remove_bit, bit_is_set, manipulate_bit
import random

class MainWindow(QtWidgets.QMainWindow):
    def __init__(self, *args, **kwargs):
        pg.setConfigOption("background", (247, 247, 247))
        pg.setConfigOption("foreground", "k")

        self.det = Detector()
        self.setup_zmq()

        super(MainWindow, self).__init__()
        uic.loadUi("CtbGui.ui", self)
        self.setup_ui()
        self.tabWidget.setCurrentIndex(6)
        self.tabWidget.currentChanged.connect(self.refresh_tab)
        self.connect_ui()
        self.refresh_tab_dac()
        self.refresh_tab_power()
        self.refresh_tab_sense()
        self.refresh_tab_signals()
        self.refresh_tab_adc()
        self.refresh_tab_pattern()
        self.refresh_tab_acquisition()


    # For Action options function
    # TODO Only add the components of action option+ functions
    # Function to show info
    def showInfo(self):
        msg = QtWidgets.QMessageBox()
        msg.setWindowTitle("Info about CTB")
        msg.setText("This Gui is for chip test board.\n Current Phase: Development")
        x = msg.exec_()

    # Function to open file
    def openFile(self):
        response = QtWidgets.QFileDialog.getOpenFileName(
            parent=self,
            caption="Select a file to open",
            directory=os.getcwd(),
            # filter='README (*.md *.ui)'
        )
        if response[0]:
            print(response[0])


    # DACs tab functions
    def getDAC(self, i):
        checkBox = getattr(self, f"checkBoxDAC{i}")
        checkBoxmV = getattr(self, f"checkBoxDAC{i}mV")
        spinBox = getattr(self, f"spinBoxDAC{i}")
        label = getattr(self, f"labelDAC{i}")
        dac = getattr(dacIndex, f"DAC_{i}")

        checkBox.clicked.disconnect()
        checkBoxmV.clicked.disconnect()
        spinBox.editingFinished.disconnect()

        if (self.det.getDAC(dac)[0]) == -100:
            checkBox.setChecked(False)
            spinBox.setDisabled(True)
            checkBoxmV.setDisabled(True)
        else:
            checkBox.setChecked(True)
            spinBox.setEnabled(True)
            checkBoxmV.setEnabled(True)

        checkBox.setText(self.det.getDacName(dac))
        if checkBoxmV.isChecked():
            label.setText(str(self.det.getDAC(dac, True)[0]))
        else:
            label.setText(str(self.det.getDAC(dac)[0]))

        checkBox.clicked.connect(partial(self.setDAC, i))
        checkBoxmV.clicked.connect(partial(self.setDAC, i))
        spinBox.editingFinished.connect(partial(self.setDAC, i))

    def setDAC(self, i):
        checkBoxDac = getattr(self, f"checkBoxDAC{i}")
        checkBoxmV = getattr(self, f"checkBoxDAC{i}mV")
        spinBox = getattr(self, f"spinBoxDAC{i}")
        dac = getattr(dacIndex, f"DAC_{i}")

        value = -100
        if checkBoxDac.isChecked():
            value = spinBox.value()

        self.det.setDAC(dac, value, checkBoxmV.isChecked())
        self.getDAC(i)

    def getADCVpp(self):
        retval = self.det.adcvpp
        self.labelADCVpp.setText(f'Mode: {str(retval)}')

        self.comboBoxADCVpp.activated.disconnect()
        self.comboBoxADCVpp.setCurrentIndex(retval)
        self.comboBoxADCVpp.activated.connect(self.setADCVpp)

    def setADCVpp(self):
        self.det.adcvpp = self.comboBoxADCVpp.currentIndex()
        self.getADCVpp()

    def getHighVoltage(self):
        retval = self.det.highvoltage
        self.labelHighVoltage.setText(str(retval))

        self.spinBoxHighVoltage.editingFinished.disconnect()
        self.checkBoxHighVoltage.clicked.disconnect()

        self.spinBoxHighVoltage.setValue(retval)
        if retval:
            self.checkBoxHighVoltage.setChecked(True)
        if self.checkBoxHighVoltage.isChecked():
            self.spinBoxHighVoltage.setEnabled(True)

        self.spinBoxHighVoltage.editingFinished.connect(self.setHighVoltage)
        self.checkBoxHighVoltage.clicked.connect(self.setHighVoltage)

    def setHighVoltage(self):
        value = 0
        if self.checkBoxHighVoltage.isChecked():
            value = self.spinBoxHighVoltage.value()
        try:
            self.det.highvoltage = value
        except Exception as e:
            QtWidgets.QMessageBox.warning(self, "High Voltage Fail", str(e), QtWidgets.QMessageBox.Ok)
            pass
        self.getHighVoltage()

    # Power Supplies Tab functions
    def getPower(self, i):
        spinBox = getattr(self, f"spinBoxV{i}")
        checkBox = getattr(self, f"checkBoxV{i}")
        power = getattr(dacIndex, f"V_POWER_{i}")
        label = getattr(self, f"labelV{i}")

        spinBox.editingFinished.disconnect()
        checkBox.clicked.disconnect()

        retval = self.det.getVoltage(power)[0]
        spinBox.setValue(retval)
        if retval:
            checkBox.setChecked(True)  
        if checkBox.isChecked():
            spinBox.setEnabled(True)
        else:
            spinBox.setDisabled(True)
        #checkBox.setText(self.det.getPowerName(power))
        label.setText(str(retval))

        spinBox.editingFinished.connect(partial(self.setPower, i))
        checkBox.clicked.connect(partial(self.setPower, i))

    #TODO: handle multiple events when pressing enter (twice)
    def setPower(self, i):
        checkBox = getattr(self, f"checkBoxV{i}")
        spinBox = getattr(self, f"spinBoxV{i}")
        power = getattr(dacIndex, f"V_POWER_{i}")
        spinBox.editingFinished.disconnect()

        value = 0
        if checkBox.isChecked():
            value = spinBox.value()
        try:
            self.det.setVoltage(power, value)
        except Exception as e:
            QtWidgets.QMessageBox.warning(self, "Power Fail", str(e), QtWidgets.QMessageBox.Ok)
            pass

        # TODO: (properly) disconnecting and connecting to handle multiple events (out of focus and pressing enter).
        spinBox.editingFinished.connect(partial(self.setPower, i))
        self.getPower(i)

    def getVChip(self):
        self.labelVCHIP.setText(str(self.det.getVoltage(dacIndex.V_POWER_CHIP)[0]))

    # Sense Tab functions
    def updateSense(self, i):
        slowADC = getattr(dacIndex, f"SLOW_ADC{i}")
        label = getattr(self, f"labelSense{i}_2")
        sense0 = self.det.getSlowADC(slowADC)
        label.setText(str(sense0[0]))

    def updateTemperature(self):
        sense0 = self.det.getTemperature(dacIndex.SLOW_ADC_TEMP)
        self.labelTemp_2.setText(f'{str(sense0[0])} °C')

    # Signals Tab functions
    def getDigitalBitEnable(self, i, dbitList):
        checkBox = getattr(self, f"checkBoxBIT{i}DB")
        checkBox.clicked.disconnect()
        checkBox.setChecked(i in list(dbitList))
        checkBox.clicked.connect(partial(self.setDigitalBitEnable, i))

        # enable plot option only if in dblist (also enables color)
        checkBoxPlot = getattr(self, f"checkBoxBIT{i}Plot")
        checkBoxPlot.setEnabled(checkBox.isChecked())
        pushButton = getattr(self, f"pushButtonBIT{i}")
        pushButton.setEnabled(checkBox.isChecked())

        
    def updateDigitalBitEnable(self):
        retval = self.det.rx_dbitlist       
        for i in range(64):
            self.getDigitalBitEnable(i, retval)

    def setDigitalBitEnable(self, i):
        checkBox = getattr(self, f"checkBoxBIT{i}DB")
        bitList = self.det.rx_dbitlist
        if checkBox.isChecked():
            bitList.append(i)
            self.det.rx_dbitlist = bitList
        else:
            bitList.remove(i)
            self.det.rx_dbitlist = bitList
        
        retval = self.det.rx_dbitlist       
        self.getDigitalBitEnable(i, retval)

    def getIOOutReg(self):
        retval = self.det.patioctrl
        self.lineEditPatIOCtrl.editingFinished.disconnect()
        self.lineEditPatIOCtrl.setText("0x{:016x}".format(retval))
        self.lineEditPatIOCtrl.editingFinished.connect(self.setIOOutReg)
        return retval

    def setIOOutReg(self):
        self.det.patioctrl = int(self.lineEditPatIOCtrl.text(), 16)
        self.updateIOOut()

    def updateCheckBoxIOOut(self, i, out):
        checkBox = getattr(self, f"checkBoxBIT{i}Out")
        checkBox.clicked.disconnect()
        checkBox.setChecked(bit_is_set(out, i))
        checkBox.clicked.connect(partial(self.setIOout, i))

    def updateIOOut(self):
        retval = self.getIOOutReg()
        for i in range(64):
            self.updateCheckBoxIOOut(i, retval)

    def setIOout(self, i):
        checkBox = getattr(self, f"checkBoxBIT{i}Out")
        out = self.det.patioctrl
        mask = manipulate_bit(checkBox.isChecked(), out, i)
        self.det.patioctrl = mask

        retval = self.getIOOutReg()
        self.updateCheckBoxIOOut(i, retval)

    def getDBitOffset(self):
        self.spinBoxDBitOffset.editingFinished.disconnect()
        self.spinBoxDBitOffset.setValue(self.det.rx_dbitoffset)
        self.spinBoxDBitOffset.editingFinished.connect(self.setDbitOffset)

    def setDbitOffset(self):
        self.det.rx_dbitoffset = self.spinBoxDBitOffset.value()

    def setBitPlot(self, i):
        pushButton = getattr(self, f"pushButtonBIT{i}")
        checkBox = getattr(self, f"checkBoxBIT{i}Plot")
        pushButton.clicked.disconnect()
        # enable color pick only if plot enabled
        pushButton.setEnabled(checkBox.isChecked())
        pushButton.clicked.connect(partial(self.selectBitColor, i))
        #TODO: enable plotting for this bit

    def selectBitColor(self, i):
        pushButton = getattr(self, f"pushButtonBIT{i}")
        self.showPalette(pushButton)

    # ADCs Tab functions
    def updateADCNames(self):
        for i, adc_name in enumerate(self.det.getAdcNames()):
            getattr(self, f"labelADC{i}").setText(adc_name)    

    def getADCInvReg(self):
        retval = self.det.adcinvert
        self.lineEditADCInversion.editingFinished.disconnect()
        self.lineEditADCInversion.setText("0x{:08x}".format(retval))
        self.lineEditADCInversion.editingFinished.connect(self.setADCInvReg)
        return retval

    def setADCInvReg(self):
        self.det.adcinvert = int(self.lineEditADCInversion.text(), 16)
        self.updateADCInv()

    def updateCheckBoxADCInv(self, i, inv):
        checkBox = getattr(self, f"checkBoxADC{i}Inv")
        checkBox.clicked.disconnect()
        checkBox.setChecked(bit_is_set(inv, i))
        checkBox.clicked.connect(partial(self.setADCInv, i))

    def updateADCInv(self):
        retval = self.getADCInvReg()
        for i in range(32):
            self.updateCheckBoxADCInv(i, retval)

    def setADCInv(self, i):
        checkBox = getattr(self, f"checkBoxADC{i}Inv")
        out = self.det.adcinvert
        mask = manipulate_bit(checkBox.isChecked(), out, i)
        self.det.adcinvert = mask

        retval = self.getADCInvReg()
        self.updateCheckBoxADCInv(i, retval)

    def getADCEnableReg(self):
        retval = self.det.adcenable
        if self.det.tengiga:
            retval = self.det.adcenable10g   
        self.lineEditADCEnable.editingFinished.disconnect()
        self.lineEditADCEnable.setText("0x{:08x}".format(retval))
        self.lineEditADCEnable.editingFinished.connect(self.setADCEnableReg)
        return retval

    def setADCEnableReg(self):
        mask = int(self.lineEditADCInversion.text(), 16)
        if self.det.tengiga:
            self.det.adcenable10g = mask
        else:
            self.det.adcenable = mask
        self.updateADCEnable()

    def updateCheckBoxADCEnable(self, i, mask):
        # check box if enabled in mask
        checkBox = getattr(self, f"checkBoxADC{i}En")
        checkBox.clicked.disconnect()
        checkBox.setChecked(bit_is_set(mask, i))
        checkBox.clicked.connect(partial(self.setADCEnable, i))

        # enable plot option only if in mask (also enables color)
        checkBoxPlot = getattr(self, f"checkBoxADC{i}Plot")
        checkBoxPlot.setEnabled(checkBox.isChecked())
        pushButton = getattr(self, f"pushButtonADC{i}")
        pushButton.setEnabled(checkBox.isChecked())

    def updateADCEnable(self):
        retval = self.getADCEnableReg()
        for i in range(32):
            self.updateCheckBoxADCEnable(i, retval)

    def setADCEnable(self, i):
        checkBox = getattr(self, f"checkBoxADC{i}En")
        if self.det.tengiga:
            enableMask = manipulate_bit(checkBox.isChecked(), self.det.adcenable10g, i)
            self.det.adcenable10g = enableMask
        else:
            enableMask = manipulate_bit(checkBox.isChecked(), self.det.adcenable, i)
            self.det.adcenable = enableMask

        retval = self.getADCEnableReg()
        self.updateCheckBoxADCEnable(i, retval)

    def setADCEnableRange(self, enable, start_bit_nr, end_bit_nr):
        mask = self.getADCEnableReg()
        retval = 0
        for i in range(start_bit_nr, end_bit_nr + 1):
            retval |= manipulate_bit(enable, mask, i)
        self.lineEditADCEnable.setText("0x{:08x}".format(retval))

    def setADCPlot(self, i):
        pushButton = getattr(self, f"pushButtonADC{i}")
        checkBox = getattr(self, f"checkBoxADC{i}Plot")
        pushButton.clicked.disconnect()
        # enable color pick only if plot enabled
        pushButton.setEnabled(checkBox.isChecked())
        pushButton.clicked.connect(partial(self.selectADCColor, i))
        #TODO: enable plotting for this bit

    def selectADCColor(self, i):
        pushButton = getattr(self, f"pushButtonADC{i}")
        self.showPalette(pushButton)

    def getAnalog(self):
        self.spinBoxAnalog.editingFinished.disconnect()
        self.spinBoxAnalog.setValue(self.det.asamples)
        self.spinBoxAnalog.editingFinished.connect(self.setAnalog)

    def setAnalog(self):
        self.det.asamples = self.spinBoxAnalog.value()
        self.getAnalog()

    def getDigital(self):
        self.spinBoxDigital.editingFinished.disconnect()
        self.spinBoxDigital.setValue(self.det.dsamples)
        self.spinBoxDigital.editingFinished.connect(self.setDigital)

    def setDigital(self):
        self.det.dsamples = self.spinBoxDigital.value()
        self.getDigital()

    def getReadout(self):
        self.comboBoxROMode.activated.disconnect
        self.spinBoxAnalog.editingFinished.disconnect()
        self.spinBoxDigital.editingFinished.disconnect()

        romode = self.det.romode
        self.comboBoxROMode.setCurrentIndex(romode.value)
        match romode:
            case readoutMode.ANALOG_ONLY:
                self.spinBoxAnalog.setEnabled(True)
                self.labelAnalog.setEnabled(True)
                self.spinBoxDigital.setDisabled(True)
                self.labelDigital.setDisabled(True)
            case readoutMode.DIGITAL_ONLY:
                self.spinBoxAnalog.setDisabled(True)
                self.labelAnalog.setDisabled(True)
                self.spinBoxDigital.setEnabled(True)
                self.labelDigital.setEnabled(True)
            case _:
                self.spinBoxAnalog.setEnabled(True)
                self.labelAnalog.setEnabled(True)
                self.spinBoxDigital.setEnabled(True)
                self.labelDigital.setEnabled(True)

        self.comboBoxROMode.activated.connect(self.setReadOut)
        self.spinBoxAnalog.editingFinished.connect(self.setAnalog)
        self.spinBoxDigital.editingFinished.connect(self.setDigital)

    def setReadOut(self):
        if self.comboBoxROMode.currentIndex() == 0:
            self.det.romode = readoutMode.ANALOG_ONLY
        elif self.comboBoxROMode.currentIndex() == 1:
            self.det.romode = readoutMode.DIGITAL_ONLY
        else:
            self.det.romode = readoutMode.ANALOG_AND_DIGITAL
        self.getReadout()

    # Pattern Tab functions
    def getRunFrequency(self):
        self.spinBoxRunF.editingFinished.disconnect()
        self.spinBoxRunF.setValue(self.det.runclk)
        self.spinBoxRunF.editingFinished.connect(self.setRunFrequency)

    def setRunFrequency(self):
        self.det.runclk = self.spinBoxRunF.value()
        self.getRunFrequency()

    def getADCFrequency(self):
        self.spinBoxADCF.editingFinished.disconnect()
        self.spinBoxADCF.setValue(self.det.adcclk)
        self.spinBoxADCF.editingFinished.connect(self.setADCFrequency)

    def setADCFrequency(self):
        self.det.adcclk = self.spinBoxADCF.value()
        self.getADCFrequency()

    def getADCPhase(self):
        self.spinBoxADCPhase.editingFinished.disconnect()
        self.spinBoxADCPhase.setValue(self.det.adcclk)
        self.spinBoxADCPhase.editingFinished.connect(self.setADCPhase)

    def setADCPhase(self):
        self.det.adcphase = self.spinBoxADCPhase.value()
        self.getADCPhase()

    def getADCPipeline(self):
        self.spinBoxADCPipeline.editingFinished.disconnect()
        self.spinBoxADCPipeline.setValue(self.det.adcpipeline)
        self.spinBoxADCPipeline.editingFinished.connect(self.setADCPipeline)

    def setADCPipeline(self):
        self.det.adcpipeline = self.spinBoxADCPipeline.value()
        self.getADCPipeline()

    def getDBITFrequency(self):
        self.spinBoxDBITF.editingFinished.disconnect()
        self.spinBoxDBITF.setValue(self.det.dbitclk)
        self.spinBoxDBITF.editingFinished.connect(self.setDBITFrequency)

    def setDBITFrequency(self):
        self.det.dbitclk = self.spinBoxDBITF.value()
        self.getDBITFrequency()

    def getDBITPhase(self):
        self.spinBoxDBITPhase.editingFinished.disconnect()
        self.spinBoxDBITPhase.setValue(self.det.dbitphase)
        self.spinBoxDBITPhase.editingFinished.connect(self.setDBITPhase)

    def setDBITPhase(self):
        self.det.dbitphase = self.spinBoxDBITPhase.value()
        self.getDBITPhase()

    def getDBITPipeline(self):
        self.spinBoxDBITPipeline.editingFinished.disconnect()
        self.spinBoxDBITPipeline.setValue(self.det.dbitpipeline)
        self.spinBoxDBITPipeline.editingFinished.connect(self.setDBITPipeline)

    def setDBITPipeline(self):
        self.det.dbitpipeline = self.spinBoxDBITPipeline.value()
        self.getDBITPipeline()

    def getPatLimitAddress(self):
        retval = self.det.patlimits
        self.lineEditStartAddress.editingFinished.disconnect()
        self.lineEditStopAddress.editingFinished.disconnect()
        self.lineEditStartAddress.setText("0x{:04x}".format(retval[0]))
        self.lineEditStopAddress.setText("0x{:04x}".format(retval[1]))
        self.lineEditStartAddress.editingFinished.connect(self.setPatLimitAddress)
        self.lineEditStopAddress.editingFinished.connect(self.setPatLimitAddress)

    def setPatLimitAddress(self):
        start = int(self.lineEditStartAddress.text(), 16)
        stop = int(self.lineEditStopAddress.text(), 16)
        self.det.patlimits = [start, stop]
        self.getPatLimitAddress()

    def getPatLoopStartStopAddress(self, level):
        retval = self.det.patloop[level]
        lineEditStart = getattr(self, f"lineEditLoop{level}Start")
        lineEditStop = getattr(self, f"lineEditLoop{level}Stop")
        lineEditStart.editingFinished.disconnect()
        lineEditStop.editingFinished.disconnect()
        lineEditStart.setText("0x{:04x}".format(retval[0]))
        lineEditStop.setText("0x{:04x}".format(retval[1]))
        lineEditStart.editingFinished.connect(partial(self.setPatLoopStartStopAddress, level))
        lineEditStop.editingFinished.connect(partial(self.setPatLoopStartStopAddress, level))

    def setPatLoopStartStopAddress(self, level):
        lineEditStart = getattr(self, f"lineEditLoop{level}Start")
        lineEditStop = getattr(self, f"lineEditLoop{level}Stop")
        start = int(lineEditStart.text(), 16)
        stop = int(lineEditStop.text(), 16)
        self.det.patloop[level] = [start, stop]
        self.getPatLoopStartStopAddress(level)

    def getPatLoopWaitAddress(self, level):
        retval = self.det.patwait[level]
        lineEdit = getattr(self, f"lineEditLoop{level}Wait")
        lineEdit.editingFinished.disconnect()
        lineEdit.setText("0x{:04x}".format(retval))
        lineEdit.editingFinished.connect(partial(self.setPatLoopWaitAddress, level))

    def setPatLoopWaitAddress(self, level):
        lineEdit = getattr(self, f"lineEditLoop{level}Wait")
        addr = int(lineEdit.text(), 16)
        self.det.patwait[level] = addr
        self.getPatLoopWaitAddress(level)

    def getPatLoopRepetition(self, level):
        retval = self.det.patnloop[level]
        spinBox = getattr(self, f"spinBoxLoop{level}Repetition")
        spinBox.editingFinished.disconnect()
        spinBox.setValue(retval)
        spinBox.editingFinished.connect(partial(self.setPatLoopRepetition, level))

    def setPatLoopRepetition(self, level):
        spinBox = getattr(self, f"spinBoxLoop{level}Repetition")
        self.det.patnloop[level] = spinBox.value()
        self.getPatLoopRepetition(level)

    def getPatLoopWaitTime(self, level):
        retval = self.det.patwaittime[level]
        spinBox = getattr(self, f"spinBoxLoop{level}WaitTime")
        spinBox.editingFinished.disconnect()
        spinBox.setValue(retval)
        spinBox.editingFinished.connect(partial(self.setPatLoopWaitTime, level))

    def setPatLoopWaitTime(self, level):
        spinBox = getattr(self, f"spinBoxLoop{level}WaitTime")
        self.det.patwaittime[level] = spinBox.value()
        self.getPatLoopWaitTime(level)

    def setCompiler(self):
        response = QtWidgets.QFileDialog.getOpenFileName(
            parent=self,
            caption="Select a compiler file",
            directory=os.getcwd(),
            # filter='README (*.md *.ui)'
        )
        if response[0]:
            self.lineEditCompiler.setText(response[0])

    def setPatternFile(self):
        if self.checkBoxCompile.isChecked():
            filt='Pattern code(*.py *.c)'
        else:
            filt='Pattern file(*.pyat *.pat)'
        response = QtWidgets.QFileDialog.getOpenFileName(
            parent=self,
            caption="Select a pattern file",
            directory=os.getcwd(),
            filter=filt
        )
        if response[0]:
            self.lineEditPatternFile.setText(response[0])

    def loadPattern(self):
        pattern_file = self.lineEditPatternFile.text()
        if not pattern_file:
            QtWidgets.QMessageBox.warning(self, "Pattern Fail", "No pattern file selected. Please select one.", QtWidgets.QMessageBox.Ok)
            return
        # compile
        if self.checkBoxCompile.isChecked():
            compilerFile = self.lineEditCompiler.text()
            if not compilerFile:
                QtWidgets.QMessageBox.warning(self, "Compile Fail", "No compiler selected. Please select one.", QtWidgets.QMessageBox.Ok)
                return

            # if old compile file exists, backup and remove to ensure old copy not loaded
            oldFile = Path(pattern_file + 'at')
            if oldFile.is_file():
                print("Moving old compiled pattern file to _bck") 
                exit_status = os.system('mv '+ str(oldFile) + ' ' + str(oldFile) + '_bkup')
                if exit_status != 0:
                    retval = QtWidgets.QMessageBox.question(self, "Backup Fail", "Could not make a backup of old compiled code. Proceed anyway to compile and overwrite?", QtWidgets.QMessageBox.Yes, QtWidgets.QMessageBox.No)
                    if retval == QtWidgets.QMessageBox.No:              
                        return

            compileCommand = compilerFile + ' ' + pattern_file
            print(compileCommand)
            print("Compiling pattern code to .pat file")
            exit_status = os.system(compileCommand)
            if exit_status != 0:
                QtWidgets.QMessageBox.warning(self, "Compile Fail", "Could not compile pattern.", QtWidgets.QMessageBox.Ok)
                return
            pattern_file += 'at'
        # load pattern
        self.det.pattern = pattern_file
            
    # Acquistions Tab functions
    def plotOptions(self):
        print("plot options - Not implemented yet")
        # TODO: Implement no plot, waveform, distribution, image and image type

    def setSerialOffset(self):
        print("plot options - Not implemented yet")
        #TODO:

    def setNCounter(self):
        print("plot options - Not implemented yet")
        #TODO:

    def setDynamicRange(self):
        print("plot options - Not implemented yet")
        #TODO:

    def setImageX(self):
        print("plot options - Not implemented yet")
        #TODO:

    def setImageY(self):
        print("plot options - Not implemented yet")
        #TODO:

    def setPedestal(self):
        print("plot options - Not implemented yet")
        #TODO: acquire, subtract, common mode
        
    def resetPedestal(self):
        print("plot options - Not implemented yet")
        #TODO:

    def setRawData(self):
        print("plot options - Not implemented yet")
        #TODO: raw data, min, max
        
    def setPedestalSubtract(self):
        print("plot options - Not implemented yet")
        #TODO: pedestal, min, max

    def setFitADC(self):
        print("plot options - Not implemented yet")
        #TODO:
        
    def setPlotBit(self):
        print("plot options - Not implemented yet")
        #TODO:

    def plotReferesh(self):
        self.read_zmq()
        
    def getFileWrite(self):
        self.checkBoxFileWrite.clicked.disconnect()
        self.checkBoxFileWrite.setChecked(self.det.fwrite)
        self.checkBoxFileWrite.clicked.connect(self.setFileWrite)

    def setFileWrite(self):
        self.det.fwrite = self.checkBoxFileWrite.isChecked()
        self.getFileWrite()

    def getFileName(self):
        self.lineEditFileName.editingFinished.disconnect()
        self.lineEditFileName.setText(self.det.fname)
        self.lineEditFileName.editingFinished.connect(self.setFileName)

    def setFileName(self):
        self.det.fname = self.lineEditFileName.text()
        self.getFileName()

    def getFilePath(self):
        self.lineEditFilePath.editingFinished.disconnect()
        self.lineEditFilePath.setText(self.det.fname)
        self.lineEditFilePath.editingFinished.connect(self.setFilePath)

    def setFilePath(self):
        self.det.fname = self.lineEditFilePath.text()
        self.getFilePath()

    def browseFilePath(self):
        response = QtWidgets.QFileDialog.getSaveFileName(
            parent=self,
            caption="Select Path to Save Output File",
            directory=os.getcwd(),
            # filter='README (*.md *.ui)'
        )
        if response[0]:
            self.lineEditFilePath.setText(response[0])

    def getAccquisitionIndex(self):
        self.spinBoxAcquisitionIndex.editingFinished.disconnect()
        self.spinBoxAcquisitionIndex.setValue(self.det.findex)
        self.spinBoxAcquisitionIndex.editingFinished.connect(self.setAccquisitionIndex)

    def setAccquisitionIndex(self):
        self.det.findex = self.spinBoxAcquisitionIndex.value()
        self.getAccquisitionIndex()

    def getFrames(self):
        self.spinBoxFrames.editingFinished.disconnect()
        self.spinBoxFrames.setValue(self.det.frames)
        self.spinBoxFrames.editingFinished.connect(self.setFrames)

    def setFrames(self):
        self.det.frames = self.spinBoxFrames.value()
        self.getFrames()
    
    def getPeriod(self):
        self.spinBoxPeriod.editingFinished.disconnect()
        self.comboBoxPeriod.activated.disconnect()

        # Converting to right time unit for period
        tPeriod = self.det.period
        if tPeriod < 100e-9:
            self.comboBoxPeriod.setCurrentIndex(3)
            self.spinBoxPeriod.setValue(tPeriod / 1e-9)
        elif tPeriod < 100e-6:
            self.comboBoxPeriod.setCurrentIndex(2)
            self.spinBoxPeriod.setValue(tPeriod / 1e-6)
        elif tPeriod < 100e-3:
            self.comboBoxPeriod.setCurrentIndex(1)
            self.spinBoxPeriod.setValue(tPeriod / 1e-3)
        else:
            self.comboBoxPeriod.setCurrentIndex(0)
            self.spinBoxPeriod.setValue(tPeriod)

        self.spinBoxPeriod.editingFinished.connect(self.setPeriod)
        self.comboBoxPeriod.activated.connect(self.setPeriod)

    def setPeriod(self):
        if self.comboBoxPeriod.currentIndex() == 0:
            self.det.period = self.spinBoxPeriod.value()
        elif self.comboBoxPeriod.currentIndex() == 1:
            self.det.period = self.spinBoxPeriod.value() * (1e-3)
        elif self.comboBoxPeriod.currentIndex() == 2:
            self.det.period = self.spinBoxPeriod.value() * (1e-6)
        else:
            self.det.period = self.spinBoxPeriod.value() * (1e-9)

        self.getPeriod()

    def getTriggers(self):
        self.spinBoxTriggers.editingFinished.disconnect()
        self.spinBoxTriggers.setValue(self.det.triggers)
        self.spinBoxTriggers.editingFinished.connect(self.setTriggers)

    def setTriggers(self):
        self.det.triggers = self.spinBoxTriggers.value()
        self.getTriggers()

    def getDetectorStatus(self):
        self.labelDetectorStatus.setText(self.det.status.name)

    def updateCurrentMeasurement(self):
        self.labelCurrentMeasurement.setText(str(self.currentMeasurement))
    
    def stopAcquisition(self):
        self.det.stop()
        self.stoppedFlag = True
        
    def acquire(self):
        self.pushButtonStart.setEnabled(False)
        self.stoppedFlag = False
        self.currentMeasurement = 0
        self.read_timer.start(20)
        self.statusTimer.start(20)
        self.startMeasurement()

    def startMeasurement(self):
        #print(f"Meausrement {self.currentMeasurement}")
        self.updateCurrentMeasurement()
        self.det.rx_start()
        self.progressBar.setValue(0)
        self.det.start()

    def checkEndofAcquisition(self):
        self.getDetectorStatus()
        measurementDone = False
        match self.det.status:
            case runStatus.RUNNING:
                pass
            case runStatus.WAITING:
                pass
            case runStatus.TRANSMITTING:
                pass
            case _:
                measurementDone = True

        numMeasurments = self.spinBoxMeasurements.value()
        if measurementDone:
            if self.det.rx_status == runStatus.RUNNING:
                self.det.rx_stop()
            if self.checkBoxFileWrite.isChecked():
                self.spinBoxIndex.stepUp()
            # next measurement
            self.currentMeasurement += 1
            if self.currentMeasurement < numMeasurments and not self.stoppedFlag:
                self.startMeasurement()
            else:
                self.read_timer.stop()
                self.statusTimer.stop()
                self.pushButtonStart.setEnabled(True)



    # For other functios
    #Reading data from zmq and decoding it
    def read_zmq(self):
        try:
            msg = self.socket.recv_multipart(flags=zmq.NOBLOCK)
            if len(msg) != 2:
                print(f'len(msg) = {len(msg)}')
                return
            header, data = msg
            jsonHeader = json.loads(header)
            #print(jsonHeader)
            self.progressBar.setValue(jsonHeader['progress'])
            #print(f'Data size: {len(data)}')
            
            data_array = np.array(np.frombuffer(data, dtype=np.uint16))

            '''
            # moench analog
            # return data_array
            adc_numbers = [9, 8, 11, 10, 13, 12, 15, 14, 1, 0, 3, 2, 5, 4, 7, 6, 23, 22, 21, 20, 19, 18, 17, 16, 31, 30, 29, 28,
                            27, 26, 25, 24]

            n_pixels_per_sc = 5000

            sc_width = 25
            analog_frame = np.zeros((400, 400))
            order_sc = np.zeros((400, 400))

            for n_pixel in range(n_pixels_per_sc):
                for i_sc, adc_nr in enumerate(adc_numbers):
                    # ANALOG
                    col = ((adc_nr % 16) * sc_width) + (n_pixel % sc_width)
                    if i_sc < 16:
                        row = 199 - int(n_pixel / sc_width)
                    else:
                        row = 200 + int(n_pixel / sc_width)

                    index_min = n_pixel * 32 + i_sc

                    pixel_value = data_array[index_min]
                    analog_frame[row, col] = pixel_value
                    order_sc[row, col] = i_sc
            '''

            '''
            fig, ax = plt.subplots()
            im = ax.imshow(analog_frame)
            ax.invert_yaxis()
            fig.colorbar(im)
            plt.show()
            '''
            '''
            plot1 = pg.ImageView(self.plotWidget, view=pg.PlotItem())
            plot1.show()
            plot1.setImage(analog_frame)
            '''
            
        except zmq.ZMQError as e:
            pass
        except Exception as e:
            print(f'Caught exception: {str(e)}')

    def getRandomColor(self):
        '''
        Returns a random color range (except white) in format string eg. "#aabbcc"
        '''
        randomColor = random.randrange(0, 0xffffaa, 0xaa)
        return "#{:06x}".format(randomColor)

    def setActiveColor(self, button, str_color):
        button.setStyleSheet(":enabled {background-color: %s" % str_color 
            + "} :disabled {background-color: grey}")

    def showPalette(self, button):
        color = QtWidgets.QColorDialog.getColor()
        if color.isValid():
            self.setActiveColor(button, color.name())
            # get the RGB Values
            #print(color.getRgb())

    def refresh_tab(self, tab_index):
        match tab_index:
            case 0:
                self.refresh_tab_dac()
            case 1:
                self.refresh_tab_power()
            case 2:
                self.refresh_tab_sense()
            case 3:
                self.refresh_tab_signals()
            case 4:
                self.refresh_tab_adc()
            case 5:
                self.refresh_tab_pattern()
            case 6:
                self.refresh_tab_acquisition()


    def refresh_tab_dac(self):
        for i in range(18):
            self.getDAC(i)

        self.getADCVpp()
        self.getHighVoltage()

    def refresh_tab_power(self):
        for i in ('A', 'B', 'C', 'D', 'IO'):
            self.getPower(i)

        # TODO: was not getting vchip earlier, why?
        self.getVChip()

    def refresh_tab_sense(self):
        for i in range(8):
            self.updateSense(i)
        self.updateTemperature()


    def refresh_tab_signals(self):
        self.updateDigitalBitEnable()
        self.updateIOOut()
        self.getDBitOffset()


    def refresh_tab_adc(self):
        self.updateADCNames()
        self.updateADCInv()
        self.updateADCEnable()
        self.getAnalog()
        self.getDigital()
        self.getReadout()

    def refresh_tab_pattern(self):
        self.getRunFrequency()
        self.getADCFrequency()
        self.getADCPhase()
        self.getADCPipeline()
        self.getDBITFrequency()
        self.getDBITPhase()
        self.getDBITPipeline()
        self.getPatLimitAddress()
        for i in range(6):
            self.getPatLoopStartStopAddress(i)
            self.getPatLoopWaitAddress(i)
            self.getPatLoopRepetition(i)
            self.getPatLoopWaitTime(i)

        
    def refresh_tab_acquisition(self):
        self.getFileWrite()
        self.getFileName()
        self.getFilePath()
        self.getAccquisitionIndex()
        self.getFrames()
        self.getTriggers()
        self.getPeriod()
        self.getDetectorStatus()
        

    def setup_zmq(self):
        self.det.rx_zmqstream = 1
        self.zmqIp = self.det.rx_zmqip
        self.zmqport = self.det.rx_zmqport
        self.zmq_stream = self.det.rx_zmqstream

        self.context = zmq.Context()
        self.socket = self.context.socket(zmq.SUB)
        self.socket.connect(f"tcp://{self.zmqIp}:{self.zmqport}")
        self.socket.subscribe("")


    def setup_ui(self):
        #To check detector status
        self.statusTimer = QtCore.QTimer()
        self.statusTimer.timeout.connect(self.checkEndofAcquisition)

        #To auto trigger the read
        self.read_timer =  QtCore.QTimer()
        self.read_timer.timeout.connect(self.read_zmq)

        # Dac Tab
        # Getting dac values for spinboxes (only for modifying)
        for i in range(18):
            dac = getattr(dacIndex, f"DAC_{i}")
            getattr(self, f"spinBoxDAC{i}").setValue(self.det.getDAC(dac)[0])

        if self.det.highvoltage == 0:
            self.spinBoxHighVoltage.setDisabled(True)
            self.checkBoxHighVoltage.setChecked(False)

        # Power Tab
        # Getting power values for spinboxes (only for modifying)
        for i in ('A', 'B', 'C', 'D', 'IO'):
            dac = getattr(dacIndex, f"V_POWER_{i}")
            spinBox = getattr(self, f"spinBoxV{i}")
            checkBox = getattr(self, f"checkBoxV{i}")
            retval = self.det.getVoltage(dac)[0]
            spinBox.setValue(retval)
            if retval == 0:
                checkBox.setChecked(False)
                spinBox.setDisabled(True)

        # Signals Tab
        for i in range(64):
            pushButton = getattr(self, f"pushButtonBIT{i}")
            checkBox = getattr(self, f"checkBoxBIT{i}Plot")
            checkBox.setDisabled(True)
            pushButton.setDisabled(True)
            self.setActiveColor(pushButton, self.getRandomColor())

        # Adc Tab
        for i in range(32):
            pushButton = getattr(self, f"pushButtonADC{i}")
            checkBox = getattr(self, f"checkBoxADC{i}Plot")
            checkBox.setDisabled(True)
            pushButton.setDisabled(True)
            self.setActiveColor(pushButton, self.getRandomColor())

    def connect_ui(self):
               # Plotting the data
        # For the action options in app
        # TODO Only add the components of action options
        # Show info
        self.actionInfo.triggered.connect(self.showInfo)
        self.actionOpen.triggered.connect(self.openFile)

        # For DACs tab
        n_dacs = len(self.det.daclist)
        for i in range(n_dacs):
            getattr(self, f"spinBoxDAC{i}").editingFinished.connect(
                partial(self.setDAC, i)
            )
            getattr(self, f"checkBoxDAC{i}").clicked.connect(partial(self.setDAC, i))
            getattr(self, f"checkBoxDAC{i}mV").clicked.connect(partial(self.setDAC, i))

        self.comboBoxADCVpp.activated.connect(self.setADCVpp)
        self.spinBoxHighVoltage.editingFinished.connect(self.setHighVoltage)
        self.checkBoxHighVoltage.clicked.connect(self.setHighVoltage)

        # For Power Supplies tab
        for i in ('A', 'B', 'C', 'D', 'IO'):
            spinBox = getattr(self, f"spinBoxV{i}")
            checkBox = getattr(self, f"checkBoxV{i}")
            spinBox.editingFinished.connect(partial(self.setPower, i))
            checkBox.clicked.connect(partial(self.setPower, i))

        # For Sense Tab
        for i in range(8):
            getattr(self, f"pushButtonSense{i}").clicked.connect(partial(self.updateSense, i))
        self.pushButtonTemp.clicked.connect(self.updateTemperature)

        # For Signals Tab
        for i in range(64):
            getattr(self, f"checkBoxBIT{i}DB").clicked.connect(partial(self.setDigitalBitEnable, i))
            getattr(self, f"checkBoxBIT{i}Out").clicked.connect(partial(self.setIOout, i))
            getattr(self, f"checkBoxBIT{i}Plot").clicked.connect(partial(self.setBitPlot, i))
            getattr(self, f"pushButtonBIT{i}").clicked.connect(partial(self.selectBitColor, i))
        self.lineEditPatIOCtrl.editingFinished.connect(self.setIOOutReg)
        self.spinBoxDBitOffset.editingFinished.connect(self.setDbitOffset)

        # For ADCs Tab
        for i in range(32):
            getattr(self, f"checkBoxADC{i}Inv").clicked.connect(partial(self.setADCEnable, i))
            getattr(self, f"checkBoxADC{i}En").clicked.connect(partial(self.setADCInv, i))
            getattr(self, f"checkBoxADC{i}Plot").clicked.connect(partial(self.setADCPlot, i))
            getattr(self, f"pushButtonADC{i}").clicked.connect(partial(self.selectADCColor, i))
        self.lineEditADCInversion.editingFinished.connect(self.setADCInvReg)
        self.lineEditADCEnable.editingFinished.connect(self.setADCEnableReg)
        self.pushButtonAll15.clicked.connect(partial(self.setADCEnableRange, 1, 0, 15))
        self.pushButtonNone15.clicked.connect(partial(self.setADCEnableRange, 0, 0, 15))
        self.pushButtonAll16.clicked.connect(partial(self.setADCEnableRange, 1, 16, 31))
        self.pushButtonNone16.clicked.connect(partial(self.setADCEnableRange, 0, 16, 31))
        self.pushButtonAll.clicked.connect(partial(self.setADCEnableRange, 1, 0, 31))
        # Cannot set adcmask to 0 anyway
        #self.pushButtonNone.clicked.connect(partial(self.setADCEnableRange, 0, 0, 31))
        self.spinBoxAnalog.editingFinished.connect(self.setAnalog)
        self.spinBoxDigital.editingFinished.connect(self.setDigital)
        self.comboBoxROMode.activated.connect(self.setReadOut)

        # For Pattern Tab
        self.spinBoxRunF.editingFinished.connect(self.setRunFrequency)
        self.spinBoxADCF.editingFinished.connect(self.setADCFrequency)
        self.spinBoxADCPhase.editingFinished.connect(self.setADCPhase)
        self.spinBoxADCPipeline.editingFinished.connect(self.setADCPipeline)
        self.spinBoxDBITF.editingFinished.connect(self.setDBITFrequency)
        self.spinBoxDBITPhase.editingFinished.connect(self.setDBITPhase)
        self.spinBoxDBITPipeline.editingFinished.connect(self.setDBITPipeline)
        self.lineEditStartAddress.editingFinished.connect(self.setPatLimitAddress)
        self.lineEditStopAddress.editingFinished.connect(self.setPatLimitAddress)
        for i in range(6):
            getattr(self, f"lineEditLoop{i}Start").editingFinished.connect(
                partial(self.setPatLoopStartStopAddress, i)
            )
            getattr(self, f"lineEditLoop{i}Stop").editingFinished.connect(
                partial(self.setPatLoopStartStopAddress, i)
            )
            getattr(self, f"lineEditLoop{i}Wait").editingFinished.connect(
                partial(self.setPatLoopWaitAddress, i)
            )
            getattr(self, f"spinBoxLoop{i}Repetition").editingFinished.connect(
                partial(self.setPatLoopRepetition, i)
            )
            getattr(self, f"spinBoxLoop{i}WaitTime").editingFinished.connect(
                partial(self.setPatLoopWaitTime, i)
            )
        self.pushButtonCompiler.clicked.connect(self.setCompiler)
        self.pushButtonPatternFile.clicked.connect(self.setPatternFile)
        self.pushButtonLoad.clicked.connect(self.loadPattern)


        # For Acquistions Tab
        self.radioButtonNoPlot.clicked.connect(self.plotOptions)
        self.radioButtonWaveform.clicked.connect(self.plotOptions)
        self.radioButtonDistribution.clicked.connect(self.plotOptions)
        self.radioButtonImage.clicked.connect(self.plotOptions)
        self.comboBoxPlot.activated.connect(self.plotOptions)
        self.spinBoxSerialOffset.editingFinished.connect(self.setSerialOffset)
        self.spinBoxNCount.editingFinished.connect(self.setNCounter)
        self.spinBoxDynamicRange.editingFinished.connect(self.setDynamicRange)
        self.spinBoxImageX.editingFinished.connect(self.setImageX)
        self.spinBoxImageY.editingFinished.connect(self.setImageY)
        self.checkBoxAcquire.clicked.connect(self.setPedestal)
        self.checkBoxSubtract.clicked.connect(self.setPedestal)
        self.checkBoxCommonMode.clicked.connect(self.setPedestal)
        self.pushButtonReset.clicked.connect(self.resetPedestal)
        self.checkBoxRaw.clicked.connect(self.setRawData)
        self.spinBoxRawMin.editingFinished.connect(self.setRawData)
        self.spinBoxRawMax.editingFinished.connect(self.setRawData)
        self.checkBoxPedestal.clicked.connect(self.setPedestalSubtract)
        self.spinBoxPedestalMin.editingFinished.connect(self.setPedestalSubtract)
        self.spinBoxPedestalMax.editingFinished.connect(self.setPedestalSubtract)
        self.spinBoxFit.editingFinished.connect(self.setFitADC)
        self.spinBoxPlot.editingFinished.connect(self.setPlotBit)
        self.pushButtonReferesh.clicked.connect(self.plotReferesh)

        self.checkBoxFileWrite.clicked.connect(self.setFileWrite)
        self.lineEditFileName.editingFinished.connect(self.setFileName)
        self.lineEditFilePath.editingFinished.connect(self.setFilePath)
        self.pushButtonFilePath.clicked.connect(self.browseFilePath)
        self.spinBoxAcquisitionIndex.editingFinished.connect(self.setAccquisitionIndex)
        self.spinBoxFrames.editingFinished.connect(self.setFrames)
        self.spinBoxPeriod.editingFinished.connect(self.setPeriod)
        self.comboBoxPeriod.activated.connect(self.setPeriod)
        self.spinBoxTriggers.editingFinished.connect(self.setTriggers)
        self.pushButtonStart.clicked.connect(self.acquire)
        self.pushButtonStop.clicked.connect(self.stopAcquisition)
        

if __name__ == "__main__":
    app = QtWidgets.QApplication(sys.argv)
    main = MainWindow()
    main.show()
    # Run the app
    app.exec_()
