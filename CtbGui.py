from PyQt5 import QtWidgets, QtCore, QtGui, uic
import sys, os
import pyqtgraph as pg
from pyqtgraph import PlotWidget

from functools import partial

from slsdet import Detector, dacIndex, readoutMode

class MainWindow(QtWidgets.QMainWindow):
    def __init__(self, *args, **kwargs):
        pg.setConfigOption('background', (247, 247, 247))
        pg.setConfigOption('foreground', 'k')

        super(MainWindow, self).__init__()
        self.det = Detector()
        uic.loadUi('CtbGui.ui', self)
        self.update_field()

        pen = pg.mkPen(color = (36, 119, 173), width=1) 
        #Plotting the data
        self.curve = self.plotWidget.plot(pen=pen)

        #For the action options in app
        #TODO Only add the components of action options
        #Show info
        self.actionInfo.triggered.connect(self.showInfo)
        self.actionOpen.triggered.connect(self.openFile)

        #For DACs tab
        #TODO Only add the components of DACs tab

        self.spinBoxDAC0.editingFinished.connect(partial(self.setDAC, 0))
        self.checkBoxDAC0.clicked.connect(partial(self.setDAC, 0))
        self.checkBoxDAC0mV.clicked.connect(partial(self.setDAC, 0))
        self.spinBoxDAC1.editingFinished.connect(partial(self.setDAC, 1))
        self.checkBoxDAC1.clicked.connect(partial(self.setDAC, 1))
        self.checkBoxDAC1mV.clicked.connect(partial(self.setDAC, 1))
        self.spinBoxDAC2.editingFinished.connect(partial(self.setDAC, 2))
        self.checkBoxDAC2.clicked.connect(partial(self.setDAC, 2))
        self.checkBoxDAC2mV.clicked.connect(partial(self.setDAC, 2))
        self.spinBoxDAC3.editingFinished.connect(partial(self.setDAC, 3))
        self.checkBoxDAC3.clicked.connect(partial(self.setDAC, 3))
        self.checkBoxDAC3mV.clicked.connect(partial(self.setDAC, 3))
        self.spinBoxDAC4.editingFinished.connect(partial(self.setDAC, 4))
        self.checkBoxDAC4.clicked.connect(partial(self.setDAC, 4))
        self.checkBoxDAC4mV.clicked.connect(partial(self.setDAC, 4))
        self.spinBoxDAC5.editingFinished.connect(partial(self.setDAC, 5))
        self.checkBoxDAC5.clicked.connect(partial(self.setDAC, 5))
        self.checkBoxDAC5mV.clicked.connect(partial(self.setDAC, 5))
        self.spinBoxDAC6.editingFinished.connect(partial(self.setDAC, 6))
        self.checkBoxDAC6.clicked.connect(partial(self.setDAC, 6))
        self.checkBoxDAC6mV.clicked.connect(partial(self.setDAC, 6))
        self.spinBoxDAC7.editingFinished.connect(partial(self.setDAC, 7))
        self.checkBoxDAC7.clicked.connect(partial(self.setDAC, 7))
        self.checkBoxDAC7mV.clicked.connect(partial(self.setDAC, 7))
        self.spinBoxDAC8.editingFinished.connect(partial(self.setDAC, 8))
        self.checkBoxDAC8.clicked.connect(partial(self.setDAC, 8))
        self.checkBoxDAC8mV.clicked.connect(partial(self.setDAC, 8))
        self.spinBoxDAC9.editingFinished.connect(partial(self.setDAC, 9))
        self.checkBoxDAC9.clicked.connect(partial(self.setDAC, 9))
        self.checkBoxDAC9mV.clicked.connect(partial(self.setDAC, 9))
        self.spinBoxDAC10.editingFinished.connect(partial(self.setDAC, 10))
        self.checkBoxDAC10.clicked.connect(partial(self.setDAC, 10))
        self.checkBoxDAC10mV.clicked.connect(partial(self.setDAC, 10))
        self.spinBoxDAC11.editingFinished.connect(partial(self.setDAC, 11))
        self.checkBoxDAC11.clicked.connect(partial(self.setDAC, 11))
        self.checkBoxDAC11mV.clicked.connect(partial(self.setDAC, 11))
        self.spinBoxDAC12.editingFinished.connect(partial(self.setDAC, 12))
        self.checkBoxDAC12.clicked.connect(partial(self.setDAC, 12))
        self.checkBoxDAC12mV.clicked.connect(partial(self.setDAC, 12))
        self.spinBoxDAC13.editingFinished.connect(partial(self.setDAC, 13))
        self.checkBoxDAC13.clicked.connect(partial(self.setDAC, 13))
        self.checkBoxDAC13mV.clicked.connect(partial(self.setDAC, 13))
        self.spinBoxDAC14.editingFinished.connect(partial(self.setDAC, 14))
        self.checkBoxDAC14.clicked.connect(partial(self.setDAC, 14))
        self.checkBoxDAC14mV.clicked.connect(partial(self.setDAC, 14))
        self.spinBoxDAC15.editingFinished.connect(partial(self.setDAC, 15))
        self.checkBoxDAC15.clicked.connect(partial(self.setDAC, 15))
        self.checkBoxDAC15mV.clicked.connect(partial(self.setDAC, 15))
        self.spinBoxDAC16.editingFinished.connect(partial(self.setDAC, 16))
        self.checkBoxDAC16.clicked.connect(partial(self.setDAC, 16))
        self.checkBoxDAC16mV.clicked.connect(partial(self.setDAC, 16))
        self.spinBoxDAC17.editingFinished.connect(partial(self.setDAC, 17))
        self.checkBoxDAC17.clicked.connect(partial(self.setDAC, 17))
        self.checkBoxDAC17mV.clicked.connect(partial(self.setDAC, 17))
        #self.spinBoxADC.editingFinished.connect(self.setADC)
        self.checkBoxADC.clicked.connect(self.setADC)
        self.spinBoxHighVoltage.editingFinished.connect(self.setHighVoltage)
        self.checkBoxHighVoltage.clicked.connect(self.setHighVoltage)

        #For Power Supplies tab
        #TODO Only add the components of Power supplies tab

        self.spinBoxVA.editingFinished.connect(partial(self.setPower, 'A'))
        self.checkBoxVA.clicked.connect(partial(self.setPower, 'A'))
        self.spinBoxVB.editingFinished.connect(partial(self.setPower, 'B'))
        self.checkBoxVB.clicked.connect(partial(self.setPower, 'B'))
        self.spinBoxVC.editingFinished.connect(partial(self.setPower, 'C'))
        self.checkBoxVC.clicked.connect(partial(self.setPower, 'C'))
        self.spinBoxVD.editingFinished.connect(partial(self.setPower, 'D'))
        self.checkBoxVD.clicked.connect(partial(self.setPower, 'D'))
        self.spinBoxVIO.editingFinished.connect(partial(self.setPower, 'IO'))
        self.checkBoxVIO.clicked.connect(partial(self.setPower, 'IO'))

        #For Sense Tab
        #TODO Only add the components of Sense tab

        self.pushButtonSense0.clicked.connect(partial(self.updateSense, 0))
        self.pushButtonSense1.clicked.connect(partial(self.updateSense, 1))
        self.pushButtonSense2.clicked.connect(partial(self.updateSense, 2))
        self.pushButtonSense3.clicked.connect(partial(self.updateSense, 3))
        self.pushButtonSense4.clicked.connect(partial(self.updateSense, 4))
        self.pushButtonSense5.clicked.connect(partial(self.updateSense, 5))
        self.pushButtonSense6.clicked.connect(partial(self.updateSense, 6))
        self.pushButtonSense7.clicked.connect(partial(self.updateSense, 7))
        self.pushButtonTemp.clicked.connect(self.updateTemperature)

        #For Signals Tab
        #TODO Only add the components of Signals tab
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


        #For ADCs Tab
        #TODO Only add the components of ADCs tab
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
        self.pushButtonAll15.clicked.connect(self.all15)
        self.pushButtonNone15.clicked.connect(self.none15)
        self.pushButtonAll16.clicked.connect(self.all16)
        self.pushButtonNone16.clicked.connect(self.none16)
        self.pushButtonAll.clicked.connect(self.all)
        self.pushButtonNone.clicked.connect(self.none)

        #For Pattern Tab
        #TODO Only add the components of Pattern tab
        self.pushButtonCompiler.clicked.connect(self.getCompiler)
        self.pushButtonPattern.clicked.connect(self.getPattern)
        self.spinBoxFrames.editingFinished.connect(self.getFrames)
        self.spinBoxPeriod.editingFinished.connect(self.getPeriod)
        self.comboBoxTime.activated.connect(self.getPeriod)
        self.spinBoxTriggers.editingFinished.connect(self.getTriggers)
        self.spinBoxRunF.editingFinished.connect(self.getRunFrequency)
        self.spinBoxADCF.editingFinished.connect(self.getADCFrequency)
        self.spinBoxADCPhase.editingFinished.connect(self.getADCPhase)
        self.spinBoxADCPipeline.editingFinished.connect(self.getADCPipeline)
        self.spinBoxDBITF.editingFinished.connect(self.getDBITFrequency)
        self.spinBoxDBITPhase.editingFinished.connect(self.getDBITPhase)
        self.spinBoxDBITPipeline.editingFinished.connect(self.getDBITPipeline)
        self.spinBoxStartAddress.editingFinished.connect(self.setPatLimits)
        self.spinBoxStopAddress.editingFinished.connect(self.setPatLimits)
        self.spinBoxLoop0.editingFinished.connect(self.getLoop0)
        self.spinBoxLoop1.editingFinished.connect(self.getLoop1)
        self.spinBoxLoop2.editingFinished.connect(self.getLoop2)
        self.spinBoxLoop3.editingFinished.connect(self.getLoop3)
        self.spinBoxLoop4.editingFinished.connect(self.getLoop4)
        self.spinBoxLoop5.editingFinished.connect(self.getLoop5)
        self.spinBoxLoop0Start.editingFinished.connect(self.getLoop0StartStop)
        self.spinBoxLoop1Start.editingFinished.connect(self.getLoop1StartStop)
        self.spinBoxLoop2Start.editingFinished.connect(self.getLoop2StartStop)
        self.spinBoxLoop3Start.editingFinished.connect(self.getLoop3StartStop)
        self.spinBoxLoop4Start.editingFinished.connect(self.getLoop4StartStop)
        self.spinBoxLoop5Start.editingFinished.connect(self.getLoop5StartStop)
        self.spinBoxLoop0Stop.editingFinished.connect(self.getLoop0StartStop)
        self.spinBoxLoop1Stop.editingFinished.connect(self.getLoop1StartStop)
        self.spinBoxLoop2Stop.editingFinished.connect(self.getLoop2StartStop)
        self.spinBoxLoop3Stop.editingFinished.connect(self.getLoop3StartStop)
        self.spinBoxLoop4Stop.editingFinished.connect(self.getLoop4StartStop)
        self.spinBoxLoop5Stop.editingFinished.connect(self.getLoop5StartStop)
        self.spinBoxWait0.editingFinished.connect(self.getWait0)
        self.spinBoxWait1.editingFinished.connect(self.getWait1)
        self.spinBoxWait2.editingFinished.connect(self.getWait2)
        self.spinBoxAnalog.editingFinished.connect(self.getAnalog)
        self.spinBoxDigital.editingFinished.connect(self.getDigital)
        self.comboBoxROMode.activated.connect(self.getReadOut)
        self.pushButtonLoad.clicked.connect(self.loadPattern)

        
        #For Acquistions Tab
        #TODO Only add the components of Acquistions tab
        self.radioButtonNoPlot.clicked.connect(self.plotOptions)
        self.radioButtonWaveform.clicked.connect(self.plotOptions)
        self.radioButtonDistribution.clicked.connect(self.plotOptions)
        self.radioButtonImage.clicked.connect(self.plotOptions)
        self.comboBoxPlot.activated.connect(self.plotOptions)
        self.spinBoxSerialOffset.editingFinished.connect(self.getSerialOffset)
        self.spinBoxNCount.editingFinished.connect(self.getNCounter)
        self.spinBoxDynamicRange.editingFinished.connect(self.getDynamicRange)
        self.spinBoxImageX.editingFinished.connect(self.getImageX)
        self.spinBoxImageY.editingFinished.connect(self.getImageY)
        self.checkBoxAcquire.clicked.connect(self.getPedestal)
        self.checkBoxSubtract.clicked.connect(self.getPedestal)
        self.checkBoxCommonMode.clicked.connect(self.getPedestal)
        self.pushButtonReset.clicked.connect(self.resetPedestal)
        self.checkBoxRaw.clicked.connect(self.getRawData)
        self.spinBoxRawMin.editingFinished.connect(self.getRawData)
        self.spinBoxRawMax.editingFinished.connect(self.getRawData)
        self.checkBoxPedestal.clicked.connect(self.getPedestalSubtract)
        self.spinBoxPedestalMin.editingFinished.connect(self.getPedestalSubtract)
        self.spinBoxPedestalMax.editingFinished.connect(self.getPedestalSubtract)
        self.spinBoxFit.editingFinished.connect(self.getFitADC)
        self.spinBoxPlot.editingFinished.connect(self.getPlotBit)
        self.lineEditFileName.editingFinished.connect(self.setFileName)
        self.lineEditFilePath.editingFinished.connect(self.setFilePath)
        self.spinBoxIndex.editingFinished.connect(self.setIndex)
        self.spinBoxMeasurements.editingFinished.connect(self.getMeasurements)
        self.pushButtonStart.clicked.connect(self.acquire)
        self.pushButtonReferesh.clicked.connect(self.plotReferesh)
        self.pushButtonBrowse.clicked.connect(self.browseFile)

    #For Action options function 
    #TODO Only add the components of action option+ functions    
    #Function to show info
    def showInfo(self):
        msg = QtWidgets.QMessageBox()
        msg.setWindowTitle("Info about CTB")
        msg.setText("This Gui is for chip test board.\n Current Phase: Development")
        x = msg.exec_()

    #Function to open file
    def openFile(self):
        response = QtWidgets.QFileDialog.getOpenFileName(
            parent=self,
            caption='Select a file to open',
            directory=os.getcwd(),
            #filter='README (*.md *.ui)'
        )
        if (response[0]):
            print(response[0])
    #For the DACs tab functions
    # TODO Only add DACs tab functions

    def setDAC(self, i):
        checkBoxDac = getattr(self, f'checkBoxDAC{i}')
        checkBoxmV = getattr(self, f'checkBoxDAC{i}mV')
        spinBoxDac = getattr(self,f'spinBoxDAC{i}')
        dac  = getattr(dacIndex, f'DAC_{i}')
        dacLabel = getattr(self, f'labelDAC{i}')

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

    #TODO yet to implement the ADC and HV
    def setADC(self):
        if self.checkBoxADC.isChecked():
            ADCValues = self.spinBoxADC.value()
            self.det.setDAC(dacIndex.ADC_VPP, ADCValues)
            self.spinBoxADC.setDisabled(False)
        else:
            self.det.setDAC(dacIndex.ADC_VPP, 0)
            self.spinBoxADC.setDisabled(True)


    def setHighVoltage(self):
        if self.checkBoxHighVoltage.isChecked():
            HVValues = self.spinBoxHighVoltage.value()
            self.det.setHighVoltage(HVValues)
            self.spinBoxHighVoltage.setDisabled(False)
        else:
            self.det.setHighVoltage(0)
            self.spinBoxHighVoltage.setDisabled(True)
        
    
    #For Power Supplies Tab functions
    #TODO Only add the components of Power Supplies tab functions
    def setPower(self, i):
        checkBox = getattr(self, f'checkBoxV{i}')
        spinBox = getattr(self, f'spinBoxV{i}')
        power = getattr(dacIndex, f'V_POWER_{i}')
        label = getattr(self, f'labelV{i}')

        if checkBox.isChecked():
            self.det.setVoltage(power, spinBox.value())
            spinBox.setDisabled(False)
        else:
            self.det.setVoltage(power, 0)
            spinBox.setDisabled(True)
        label.setText(str(self.det.getVoltage(power)[0]))

    #For Sense Tab functions
    #TODO Only add the components of Sense tab functions
    def updateSense(self, i):
        slowADC = getattr(dacIndex, f'SLOW_ADC{i}')
        label = getattr(self, f'labelSense{i}_2')
        sense0 = self.det.getSlowADC(slowADC)
        label.setText(str(sense0[0]))

    def updateTemperature(self):
        sense0 = self.det.getTemperature(dacIndex.SLOW_ADC_TEMP)
        self.labelTemp_2.setText(str(sense0[0]))

    #For Signals Tab functions
    #TODO Only add the components of Signals tab functions
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

    #For ADCs Tab functions
    #TODO Only add the components of ADCs tab functions
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

    def all15(self):
        print("all 0-15")

    def none15(self):
        print("none 0-15")

    def all16(self):
        print("all 16-31")

    def none16(self):
        print("None 16-13")

    def all(self):
        print("all ")

    def none(self):
        print("None")


    #For Pattern Tab functions
    #TODO Only add the components of Pattern tab functions
    def getCompiler(self):
        response = QtWidgets.QFileDialog.getOpenFileName(
            parent=self,
            caption='Select a compiler file',
            directory=os.getcwd(),
            #filter='README (*.md *.ui)'
        )
        if (response[0]):
            self.lineEditCompiler.setText(response[0])

    def getPattern(self):
        response = QtWidgets.QFileDialog.getOpenFileName(
            parent=self,
            caption='Select a pattern file',
            directory=os.getcwd(),
            #filter='README (*.md *.ui)'
        )
        if (response[0]):
            self.lineEditPattern.setText(response[0])

    def getFrames(self):
        self.det.frames = self.spinBoxFrames.value()

    def getPeriod(self):
        match self.comboBoxTime.currentIndex():
            case 0:
                unit = 1
            case 1:
                unit = 1e-3
            case 2:
                unit = 1e-6
            case 3:
                unit = 1e-9
        self.det.period = self.spinBoxPeriod.value()*unit

    def getTriggers(self):
        self.det.triggers = self.spinBoxTriggers.value()

    def getRunFrequency(self):
        self.det.runclk = self.spinBoxRunF.value()

    def getADCFrequency(self):
        self.det.adcclk = self.spinBoxADCF.value()

    def getADCPhase(self):
        self.det.adcphase = self.spinBoxADCPhase.value()

    def getADCPipeline(self):
        self.det.adcpipeline = self.spinBoxADCPipeline.value()

    def getDBITFrequency(self):
        self.det.dbitclk = self.spinBoxDBITF.value()

    def getDBITPhase(self):
        self.det.dbitphase = self.spinBoxDBITPhase.value()

    def getDBITPipeline(self):
        self.det.dbitpipeline = self.spinBoxDBITPipeline.value()

    def setPatLimits(self):
        self.det.patlimits = (self.spinBoxStartAddress.value(), self.spinBoxStopAddress.value())

    def getLoop0(self):
        self.det.patnloop[0] = self.spinBoxLoop0.value()

    def getLoop1(self):
        self.det.patnloop[1] = self.spinBoxLoop1.value()

    def getLoop2(self):
        self.det.patnloop[2] = self.spinBoxLoop2.value()

    def getLoop3(self):
        self.det.patnloop[3] = self.spinBoxLoop3.value()

    def getLoop4(self):
        self.det.patnloop[4] = self.spinBoxLoop4.value()

    def getLoop5(self):
        self.det.patnloop[5] = self.spinBoxLoop5.value()

    def getLoop0StartStop(self):
        self.det.patloop [0] = (self.spinBoxLoop0Start.value(), self.spinBoxLoop0Stop.value())

    def getLoop1StartStop(self):
        self.det.patloop [1] = (self.spinBoxLoop1Start.value(), self.spinBoxLoop1Stop.value())

    def getLoop2StartStop(self):
        self.det.patloop [2] = (self.spinBoxLoop2Start.value(), self.spinBoxLoop2Stop.value())

    def getLoop3StartStop(self):
        self.det.patloop [3] = (self.spinBoxLoop3Start.value(), self.spinBoxLoop3Stop.value())

    def getLoop4StartStop(self):
        self.det.patloop [4] = (self.spinBoxLoop4Start.value(), self.spinBoxLoop4Stop.value())

    def getLoop5StartStop(self):
        self.det.patloop [5] = (self.spinBoxLoop5Start.value(), self.spinBoxLoop5Stop.value())

    def getWait0(self):
        self.det.patwaittime0 = self.spinBoxWait0.value()

    def getWait1(self):
        self.det.patwaittime1 = self.spinBoxWait1.value()

    def getWait2(self):
        self.det.patwaittime2 = self.spinBoxWait2.value()

    def getAnalog(self):
        self.det.asamples = self.spinBoxAnalog.value()

    def getDigital(self):
        self.det.dsamples = self.spinBoxDigital.value()

    def getReadOut(self):
        match self.comboBoxROMode.currentIndex():
            case 0:
                self.det.romode = readoutMode.ANALOG_ONLY
                self.spinBoxDigital.setDisabled(True)
                self.spinBoxAnalog.setDisabled(False)
            case 1:
                self.det.romode = readoutMode.DIGITAL_ONLY
                self.spinBoxAnalog.setDisabled(True)
                self.spinBoxDigital.setDisabled(False)
            case 2:
                self.det.romode = readoutMode.ANALOG_AND_DIGITAL
                self.spinBoxDigital.setDisabled(False)
                self.spinBoxAnalog.setDisabled(False)

    def loadPattern(self):
        print("loading pattern")
    
    #For Acquistions Tab functions
    #TODO Only add the components of Acquistions tab functions
    def plotOptions(self):
        print("plot options")

    def getSerialOffset(self):
        print("SerialOffSet")

    def getNCounter(self):
        print("plot options")

    def getDynamicRange(self):
        print("plot options")

    def getImageX(self):
        print("plot options")

    def getImageY(self):
        print("plot options")

    def getPedestal(self):
        print("plot options")

    def resetPedestal(self):
        print("plot options")

    def getRawData(self):
        print("plot options")

    def getPedestalSubtract(self):
        print("plot options")

    def getFitADC(self):
        print("plot options")

    def getPlotBit(self):
        print("plot options")

    def setFileName(self):
        print("plot options")

    def setFilePath(self):
        print("plot options")

    def setIndex(self):
        print("plot options")

    def getMeasurements(self):
        print("plot options")

    def acquire(self):
        print("plot options")
        self.spinBoxMeasurements.stepUp()
        if self.radioButtonYes.isChecked():
            self.spinBoxIndex.stepUp()
            print ('random')
            
    def plotReferesh(self):
        print("plot options")

    def browseFile(self):
        response = QtWidgets.QFileDialog.getSaveFileName(
            parent=self,
            caption='Select a file to save the Output',
            directory=os.getcwd(),
            
            #filter='README (*.md *.ui)'
        )
        if (response[0]):
            self.lineEditFilePath.setText(response[0])


    #For other functios
    #TODO Add other functions which will be reused 
    def showPalette(self, button):
        color = QtWidgets.QColorDialog.getColor()
        if color.isValid():
            button.setStyleSheet("background-color: %s" % color.name())
            #get the RGB Values
            print(color.getRgb())

    #Getting the checkbox status
    def getDac(self, i):
        checkBox = getattr(self, f'checkBoxDAC{i}')
        spinBox = getattr(self, f'spinBoxDAC{i}')
        dac  = getattr(dacIndex, f'DAC_{i}')
        if (self.det.getDAC(dac)[0]) == -100:
            spinBox.setDisabled(True)
        else:
            checkBox.setChecked(True)

    def getPower(self, i):
        spinBox = getattr(self, f'spinBoxV{i}')
        dac  = getattr(dacIndex, f'V_POWER_{i}')
        checkBox = getattr(self, f'checkBoxV{i}')

        if (self.det.getVoltage(dac)[0]) == 0:
            spinBox.setDisabled(True)
        else:
            checkBox.setChecked(True)

    #updating fields with values 
    def update_field(self):
        #Getting dac Name
        self.checkBoxDAC0.setText(self.det.getDacNames()[0])
        self.checkBoxDAC1.setText(self.det.getDacNames()[1])
        self.checkBoxDAC2.setText(self.det.getDacNames()[2])
        self.checkBoxDAC3.setText(self.det.getDacNames()[3])
        self.checkBoxDAC4.setText(self.det.getDacNames()[4])
        self.checkBoxDAC5.setText(self.det.getDacNames()[5])
        self.checkBoxDAC6.setText(self.det.getDacNames()[6])
        self.checkBoxDAC7.setText(self.det.getDacNames()[7])
        self.checkBoxDAC8.setText(self.det.getDacNames()[8])
        self.checkBoxDAC9.setText(self.det.getDacNames()[9])
        self.checkBoxDAC10.setText(self.det.getDacNames()[10])
        self.checkBoxDAC11.setText(self.det.getDacNames()[11])
        self.checkBoxDAC12.setText(self.det.getDacNames()[12])
        self.checkBoxDAC13.setText(self.det.getDacNames()[13])
        self.checkBoxDAC14.setText(self.det.getDacNames()[14])
        self.checkBoxDAC15.setText(self.det.getDacNames()[15])
        self.checkBoxDAC16.setText(self.det.getDacNames()[16])
        self.checkBoxDAC17.setText(self.det.getDacNames()[17])

        self.labelDAC0.setText(str(self.det.getDAC(dacIndex.DAC_0)[0]))
        self.labelDAC1.setText(str(self.det.getDAC(dacIndex.DAC_1)[0]))
        self.labelDAC2.setText(str(self.det.getDAC(dacIndex.DAC_2)[0]))
        self.labelDAC3.setText(str(self.det.getDAC(dacIndex.DAC_3)[0]))
        self.labelDAC4.setText(str(self.det.getDAC(dacIndex.DAC_4)[0]))
        self.labelDAC5.setText(str(self.det.getDAC(dacIndex.DAC_5)[0]))
        self.labelDAC6.setText(str(self.det.getDAC(dacIndex.DAC_6)[0]))
        self.labelDAC7.setText(str(self.det.getDAC(dacIndex.DAC_7)[0]))
        self.labelDAC8.setText(str(self.det.getDAC(dacIndex.DAC_8)[0]))
        self.labelDAC9.setText(str(self.det.getDAC(dacIndex.DAC_9)[0]))
        self.labelDAC10.setText(str(self.det.getDAC(dacIndex.DAC_10)[0]))
        self.labelDAC11.setText(str(self.det.getDAC(dacIndex.DAC_11)[0]))
        self.labelDAC12.setText(str(self.det.getDAC(dacIndex.DAC_12)[0]))
        self.labelDAC13.setText(str(self.det.getDAC(dacIndex.DAC_13)[0]))
        self.labelDAC14.setText(str(self.det.getDAC(dacIndex.DAC_14)[0]))
        self.labelDAC15.setText(str(self.det.getDAC(dacIndex.DAC_15)[0]))
        self.labelDAC16.setText(str(self.det.getDAC(dacIndex.DAC_16)[0]))
        self.labelDAC17.setText(str(self.det.getDAC(dacIndex.DAC_17)[0]))
        self.labelADC.setText(str(self.det.getDAC(dacIndex.ADC_VPP)[0]))
        self.labelHighVoltage.setText(str(self.det.getHighVoltage()[0]))

        #Getting dac values
        self.spinBoxDAC0.setValue(self.det.getDAC(dacIndex.DAC_0)[0])
        self.getDac(0)
    
        self.spinBoxDAC1.setValue(self.det.getDAC(dacIndex.DAC_1)[0])
        self.getDac(1)
            
        self.spinBoxDAC2.setValue(self.det.getDAC(dacIndex.DAC_2)[0])
        self.getDac(2)

        self.spinBoxDAC3.setValue(self.det.getDAC(dacIndex.DAC_3)[0])
        self.getDac(3)

        self.spinBoxDAC4.setValue(self.det.getDAC(dacIndex.DAC_4)[0])
        self.getDac(4)

        self.spinBoxDAC5.setValue(self.det.getDAC(dacIndex.DAC_5)[0])
        self.getDac(5)

        self.spinBoxDAC6.setValue(self.det.getDAC(dacIndex.DAC_6)[0])
        self.getDac(6)

        self.spinBoxDAC7.setValue(self.det.getDAC(dacIndex.DAC_7)[0])
        self.getDac(7)

        self.spinBoxDAC8.setValue(self.det.getDAC(dacIndex.DAC_8)[0])
        self.getDac(8)

        self.spinBoxDAC9.setValue(self.det.getDAC(dacIndex.DAC_9)[0])
        self.getDac(9)

        self.spinBoxDAC10.setValue(self.det.getDAC(dacIndex.DAC_10)[0])
        self.getDac(10)

        self.spinBoxDAC11.setValue(self.det.getDAC(dacIndex.DAC_11)[0])
        self.getDac(11)

        self.spinBoxDAC12.setValue(self.det.getDAC(dacIndex.DAC_12)[0])
        self.getDac(12)

        self.spinBoxDAC13.setValue(self.det.getDAC(dacIndex.DAC_13)[0])
        self.getDac(13)

        self.spinBoxDAC14.setValue(self.det.getDAC(dacIndex.DAC_14)[0])
        self.getDac(14)

        self.spinBoxDAC15.setValue(self.det.getDAC(dacIndex.DAC_15)[0])
        self.getDac(15)

        self.spinBoxDAC16.setValue(self.det.getDAC(dacIndex.DAC_16)[0])
        self.getDac(16)

        self.spinBoxDAC17.setValue(self.det.getDAC(dacIndex.DAC_17)[0])
        self.getDac(17)

        #self.spinBoxADC.setValue(self.det.getDAC(dacIndex.ADC_VPP)[0])

        self.spinBoxHighVoltage.setValue(self.det.getHighVoltage()[0])
        if (self.det.getHighVoltage()[0]) == 0:
            self.spinBoxHighVoltage.setDisabled(True)
        else:
            self.checkBoxHighVoltage.setChecked(True)

        self.labelVA.setText(str(self.det.getVoltage(dacIndex.V_POWER_A)[0]))
        self.labelVB.setText(str(self.det.getVoltage(dacIndex.V_POWER_B)[0]))
        self.labelVC.setText(str(self.det.getVoltage(dacIndex.V_POWER_C)[0]))
        self.labelVD.setText(str(self.det.getVoltage(dacIndex.V_POWER_D)[0]))
        self.labelVIO.setText(str(self.det.getVoltage(dacIndex.V_POWER_IO)[0]))
        self.labelVCHIP.setText(str(self.det.getVoltage(dacIndex.V_POWER_CHIP)[0]))

        #Updating values for Power Supply
        self.spinBoxVA.setValue(self.det.getVoltage(dacIndex.V_POWER_A)[0])
        self.getPower('A')
        
        self.spinBoxVB.setValue(self.det.getVoltage(dacIndex.V_POWER_B)[0])
        self.getPower('B')
        
        self.spinBoxVC.setValue(self.det.getVoltage(dacIndex.V_POWER_C)[0])
        self.getPower('C')

        self.spinBoxVD.setValue(self.det.getVoltage(dacIndex.V_POWER_D)[0])
        self.getPower('D')

        self.spinBoxVIO.setValue(self.det.getVoltage(dacIndex.V_POWER_IO)[0])
        self.getPower('IO')

        self.spinBoxVCHIP.setValue(self.det.getVoltage(dacIndex.V_POWER_CHIP)[0])

        #Updating values for patterns
        self.spinBoxFrames.setValue(self.det.frames)

        #Converting to right time unit for period
        tPeriod = self.det.period
        if tPeriod < 100e-9:
            self.comboBoxTime.setCurrentIndex(3)
            periodTime = (tPeriod/1e-9)
            self.spinBoxPeriod.setValue(periodTime)
        elif tPeriod < 100e-6:
            self.comboBoxTime.setCurrentIndex(2)
            periodTime1 = (tPeriod/1e-6)
            self.spinBoxPeriod.setValue(periodTime1)
        elif tPeriod < 100e-3:
            self.comboBoxTime.setCurrentIndex(1)
            periodTime0 = (tPeriod/1e-3)
            self.spinBoxPeriod.setValue(periodTime0)
        else:
            self.comboBoxTime.setCurrentIndex(0)
            self.spinBoxPeriod.setValue(tPeriod)

        self.spinBoxTriggers.setValue(self.det.triggers)
        self.spinBoxRunF.setValue(self.det.runclk)
        self.spinBoxADCF.setValue(self.det.adcclk)
        self.spinBoxADCPhase.setValue(self.det.adcphase)
        self.spinBoxADCPipeline.setValue(self.det.adcpipeline)
        self.spinBoxDBITF.setValue(self.det.dbitclk)
        self.spinBoxDBITPhase.setValue(self.det.dbitphase)
        self.spinBoxDBITPipeline.setValue(self.det.dbitpipeline)

        #Sample per frame
        self.spinBoxAnalog.setValue(self.det.asamples)
        self.spinBoxDigital.setValue(self.det.dsamples)
        if (self.det.romode == (readoutMode.ANALOG_ONLY)):
            self.spinBoxDigital.setDisabled(True)
            self.comboBoxROMode.setCurrentIndex(0)
        elif (self.det.romode == (readoutMode.DIGITAL_ONLY)):
            self.spinBoxAnalog.setDisabled(True)
            self.comboBoxROMode.setCurrentIndex(1)
        elif (self.det.romode == (readoutMode.ANALOG_AND_DIGITAL)):
            self.comboBoxROMode.setCurrentIndex(2)

        #TODO yet to decide on hex or int
        self.spinBoxStartAddress.setValue(int((self.det.patlimits)[0]))
        self.spinBoxStopAddress.setValue(int((self.det.patlimits)[1]))

        self.lineEditWait0Address.setText(hex(self.det.patwait [0]))
        self.lineEditWait1Address.setText(hex(self.det.patwait [1]))
        self.lineEditWait2Address.setText(hex(self.det.patwait [2]))

        self.spinBoxWait0.setValue(self.det.patwaittime [0])
        self.spinBoxWait1.setValue(self.det.patwaittime [1])
        self.spinBoxWait2.setValue(self.det.patwaittime [2])

        self.spinBoxLoop0.setValue(self.det.patnloop [0])
        self.spinBoxLoop1.setValue(self.det.patnloop [1])
        self.spinBoxLoop2.setValue(self.det.patnloop [2])
        self.spinBoxLoop3.setValue(self.det.patnloop [3])
        self.spinBoxLoop4.setValue(self.det.patnloop [4])
        self.spinBoxLoop5.setValue(self.det.patnloop [5])

        #TODO yet to decide on hex or int
        self.spinBoxLoop0Start.setValue(int((self.det.patloop [0])[0]))
        self.spinBoxLoop0Stop.setValue(int((self.det.patloop [0])[1]))
        self.spinBoxLoop1Start.setValue(int((self.det.patloop [1])[0]))
        self.spinBoxLoop1Stop.setValue(int((self.det.patloop [1])[1]))
        self.spinBoxLoop2Start.setValue(int((self.det.patloop [2])[0]))
        self.spinBoxLoop2Stop.setValue(int((self.det.patloop [2])[1]))
        self.spinBoxLoop3Start.setValue(int((self.det.patloop [3])[0]))
        self.spinBoxLoop3Stop.setValue(int((self.det.patloop [3])[1]))
        self.spinBoxLoop4Start.setValue(int((self.det.patloop [4])[0]))
        self.spinBoxLoop4Stop.setValue(int((self.det.patloop [4])[1]))
        self.spinBoxLoop5Start.setValue(int((self.det.patloop [5])[0]))
        self.spinBoxLoop5Stop.setValue(int((self.det.patloop [5])[1]))

if __name__ == '__main__':
    app = QtWidgets.QApplication(sys.argv)
    main = MainWindow()
    main.show()
    #Run the app
    app.exec_()
