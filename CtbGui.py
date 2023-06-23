import time
from PyQt5 import QtWidgets, QtCore, QtGui, uic
import sys, os
import pyqtgraph as pg
from pyqtgraph import PlotWidget, GraphicsLayoutWidget
import multiprocessing as mp
from threading import Thread
from PIL import Image as im

import json
import zmq
import numpy as np
import posixpath
from pathlib import Path

from functools import partial
from slsdet import Detector, dacIndex, readoutMode, runStatus
from bit_utils import set_bit, remove_bit, bit_is_set, manipulate_bit
import random

from defines import *
from plotPattern import PlotPattern
import matplotlib.pyplot as plt
from matplotlib.backends.backend_qt5agg import FigureCanvasQTAgg as FigureCanvas
from matplotlib.backends.backend_qt5agg import NavigationToolbar2QT as NavigationToolbar

import argparse
import alias_utility 
import signal


class MainWindow(QtWidgets.QMainWindow):
    signalAcquire = QtCore.pyqtSignal()

    def __init__(self, *args, **kwargs):

        parser = argparse.ArgumentParser()
        parser.add_argument('-a', '--alias', help = "Alias file complete path")
        arglist = parser.parse_args()
        self.alias_file = arglist.alias

        pg.setConfigOption("background", (247, 247, 247))
        pg.setConfigOption("foreground", "k")

        self.det = Detector()
        self.setup_zmq()

        super(MainWindow, self).__init__()
        uic.loadUi("CtbGui.ui", self)

        self.setup_ui()
        self.tabWidget.setCurrentIndex(7)
        self.tabWidget.currentChanged.connect(self.refresh_tab)
        self.connect_ui()
        self.refresh_tab_dac()
        self.refresh_tab_power()
        self.refresh_tab_sense()
        self.refresh_tab_signals()
        self.refresh_tab_adc()
        self.refresh_tab_pattern()
        self.refresh_tab_acquisition()

        # also refreshes timer to start plotting 
        self.plotOptions()

        self.getPatViewerColors()
        self.getPatViewerWaitParameters()
        self.getPatViewerLoopParameters()
        self.updatePatViewerParameters()

        if self.alias_file is not None:
            self.loadAliasFile()

        self.signalAcquire.connect(self.pushButtonStart.click)
        signal.signal(signal.SIGINT, signal.SIG_DFL)
        

    def loadAliasFile(self):
        print(f'Loading Alias file: {self.alias_file}')
        try:
            bit_names, bit_plots, bit_colors, adc_names, adc_plots, adc_colors, dac_names, slowadc_names, voltage_names, pat_file_name = alias_utility.read_alias_file(self.alias_file)
        except Exception as e:
            QtWidgets.QMessageBox.warning(self, "Alias File Fail", e + "<br> " + self.alias_file, QtWidgets.QMessageBox.Ok)
            return

        for i in range(64):
            if bit_names[i]:
                self.det.setSignalName(i, bit_names[i])
            if bit_plots[i]:
                getattr(self, f"checkBoxBIT{i}DB").setChecked(bit_plots[i])
                getattr(self, f"checkBoxBIT{i}Plot").setChecked(bit_plots[i])
            if bit_colors[i]:
                self.setDBitButtonColor(i, bit_colors[i])

        for i in range(32):
            if adc_names[i]:
                self.det.setAdcName(i, adc_names[i])
            if adc_plots[i]:
                getattr(self, f"checkBoxADC{i}En").setChecked(adc_plots[i])
                getattr(self, f"checkBoxADC{i}Plot").setChecked(adc_plots[i])
            if adc_colors[i]:
                self.setADCButtonColor(i, adc_colors[i])

        for i in range(18):
            if dac_names[i]:
                iDac = getattr(dacIndex, f"DAC_{i}")
                self.det.setDacName(iDac, dac_names[i])

        for i in range(8):
            if slowadc_names[i]:
                self.det.setSlowAdcName(i, slowadc_names[i])

        for i in range(5):
            if voltage_names[i]:
                self.det.setVoltageName(i, voltage_names[i])

        if pat_file_name:
            self.lineEditPatternFile.setText(pat_file_name)

        self.updateSignalNames()
        self.updateADCNames()
        self.updateSlowAdcNames()
        self.updateDACNames()
        self.updateVoltageNames()
        

    # For Action options function
    # TODO Only add the components of action option+ functions
    # Function to show info
    def showInfo(self):
        msg = QtWidgets.QMessageBox()
        msg.setWindowTitle("About")
        msg.setText("This Gui is for Chip Test Boards.\n Current Phase: Development")
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

    def updateDACNames(self):
        for i, name in enumerate(self.det.getDacNames()):
            getattr(self, f"checkBoxDAC{i}").setText(name)    

    def getDACTristate(self, i):
        checkBox = getattr(self, f"checkBoxDAC{i}")
        dac = getattr(dacIndex, f"DAC_{i}")
        checkBox.stateChanged.disconnect()
        if (self.det.getDAC(dac)[0]) == -100:
            checkBox.setChecked(False)
        else:
            checkBox.setChecked(True)
        checkBox.stateChanged.connect(partial(self.setDACTristate, i))

    def setDACTristate(self, i):
        checkBox = getattr(self, f"checkBoxDAC{i}")
        if not checkBox.isChecked():
            self.setDAC(i)
        self.getDAC(i)

    def getDAC(self, i):
        checkBox = getattr(self, f"checkBoxDAC{i}")
        checkBoxmV = getattr(self, f"checkBoxDAC{i}mV")
        spinBox = getattr(self, f"spinBoxDAC{i}")
        label = getattr(self, f"labelDAC{i}")
        dac = getattr(dacIndex, f"DAC_{i}")

        checkBox.stateChanged.disconnect()
        checkBoxmV.stateChanged.disconnect()
        spinBox.editingFinished.disconnect()

        # do not uncheck automatically
        if (self.det.getDAC(dac)[0]) != -100:
            checkBox.setChecked(True)
            
        if checkBox.isChecked():
            spinBox.setEnabled(True)
            checkBoxmV.setEnabled(True)
        else:
            spinBox.setDisabled(True)
            checkBoxmV.setDisabled(True)

        if checkBoxmV.isChecked():
            label.setText(str(self.det.getDAC(dac, True)[0]))
        else:
            label.setText(str(self.det.getDAC(dac)[0]))

        checkBox.stateChanged.connect(partial(self.setDACTristate, i))
        checkBoxmV.stateChanged.connect(partial(self.getDAC, i))
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

        self.comboBoxADCVpp.currentIndexChanged.disconnect()
        self.comboBoxADCVpp.setCurrentIndex(retval)
        self.comboBoxADCVpp.currentIndexChanged.connect(self.setADCVpp)

    def setADCVpp(self):
        self.det.adcvpp = self.comboBoxADCVpp.currentIndex()
        self.getADCVpp()

    def getHighVoltage(self):
        retval = self.det.highvoltage
        self.labelHighVoltage.setText(str(retval))

        self.spinBoxHighVoltage.editingFinished.disconnect()
        self.checkBoxHighVoltage.stateChanged.disconnect()

        self.spinBoxHighVoltage.setValue(retval)
        if retval:
            self.checkBoxHighVoltage.setChecked(True)
        if self.checkBoxHighVoltage.isChecked():
            self.spinBoxHighVoltage.setEnabled(True)

        self.spinBoxHighVoltage.editingFinished.connect(self.setHighVoltage)
        self.checkBoxHighVoltage.stateChanged.connect(self.setHighVoltage)

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
    def updateVoltageNames(self):
        retval = self.det.getVoltageNames()
        getattr(self, f"checkBoxVA").setText(retval[0])  
        getattr(self, f"checkBoxVB").setText(retval[1])  
        getattr(self, f"checkBoxVC").setText(retval[2])  
        getattr(self, f"checkBoxVD").setText(retval[3])  
        getattr(self, f"checkBoxVIO").setText(retval[4])  

    def getVoltage(self, i):
        spinBox = getattr(self, f"spinBoxV{i}")
        checkBox = getattr(self, f"checkBoxV{i}")
        voltageIndex = getattr(dacIndex, f"V_POWER_{i}")
        label = getattr(self, f"labelV{i}")

        spinBox.editingFinished.disconnect()
        checkBox.stateChanged.disconnect()

        retval = self.det.getMeasuredVoltage(voltageIndex)[0]
        #spinBox.setValue(retval)
        if retval > 1:
            checkBox.setChecked(True)  
        if checkBox.isChecked():
            spinBox.setEnabled(True)
        else:
            spinBox.setDisabled(True)
        label.setText(f'{str(retval)} mV')

        spinBox.editingFinished.connect(partial(self.setVoltage, i))
        checkBox.stateChanged.connect(partial(self.setVoltage, i))

        self.getVChip()

    #TODO: handle multiple events when pressing enter (twice)
    def setVoltage(self, i):
        checkBox = getattr(self, f"checkBoxV{i}")
        spinBox = getattr(self, f"spinBoxV{i}")
        voltageIndex = getattr(dacIndex, f"V_POWER_{i}")
        spinBox.editingFinished.disconnect()

        value = 0
        if checkBox.isChecked():
            value = spinBox.value()
        try:
            self.det.setVoltage(voltageIndex, value)
        except Exception as e:
            QtWidgets.QMessageBox.warning(self, "Voltage Fail", str(e), QtWidgets.QMessageBox.Ok)
            pass

        # TODO: (properly) disconnecting and connecting to handle multiple events (out of focus and pressing enter).
        spinBox.editingFinished.connect(partial(self.setVoltage, i))
        self.getVoltage(i)
        self.getCurrent(i)

    def getCurrent(self, i):
        label = getattr(self, f"labelI{i}")
        currentIndex = getattr(dacIndex, f"I_POWER_{i}")
        retval = self.det.getMeasuredCurrent(currentIndex)[0]
        label.setText(f'{str(retval)} mA')

    def getVChip(self):
        self.spinBoxVChip.setValue(self.det.getVoltage(dacIndex.V_POWER_CHIP)[0])

    def powerOff(self):
        for i in ('A', 'B', 'C', 'D', 'IO'):
            # set all voltages to 0
            checkBox = getattr(self, f"checkBoxV{i}")
            checkBox.stateChanged.disconnect()
            checkBox.setChecked(False)
            checkBox.stateChanged.connect(partial(self.setVoltage, i))
            self.setVoltage(i)

    # Sense Tab functions
    def updateSlowAdcNames(self):
        for i, name in enumerate(self.det.getSlowAdcNames()):
            getattr(self, f"labelSlowAdc{i}").setText(name)    

    def updateSlowAdc(self, i):
        slowADCIndex = getattr(dacIndex, f"SLOW_ADC{i}")
        label = getattr(self, f"labelSlowAdcValue{i}")
        slowadc = (self.det.getSlowADC(slowADCIndex))[0] / 1000
        label.setText(f'{slowadc:.2f} mV')

    def updateTemperature(self):
        slowadc = self.det.getTemperature(dacIndex.SLOW_ADC_TEMP)
        self.labelTempValue.setText(f'{str(slowadc[0])} °C')

    # Signals Tab functions
    def updateSignalNames(self):
        for i, name in enumerate(self.det.getSignalNames()):
            getattr(self, f"labelBIT{i}").setText(name)    

    def getDigitalBitEnable(self, i, dbitList):
        checkBox = getattr(self, f"checkBoxBIT{i}DB")
        checkBox.stateChanged.disconnect()
        checkBox.setChecked(i in list(dbitList))
        checkBox.stateChanged.connect(partial(self.setDigitalBitEnable, i))

    def updateDigitalBitEnable(self):
        retval = self.det.rx_dbitlist    
        self.rx_dbitlist = list(retval)
        self.nDbitEnabled = len(list(retval)) 
        for i in range(64):
            self.getDigitalBitEnable(i, retval)
            self.getEnableBitPlot(i)
            self.getEnableBitColor(i)
        self.getDigitalBitEnableRange(retval)
        self.getEnableBitPlotRange()

    def setDigitalBitEnable(self, i):
        bitList = self.det.rx_dbitlist
        checkBox = getattr(self, f"checkBoxBIT{i}DB")
        if checkBox.isChecked():
            bitList.append(i)
        else:
            bitList.remove(i)
        self.det.rx_dbitlist = bitList
        
        self.updateDigitalBitEnable()

    def getDigitalBitEnableRange(self, dbitList):
        self.checkBoxBIT0_31DB.stateChanged.disconnect()
        self.checkBoxBIT32_63DB.stateChanged.disconnect()
        self.checkBoxBIT0_31DB.setChecked(all(x in list(dbitList) for x in range(32)))
        self.checkBoxBIT32_63DB.setChecked(all(x in list(dbitList) for x in range(32, 64)))
        self.checkBoxBIT0_31DB.stateChanged.connect(partial(self.setDigitalBitEnableRange, 0, 32))
        self.checkBoxBIT32_63DB.stateChanged.connect(partial(self.setDigitalBitEnableRange, 32, 64)) 

    def setDigitalBitEnableRange(self, start_nr, end_nr):
        bitList = self.det.rx_dbitlist
        checkBox = getattr(self, f"checkBoxBIT{start_nr}_{end_nr - 1}DB")
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
        checkBox = getattr(self, f"checkBoxBIT{i}DB")
        checkBoxPlot = getattr(self, f"checkBoxBIT{i}Plot")
        checkBoxPlot.setEnabled(checkBox.isChecked())

    def setEnableBitPlot(self, i):
        pushButton = getattr(self, f"pushButtonBIT{i}")
        checkBox = getattr(self, f"checkBoxBIT{i}Plot")
        pushButton.setEnabled(checkBox.isChecked())

        self.getEnableBitPlotRange()

    def getEnableBitPlotRange(self):
        self.checkBoxBIT0_31Plot.stateChanged.disconnect()
        self.checkBoxBIT32_63Plot.stateChanged.disconnect()
        self.checkBoxBIT0_31Plot.setEnabled(all(getattr(self, f"checkBoxBIT{i}Plot").isEnabled() for i in range(32)))
        self.checkBoxBIT32_63Plot.setEnabled(all(getattr(self, f"checkBoxBIT{i}Plot").isEnabled() for i in range(32, 64)))
        self.checkBoxBIT0_31Plot.setChecked(all(getattr(self, f"checkBoxBIT{i}Plot").isChecked() for i in range(32)))
        self.checkBoxBIT32_63Plot.setChecked(all(getattr(self, f"checkBoxBIT{i}Plot").isChecked() for i in range(32, 64)))
        self.checkBoxBIT0_31Plot.stateChanged.connect(partial(self.setEnableBitPlotRange, 0, 32))
        self.checkBoxBIT32_63Plot.stateChanged.connect(partial(self.setEnableBitPlotRange, 32, 64)) 

    def setEnableBitPlotRange(self, start_nr, end_nr):
        checkBox = getattr(self, f"checkBoxBIT{start_nr}_{end_nr - 1}Plot")
        enable = checkBox.isChecked()
        for i in range(start_nr, end_nr):
            checkBox = getattr(self, f"checkBoxBIT{i}Plot")
            checkBox.setChecked(enable)

    def getEnableBitColor(self, i):
        checkBox = getattr(self, f"checkBoxBIT{i}Plot")
        pushButton = getattr(self, f"pushButtonBIT{i}")
        pushButton.setEnabled(checkBox.isEnabled() and checkBox.isChecked())

    def selectBitColor(self, i):
        pushButton = getattr(self, f"pushButtonBIT{i}")
        self.showPalette(pushButton)

    def getDBitButtonColor(self, i):
        pushButton = getattr(self, f"pushButtonBIT{i}")
        return self.getActiveColor(pushButton)

    def setDBitButtonColor(self, i, color):
        pushButton = getattr(self, f"pushButtonBIT{i}")
        return self.setActiveColor(pushButton, color)

    def getIOOutReg(self):
        retval = self.det.patioctrl
        self.lineEditPatIOCtrl.editingFinished.disconnect()
        self.lineEditPatIOCtrl.setText("0x{:016x}".format(retval))
        self.lineEditPatIOCtrl.editingFinished.connect(self.setIOOutReg)
        return retval

    def setIOOutReg(self):
        self.lineEditPatIOCtrl.editingFinished.disconnect()
        try:
            self.det.patioctrl = int(self.lineEditPatIOCtrl.text(), 16)
        except Exception as e:
            QtWidgets.QMessageBox.warning(self, "IO Out Fail", str(e), QtWidgets.QMessageBox.Ok)
            pass
        #TODO: handling double event exceptions
        self.lineEditPatIOCtrl.editingFinished.connect(self.setIOOutReg)
        self.updateIOOut()

    def updateCheckBoxIOOut(self, i, out):
        checkBox = getattr(self, f"checkBoxBIT{i}Out")
        checkBox.stateChanged.disconnect()
        checkBox.setChecked(bit_is_set(out, i))
        checkBox.stateChanged.connect(partial(self.setIOOut, i))

    def updateIOOut(self):
        retval = self.getIOOutReg()
        for i in range(64):
            self.updateCheckBoxIOOut(i, retval)
        self.getIOoutRange(retval)
        
    def setIOOut(self, i):
        out = self.det.patioctrl
        checkBox = getattr(self, f"checkBoxBIT{i}Out")
        mask = manipulate_bit(checkBox.isChecked(), out, i)
        self.det.patioctrl = mask

        retval = self.getIOOutReg()
        self.updateCheckBoxIOOut(i, retval)
        self.getIOoutRange(retval)

    def getIOoutRange(self, out):
        self.checkBoxBIT0_31Out.stateChanged.disconnect()
        self.checkBoxBIT32_63Out.stateChanged.disconnect()
        self.checkBoxBIT0_31Out.setChecked((out & Defines.BIT0_31_MASK) == Defines.BIT0_31_MASK)
        self.checkBoxBIT32_63Out.setChecked((out & Defines.BIT32_63_MASK) == Defines.BIT32_63_MASK)
        self.checkBoxBIT0_31Out.stateChanged.connect(partial(self.setIOOutRange, 0, 32))
        self.checkBoxBIT32_63Out.stateChanged.connect(partial(self.setIOOutRange, 32, 64)) 

    def setIOOutRange(self, start_nr, end_nr):
        out = self.det.patioctrl
        checkBox = getattr(self, f"checkBoxBIT{start_nr}_{end_nr - 1}Out")
        mask = getattr(Defines, f"BIT{start_nr}_{end_nr - 1}_MASK")
        if checkBox.isChecked():
            self.det.patioctrl = out | mask
        else:   
            self.det.patioctrl = out & ~mask
        self.updateIOOut()
        
    def getDBitOffset(self):
        self.spinBoxDBitOffset.editingFinished.disconnect()
        self.rx_dbitoffset = self.det.rx_dbitoffset
        self.spinBoxDBitOffset.setValue(self.rx_dbitoffset)
        self.spinBoxDBitOffset.editingFinished.connect(self.setDbitOffset)

    def setDbitOffset(self):
        self.det.rx_dbitoffset = self.spinBoxDBitOffset.value()

    # ADCs Tab functions
    def updateADCNames(self):
        for i, adc_name in enumerate(self.det.getAdcNames()):
            getattr(self, f"labelADC{i}").setText(adc_name)    
       
    def getADCEnableReg(self):
        retval = self.det.adcenable
        if self.det.tengiga:
            retval = self.det.adcenable10g   
        self.lineEditADCEnable.editingFinished.disconnect()
        self.lineEditADCEnable.setText("0x{:08x}".format(retval))
        self.lineEditADCEnable.editingFinished.connect(self.setADCEnableReg)
        return retval

    def setADCEnableReg(self):
        self.lineEditADCEnable.editingFinished.disconnect()
        try:
            mask = int(self.lineEditADCEnable.text(), 16)
            if self.det.tengiga:
                self.det.adcenable10g = mask
            else:
                self.det.adcenable = mask
        except Exception as e:
            QtWidgets.QMessageBox.warning(self, "ADC Enable Fail", str(e), QtWidgets.QMessageBox.Ok)
            pass
        #TODO: handling double event exceptions
        self.lineEditADCEnable.editingFinished.connect(self.setADCEnableReg)
        self.updateADCEnable()

    def getADCEnable(self, i, mask):
        checkBox = getattr(self, f"checkBoxADC{i}En")
        checkBox.stateChanged.disconnect()
        checkBox.setChecked(bit_is_set(mask, i))
        checkBox.stateChanged.connect(partial(self.setADCEnable, i))

    def updateADCEnable(self):
        retval = self.getADCEnableReg()
        self.nADCEnabled = bin(retval).count('1')
        for i in range(32):
            self.getADCEnable(i, retval)
            self.getADCEnablePlot(i)
            self.getADCEnableColor(i)
        self.getADCEnableRange(retval)
        self.getADCEnablePlotRange()

    def setADCEnable(self, i):
        checkBox = getattr(self, f"checkBoxADC{i}En")
        try:
            if self.det.tengiga:
                enableMask = manipulate_bit(checkBox.isChecked(), self.det.adcenable10g, i)
                self.det.adcenable10g = enableMask
            else:
                enableMask = manipulate_bit(checkBox.isChecked(), self.det.adcenable, i)
                self.det.adcenable = enableMask
        except Exception as e:
            QtWidgets.QMessageBox.warning(self, "ADC Enable Fail", str(e), QtWidgets.QMessageBox.Ok)
            pass

        self.updateADCEnable()

    def getADCEnableRange(self, mask):
        self.checkBoxADC0_15En.stateChanged.disconnect()
        self.checkBoxADC16_31En.stateChanged.disconnect()
        self.checkBoxADC0_15En.setChecked((mask & Defines.BIT0_15_MASK) == Defines.BIT0_15_MASK)
        self.checkBoxADC16_31En.setChecked((mask & Defines.BIT16_31_MASK) == Defines.BIT16_31_MASK)
        self.checkBoxADC0_15En.stateChanged.connect(partial(self.setADCEnableRange, 0, 16))
        self.checkBoxADC16_31En.stateChanged.connect(partial(self.setADCEnableRange, 16, 32)) 

    def setADCEnableRange(self, start_nr, end_nr):
        mask = self.getADCEnableReg()
        retval = 0
        checkBox = getattr(self, f"checkBoxADC{start_nr}_{end_nr - 1}En")
        for i in range(start_nr, end_nr):
            mask = manipulate_bit(checkBox.isChecked(), mask, i)
        try:
            if self.det.tengiga:
                self.det.adcenable10g = mask
            else:
                self.det.adcenable = mask
        except Exception as e:
            QtWidgets.QMessageBox.warning(self, "ADC Enable Fail", str(e), QtWidgets.QMessageBox.Ok)
            pass
        self.updateADCEnable()

    def getADCEnablePlot(self, i):
        checkBox = getattr(self, f"checkBoxADC{i}En")
        checkBoxPlot = getattr(self, f"checkBoxADC{i}Plot")
        checkBoxPlot.setEnabled(checkBox.isChecked())

    def setADCEnablePlot(self, i):
        pushButton = getattr(self, f"pushButtonADC{i}")
        checkBox = getattr(self, f"checkBoxADC{i}Plot")
        pushButton.setEnabled(checkBox.isChecked())

        self.getADCEnablePlotRange()

    def getADCEnablePlotRange(self):
        self.checkBoxADC0_15Plot.stateChanged.disconnect()
        self.checkBoxADC16_31Plot.stateChanged.disconnect()
        self.checkBoxADC0_15Plot.setEnabled(all(getattr(self, f"checkBoxADC{i}Plot").isEnabled() for i in range(16)))
        self.checkBoxADC16_31Plot.setEnabled(all(getattr(self, f"checkBoxADC{i}Plot").isEnabled() for i in range(16, 32)))
        self.checkBoxADC0_15Plot.setChecked(all(getattr(self, f"checkBoxADC{i}Plot").isChecked() for i in range(16)))
        self.checkBoxADC16_31Plot.setChecked(all(getattr(self, f"checkBoxADC{i}Plot").isChecked() for i in range(16, 32)))
        self.checkBoxADC0_15Plot.stateChanged.connect(partial(self.setADCEnablePlotRange, 0, 16))
        self.checkBoxADC16_31Plot.stateChanged.connect(partial(self.setADCEnablePlotRange, 16, 32)) 
   
    def setADCEnablePlotRange(self, start_nr, end_nr):
        checkBox = getattr(self, f"checkBoxADC{start_nr}_{end_nr - 1}Plot")
        enable = checkBox.isChecked()
        for i in range(start_nr, end_nr):
            checkBox = getattr(self, f"checkBoxADC{i}Plot")
            checkBox.setChecked(enable)

    def getADCEnableColor(self, i):
        checkBox = getattr(self, f"checkBoxADC{i}Plot")
        pushButton = getattr(self, f"pushButtonADC{i}")
        pushButton.setEnabled(checkBox.isEnabled() and checkBox.isChecked())

    def selectADCColor(self, i):
        pushButton = getattr(self, f"pushButtonADC{i}")
        self.showPalette(pushButton)

    def getADCButtonColor(self, i):
        pushButton = getattr(self, f"pushButtonADC{i}")
        return self.getActiveColor(pushButton)

    def setADCButtonColor(self, i, color):
        pushButton = getattr(self, f"pushButtonADC{i}")
        return self.setActiveColor(pushButton, color)

    def getADCInvReg(self):
        retval = self.det.adcinvert
        self.lineEditADCInversion.editingFinished.disconnect()
        self.lineEditADCInversion.setText("0x{:08x}".format(retval))
        self.lineEditADCInversion.editingFinished.connect(self.setADCInvReg)
        return retval

    def setADCInvReg(self):
        self.lineEditADCInversion.editingFinished.disconnect()
        try:
            self.det.adcinvert = int(self.lineEditADCInversion.text(), 16)
        except Exception as e:
            QtWidgets.QMessageBox.warning(self, "ADC Inversion Fail", str(e), QtWidgets.QMessageBox.Ok)
            pass
        #TODO: handling double event exceptions
        self.lineEditADCInversion.editingFinished.connect(self.setADCInvReg)
        self.updateADCInv()

    def getADCInv(self, i, inv):
        checkBox = getattr(self, f"checkBoxADC{i}Inv")
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
        checkBox = getattr(self, f"checkBoxADC{i}Inv")
        mask = manipulate_bit(checkBox.isChecked(), out, i)
        self.det.adcinvert = mask

        retval = self.getADCInvReg()
        self.getADCInv(i, retval)
        self.getADCInvRange(retval)

    def getADCInvRange(self, inv):
        self.checkBoxADC0_15Inv.stateChanged.disconnect()
        self.checkBoxADC16_31Inv.stateChanged.disconnect()
        self.checkBoxADC0_15Inv.setChecked((inv & Defines.BIT0_15_MASK) == Defines.BIT0_15_MASK)
        self.checkBoxADC16_31Inv.setChecked((inv & Defines.BIT16_31_MASK) == Defines.BIT16_31_MASK)
        self.checkBoxADC0_15Inv.stateChanged.connect(partial(self.setADCInvRange, 0, 16))
        self.checkBoxADC16_31Inv.stateChanged.connect(partial(self.setADCInvRange, 16, 32)) 

    def setADCInvRange(self, start_nr, end_nr):
        out = self.det.adcinvert
        checkBox = getattr(self, f"checkBoxADC{start_nr}_{end_nr - 1}Inv")
        mask = getattr(Defines, f"BIT{start_nr}_{end_nr - 1}_MASK")
        if checkBox.isChecked():
            self.det.adcinvert = out | mask
        else:   
            self.det.adcinvert = out & ~mask

        self.updateADCInv()
        
    # Pattern Tab functions

    def getPatLimitAddress(self):
        retval = self.det.patlimits
        self.lineEditStartAddress.editingFinished.disconnect()
        self.lineEditStopAddress.editingFinished.disconnect()
        self.lineEditStartAddress.setText("0x{:04x}".format(retval[0]))
        self.lineEditStopAddress.setText("0x{:04x}".format(retval[1]))
        self.lineEditStartAddress.editingFinished.connect(self.setPatLimitAddress)
        self.lineEditStopAddress.editingFinished.connect(self.setPatLimitAddress)

    def setPatLimitAddress(self):
        self.lineEditStartAddress.editingFinished.disconnect()
        self.lineEditStopAddress.editingFinished.disconnect()      
        try: 
            start = int(self.lineEditStartAddress.text(), 16)
            stop = int(self.lineEditStopAddress.text(), 16)
            self.det.patlimits = [start, stop]
        except Exception as e:
            QtWidgets.QMessageBox.warning(self, "Pattern Limit Address Fail", str(e), QtWidgets.QMessageBox.Ok)
            pass
        #TODO: handling double event exceptions
        self.lineEditStartAddress.editingFinished.connect(self.setPatLimitAddress)
        self.lineEditStopAddress.editingFinished.connect(self.setPatLimitAddress)
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
        lineEditStart.editingFinished.disconnect()
        lineEditStop.editingFinished.disconnect()
        try:
            start = int(lineEditStart.text(), 16)
            stop = int(lineEditStop.text(), 16)
            self.det.patloop[level] = [start, stop]
        except Exception as e:
            QtWidgets.QMessageBox.warning(self, "Pattern Loop Start Stop Address Fail", str(e), QtWidgets.QMessageBox.Ok)
            pass
        #TODO: handling double event exceptions
        lineEditStart.editingFinished.connect(partial(self.setPatLoopStartStopAddress, level))
        lineEditStop.editingFinished.connect(partial(self.setPatLoopStartStopAddress, level))
        self.getPatLoopStartStopAddress(level)

    def getPatLoopWaitAddress(self, level):
        retval = self.det.patwait[level]
        lineEdit = getattr(self, f"lineEditLoop{level}Wait")
        lineEdit.editingFinished.disconnect()
        lineEdit.setText("0x{:04x}".format(retval))
        lineEdit.editingFinished.connect(partial(self.setPatLoopWaitAddress, level))

    def setPatLoopWaitAddress(self, level):
        lineEdit = getattr(self, f"lineEditLoop{level}Wait")
        lineEdit.editingFinished.disconnect()
        try:
            addr = int(lineEdit.text(), 16)
            self.det.patwait[level] = addr
        except Exception as e:
            QtWidgets.QMessageBox.warning(self, "Pattern Wait Address Fail", str(e), QtWidgets.QMessageBox.Ok)
            pass
        #TODO: handling double event exceptions
        lineEdit.editingFinished.connect(partial(self.setPatLoopWaitAddress, level))
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

    def compilePattern(self):
        pattern_file = self.lineEditPatternFile.text()
        if not pattern_file:
            QtWidgets.QMessageBox.warning(self, "Pattern Fail", "No pattern file selected. Please select one.", QtWidgets.QMessageBox.Ok)
            return ""
        # compile
        if self.checkBoxCompile.isChecked():
            compilerFile = self.lineEditCompiler.text()
            if not compilerFile:
                QtWidgets.QMessageBox.warning(self, "Compile Fail", "No compiler selected. Please select one.", QtWidgets.QMessageBox.Ok)
                return ""

            # if old compile file exists, backup and remove to ensure old copy not loaded
            oldFile = Path(pattern_file + 'at')
            if oldFile.is_file():
                print("Moving old compiled pattern file to _bck") 
                exit_status = os.system('mv '+ str(oldFile) + ' ' + str(oldFile) + '_bkup')
                if exit_status != 0:
                    retval = QtWidgets.QMessageBox.question(self, "Backup Fail", "Could not make a backup of old compiled code. Proceed anyway to compile and overwrite?", QtWidgets.QMessageBox.Yes, QtWidgets.QMessageBox.No)
                    if retval == QtWidgets.QMessageBox.No:              
                        return ""

            compileCommand = compilerFile + ' ' + pattern_file
            print(compileCommand)
            print("Compiling pattern code to .pat file")
            exit_status = os.system(compileCommand)
            if exit_status != 0:
                QtWidgets.QMessageBox.warning(self, "Compile Fail", "Could not compile pattern.", QtWidgets.QMessageBox.Ok)
                return ""
            pattern_file += 'at'
        
        return pattern_file

    def loadPattern(self):
        pattern_file = self.compilePattern()
        if not pattern_file:
            return
        # load pattern
        self.det.pattern = pattern_file
        self.lineEditPatternFile.setText(self.det.patfname[0])


    def getPatViewerColors(self):
        colorLevel = self.comboBoxPatColorSelect.currentIndex()
        color = self.colors_plot[colorLevel]
        self.comboBoxPatColor.currentIndexChanged.disconnect()
        self.comboBoxPatColor.setCurrentIndex(Defines.Colors.index(color))
        self.comboBoxPatColor.currentIndexChanged.connect(self.updatePatViewerParameters)
        
    def getPatViewerWaitParameters(self):
        waitLevel = self.comboBoxPatWait.currentIndex()
        color = self.colors_wait[waitLevel]
        line_style = self.linestyles_wait[waitLevel]
        alpha = self.alpha_wait[waitLevel]
        alpha_rect = self.alpha_wait_rect[waitLevel]

        self.comboBoxPatWaitColor.currentIndexChanged.disconnect()
        self.comboBoxPatWaitLineStyle.currentIndexChanged.disconnect()
        self.doubleSpinBoxWaitAlpha.editingFinished.disconnect()
        self.doubleSpinBoxWaitAlphaRect.editingFinished.disconnect()

        self.comboBoxPatWaitColor.setCurrentIndex(Defines.Colors.index(color))
        self.comboBoxPatWaitLineStyle.setCurrentIndex(Defines.LineStyles.index(line_style))
        self.doubleSpinBoxWaitAlpha.setValue(alpha)
        self.doubleSpinBoxWaitAlphaRect.setValue(alpha_rect)

        self.comboBoxPatWaitColor.currentIndexChanged.connect(self.updatePatViewerParameters)
        self.comboBoxPatWaitLineStyle.currentIndexChanged.connect(self.updatePatViewerParameters)
        self.doubleSpinBoxWaitAlpha.editingFinished.connect(self.updatePatViewerParameters)
        self.doubleSpinBoxWaitAlphaRect.editingFinished.connect(self.updatePatViewerParameters)
         
    def getPatViewerLoopParameters(self):
        loopLevel = self.comboBoxPatLoop.currentIndex()
        color = self.colors_loop[loopLevel]
        line_style = self.linestyles_loop[loopLevel]
        alpha = self.alpha_loop[loopLevel]
        alpha_rect = self.alpha_loop_rect[loopLevel]

        self.comboBoxPatLoopColor.currentIndexChanged.disconnect()
        self.comboBoxPatLoopLineStyle.currentIndexChanged.disconnect()
        self.doubleSpinBoxLoopAlpha.editingFinished.disconnect()
        self.doubleSpinBoxLoopAlphaRect.editingFinished.disconnect()

        self.comboBoxPatLoopColor.setCurrentIndex(Defines.Colors.index(color))
        self.comboBoxPatLoopLineStyle.setCurrentIndex(Defines.LineStyles.index(line_style))
        self.doubleSpinBoxLoopAlpha.setValue(alpha)
        self.doubleSpinBoxLoopAlphaRect.setValue(alpha_rect)

        self.comboBoxPatLoopColor.currentIndexChanged.connect(self.updatePatViewerParameters)
        self.comboBoxPatLoopLineStyle.currentIndexChanged.connect(self.updatePatViewerParameters)
        self.doubleSpinBoxLoopAlpha.editingFinished.connect(self.updatePatViewerParameters)
        self.doubleSpinBoxLoopAlphaRect.editingFinished.connect(self.updatePatViewerParameters)

    # only at start up
    def updateDefaultPatViewerParameters(self):
        self.colors_plot = Defines.Colors_plot.copy()
        self.colors_wait = Defines.Colors_wait.copy()
        self.linestyles_wait = Defines.Linestyles_wait.copy()
        self.alpha_wait = Defines.Alpha_wait.copy()
        self.alpha_wait_rect = Defines.Alpha_wait_rect.copy()
        self.colors_loop = Defines.Colors_loop.copy()
        self.linestyles_loop = Defines.Linestyles_loop.copy()
        self.alpha_loop = Defines.Alpha_loop.copy()
        self.alpha_loop_rect = Defines.Alpha_loop_rect.copy()
        self.clock_vertical_lines_spacing = Defines.Clock_vertical_lines_spacing
        self.show_clocks_number = Defines.Show_clocks_number
        self.line_width = Defines.Line_width

        #print('default')
        #self.printPatViewerParameters()


    def updatePatViewerParameters(self):
        colorLevel = self.comboBoxPatColorSelect.currentIndex()
        color = self.comboBoxPatColor.currentIndex()
        #self.colors_plot[colorLevel] = f'tab:{Defines.Colors[color].lower()}'
        self.colors_plot[colorLevel] = Defines.Colors[color]
        
        waitLevel = self.comboBoxPatWait.currentIndex()
        color = self.comboBoxPatWaitColor.currentIndex()
        line_style = self.comboBoxPatWaitLineStyle.currentIndex()
        alpha = self.doubleSpinBoxWaitAlpha.value()
        alpha_rect = self.doubleSpinBoxWaitAlphaRect.value()

        self.colors_wait[waitLevel] = Defines.Colors[color]
        self.linestyles_wait[waitLevel] = Defines.LineStyles[line_style]
        self.alpha_wait[waitLevel] = alpha
        self.alpha_wait_rect[waitLevel] = alpha_rect

        
        loopLevel = self.comboBoxPatLoop.currentIndex()
        color = self.comboBoxPatLoopColor.currentIndex()
        line_style = self.comboBoxPatLoopLineStyle.currentIndex()
        alpha = self.doubleSpinBoxLoopAlpha.value()
        alpha_rect = self.doubleSpinBoxLoopAlphaRect.value()

        self.colors_loop[loopLevel] = Defines.Colors[color]
        self.linestyles_loop[loopLevel] = Defines.LineStyles[line_style]
        self.alpha_loop[loopLevel] = alpha
        self.alpha_loop_rect[loopLevel] = alpha_rect

        self.clock_vertical_lines_spacing = self.spinBoxPatClockSpacing.value()
        self.show_clocks_number = self.checkBoxPatShowClockNumber.isChecked()
        self.line_width = self.doubleSpinBoxLineWidth.value()

        # for debugging
        #self.printPatViewerParameters()

    def printPatViewerParameters(self):
        print('Pattern Viewer Parameters:')
        print(f'\tcolor1: {self.colors_plot[0]}, color2: {self.colors_plot[1]}')
        print(f"\twait color: {self.colors_wait}")
        print(f"\twait linestyles: {self.linestyles_wait}")
        print(f"\twait alpha: {self.alpha_wait}")
        print(f"\twait alpha rect: {self.alpha_wait_rect}")
        print(f"\tloop color: {self.colors_loop}")
        print(f"\tloop linestyles: {self.linestyles_loop}")
        print(f"\tloop alpha: {self.alpha_loop}")
        print(f"\tloop alpha rect: {self.alpha_loop_rect}")
        print(f'\tclock vertical lines spacing: {self.clock_vertical_lines_spacing}')
        print(f'\tshow clocks number: {self.show_clocks_number}')
        print(f'\tline width: {self.line_width}')
        print('\n')

    def viewPattern(self):
        pattern_file = self.compilePattern()
        if not pattern_file:
            return


        signalNames = self.det.getSignalNames()
        p = PlotPattern(pattern_file, signalNames, self.colors_plot, self.colors_wait, self.linestyles_wait, self.alpha_wait, self.alpha_wait_rect, self.colors_loop, self.linestyles_loop, self.alpha_loop, self.alpha_loop_rect, self.clock_vertical_lines_spacing, self.show_clocks_number, self.line_width, )
        
        plt.close(self.figure)
        self.gridLayoutPatternViewer.removeWidget(self.canvas)
        self.canvas.close()
        self.gridLayoutPatternViewer.removeWidget(self.toolbar)
        self.toolbar.close()

        try:
            self.figure = p.patternPlot()
            self.canvas = FigureCanvas(self.figure)
            self.toolbar = NavigationToolbar(self.canvas, self)
            self.gridLayoutPatternViewer.addWidget(self.toolbar)
            self.gridLayoutPatternViewer.addWidget(self.canvas)
        except Exception as e:
            QtWidgets.QMessageBox.warning(self, "Pattern Viewer Fail", str(e), QtWidgets.QMessageBox.Ok)
            pass

    # Plot Tab functions
    def plotOptions(self):

        self.framePatternViewer.hide()
        # disable image widgets
        self.comboBoxPlot.setDisabled(True)
        if hasattr(self, 'imageViewAnalog'):
            self.imageViewAnalog.close()   
        if hasattr(self, 'imageViewDigital'):
            self.imageViewDigital.close()
        self.plotWidgetAnalog.clear()
        self.plotWidgetDigital.clear()

        # disable plotting
        self.read_timer.stop()
        
        if self.radioButtonWaveform.isChecked():
            self.plotWidgetAnalog.setLabel('left',"<span style=\"color:black;font-size:14px\">Output [ADC]</span>")
            self.plotWidgetAnalog.setLabel('bottom',"<span style=\"color:black;font-size:14px\">Analog Sample [#]</span>")
            self.plotWidgetAnalog.addLegend(colCount = 4)
            self.plotWidgetDigital.setLabel('left',"<span style=\"color:black;font-size:14px\">Digital Bit</span>")
            self.plotWidgetDigital.setLabel('bottom',"<span style=\"color:black;font-size:14px\">Digital Sample [#]</span>")
            self.plotWidgetDigital.addLegend(colCount = 4)

        elif self.radioButtonImage.isChecked():
            self.comboBoxPlot.setEnabled(True)

            self.imageViewAnalog = pg.ImageView(self.plotWidgetAnalog, view=pg.PlotItem())
            self.imageViewAnalog.show()
            self.imageViewDigital = pg.ImageView(self.plotWidgetDigital, view=pg.PlotItem())
            self.imageViewDigital.show()

            self.comboBoxPlot.currentIndexChanged.disconnect()
            if self.comboBoxPlot.currentIndex() >= 1:
                QtWidgets.QMessageBox.warning(self, "Not Implemented Yet", "Sorry, this is not implemented yet.", QtWidgets.QMessageBox.Ok)
                self.comboBoxPlot.setCurrentIndex(0)
            self.comboBoxPlot.currentIndexChanged.connect(self.plotOptions)
                
                
        # enable plotting
        if not self.radioButtonNoPlot.isChecked():
            self.read_timer.start(Defines.Time_Plot_Refresh_ms)
        
        #self.showPlot()

    ''' after being able to resize windows
    def showPlot(self):
        self.plotWidgetAnalog.hide()
        self.plotWidgetDigital.hide()
        # only enable required plot and adc/digital bits enabled
        if not self.radioButtonNoPlot.isChecked():
            if self.romode.value in [1, 2] and self.nDbitEnabled > 0:
                for i in range(64):
                    checkBox = getattr(self, f"checkBoxBIT{i}Plot")
                    if checkBox.isChecked():
                        self.plotWidgetDigital.show()
                        break
            if self.romode.value in [0, 2] and self.nADCEnabled > 0:
                for i in range(32):
                    checkBox = getattr(self, f"checkBoxADC{i}Plot")
                    if checkBox.isChecked():
                        self.plotWidgetAnalog.show()
                        break   
    ''' 

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
        

    # Acquisition Tab functions

    def getReadout(self):
        self.comboBoxROMode.currentIndexChanged.disconnect()
        self.spinBoxAnalog.editingFinished.disconnect()
        self.spinBoxDigital.editingFinished.disconnect()

        self.romode = self.det.romode
        self.comboBoxROMode.setCurrentIndex(self.romode.value)
        match self.romode:
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

        self.comboBoxROMode.currentIndexChanged.connect(self.setReadOut)
        self.spinBoxAnalog.editingFinished.connect(self.setAnalog)
        self.spinBoxDigital.editingFinished.connect(self.setDigital)
        self.getAnalog()
        self.getDigital()
        #self.showPlot()

    def setReadOut(self):
        if self.comboBoxROMode.currentIndex() == 0:
            self.det.romode = readoutMode.ANALOG_ONLY
        elif self.comboBoxROMode.currentIndex() == 1:
            self.det.romode = readoutMode.DIGITAL_ONLY
        else:
            self.det.romode = readoutMode.ANALOG_AND_DIGITAL
        self.getReadout()

    def getRunFrequency(self):
        self.spinBoxRunF.editingFinished.disconnect()
        self.spinBoxRunF.setValue(self.det.runclk)
        self.spinBoxRunF.editingFinished.connect(self.setRunFrequency)

    def setRunFrequency(self):
        self.det.runclk = self.spinBoxRunF.value()
        self.getRunFrequency()

    def getAnalog(self):
        self.spinBoxAnalog.editingFinished.disconnect()
        self.asamples = self.det.asamples
        self.spinBoxAnalog.setValue(self.asamples)
        self.spinBoxAnalog.editingFinished.connect(self.setAnalog)

    def setAnalog(self):
        self.det.asamples = self.spinBoxAnalog.value()
        self.getAnalog()

    def getDigital(self):
        self.spinBoxDigital.editingFinished.disconnect()
        self.dsamples = self.det.dsamples
        self.spinBoxDigital.setValue(self.dsamples)
        self.spinBoxDigital.editingFinished.connect(self.setDigital)

    def setDigital(self):
        self.det.dsamples = self.spinBoxDigital.value()
        self.getDigital()

    def getADCFrequency(self):
        self.spinBoxADCF.editingFinished.disconnect()
        self.spinBoxADCF.setValue(self.det.adcclk)
        self.spinBoxADCF.editingFinished.connect(self.setADCFrequency)

    def setADCFrequency(self):
        self.det.adcclk = self.spinBoxADCF.value()
        self.getADCFrequency()

    def getADCPhase(self):
        self.spinBoxADCPhase.editingFinished.disconnect()
        self.spinBoxADCPhase.setValue(self.det.adcphase)
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

    def getFileWrite(self):
        self.checkBoxFileWrite.stateChanged.disconnect()
        self.checkBoxFileWrite.setChecked(self.det.fwrite)
        self.checkBoxFileWrite.stateChanged.connect(self.setFileWrite)

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
        self.lineEditFilePath.setText(str(self.det.fpath))
        self.lineEditFilePath.editingFinished.connect(self.setFilePath)

    def setFilePath(self):
        self.det.fpath = Path(self.lineEditFilePath.text())
        self.getFilePath()

    def browseFilePath(self):
        response = QtWidgets.QFileDialog.getExistingDirectory(
            parent = self,
            caption = "Select Path to Save Output File",
            directory = os.getcwd(),
            options = (QtWidgets.QFileDialog.ShowDirsOnly | QtWidgets.QFileDialog.DontResolveSymlinks)
            # filter='README (*.md *.ui)'
        )
        if response:
            self.lineEditFilePath.setText(response)
            self.setFilePath()

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
        self.comboBoxPeriod.currentIndexChanged.disconnect()

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
        self.comboBoxPeriod.currentIndexChanged.connect(self.setPeriod)

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

    def updateDetectorStatus(self, status):
        self.labelDetectorStatus.setText(status.name)

    def updateCurrentMeasurement(self):
        self.labelCurrentMeasurement.setText(str(self.currentMeasurement))
        #print(f"Meausrement {self.currentMeasurement}")
    
    def updateCurrentFrame(self, val):
        self.labelCurrentFrame.setText(str(val))

    def updateAcquiredFrames(self, val):
        self.labelAcquiredFrames.setText(str(val))

    def toggleAcquire(self):
        if self.pushButtonStart.isChecked():
            self.acquire()
        else:
            self.stopAcquisition()

    def toggleStartButton(self, started):
        if started:
            self.pushButtonStart.setChecked(True)
            self.pushButtonStart.setText('Stop')
        else:        
            self.pushButtonStart.setChecked(False)
            self.pushButtonStart.setText('Start')

    def stopAcquisition(self):
        self.det.stop()
        self.stoppedFlag = True
        
    def acquire(self):
        self.stoppedFlag = False
        self.toggleStartButton(True)
        self.currentMeasurement = 0

        # some functions that must be updated for local values
        self.getAnalog()
        self.getDigital()
        self.getReadout()
        self.getDBitOffset()
        self.getADCEnableReg()
        self.updateDigitalBitEnable()
        self.startMeasurement()

    def startMeasurement(self):
        self.updateCurrentMeasurement()
        self.updateCurrentFrame(0)
        self.updateAcquiredFrames(0)
        self.progressBar.setValue(0)

        self.det.rx_start()
        self.det.start()
        self.checkEndofAcquisition()

    def checkEndofAcquisition(self):
        caught = self.det.rx_framescaught
        self.updateAcquiredFrames(caught)
        status = self.det.status
        self.updateDetectorStatus(status)
        measurementDone = False
        #print(f'status:{val}')
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
            if self.det.rx_framescaught != caught:
                measurementDone = False

        numMeasurments = self.spinBoxMeasurements.value()
        if measurementDone:
            if self.det.rx_status == runStatus.RUNNING:
                self.det.rx_stop()
            if self.checkBoxFileWrite.isChecked():
                self.spinBoxAcquisitionIndex.stepUp()
            # next measurement
            self.currentMeasurement += 1
            if self.currentMeasurement < numMeasurments and not self.stoppedFlag:
                self.startMeasurement()
            else:
                self.statusTimer.stop()
                self.toggleStartButton(False)
        else:
            self.statusTimer.start(Defines.Time_Status_Refresh_ms)


    # For other functios
    #Reading data from zmq and decoding it
    def read_zmq(self):
        #print("in readzmq")
        try:
            msg = self.socket.recv_multipart(flags=zmq.NOBLOCK)
            if len(msg) != 2:
                if len(msg) != 1:
                    print(f'len(msg) = {len(msg)}')
                return
            header, data = msg
            jsonHeader = json.loads(header)
            #print(jsonHeader)
            self.progressBar.setValue(int(jsonHeader['progress']))
            self.updateCurrentFrame(jsonHeader['frameIndex'])
            #print(f"image size:{int(jsonHeader['size'])}")
            #print(f'Data size: {len(data)}')
            

            
            self.plotWidgetAnalog.clear()
            self.plotWidgetDigital.clear()
            

            # waveform
            if self.radioButtonWaveform.isChecked():
                # analog
                if self.romode.value in [0, 2]:
                    analog_array = np.array(np.frombuffer(data, dtype=np.uint16, count= self.nADCEnabled * self.asamples))
                    for i in range(32):
                        checkBox = getattr(self, f"checkBoxADC{i}Plot")
                        if checkBox.isChecked():
                            waveform = np.zeros(self.asamples)
                            for iSample in range(self.asamples):
                                # all adc for 1 sample together
                                waveform[iSample] = analog_array[iSample * self.nADCEnabled + i]
                            pen = pg.mkPen(color = self.getADCButtonColor(i), width = 1)
                            legendName = getattr(self, f"labelADC{i}").text()
                            self.plotWidgetAnalog.plot(waveform, pen=pen, name = legendName)
                # digital
                if self.romode.value in [1, 2]:
                    dbitoffset = self.rx_dbitoffset
                    if self.romode.value == 2:
                        dbitoffset += self.nADCEnabled * 2 * self.asamples
                    digital_array = np.array(np.frombuffer(data, offset = dbitoffset, dtype=np.uint8))
                    offset = 0
                    for i in self.rx_dbitlist:
                        # where numbits * numsamples is not a multiple of 8
                        if offset % 8 != 0:
                            offset += (8 - (offset % 8))

                        checkBox = getattr(self, f"checkBoxBIT{i}Plot")
                        # bits enabled but not plotting
                        if not checkBox.isChecked():
                            offset += self.dsamples
                            continue
                        # to plot
                        if checkBox.isChecked():
                            waveform = np.zeros(self.dsamples)
                            for iSample in range(self.dsamples):
                                # all samples for digital bit together from slsReceiver
                                index = (int)(offset / 8)
                                iBit = offset % 8
                                #print(f" bit:{iBit} index:{index} iBit:{iBit} offset:{offset}")
                                bit = (digital_array[index] >> iBit) & 1
                                waveform[iSample] = bit
                                offset += 1
                            pen = pg.mkPen(color = self.getDBitButtonColor(i), width = 1)
                            legendName = getattr(self, f"labelBIT{i}").text()
                            self.plotWidgetDigital.plot(waveform, pen=pen, name = legendName)
            # image
            else:           
                # analog
                if self.romode.value in [0, 2]:
                    analog_array = np.array(np.frombuffer(data, dtype=np.uint16, count= self.nADCEnabled * self.asamples))
                    analog_frame = np.zeros((self.nADCEnabled, self.asamples))
                    analogIndex = 0
                    for row in range(self.nADCEnabled):
                        for col in range(self.asamples):
                            analog_frame[row, col] = analog_array[analogIndex]
                            analogIndex += 1
                    self.imageViewAnalog.setImage(analog_array)   

                # digital
                if self.romode.value in [1, 2]:
                    dbitoffset = self.rx_dbitoffset
                    if self.romode.value == 2:
                        dbitoffset += self.nADCEnabled * 2 * self.asamples
                    digital_array = np.array(np.frombuffer(data, offset = dbitoffset, dtype=np.uint8))
                    digital_frame = np.zeros((400, 400))
                    self.imageViewDigital.setImage(digital_frame)   
            

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

    def getActiveColor(self, button):
        return button.palette().color(QtGui.QPalette.Window)

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
        patternViewer = False
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
                patternViewer = True
            case 7:
                self.refresh_tab_acquisition()
        
        if patternViewer:
            self.plotWidgetAnalog.hide()
            self.plotWidgetDigital.hide()
            self.framePatternViewer.show()
        else:
            self.plotWidgetAnalog.show()
            self.plotWidgetDigital.show()
            self.framePatternViewer.hide()           


    def refresh_tab_dac(self):
        self.updateDACNames()
        for i in range(18):
            self.getDACTristate(i)
            self.getDAC(i)

        self.getADCVpp()
        self.getHighVoltage()

    def refresh_tab_power(self):
        self.updateVoltageNames()
        for i in ('A', 'B', 'C', 'D', 'IO'):
            self.getVoltage(i)
            self.getCurrent(i)

    def refresh_tab_sense(self):
        self.updateSlowAdcNames()
        for i in range(8):
            self.updateSlowAdc(i)
        self.updateTemperature()

    def refresh_tab_signals(self):
        self.updateSignalNames()
        self.updateDigitalBitEnable()
        self.updateIOOut()
        self.getDBitOffset()

    def refresh_tab_adc(self):
        self.updateADCNames()
        self.updateADCInv()
        self.updateADCEnable()

    def refresh_tab_pattern(self):
        self.getPatLimitAddress()
        for i in range(6):
            self.getPatLoopStartStopAddress(i)
            self.getPatLoopWaitAddress(i)
            self.getPatLoopRepetition(i)
            self.getPatLoopWaitTime(i)
        
    def refresh_tab_acquisition(self):
        self.getReadout()
        self.getRunFrequency()
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
        # Getting voltage values for spinboxes (only for modifying)
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
            self.setDBitButtonColor(i, self.getRandomColor())

        # Adc Tab
        for i in range(32):
            self.setADCButtonColor(i, self.getRandomColor())

        # Pattern Tab
        for i in range(len(Defines.Colors)):
            self.comboBoxPatColor.addItem(Defines.Colors[i])
            self.comboBoxPatWaitColor.addItem(Defines.Colors[i])
            self.comboBoxPatLoopColor.addItem(Defines.Colors[i])
        for i in range(len(Defines.LineStyles)):
            self.comboBoxPatWaitLineStyle.addItem(Defines.LineStyles[i])
            self.comboBoxPatLoopLineStyle.addItem(Defines.LineStyles[i])       
        self.updateDefaultPatViewerParameters()
        self.comboBoxPatColorSelect.setCurrentIndex(0)
        self.comboBoxPatWait.setCurrentIndex(0)
        self.comboBoxPatLoop.setCurrentIndex(0)
        self.spinBoxPatClockSpacing.setValue(self.clock_vertical_lines_spacing)
        self.checkBoxPatShowClockNumber.setChecked(self.show_clocks_number)
        self.doubleSpinBoxLineWidth.setValue(self.line_width)
        self.lineEditPatternFile.setText(self.det.patfname[0])
        # rest gets updated after connecting to slots

        # Acquisition Tab
        self.toggleStartButton(False)

        # plot area
        self.figure, self.ax = plt.subplots()
        self.canvas = FigureCanvas(self.figure)
        self.toolbar = NavigationToolbar(self.canvas, self)
        self.gridLayoutPatternViewer.addWidget(self.toolbar)
        self.gridLayoutPatternViewer.addWidget(self.canvas)
        self.figure.clear()


    def keyPressEvent(self, event):
        if event.modifiers() & QtCore.Qt.ShiftModifier:
            if event.key() == QtCore.Qt.Key_Return:
                self.signalAcquire.emit()


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
            getattr(self, f"checkBoxDAC{i}").stateChanged.connect(partial(self.setDACTristate, i))
            getattr(self, f"checkBoxDAC{i}mV").stateChanged.connect(partial(self.getDAC, i))

        self.comboBoxADCVpp.currentIndexChanged.connect(self.setADCVpp)
        self.spinBoxHighVoltage.editingFinished.connect(self.setHighVoltage)
        self.checkBoxHighVoltage.stateChanged.connect(self.setHighVoltage)

        # For Power Supplies tab
        for i in ('A', 'B', 'C', 'D', 'IO'):
            spinBox = getattr(self, f"spinBoxV{i}")
            checkBox = getattr(self, f"checkBoxV{i}")
            spinBox.editingFinished.connect(partial(self.setVoltage, i))
            checkBox.stateChanged.connect(partial(self.setVoltage, i))
        self.pushButtonPowerOff.clicked.connect(self.powerOff)

        # For Sense Tab
        for i in range(8):
            getattr(self, f"pushButtonSlowAdc{i}").clicked.connect(partial(self.updateSlowAdc, i))
        self.pushButtonTemp.clicked.connect(self.updateTemperature)

        # For Signals Tab
        for i in range(64):
            getattr(self, f"checkBoxBIT{i}DB").stateChanged.connect(partial(self.setDigitalBitEnable, i))
            getattr(self, f"checkBoxBIT{i}Out").stateChanged.connect(partial(self.setIOOut, i))
            getattr(self, f"checkBoxBIT{i}Plot").stateChanged.connect(partial(self.setEnableBitPlot, i))
            getattr(self, f"pushButtonBIT{i}").clicked.connect(partial(self.selectBitColor, i))
        self.checkBoxBIT0_31DB.stateChanged.connect(partial(self.setDigitalBitEnableRange, 0, 32))
        self.checkBoxBIT32_63DB.stateChanged.connect(partial(self.setDigitalBitEnableRange, 32, 64)) 
        self.checkBoxBIT0_31Plot.stateChanged.connect(partial(self.setEnableBitPlotRange, 0, 32))
        self.checkBoxBIT32_63Plot.stateChanged.connect(partial(self.setEnableBitPlotRange, 32, 64)) 
        self.checkBoxBIT0_31Out.stateChanged.connect(partial(self.setIOOutRange, 0, 32))
        self.checkBoxBIT32_63Out.stateChanged.connect(partial(self.setIOOutRange, 32, 64)) 
        self.lineEditPatIOCtrl.editingFinished.connect(self.setIOOutReg)
        self.spinBoxDBitOffset.editingFinished.connect(self.setDbitOffset)

        # For ADCs Tab
        for i in range(32):
            getattr(self, f"checkBoxADC{i}Inv").stateChanged.connect(partial(self.setADCInv, i))
            getattr(self, f"checkBoxADC{i}En").stateChanged.connect(partial(self.setADCEnable, i))
            getattr(self, f"checkBoxADC{i}Plot").stateChanged.connect(partial(self.setADCEnablePlot, i))
            getattr(self, f"pushButtonADC{i}").clicked.connect(partial(self.selectADCColor, i))
        self.checkBoxADC0_15En.stateChanged.connect(partial(self.setADCEnableRange, 0, 16))
        self.checkBoxADC16_31En.stateChanged.connect(partial(self.setADCEnableRange, 16, 32)) 
        self.checkBoxADC0_15Plot.stateChanged.connect(partial(self.setADCEnablePlotRange, 0, 16))
        self.checkBoxADC16_31Plot.stateChanged.connect(partial(self.setADCEnablePlotRange, 16, 32)) 
        self.checkBoxADC0_15Inv.stateChanged.connect(partial(self.setADCInvRange, 0, 16))
        self.checkBoxADC16_31Inv.stateChanged.connect(partial(self.setADCInvRange, 16, 32)) 
        self.lineEditADCInversion.editingFinished.connect(self.setADCInvReg)
        self.lineEditADCEnable.editingFinished.connect(self.setADCEnableReg)
        # Cannot set adcmask to 0 anyway


        # For Pattern Tab
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
        self.pushButtonLoadPattern.clicked.connect(self.loadPattern)
        
        self.comboBoxPatColorSelect.currentIndexChanged.connect(self.getPatViewerColors)
        self.comboBoxPatWait.currentIndexChanged.connect(self.getPatViewerWaitParameters)
        self.comboBoxPatLoop.currentIndexChanged.connect(self.getPatViewerLoopParameters)

        self.comboBoxPatColor.currentIndexChanged.connect(self.updatePatViewerParameters)
        self.comboBoxPatWaitColor.currentIndexChanged.connect(self.updatePatViewerParameters)
        self.comboBoxPatLoopColor.currentIndexChanged.connect(self.updatePatViewerParameters)
        self.comboBoxPatWaitLineStyle.currentIndexChanged.connect(self.updatePatViewerParameters)
        self.comboBoxPatLoopLineStyle.currentIndexChanged.connect(self.updatePatViewerParameters)
        self.doubleSpinBoxWaitAlpha.editingFinished.connect(self.updatePatViewerParameters)
        self.doubleSpinBoxLoopAlpha.editingFinished.connect(self.updatePatViewerParameters)
        self.doubleSpinBoxWaitAlphaRect.editingFinished.connect(self.updatePatViewerParameters)
        self.doubleSpinBoxLoopAlphaRect.editingFinished.connect(self.updatePatViewerParameters)
        self.spinBoxPatClockSpacing.editingFinished.connect(self.updatePatViewerParameters)
        self.checkBoxPatShowClockNumber.stateChanged.connect(self.updatePatViewerParameters)
        self.doubleSpinBoxLineWidth.editingFinished.connect(self.updatePatViewerParameters)        
        self.pushButtonViewPattern.clicked.connect(self.viewPattern)


        # For Acquistions Tab
        self.comboBoxROMode.currentIndexChanged.connect(self.setReadOut)
        self.spinBoxRunF.editingFinished.connect(self.setRunFrequency)
        self.spinBoxAnalog.editingFinished.connect(self.setAnalog)
        self.spinBoxDigital.editingFinished.connect(self.setDigital)
        self.spinBoxADCF.editingFinished.connect(self.setADCFrequency)
        self.spinBoxADCPhase.editingFinished.connect(self.setADCPhase)
        self.spinBoxADCPipeline.editingFinished.connect(self.setADCPipeline)
        self.spinBoxDBITF.editingFinished.connect(self.setDBITFrequency)
        self.spinBoxDBITPhase.editingFinished.connect(self.setDBITPhase)
        self.spinBoxDBITPipeline.editingFinished.connect(self.setDBITPipeline)
        
        self.radioButtonNoPlot.clicked.connect(self.plotOptions)
        self.radioButtonWaveform.clicked.connect(self.plotOptions)
        self.radioButtonDistribution.clicked.connect(self.plotOptions)
        self.radioButtonImage.clicked.connect(self.plotOptions)
        self.comboBoxPlot.currentIndexChanged.connect(self.plotOptions)
        self.spinBoxSerialOffset.editingFinished.connect(self.setSerialOffset)
        self.spinBoxNCount.editingFinished.connect(self.setNCounter)
        self.spinBoxDynamicRange.editingFinished.connect(self.setDynamicRange)
        self.spinBoxImageX.editingFinished.connect(self.setImageX)
        self.spinBoxImageY.editingFinished.connect(self.setImageY)
        self.checkBoxAcquire.stateChanged.connect(self.setPedestal)
        self.checkBoxSubtract.stateChanged.connect(self.setPedestal)
        self.checkBoxCommonMode.stateChanged.connect(self.setPedestal)
        self.pushButtonReset.clicked.connect(self.resetPedestal)
        self.checkBoxRaw.stateChanged.connect(self.setRawData)
        self.spinBoxRawMin.editingFinished.connect(self.setRawData)
        self.spinBoxRawMax.editingFinished.connect(self.setRawData)
        self.checkBoxPedestal.stateChanged.connect(self.setPedestalSubtract)
        self.spinBoxPedestalMin.editingFinished.connect(self.setPedestalSubtract)
        self.spinBoxPedestalMax.editingFinished.connect(self.setPedestalSubtract)
        self.spinBoxFit.editingFinished.connect(self.setFitADC)
        self.spinBoxPlot.editingFinished.connect(self.setPlotBit)
        self.pushButtonReferesh.clicked.connect(self.plotReferesh)

        self.checkBoxFileWrite.stateChanged.connect(self.setFileWrite)
        self.lineEditFileName.editingFinished.connect(self.setFileName)
        self.lineEditFilePath.editingFinished.connect(self.setFilePath)
        self.pushButtonFilePath.clicked.connect(self.browseFilePath)
        self.spinBoxAcquisitionIndex.editingFinished.connect(self.setAccquisitionIndex)
        self.spinBoxFrames.editingFinished.connect(self.setFrames)
        self.spinBoxPeriod.editingFinished.connect(self.setPeriod)
        self.comboBoxPeriod.currentIndexChanged.connect(self.setPeriod)
        self.spinBoxTriggers.editingFinished.connect(self.setTriggers)
        self.pushButtonStart.clicked.connect(self.toggleAcquire)
        

if __name__ == "__main__":
    app = QtWidgets.QApplication(sys.argv)
    main = MainWindow()
    main.show()
    # Run the app
    app.exec_()
