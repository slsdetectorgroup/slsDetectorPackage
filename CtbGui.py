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

from functools import partial
from slsdet import Detector, dacIndex, readoutMode, runStatus
from bit_utils import set_bit, remove_bit, bit_is_set


class MainWindow(QtWidgets.QMainWindow):
    def __init__(self, *args, **kwargs):
        pg.setConfigOption("background", (247, 247, 247))
        pg.setConfigOption("foreground", "k")

        self.det = Detector()
        self.setup_zmq()

        super(MainWindow, self).__init__()
        uic.loadUi("CtbGui.ui", self)
        self.setup_qtimer()
        self.refresh_tab_dac()
        self.refresh_tab_power()
        self.refresh_tab_sense()
        self.refresh_tab_signals()
        self.refresh_tab_adc()
        self.refresh_tab_pattern()
        self.refresh_tab_acquisition()
        self.tabWidget.setCurrentIndex(6)
        self.tabWidget.currentChanged.connect(self.refresh_tab)
        self.connect_ui()


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

    # For the DACs tab functions
    # TODO Only add DACs tab functions
    def setDAC(self, i):
        checkBoxDac = getattr(self, f"checkBoxDAC{i}")
        checkBoxmV = getattr(self, f"checkBoxDAC{i}mV")
        spinBoxDac = getattr(self, f"spinBoxDAC{i}")
        dac = getattr(dacIndex, f"DAC_{i}")
        dacLabel = getattr(self, f"labelDAC{i}")

        if checkBoxDac.isChecked():
            if checkBoxmV.isChecked():
                self.det.setDAC(dac, spinBoxDac.value(), True)
                dacLabel.setText(str(self.det.getDAC(dac, True)[0]))
            else:
                self.det.setDAC(dac, spinBoxDac.value())
                dacLabel.setText(str(self.det.getDAC(dac)[0]))
            spinBoxDac.setDisabled(False)
        else:
            self.det.setDAC(dac, -100)
            spinBoxDac.setDisabled(True)
            dacLabel.setText(str(self.det.getDAC(dac)[0]))

    # TODO yet to implement the ADC and HV
    def setADC(self):
        self.det.setDAC(dacIndex.ADC_VPP, self.comboBoxADC.currentIndex())
        self.labelADC.setText(f'Mode: {str(self.det.getDAC(dacIndex.ADC_VPP)[0])}')

    def setHighVoltage(self):
        try:
            if self.checkBoxHighVoltage.isChecked():
                self.det.setHighVoltage(self.spinBoxHighVoltage.value())
                self.spinBoxHighVoltage.setDisabled(False)
            else:
                self.det.setHighVoltage(0)
                self.spinBoxHighVoltage.setDisabled(True)
            self.labelHighVoltage.setText(str(self.det.getHighVoltage()[0]))
        except Exception as e:
            print(e)

    # For Power Supplies Tab functions
    # TODO Only add the components of Power Supplies tab functions
    def setPower(self, i):
        checkBox = getattr(self, f"checkBoxV{i}")
        spinBox = getattr(self, f"spinBoxV{i}")
        power = getattr(dacIndex, f"V_POWER_{i}")
        label = getattr(self, f"labelV{i}")
        try:
            if checkBox.isChecked():
                self.det.setVoltage(power, spinBox.value())
                spinBox.setDisabled(False)
            else:
                self.det.setVoltage(power, 0)
                spinBox.setDisabled(True)
            label.setText(str(self.det.getVoltage(power)[0]))
        except Exception as e:
            print(e)

    # For Sense Tab functions
    # TODO Only add the components of Sense tab functions
    def updateSense(self, i):
        slowADC = getattr(dacIndex, f"SLOW_ADC{i}")
        label = getattr(self, f"labelSense{i}_2")
        sense0 = self.det.getSlowADC(slowADC)
        label.setText(str(sense0[0]))

    def updateTemperature(self):
        sense0 = self.det.getTemperature(dacIndex.SLOW_ADC_TEMP)
        self.labelTemp_2.setText(f'{str(sense0[0])} °C')

    # For Signals Tab functions
    # TODO Only add the components of Signals tab functions
    def dbit(self, i):
        checkBox = getattr(self, f"checkBoxBIT{i}DB")
        bitList = self.det.rx_dbitlist
        if checkBox.isChecked():
            bitList.append(i)
            self.det.rx_dbitlist = bitList
        else:
            bitList.remove(i)
            self.det.rx_dbitlist = bitList

    def IOout(self, i):
        checkBox = getattr(self, f"checkBoxBIT{i}Out")
        out = self.det.patioctrl
        if checkBox.isChecked():
            mask = set_bit(out, i)
            self.det.patioctrl = mask
        else:
            mask = remove_bit(out, i)
            self.det.patioctrl = mask
        self.lineEditBoxIOControl.setText(hex(self.det.patioctrl))

    def setDbitOffset(self):
        self.det.rx_dbitoffset = self.spinBoxDBitOffset.value()

    def colorBIT0(self):
        self.showPalette(self.pushButtonBIT0)

    def colorBIT1(self):
        self.showPalette(self.pushButtonBIT1)

    def colorBIT2(self):
        self.showPalette(self.pushButtonBIT2)

    def colorBIT3(self):
        self.showPalette(self.pushButtonBIT3)

    def colorBIT4(self):
        self.showPalette(self.pushButtonBIT4)

    def colorBIT5(self):
        self.showPalette(self.pushButtonBIT5)

    def colorBIT6(self):
        self.showPalette(self.pushButtonBIT6)

    def colorBIT7(self):
        self.showPalette(self.pushButtonBIT7)

    def colorBIT8(self):
        self.showPalette(self.pushButtonBIT8)

    def colorBIT9(self):
        self.showPalette(self.pushButtonBIT9)

    def colorBIT10(self):
        self.showPalette(self.pushButtonBIT10)

    def colorBIT11(self):
        self.showPalette(self.pushButtonBIT11)

    def colorBIT12(self):
        self.showPalette(self.pushButtonBIT12)

    def colorBIT13(self):
        self.showPalette(self.pushButtonBIT13)

    def colorBIT14(self):
        self.showPalette(self.pushButtonBIT14)

    def colorBIT15(self):
        self.showPalette(self.pushButtonBIT15)

    def colorBIT16(self):
        self.showPalette(self.pushButtonBIT16)

    def colorBIT17(self):
        self.showPalette(self.pushButtonBIT17)

    def colorBIT18(self):
        self.showPalette(self.pushButtonBIT18)

    def colorBIT19(self):
        self.showPalette(self.pushButtonBIT19)

    def colorBIT20(self):
        self.showPalette(self.pushButtonBIT20)

    def colorBIT21(self):
        self.showPalette(self.pushButtonBIT21)

    def colorBIT22(self):
        self.showPalette(self.pushButtonBIT22)

    def colorBIT23(self):
        self.showPalette(self.pushButtonBIT23)

    def colorBIT24(self):
        self.showPalette(self.pushButtonBIT24)

    def colorBIT25(self):
        self.showPalette(self.pushButtonBIT25)

    def colorBIT26(self):
        self.showPalette(self.pushButtonBIT26)

    def colorBIT27(self):
        self.showPalette(self.pushButtonBIT27)

    def colorBIT28(self):
        self.showPalette(self.pushButtonBIT28)

    def colorBIT29(self):
        self.showPalette(self.pushButtonBIT29)

    def colorBIT30(self):
        self.showPalette(self.pushButtonBIT30)

    def colorBIT31(self):
        self.showPalette(self.pushButtonBIT31)

    def colorBIT32(self):
        self.showPalette(self.pushButtonBIT32)

    def colorBIT33(self):
        self.showPalette(self.pushButtonBIT33)

    def colorBIT34(self):
        self.showPalette(self.pushButtonBIT34)

    def colorBIT35(self):
        self.showPalette(self.pushButtonBIT35)

    def colorBIT36(self):
        self.showPalette(self.pushButtonBIT36)

    def colorBIT37(self):
        self.showPalette(self.pushButtonBIT37)

    def colorBIT38(self):
        self.showPalette(self.pushButtonBIT38)

    def colorBIT39(self):
        self.showPalette(self.pushButtonBIT39)

    def colorBIT40(self):
        self.showPalette(self.pushButtonBIT40)

    def colorBIT41(self):
        self.showPalette(self.pushButtonBIT41)

    def colorBIT42(self):
        self.showPalette(self.pushButtonBIT42)

    def colorBIT43(self):
        self.showPalette(self.pushButtonBIT43)

    def colorBIT44(self):
        self.showPalette(self.pushButtonBIT44)

    def colorBIT45(self):
        self.showPalette(self.pushButtonBIT45)

    def colorBIT46(self):
        self.showPalette(self.pushButtonBIT46)

    def colorBIT47(self):
        self.showPalette(self.pushButtonBIT47)

    def colorBIT48(self):
        self.showPalette(self.pushButtonBIT48)

    def colorBIT49(self):
        self.showPalette(self.pushButtonBIT49)

    def colorBIT50(self):
        self.showPalette(self.pushButtonBIT50)

    def colorBIT51(self):
        self.showPalette(self.pushButtonBIT51)

    def colorBIT52(self):
        self.showPalette(self.pushButtonBIT52)

    def colorBIT53(self):
        self.showPalette(self.pushButtonBIT53)

    def colorBIT54(self):
        self.showPalette(self.pushButtonBIT54)

    def colorBIT55(self):
        self.showPalette(self.pushButtonBIT55)

    def colorBIT56(self):
        self.showPalette(self.pushButtonBIT56)

    def colorBIT57(self):
        self.showPalette(self.pushButtonBIT57)

    def colorBIT58(self):
        self.showPalette(self.pushButtonBIT58)

    def colorBIT59(self):
        self.showPalette(self.pushButtonBIT59)

    def colorBIT60(self):
        self.showPalette(self.pushButtonBIT60)

    def colorBIT61(self):
        self.showPalette(self.pushButtonBIT61)

    def colorBIT62(self):
        self.showPalette(self.pushButtonBIT62)

    def colorBIT63(self):
        self.showPalette(self.pushButtonBIT63)

    # For ADCs Tab functions
    # TODO Only add the components of ADCs tab functions
    def colorADC0(self):
        self.showPalette(self.pushButtonADC0)

    def colorADC1(self):
        self.showPalette(self.pushButtonADC1)

    def colorADC2(self):
        self.showPalette(self.pushButtonADC2)

    def colorADC3(self):
        self.showPalette(self.pushButtonADC3)

    def colorADC4(self):
        self.showPalette(self.pushButtonADC4)

    def colorADC5(self):
        self.showPalette(self.pushButtonADC5)

    def colorADC6(self):
        self.showPalette(self.pushButtonADC6)

    def colorADC7(self):
        self.showPalette(self.pushButtonADC7)

    def colorADC8(self):
        self.showPalette(self.pushButtonADC8)

    def colorADC9(self):
        self.showPalette(self.pushButtonADC9)

    def colorADC10(self):
        self.showPalette(self.pushButtonADC10)

    def colorADC11(self):
        self.showPalette(self.pushButtonADC11)

    def colorADC12(self):
        self.showPalette(self.pushButtonADC12)

    def colorADC13(self):
        self.showPalette(self.pushButtonADC13)

    def colorADC14(self):
        self.showPalette(self.pushButtonADC14)

    def colorADC15(self):
        self.showPalette(self.pushButtonADC15)

    def colorADC16(self):
        self.showPalette(self.pushButtonADC16)

    def colorADC17(self):
        self.showPalette(self.pushButtonADC17)

    def colorADC18(self):
        self.showPalette(self.pushButtonADC18)

    def colorADC19(self):
        self.showPalette(self.pushButtonADC19)

    def colorADC20(self):
        self.showPalette(self.pushButtonADC20)

    def colorADC21(self):
        self.showPalette(self.pushButtonADC21)

    def colorADC22(self):
        self.showPalette(self.pushButtonADC22)

    def colorADC23(self):
        self.showPalette(self.pushButtonADC23)

    def colorADC24(self):
        self.showPalette(self.pushButtonADC24)

    def colorADC25(self):
        self.showPalette(self.pushButtonADC25)

    def colorADC26(self):
        self.showPalette(self.pushButtonADC26)

    def colorADC27(self):
        self.showPalette(self.pushButtonADC27)

    def colorADC28(self):
        self.showPalette(self.pushButtonADC28)

    def colorADC29(self):
        self.showPalette(self.pushButtonADC29)

    def colorADC30(self):
        self.showPalette(self.pushButtonADC30)

    def colorADC31(self):
        self.showPalette(self.pushButtonADC31)

    def enableMask_Enable(self, i):
        enableMaskCheckBox = getattr(self, f"checkBoxADC{i}En")
        if self.det.tengiga:
            enableMask = set_bit(self.det.adcenable10g, i)
            self.det.adcenable10g = enableMask
            self.lineEditEnable.setText(hex(self.det.adcenable10g))
        else:
            enableMask = set_bit(self.det.adcenable, i)
            self.det.adcenable = enableMask
            self.lineEditEnable.setText(hex(self.det.adcenable))
        enableMaskCheckBox.setChecked(True)

    def enableMask_Disable(self, i):
        enableMaskCheckBox = getattr(self, f"checkBoxADC{i}En")
        if self.det.tengiga:
            enableMask = remove_bit(self.det.adcenable10g, i)
            self.det.adcenable10g = enableMask
            self.lineEditEnable.setText(hex(self.det.adcenable10g))
        else:
            enableMask = remove_bit(self.det.adcenable, i)
            self.det.adcenable = enableMask
            self.lineEditEnable.setText(hex(self.det.adcenable))
        enableMaskCheckBox.setChecked(False)

    def all_0_15(self):
        for i in range(0, 16):
            enableMaskCheckBox = getattr(self, f"checkBoxADC{i}En")
            if enableMaskCheckBox.isChecked():
                pass
            else:
                self.enableMask_Enable(i)

    def none_0_15(self):
        for i in range(0, 16):
            self.enableMask_Disable(i)

    def all_16_31(self):
        for i in range(16, 32):
            self.enableMask_Enable(i)

    def none_16_31(self):
        for i in range(16, 32):
            self.enableMask_Disable(i)

    def enable_mask_all(self):
        for i in range(0, 32):
            self.enableMask_Enable(i)

    def enable_mask_none(self):
        for i in range(0, 32):
            self.enableMask_Disable(i)

    def ADCEnable(self, i):
        enableMaskCheckBox = getattr(self, f"checkBoxADC{i}En")
        if enableMaskCheckBox.isChecked():
            self.enableMask_Enable(i)
        else:
            self.enableMask_Disable(i)

    def ADCInvert(self, i):
        invertCheckBox = getattr(self, f"checkBoxADC{i}Inv")
        if invertCheckBox.isChecked():
            inversionMask = set_bit(self.det.adcinvert, i)
            self.det.adcinvert = inversionMask
        else:
            inversionMask = remove_bit(self.det.adcinvert, i)
            self.det.adcinvert = inversionMask
        self.lineEditInversion.setText(hex(self.det.adcinvert))

    # For Pattern Tab functions
    # TODO Only add the components of Pattern tab functions
    def setCompiler(self):
        response = QtWidgets.QFileDialog.getOpenFileName(
            parent=self,
            caption="Select a compiler file",
            directory=os.getcwd(),
            # filter='README (*.md *.ui)'
        )
        if response[0]:
            self.lineEditCompiler.setText(response[0])

    def setPattern(self):
        response = QtWidgets.QFileDialog.getOpenFileName(
            parent=self,
            caption="Select a pattern file",
            directory=os.getcwd(),
            filter='README (*.pat)'
        )
        if response[0]:
            self.lineEditPattern.setText(response[0])

    def setFrames(self):
        self.det.frames = self.spinBoxFrames.value()

    def setPeriod(self):
        if self.comboBoxTime.currentIndex() == 0:
            self.det.period = self.spinBoxPeriod.value()
        elif self.comboBoxTime.currentIndex() == 1:
            self.det.period = self.spinBoxPeriod.value() * (1e-3)
        elif self.comboBoxTime.currentIndex() == 2:
            self.det.period = self.spinBoxPeriod.value() * (1e-6)
        else:
            self.det.period = self.spinBoxPeriod.value() * (1e-9)

    def setTriggers(self):
        self.det.triggers = self.spinBoxTriggers.value()

    def setRunFrequency(self):
        self.det.runclk = self.spinBoxRunF.value()

    def setADCFrequency(self):
        self.det.adcclk = self.spinBoxADCF.value()

    def setADCPhase(self):
        self.det.adcphase = self.spinBoxADCPhase.value()

    def setADCPipeline(self):
        self.det.adcpipeline = self.spinBoxADCPipeline.value()

    def setDBITFrequency(self):
        self.det.dbitclk = self.spinBoxDBITF.value()

    def setDBITPhase(self):
        self.det.dbitphase = self.spinBoxDBITPhase.value()

    def setDBITPipeline(self):
        self.det.dbitpipeline = self.spinBoxDBITPipeline.value()

    def setLoop(self, i):
        loopSpinBox = getattr(self, f"spinBoxLoop{i}").value()
        self.det.patnloop[i] = loopSpinBox

    def setWait(self, i):
        waitSpinBox = getattr(self, f"spinBoxWait{i}").value()
        self.det.patwaittime[i] = waitSpinBox

    def setAnalog(self):
        self.det.asamples = self.spinBoxAnalog.value()

    def setDigital(self):
        self.det.dsamples = self.spinBoxDigital.value()

    def setReadOut(self):
        if self.comboBoxROMode.currentIndex() == 0:
            self.det.romode = readoutMode.ANALOG_ONLY
            self.spinBoxDigital.setDisabled(True)
            self.spinBoxAnalog.setDisabled(False)
        elif self.comboBoxROMode.currentIndex() == 1:
            self.det.romode = readoutMode.DIGITAL_ONLY
            self.spinBoxAnalog.setDisabled(True)
            self.spinBoxDigital.setDisabled(False)
        else:
            self.det.romode = readoutMode.ANALOG_AND_DIGITAL
            self.spinBoxDigital.setDisabled(False)
            self.spinBoxAnalog.setDisabled(False)


    def loadPattern(self):
        pattern_file = self.lineEditPattern.text()
        comp=self.checkBoxCompile.isChecked()
        if pattern_file:
            if comp: 
                print("Compiling pattern code to .pat file")
                compFile=self.lineEditCompiler.text()
                if compFile:
                    st=compFile+' '+self.lineEditPattern.text()
                    os.system(st)
                    print(st)
                    pattern_file=self.lineEditPattern.text()+"at"
                    print(pattern_file)
                    self.det.pattern = pattern_file
                else:
                    print('No compiler selected!!!')
            else:
                self.det.pattern = pattern_file
            
        else:
            print('No pattern file selected!!!')


    # For Acquistions Tab functions
    # TODO Only add the components of Acquistions tab functions
    def plotOptions(self):
        print("plot options")

    def setSerialOffset(self):
        print("SerialOffSet")

    def setNCounter(self):
        print("plot options")

    def setDynamicRange(self):
        print("plot options")

    def setImageX(self):
        print("plot options")

    def setImageY(self):
        print("plot options")

    def setPedestal(self):
        print("plot options")

    def resetPedestal(self):
        print("plot options")

    def setRawData(self):
        print("plot options")

    def setPedestalSubtract(self):
        print("plot options")

    def setFitADC(self):
        print("plot options")

    def setPlotBit(self):
        print("plot options")

    def setFileWrite(self):
        if self.checkBoxFileWrite.isChecked():
           self.det.fwrite=True
        else:
            self.det.fwrite=False
            
    def setFileName(self):
        self.det.fname = self.lineEditFileName.text()

    def setFilePath(self):
        self.det.fpath = self.lineEditFilePath.text()

    def setIndex(self):
        self.det.findex = self.spinBoxIndex.value()

    def acquire(self):
        self.pushButtonStart.setEnabled(False)
        measurement_Number = self.spinBoxMeasurements.value()
        for i in range(measurement_Number):

            self.read_timer.start(20)
            self.det.rx_start()
            self.statusTimer.start(20)
            self.progressBar.setValue(0)
            self.det.start()

    def updateDetectorStatus(self):
        self.labelDetectorStatus.setText(self.det.status.name)
        self.acqDone = False
        match self.det.status:
            case runStatus.RUNNING:
                pass
            case runStatus.WAITING:
                pass
            case runStatus.TRANSMITTING:
                pass
            case _:
                self.acqDone = True

        if self.acqDone:
            self.read_timer.stop()
            self.statusTimer.stop()
            if self.det.rx_status == runStatus.RUNNING:
                self.det.rx_stop()
            if self.checkBoxFileWrite.isChecked():
                self.spinBoxIndex.stepUp()
            self.pushButtonStart.setEnabled(True)

    def plotReferesh(self):
        self.read_zmq()

    def browseFile(self):
        response = QtWidgets.QFileDialog.getSaveFileName(
            parent=self,
            caption="Select a file to save the Output",
            directory=os.getcwd(),
            # filter='README (*.md *.ui)'
        )
        if response[0]:
            self.lineEditFilePath.setText(response[0])

    # For other functios
    # TODO Add other functions which will be reused  
    #Reading data from zmq and decoding it.
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
            fig, ax = plt.subplots()
            im = ax.imshow(analog_frame)
            ax.invert_yaxis()
            fig.colorbar(im)
            plt.show()
            '''
            
            plot1 = pg.ImageView(self.plotWidget, view=pg.PlotItem())
            plot1.show()
            plot1.setImage(analog_frame)
            
        except zmq.ZMQError as e:
            pass
        except Exception as e:
            print(f'Caught exception: {str(e)}')

    def showPalette(self, button):
        color = QtWidgets.QColorDialog.getColor()
        if color.isValid():
            button.setStyleSheet("background-color: %s" % color.name())
            # get the RGB Values
            print(color.getRgb())

    # Getting the checkbox status
    def getDac(self, i):
        checkBox = getattr(self, f"checkBoxDAC{i}")
        spinBox = getattr(self, f"spinBoxDAC{i}")
        dac = getattr(dacIndex, f"DAC_{i}")
        if (self.det.getDAC(dac)[0]) == -100:
            spinBox.setDisabled(True)
        else:
            checkBox.setChecked(True)

    def getPower(self, i):
        spinBox = getattr(self, f"spinBoxV{i}")
        dac = getattr(dacIndex, f"V_POWER_{i}")
        checkBox = getattr(self, f"checkBoxV{i}")

        if (self.det.getVoltage(dac)[0]) == 0:
            spinBox.setDisabled(True)
        else:
            checkBox.setChecked(True)

    # initializing the Out status
    def set_IO_Out(self):
        out = self.det.patioctrl
        for i in range(64):
            if bit_is_set(out, i):
                getattr(self, f"checkBoxBIT{i}Out").setChecked(True)

    def set_ADC_Enable(self):
        if self.det.tengiga:
            adcenable10g = self.det.adcenable10g
            for i in range(32):
                if bit_is_set(adcenable10g, i):
                    getattr(self, f"checkBoxADC{i}En").setChecked(True)
        else:
            adcenable = self.det.adcenable
            for i in range(32):
                if bit_is_set(adcenable, i):
                    getattr(self, f"checkBoxADC{i}En").setChecked(True)

    def set_ADC_Inversion(self):
        adcInversion = self.det.adcinvert
        for i in range(32):
            if bit_is_set(adcInversion, i):
                getattr(self, f"checkBoxADC{i}Inv").setChecked(True)


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
        # Getting dac Name
        for i, dac_name in enumerate(self.det.getDacNames()):
            getattr(self, f"checkBoxDAC{i}").setText(dac_name)

        # Getting dac values for label
        for i in range(18):
            dac = getattr(dacIndex, f"DAC_{i}")
            getattr(self, f"labelDAC{i}").setText(str(self.det.getDAC(dac)[0]))
        self.labelADC.setText(str(self.det.getDAC(dacIndex.ADC_VPP)[0]))
        self.labelHighVoltage.setText(str(self.det.getHighVoltage()[0]))

        # Getting dac values
        for i in range(18):
            dac = getattr(dacIndex, f"DAC_{i}")
            getattr(self, f"spinBoxDAC{i}").setValue(self.det.getDAC(dac)[0])
            self.getDac(i)
        self.spinBoxDAC0.setValue(self.det.getDAC(dacIndex.DAC_0)[0])
        self.getDac(0)

        # Setting ADC VPP
        self.labelADC.setText(f'Mode: {str(self.det.getDAC(dacIndex.ADC_VPP)[0])}')
        adcVPP = self.det.getDAC(dacIndex.ADC_VPP)
        for i in adcVPP:
            if (self.det.getDAC(dacIndex.ADC_VPP)[0]) == i:
                self.comboBoxADC.setCurrentIndex(i)

        # Setting High voltage
        self.spinBoxHighVoltage.setValue(self.det.getHighVoltage()[0])
        if (self.det.getHighVoltage()[0]) == 0:
            self.spinBoxHighVoltage.setDisabled(True)
        else:
            self.checkBoxHighVoltage.setChecked(True)


    def refresh_tab_power(self):
        self.labelVA.setText(str(self.det.getVoltage(dacIndex.V_POWER_A)[0]))
        self.labelVB.setText(str(self.det.getVoltage(dacIndex.V_POWER_B)[0]))
        self.labelVC.setText(str(self.det.getVoltage(dacIndex.V_POWER_C)[0]))
        self.labelVD.setText(str(self.det.getVoltage(dacIndex.V_POWER_D)[0]))
        self.labelVIO.setText(str(self.det.getVoltage(dacIndex.V_POWER_IO)[0]))
        self.labelVCHIP.setText(str(self.det.getVoltage(dacIndex.V_POWER_CHIP)[0]))

        # Updating values for Power Supply
        self.spinBoxVA.setValue(self.det.getVoltage(dacIndex.V_POWER_A)[0])
        self.getPower("A")

        self.spinBoxVB.setValue(self.det.getVoltage(dacIndex.V_POWER_B)[0])
        self.getPower("B")

        self.spinBoxVC.setValue(self.det.getVoltage(dacIndex.V_POWER_C)[0])
        self.getPower("C")

        self.spinBoxVD.setValue(self.det.getVoltage(dacIndex.V_POWER_D)[0])
        self.getPower("D")

        self.spinBoxVIO.setValue(self.det.getVoltage(dacIndex.V_POWER_IO)[0])
        self.getPower("IO")

        self.spinBoxVCHIP.setValue(self.det.getVoltage(dacIndex.V_POWER_CHIP)[0])


    def refresh_tab_sense(self):
        for i in range(8):
            self.updateSense(i)
        self.updateTemperature()


    def refresh_tab_signals(self):
        # For initializing the Out Status
        self.set_IO_Out()

        # For initializing DBit
        n_bits = self.det.rx_dbitlist
        for i in list(n_bits):
            getattr(self, f"checkBoxBIT{i}DB").setChecked(True)

        # Initialization IO Control Register
        self.lineEditBoxIOControl.setText(hex(self.det.patioctrl))
        self.spinBoxDBitOffset.setValue(self.det.rx_dbitoffset)


    def refresh_tab_adc(self):
        # Initializing the ADC Enable mask
        self.set_ADC_Enable()
        self.lineEditEnable.setText(hex(self.det.adcenable))
        # Initializing the ADC Inversion Mask
        self.set_ADC_Inversion()
        self.lineEditInversion.setText(hex(self.det.adcinvert))
        # Sample per frame
        self.spinBoxAnalog.setValue(self.det.asamples)
        self.spinBoxDigital.setValue(self.det.dsamples)
        if self.det.romode == (readoutMode.ANALOG_ONLY):
            self.spinBoxDigital.setDisabled(True)
            self.comboBoxROMode.setCurrentIndex(0)
        elif self.det.romode == (readoutMode.DIGITAL_ONLY):
            self.spinBoxAnalog.setDisabled(True)
            self.comboBoxROMode.setCurrentIndex(1)
        elif self.det.romode == (readoutMode.ANALOG_AND_DIGITAL):
            self.comboBoxROMode.setCurrentIndex(2)


    def refresh_tab_pattern(self):
        self.spinBoxRunF.setValue(self.det.runclk)
        self.spinBoxADCF.setValue(self.det.adcclk)
        self.spinBoxADCPhase.setValue(self.det.adcphase)
        self.spinBoxADCPipeline.setValue(self.det.adcpipeline)
        self.spinBoxDBITF.setValue(self.det.dbitclk)
        self.spinBoxDBITPhase.setValue(self.det.dbitphase)
        self.spinBoxDBITPipeline.setValue(self.det.dbitpipeline)

        # TODO yet to decide on hex or int
        self.lineEditStartAddress.setText(hex((self.det.patlimits)[0]))
        self.lineEditStopAddress.setText(hex((self.det.patlimits)[1]))
        # For Wait time and Wait address
        for i in range(6):
            lineEditWait = getattr(self, f"lineEditWait{i}Address")
            spinBoxWait = getattr(self, f"spinBoxWait{i}")
            lineEditWait.setText(hex(self.det.patwait[i]))
            spinBoxWait.setValue(self.det.patwaittime[i])

        for i in range(6):
            # For Loop repetitions
            spinBoxLoop = getattr(self, f"spinBoxLoop{i}")
            spinBoxLoop.setValue(self.det.patnloop[i])
            # For Loop start address
            lineEditLoopStart = getattr(self, f"lineEditLoop{i}Start")
            lineEditLoopStart.setText(hex((self.det.patloop[i])[0]))
            # For loop stop address
            lineEditLoopStop = getattr(self, f"lineEditLoop{i}Stop")
            lineEditLoopStop.setText(hex((self.det.patloop[i])[1]))
        # For compiler
        self.lineEditCompiler.setText("python")
        
    def refresh_tab_acquisition(self):
        # Updating values for patterns
        self.spinBoxFrames.setValue(self.det.frames)

        # Converting to right time unit for period
        tPeriod = self.det.period
        if tPeriod < 100e-9:
            self.comboBoxTime.setCurrentIndex(3)
            periodTime = tPeriod / 1e-9
            self.spinBoxPeriod.setValue(periodTime)
        elif tPeriod < 100e-6:
            self.comboBoxTime.setCurrentIndex(2)
            periodTime1 = tPeriod / 1e-6
            self.spinBoxPeriod.setValue(periodTime1)
        elif tPeriod < 100e-3:
            self.comboBoxTime.setCurrentIndex(1)
            periodTime0 = tPeriod / 1e-3
            self.spinBoxPeriod.setValue(periodTime0)
        else:
            self.comboBoxTime.setCurrentIndex(0)
            self.spinBoxPeriod.setValue(tPeriod)

        self.spinBoxTriggers.setValue(self.det.triggers)

        #Output Settings
        self.checkBoxFileWrite.setChecked(self.det.fwrite)
        self.lineEditFileName.setText(self.det.fname)
        self.lineEditFilePath.setText(str(self.det.fpath))
        self.spinBoxIndex.setValue(self.det.findex)

        self.updateDetectorStatus()
        

    def setup_zmq(self):
        self.det.rx_zmqstream = 1
        self.zmqIp = self.det.rx_zmqip
        self.zmqport = self.det.rx_zmqport
        self.zmq_stream = self.det.rx_zmqstream

        self.context = zmq.Context()
        self.socket = self.context.socket(zmq.SUB)
        self.socket.connect(f"tcp://{self.zmqIp}:{self.zmqport}")
        self.socket.subscribe("")


    def setup_qtimer(self):
        #To check detector status
        self.statusTimer = QtCore.QTimer()
        self.statusTimer.timeout.connect(self.updateDetectorStatus)

        #To auto trigger the read
        self.read_timer =  QtCore.QTimer()
        self.read_timer.timeout.connect(self.read_zmq)


    def connect_ui(self):
               # Plotting the data
        # For the action options in app
        # TODO Only add the components of action options
        # Show info
        self.actionInfo.triggered.connect(self.showInfo)
        self.actionOpen.triggered.connect(self.openFile)

        # For DACs tab
        # TODO Only add the components of DACs tab
        n_dacs = len(self.det.daclist)
        for i in range(n_dacs):
            getattr(self, f"spinBoxDAC{i}").editingFinished.connect(
                partial(self.setDAC, i)
            )
            getattr(self, f"checkBoxDAC{i}").clicked.connect(partial(self.setDAC, i))
            getattr(self, f"checkBoxDAC{i}mV").clicked.connect(partial(self.setDAC, i))

        self.comboBoxADC.activated.connect(self.setADC)
        self.spinBoxHighVoltage.editingFinished.connect(self.setHighVoltage)
        self.checkBoxHighVoltage.clicked.connect(self.setHighVoltage)

        # For Power Supplies tab
        # TODO Only add the components of Power supplies tab
        self.spinBoxVA.editingFinished.connect(partial(self.setPower, "A"))
        self.checkBoxVA.clicked.connect(partial(self.setPower, "A"))
        self.spinBoxVB.editingFinished.connect(partial(self.setPower, "B"))
        self.checkBoxVB.clicked.connect(partial(self.setPower, "B"))
        self.spinBoxVC.editingFinished.connect(partial(self.setPower, "C"))
        self.checkBoxVC.clicked.connect(partial(self.setPower, "C"))
        self.spinBoxVD.editingFinished.connect(partial(self.setPower, "D"))
        self.checkBoxVD.clicked.connect(partial(self.setPower, "D"))
        self.spinBoxVIO.editingFinished.connect(partial(self.setPower, "IO"))
        self.checkBoxVIO.clicked.connect(partial(self.setPower, "IO"))

        # For Sense Tab
        # TODO Only add the components of Sense tab
        self.pushButtonSense0.clicked.connect(partial(self.updateSense, 0))
        self.pushButtonSense1.clicked.connect(partial(self.updateSense, 1))
        self.pushButtonSense2.clicked.connect(partial(self.updateSense, 2))
        self.pushButtonSense3.clicked.connect(partial(self.updateSense, 3))
        self.pushButtonSense4.clicked.connect(partial(self.updateSense, 4))
        self.pushButtonSense5.clicked.connect(partial(self.updateSense, 5))
        self.pushButtonSense6.clicked.connect(partial(self.updateSense, 6))
        self.pushButtonSense7.clicked.connect(partial(self.updateSense, 7))
        self.pushButtonTemp.clicked.connect(self.updateTemperature)

        # For Signals Tab
        # TODO Only add the components of Signals tab
        for i in range(64):
            getattr(self, f"checkBoxBIT{i}DB").clicked.connect(partial(self.dbit, i))

        for i in range(64):
            getattr(self, f"checkBoxBIT{i}Out").clicked.connect(partial(self.IOout, i))

        self.spinBoxDBitOffset.editingFinished.connect(self.setDbitOffset)
        self.pushButtonBIT0.clicked.connect(self.colorBIT0)
        self.pushButtonBIT1.clicked.connect(self.colorBIT1)
        self.pushButtonBIT2.clicked.connect(self.colorBIT2)
        self.pushButtonBIT3.clicked.connect(self.colorBIT3)
        self.pushButtonBIT4.clicked.connect(self.colorBIT4)
        self.pushButtonBIT5.clicked.connect(self.colorBIT5)
        self.pushButtonBIT6.clicked.connect(self.colorBIT6)
        self.pushButtonBIT7.clicked.connect(self.colorBIT7)
        self.pushButtonBIT8.clicked.connect(self.colorBIT8)
        self.pushButtonBIT9.clicked.connect(self.colorBIT9)
        self.pushButtonBIT10.clicked.connect(self.colorBIT10)
        self.pushButtonBIT11.clicked.connect(self.colorBIT11)
        self.pushButtonBIT12.clicked.connect(self.colorBIT12)
        self.pushButtonBIT13.clicked.connect(self.colorBIT13)
        self.pushButtonBIT14.clicked.connect(self.colorBIT14)
        self.pushButtonBIT15.clicked.connect(self.colorBIT15)
        self.pushButtonBIT16.clicked.connect(self.colorBIT16)
        self.pushButtonBIT17.clicked.connect(self.colorBIT17)
        self.pushButtonBIT18.clicked.connect(self.colorBIT18)
        self.pushButtonBIT19.clicked.connect(self.colorBIT19)
        self.pushButtonBIT20.clicked.connect(self.colorBIT20)
        self.pushButtonBIT21.clicked.connect(self.colorBIT21)
        self.pushButtonBIT22.clicked.connect(self.colorBIT22)
        self.pushButtonBIT23.clicked.connect(self.colorBIT23)
        self.pushButtonBIT24.clicked.connect(self.colorBIT24)
        self.pushButtonBIT25.clicked.connect(self.colorBIT25)
        self.pushButtonBIT26.clicked.connect(self.colorBIT26)
        self.pushButtonBIT27.clicked.connect(self.colorBIT27)
        self.pushButtonBIT28.clicked.connect(self.colorBIT28)
        self.pushButtonBIT29.clicked.connect(self.colorBIT29)
        self.pushButtonBIT30.clicked.connect(self.colorBIT30)
        self.pushButtonBIT31.clicked.connect(self.colorBIT31)
        self.pushButtonBIT32.clicked.connect(self.colorBIT32)
        self.pushButtonBIT33.clicked.connect(self.colorBIT33)
        self.pushButtonBIT34.clicked.connect(self.colorBIT34)
        self.pushButtonBIT35.clicked.connect(self.colorBIT35)
        self.pushButtonBIT36.clicked.connect(self.colorBIT36)
        self.pushButtonBIT37.clicked.connect(self.colorBIT37)
        self.pushButtonBIT38.clicked.connect(self.colorBIT38)
        self.pushButtonBIT39.clicked.connect(self.colorBIT39)
        self.pushButtonBIT40.clicked.connect(self.colorBIT40)
        self.pushButtonBIT41.clicked.connect(self.colorBIT41)
        self.pushButtonBIT42.clicked.connect(self.colorBIT42)
        self.pushButtonBIT43.clicked.connect(self.colorBIT43)
        self.pushButtonBIT44.clicked.connect(self.colorBIT44)
        self.pushButtonBIT45.clicked.connect(self.colorBIT45)
        self.pushButtonBIT46.clicked.connect(self.colorBIT46)
        self.pushButtonBIT47.clicked.connect(self.colorBIT47)
        self.pushButtonBIT48.clicked.connect(self.colorBIT48)
        self.pushButtonBIT49.clicked.connect(self.colorBIT49)
        self.pushButtonBIT50.clicked.connect(self.colorBIT50)
        self.pushButtonBIT51.clicked.connect(self.colorBIT51)
        self.pushButtonBIT52.clicked.connect(self.colorBIT52)
        self.pushButtonBIT53.clicked.connect(self.colorBIT53)
        self.pushButtonBIT54.clicked.connect(self.colorBIT54)
        self.pushButtonBIT55.clicked.connect(self.colorBIT55)
        self.pushButtonBIT56.clicked.connect(self.colorBIT56)
        self.pushButtonBIT57.clicked.connect(self.colorBIT57)
        self.pushButtonBIT58.clicked.connect(self.colorBIT58)
        self.pushButtonBIT59.clicked.connect(self.colorBIT59)
        self.pushButtonBIT60.clicked.connect(self.colorBIT60)
        self.pushButtonBIT61.clicked.connect(self.colorBIT61)
        self.pushButtonBIT62.clicked.connect(self.colorBIT62)
        self.pushButtonBIT63.clicked.connect(self.colorBIT63)

        # For ADCs Tab
        # TODO Only add the components of ADCs tab
        for i in range(32):
            getattr(self, f"checkBoxADC{i}Inv").clicked.connect(
                partial(self.ADCInvert, i)
            )

        for i in range(32):
            getattr(self, f"checkBoxADC{i}En").clicked.connect(
                partial(self.ADCEnable, i)
            )

        self.pushButtonADC0.clicked.connect(self.colorADC0)
        self.pushButtonADC1.clicked.connect(self.colorADC1)
        self.pushButtonADC2.clicked.connect(self.colorADC2)
        self.pushButtonADC3.clicked.connect(self.colorADC3)
        self.pushButtonADC4.clicked.connect(self.colorADC4)
        self.pushButtonADC5.clicked.connect(self.colorADC5)
        self.pushButtonADC6.clicked.connect(self.colorADC6)
        self.pushButtonADC7.clicked.connect(self.colorADC7)
        self.pushButtonADC8.clicked.connect(self.colorADC8)
        self.pushButtonADC9.clicked.connect(self.colorADC9)
        self.pushButtonADC10.clicked.connect(self.colorADC10)
        self.pushButtonADC11.clicked.connect(self.colorADC11)
        self.pushButtonADC12.clicked.connect(self.colorADC12)
        self.pushButtonADC13.clicked.connect(self.colorADC13)
        self.pushButtonADC14.clicked.connect(self.colorADC14)
        self.pushButtonADC15.clicked.connect(self.colorADC15)
        self.pushButtonADC16.clicked.connect(self.colorADC16)
        self.pushButtonADC17.clicked.connect(self.colorADC17)
        self.pushButtonADC18.clicked.connect(self.colorADC18)
        self.pushButtonADC19.clicked.connect(self.colorADC19)
        self.pushButtonADC20.clicked.connect(self.colorADC20)
        self.pushButtonADC21.clicked.connect(self.colorADC21)
        self.pushButtonADC22.clicked.connect(self.colorADC22)
        self.pushButtonADC23.clicked.connect(self.colorADC23)
        self.pushButtonADC24.clicked.connect(self.colorADC24)
        self.pushButtonADC25.clicked.connect(self.colorADC25)
        self.pushButtonADC26.clicked.connect(self.colorADC26)
        self.pushButtonADC27.clicked.connect(self.colorADC27)
        self.pushButtonADC28.clicked.connect(self.colorADC28)
        self.pushButtonADC29.clicked.connect(self.colorADC29)
        self.pushButtonADC30.clicked.connect(self.colorADC30)
        self.pushButtonADC31.clicked.connect(self.colorADC31)
        self.pushButtonAll15.clicked.connect(self.all_0_15)
        self.pushButtonNone15.clicked.connect(self.none_0_15)
        self.pushButtonAll16.clicked.connect(self.all_16_31)
        self.pushButtonNone16.clicked.connect(self.none_16_31)
        self.pushButtonAll.clicked.connect(self.enable_mask_all)
        self.pushButtonNone.clicked.connect(self.enable_mask_none)

        # For Pattern Tab
        # TODO Only add the components of Pattern tab
        self.pushButtonCompiler.clicked.connect(self.setCompiler)
        self.pushButtonPattern.clicked.connect(self.setPattern)
        self.spinBoxFrames.editingFinished.connect(self.setFrames)
        self.spinBoxPeriod.editingFinished.connect(self.setPeriod)
        self.comboBoxTime.activated.connect(self.setPeriod)
        self.spinBoxTriggers.editingFinished.connect(self.setTriggers)
        self.spinBoxRunF.editingFinished.connect(self.setRunFrequency)
        self.spinBoxADCF.editingFinished.connect(self.setADCFrequency)
        self.spinBoxADCPhase.editingFinished.connect(self.setADCPhase)
        self.spinBoxADCPipeline.editingFinished.connect(self.setADCPipeline)
        self.spinBoxDBITF.editingFinished.connect(self.setDBITFrequency)
        self.spinBoxDBITPhase.editingFinished.connect(self.setDBITPhase)
        self.spinBoxDBITPipeline.editingFinished.connect(self.setDBITPipeline)
        for i in range(6):
            getattr(self, f"spinBoxLoop{i}").editingFinished.connect(
                partial(self.setLoop, i)
            )
        for i in range(6):
            getattr(self, f"spinBoxWait{i}").editingFinished.connect(
                partial(self.setWait, i)
            )
        self.spinBoxAnalog.editingFinished.connect(self.setAnalog)
        self.spinBoxDigital.editingFinished.connect(self.setDigital)
        self.comboBoxROMode.activated.connect(self.setReadOut)
        self.pushButtonLoad.clicked.connect(self.loadPattern)

        # For Acquistions Tab
        # TODO Only add the components of Acquistions tab
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
        self.lineEditFileName.editingFinished.connect(self.setFileName)
        self.lineEditFilePath.editingFinished.connect(self.setFilePath)
        self.spinBoxIndex.editingFinished.connect(self.setIndex)
        self.pushButtonStart.clicked.connect(self.acquire)
        self.pushButtonStop.clicked.connect(self.det.stop)
        self.pushButtonReferesh.clicked.connect(self.plotReferesh)
        self.pushButtonBrowse.clicked.connect(self.browseFile)

        

if __name__ == "__main__":
    app = QtWidgets.QApplication(sys.argv)
    main = MainWindow()
    main.show()
    # Run the app
    app.exec_()
