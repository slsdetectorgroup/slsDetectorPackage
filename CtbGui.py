from PyQt5 import QtWidgets, QtCore, QtGui, uic
import sys, os
import pyqtgraph as pg
from pyqtgraph import PlotWidget

class MainWindow(QtWidgets.QMainWindow):
    def __init__(self, *args, **kwargs):
        pg.setConfigOption('background', (247, 247, 247))
        pg.setConfigOption('foreground', 'k')

        super(MainWindow, self).__init__()
        uic.loadUi('CtbGui.ui', self)

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

        #For ADCs Tab
        #TODO Only add the components of ADCs tab

        #For Pattern Tab
        #TODO Only add the components of Pattern tab
        
        #For Acquistions Tab
        #TODO Only add the components of Acquistions tab

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
        print("Dac 0")
    
    def setDAC1(self):
        print("Dac 1")
    
    def setDAC2(self):
        print("Dac 2")

    def setDAC3(self):
        print("Dac 3")

    def setDAC4(self):
        print("Dac 4")

    def setDAC5(self):
        print("Dac 5")

    def setDAC6(self):
        print("Dac 6")

    def setDAC7(self):
        print("Dac 7")

    def setDAC8(self):
        print("Dac 8")
    
    def setDAC9(self):
        print("Dac 9")
    
    def setDAC10(self):
        print("Dac 10")
    
    def setDAC11(self):
        print("Dac 11")
    
    def setDAC12(self):
        print("Dac 12")
    
    def setDAC13(self):
        print("Dac 13")

    def setDAC14(self):
        print("Dac 14")

    def setDAC15(self):
        print("Dac 15")

    def setDAC16(self):
        print("Dac 16")

    def setDAC17(self):
        print("Dac 17")
    
    def setADC(self):
        print("ADC")

    def setHighVoltage(self):
        print("High Voltage")
    
    #For Power Supplies Tab functions
    #TODO Only add the components of Power Supplies tab functions

    def setVA(self):
        print("VA")
    
    def setVB(self):
        print("VB")
    
    def setVC(self):
        print("VC")

    def setVD(self):
        print("VD")

    def setVIO(self):
        print("VIO")

    def setVCHIP(self):
        print("VCHIP")

    #For Sense Tab functions
    #TODO Only add the components of Sense tab functions

    def updateSense0(self):
        print('Sense 0')
    
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

    #For ADCs Tab functions
    #TODO Only add the components of ADCs tab functions

    #For Pattern Tab functions
    #TODO Only add the components of Pattern tab functions
    
    #For Acquistions Tab functions
    #TODO Only add the components of Acquistions tab functions




if __name__ == '__main__':
    app = QtWidgets.QApplication(sys.argv)
    main = MainWindow()
    main.show()

    #Run the app
    app.exec_()
