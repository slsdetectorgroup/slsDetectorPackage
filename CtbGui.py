from turtle import color
from PyQt5 import QtWidgets, QtCore, QtGui, uic
import sys, os
import pyqtgraph as pg
from pyqtgraph import PlotWidget

from slsdet import Detector, dacIndex


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

        self.spinBoxDAC0.editingFinished.connect(self.setDAC0)
        self.spinBoxDAC1.editingFinished.connect(self.setDAC1)
        self.spinBoxDAC2.editingFinished.connect(self.setDAC2)
        self.spinBoxDAC3.editingFinished.connect(self.setDAC3)
        self.spinBoxDAC4.editingFinished.connect(self.setDAC4)
        self.spinBoxDAC5.editingFinished.connect(self.setDAC5)
        self.spinBoxDAC6.editingFinished.connect(self.setDAC6)
        self.spinBoxDAC7.editingFinished.connect(self.setDAC7)
        self.spinBoxDAC8.editingFinished.connect(self.setDAC8)
        self.spinBoxDAC9.editingFinished.connect(self.setDAC9)
        self.spinBoxDAC10.editingFinished.connect(self.setDAC10)
        self.spinBoxDAC11.editingFinished.connect(self.setDAC11)
        self.spinBoxDAC12.editingFinished.connect(self.setDAC12)
        self.spinBoxDAC13.editingFinished.connect(self.setDAC13)
        self.spinBoxDAC14.editingFinished.connect(self.setDAC14)
        self.spinBoxDAC15.editingFinished.connect(self.setDAC15)
        self.spinBoxDAC16.editingFinished.connect(self.setDAC16)
        self.spinBoxDAC17.editingFinished.connect(self.setDAC17)
        self.spinBoxADC.editingFinished.connect(self.setADC)
        self.spinBoxHighVoltage.editingFinished.connect(self.setHighVoltage)

        #For Power Supplies tab
        #TODO Only add the components of Power supplies tab

        self.spinBoxVA.editingFinished.connect(self.setVA)
        self.spinBoxVB.editingFinished.connect(self.setVB)
        self.spinBoxVC.editingFinished.connect(self.setVC)
        self.spinBoxVD.editingFinished.connect(self.setVD)
        self.spinBoxVIO.editingFinished.connect(self.setVIO)
        self.spinBoxVCHIP.editingFinished.connect(self.setVCHIP)

        #For Sense Tab
        #TODO Only add the components of Sense tab

        self.pushButtonSense0.clicked.connect(self.updateSense0)
        self.pushButtonSense1.clicked.connect(self.updateSense1)
        self.pushButtonSense2.clicked.connect(self.updateSense2)
        self.pushButtonSense3.clicked.connect(self.updateSense3)
        self.pushButtonSense4.clicked.connect(self.updateSense4)
        self.pushButtonSense5.clicked.connect(self.updateSense5)
        self.pushButtonSense6.clicked.connect(self.updateSense6)
        self.pushButtonSense7.clicked.connect(self.updateSense7)
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
        self.spinBoxTriggers.editingFinished.connect(self.getTriggers)
        self.spinBoxRunF.editingFinished.connect(self.getRunFrequency)
        self.spinBoxADCF.editingFinished.connect(self.getADCFrequency)
        self.spinBoxADCPhase.editingFinished.connect(self.getADCPhase)
        self.spinBoxADCPipeline.editingFinished.connect(self.getADCPipeline)
        self.spinBoxDBITF.editingFinished.connect(self.getDBITFrequency)
        self.spinBoxDBITPhase.editingFinished.connect(self.getDBITPhase)
        self.spinBoxDBITPipeline.editingFinished.connect(self.getDBITPipeline)
        self.spinBoxStartAddress.editingFinished.connect(self.getStartAddress)
        self.spinBoxStopAddress.editingFinished.connect(self.getStopAddress)
        self.spinBoxLoop0.editingFinished.connect(self.getLoop0)
        self.spinBoxLoop1.editingFinished.connect(self.getLoop1)
        self.spinBoxLoop2.editingFinished.connect(self.getLoop2)
        self.spinBoxLoop3.editingFinished.connect(self.getLoop3)
        self.spinBoxLoop4.editingFinished.connect(self.getLoop4)
        self.spinBoxLoop5.editingFinished.connect(self.getLoop5)
        self.spinBoxLoop0Start.editingFinished.connect(self.getLoop0Start)
        self.spinBoxLoop1Start.editingFinished.connect(self.getLoop1Start)
        self.spinBoxLoop2Start.editingFinished.connect(self.getLoop2Start)
        self.spinBoxLoop3Start.editingFinished.connect(self.getLoop3Start)
        self.spinBoxLoop4Start.editingFinished.connect(self.getLoop4Start)
        self.spinBoxLoop5Start.editingFinished.connect(self.getLoop5Start)
        self.spinBoxLoop0Stop.editingFinished.connect(self.getLoop0Stop)
        self.spinBoxLoop1Stop.editingFinished.connect(self.getLoop1Stop)
        self.spinBoxLoop2Stop.editingFinished.connect(self.getLoop2Stop)
        self.spinBoxLoop3Stop.editingFinished.connect(self.getLoop3Stop)
        self.spinBoxLoop4Stop.editingFinished.connect(self.getLoop4Stop)
        self.spinBoxLoop5Stop.editingFinished.connect(self.getLoop5Stop)
        self.spinBoxWait0.editingFinished.connect(self.getWait0)
        self.spinBoxWait1.editingFinished.connect(self.getWait1)
        self.spinBoxWait2.editingFinished.connect(self.getWait2)
        self.spinBoxWait0Address.editingFinished.connect(self.getWait0Address)
        self.spinBoxWait1Address.editingFinished.connect(self.getWait1Address)
        self.spinBoxWait2Address.editingFinished.connect(self.getWait2Address)
        self.spinBoxAnalog.editingFinished.connect(self.getAnalog)
        self.spinBoxDigital.editingFinished.connect(self.getDigital)
        self.checkBoxAnalog.clicked.connect(self.getReadOut)
        self.checkBoxDigital.clicked.connect(self.getReadOut)
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
        
    def setDAC0(self):
        dacValues = self.spinBoxDAC0.value()
        self.det.setDAC(dacIndex.DAC_0, dacValues)
    
    def setDAC1(self):
        dacValues = self.spinBoxDAC1.value()
        self.det.setDAC(dacIndex.DAC_1, dacValues)
    
    def setDAC2(self):
        dacValues = self.spinBoxDAC2.value()
        self.det.setDAC(dacIndex.DAC_2, dacValues)

    def setDAC3(self):
        dacValues = self.spinBoxDAC3.value()
        self.det.setDAC(dacIndex.DAC_3, dacValues)

    def setDAC4(self):
        dacValues = self.spinBoxDAC4.value()
        self.det.setDAC(dacIndex.DAC_4, dacValues)

    def setDAC5(self):
        dacValues = self.spinBoxDAC5.value()
        self.det.setDAC(dacIndex.DAC_5, dacValues)

    def setDAC6(self):
        dacValues = self.spinBoxDAC6.value()
        self.det.setDAC(dacIndex.DAC_6, dacValues)

    def setDAC7(self):
        dacValues = self.spinBoxDAC7.value()
        self.det.setDAC(dacIndex.DAC_7, dacValues)

    def setDAC8(self):
        dacValues = self.spinBoxDAC8.value()
        self.det.setDAC(dacIndex.DAC_8, dacValues)
    
    def setDAC9(self):
        dacValues = self.spinBoxDAC9.value()
        self.det.setDAC(dacIndex.DAC_9, dacValues)
    
    def setDAC10(self):
        dacValues = self.spinBoxDAC10.value()
        self.det.setDAC(dacIndex.DAC_10, dacValues)
    
    def setDAC11(self):
        dacValues = self.spinBoxDAC11.value()
        self.det.setDAC(dacIndex.DAC_11, dacValues)
    
    def setDAC12(self):
        dacValues = self.spinBoxDAC12.value()
        self.det.setDAC(dacIndex.DAC_12, dacValues)
    
    def setDAC13(self):
        dacValues = self.spinBoxDAC13.value()
        self.det.setDAC(dacIndex.DAC_13, dacValues)

    def setDAC14(self):
        dacValues = self.spinBoxDAC14.value()
        self.det.setDAC(dacIndex.DAC_14, dacValues)

    def setDAC15(self):
        dacValues = self.spinBoxDAC15.value()
        self.det.setDAC(dacIndex.DAC_15, dacValues)

    def setDAC16(self):
        dacValues = self.spinBoxDAC16.value()
        self.det.setDAC(dacIndex.DAC_16, dacValues)

    def setDAC17(self):
        dacValues = self.spinBoxDAC17.value()
        self.det.setDAC(dacIndex.DAC_17, dacValues)
    
    #TODO yet to implement the ADC and HV
    def setADC(self):
        ADCValues = self.spinBoxADC.value()
        self.det.setDAC(dacIndex.ADC_VPP, ADCValues)

    def setHighVoltage(self):
        HVValues = self.spinBoxHighVoltage.value()
        self.det.setDAC(dacIndex.HIGH_VOLTAGE, HVValues)
    
    #For Power Supplies Tab functions
    #TODO Only add the components of Power Supplies tab functions

    def setVA(self):
        dacValues = self.spinBoxVA.value()
        self.det.setDAC(dacIndex.V_POWER_A, dacValues)
    
    def setVB(self):
        dacValues = self.spinBoxVB.value()
        self.det.setDAC(dacIndex.V_POWER_B, dacValues)
    
    def setVC(self):
        dacValues = self.spinBoxVC.value()
        self.det.setDAC(dacIndex.V_POWER_C, dacValues)

    def setVD(self):
        dacValues = self.spinBoxVD.value()
        self.det.setDAC(dacIndex.V_POWER_D, dacValues)

    def setVIO(self):
        dacValues = self.spinBoxVIO.value()
        self.det.setDAC(dacIndex.V_POWER_IO, dacValues)

    def setVCHIP(self):
        dacValues = self.spinBoxVCHIP.value()
        self.det.setDAC(dacIndex.V_POWER_CHIP, dacValues)

    #For Sense Tab functions
    #TODO Only add the components of Sense tab functions

    def updateSense0(self):
        print("Sense 0")
    
    def updateSense1(self):
        print('Sense 1')
    
    def updateSense2(self):
        print('Sense 2')
    
    def updateSense3(self):
        print('Sense 3')

    def updateSense4(self):
        print('Sense 4')
    
    def updateSense5(self):
        print('Sense 5')

    def updateSense6(self):
        print('Sense 6')
    
    def updateSense7(self):
        print('Sense 7')

    def updateTemperature(self):
        print('Temperature')

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
        self.det.period = self.spinBoxPeriod.value()
       
    def getTriggers(self):
        self.det.triggers = self.spinBoxTriggers.value()

    def getRunFrequency(self):
        print("frames")

    def getADCFrequency(self):
        print("frames")

    def getADCPhase(self):
        print("frames")

    def getADCPipeline(self):
        print("frames")

    def getDBITFrequency(self):
        print("frames")

    def getDBITPhase(self):
        print("frames")

    def getDBITPipeline(self):
        print("frames")

    def getStartAddress(self):
        print("frames")

    def getStopAddress(self):
        print("frames")

    def getLoop0(self):
        print("frames")

    def getLoop1(self):
        print("frames")

    def getLoop2(self):
        print("frames")

    def getLoop3(self):
        print("frames")

    def getLoop4(self):
        print("frames")

    def getLoop5(self):
        print("frames")

    def getLoop0Start(self):
        print("frames")

    def getLoop1Start(self):
        print("frames")

    def getLoop2Start(self):
        print("frames")

    def getLoop3Start(self):
        print("frames")

    def getLoop4Start(self):
        print("frames")

    def getLoop5Start(self):
        print("frames")

    def getLoop0Stop(self):
        print("frames")

    def getLoop1Stop(self):
        print("frames")

    def getLoop2Stop(self):
        print("frames")

    def getLoop3Stop(self):
        print("frames")

    def getLoop4Stop(self):
        print("frames")

    def getLoop5Stop(self):
        print("frames")

    def getWait0(self):
        print("frames")

    def getWait1(self):
        print("frames")

    def getWait2(self):
        print("frames")

    def getWait0Address(self):
        print("frames")

    def getWait1Address(self):
        print("frames")

    def getWait2Address(self):
        print("frames")

    def getAnalog(self):
        print("frames")

    def getDigital(self):
        print("frames")

    def getReadOut(self):
        if self.checkBoxAnalog.isChecked():
            print("analog")
        elif self.checkBoxDigital.isChecked():
            print("digital")
        else:
            print("none")

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


    #For other functios
    #TODO Add other functions which will be reused 
    def showPalette(self, button):
        color = QtWidgets.QColorDialog.getColor()
        if color.isValid():
            button.setStyleSheet("background-color: %s" % color.name())
            print(color.name())

    #updating fields with values 
    def update_field(self):
        self.spinBoxDAC0.setValue(self.det.getDAC(dacIndex.DAC_0)[0])
        self.spinBoxDAC1.setValue(self.det.getDAC(dacIndex.DAC_1)[0])
        self.spinBoxDAC2.setValue(self.det.getDAC(dacIndex.DAC_2)[0])
        self.spinBoxDAC3.setValue(self.det.getDAC(dacIndex.DAC_3)[0])
        self.spinBoxDAC4.setValue(self.det.getDAC(dacIndex.DAC_4)[0])
        self.spinBoxDAC5.setValue(self.det.getDAC(dacIndex.DAC_5)[0])
        self.spinBoxDAC6.setValue(self.det.getDAC(dacIndex.DAC_6)[0])
        self.spinBoxDAC7.setValue(self.det.getDAC(dacIndex.DAC_7)[0])
        self.spinBoxDAC8.setValue(self.det.getDAC(dacIndex.DAC_8)[0])
        self.spinBoxDAC9.setValue(self.det.getDAC(dacIndex.DAC_9)[0])
        self.spinBoxDAC10.setValue(self.det.getDAC(dacIndex.DAC_10)[0])
        self.spinBoxDAC11.setValue(self.det.getDAC(dacIndex.DAC_11)[0])
        self.spinBoxDAC12.setValue(self.det.getDAC(dacIndex.DAC_12)[0])
        self.spinBoxDAC13.setValue(self.det.getDAC(dacIndex.DAC_13)[0])
        self.spinBoxDAC14.setValue(self.det.getDAC(dacIndex.DAC_14)[0])
        self.spinBoxDAC15.setValue(self.det.getDAC(dacIndex.DAC_15)[0])
        self.spinBoxDAC16.setValue(self.det.getDAC(dacIndex.DAC_16)[0])
        self.spinBoxDAC17.setValue(self.det.getDAC(dacIndex.DAC_17)[0])
        self.spinBoxADC.setValue(self.det.getDAC(dacIndex.ADC_VPP)[0])
        self.spinBoxHighVoltage.setValue(self.det.getDAC(dacIndex.HIGH_VOLTAGE)[0])

        #Updating values for Power Supply
        self.spinBoxVA.setValue(self.det.getDAC(dacIndex.V_POWER_A)[0])
        self.spinBoxVB.setValue(self.det.getDAC(dacIndex.V_POWER_B)[0])
        self.spinBoxVC.setValue(self.det.getDAC(dacIndex.V_POWER_C)[0])
        self.spinBoxVD.setValue(self.det.getDAC(dacIndex.V_POWER_D)[0])
        self.spinBoxVIO.setValue(self.det.getDAC(dacIndex.V_POWER_IO)[0])
        self.spinBoxVCHIP.setValue(self.det.getDAC(dacIndex.V_POWER_CHIP)[0])

        #Updating values for patterns
        self.spinBoxFrames.setValue(self.det.frames)
        self.spinBoxPeriod.setValue(self.det.period)
        self.spinBoxTriggers.setValue(self.det.triggers)

        name = self.det.getDacNames()
        print(name[2])


        

if __name__ == '__main__':
    app = QtWidgets.QApplication(sys.argv)
    main = MainWindow()
    main.show()
    #Run the app
    app.exec_()
