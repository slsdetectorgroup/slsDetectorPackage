import os

from PyQt5 import QtWidgets, QtCore, uic
from ..utils.defines import *

import argparse
from ..utils import alias_utility
import signal
import pyqtgraph as pg

from functools import partial
from slsdet import Detector, dacIndex


from ..services.ADC import AdcService
from ..services.Acquisition import AcquisitionService
from ..services.Pattern import PatternService
from ..services.Plot import PlotService
from ..services.Transceiver import TransceiverService
from ..services.DACs import DacService
from ..services.PowerSupplies import PowerSuppliesService
from ..services.Signals import SignalsService
from ..services.SlowADCs import SlowAdcService

from pathlib import Path
class MainWindow(QtWidgets.QMainWindow):
    signalShortcutAcquire = QtCore.pyqtSignal()
    signalShortcutTabUp = QtCore.pyqtSignal()
    signalShortcutTabDown = QtCore.pyqtSignal()

    def __init__(self, *args, **kwargs):
        parser = argparse.ArgumentParser()
        parser.add_argument('-a', '--alias', help = "Alias file complete path")
        arglist = parser.parse_args()
        self.alias_file = arglist.alias

        pg.setConfigOption("background", (247, 247, 247))
        pg.setConfigOption("foreground", "k")
        pg.setConfigOption('leftButtonPan', False)

        super(MainWindow, self).__init__()
        uic.loadUi(Path(__file__).parent/"CtbGui.ui", self)

        self.updateSettingValues()

        self.det = None
        try:
            self.det = Detector()
            # ensure detector is up
            self.det.detectorserverversion[0]
        except Exception as e:
            QtWidgets.QMessageBox.critical(self, "Connect Fail", str(e) + "Exiting Gui...",  QtWidgets.QMessageBox.Ok)
            raise

        # get Tab Classes
        self.plotService = PlotService(self)
        self.slowAdcService = SlowAdcService(self)
        self.dacService = DacService(self)
        self.powerSuppliesService = PowerSuppliesService(self)
        self.signalsService = SignalsService(self)
        self.transceiverService = TransceiverService(self)
        self.adcService = AdcService(self)
        self.patternService = PatternService(self)
        self.acquisitionService = AcquisitionService(self)

        self.acquisitionService.setup_zmq()
        self.setup_ui()
        self.tabWidget.setCurrentIndex(Defines.Acquisition_Tab_Index)
        self.tabWidget.currentChanged.connect(self.refresh_tab)
        self.connect_ui()

        self.dacService.refresh()
        self.powerSuppliesService.refresh()
        self.slowAdcService.refresh()
        self.signalsService.refresh()
        self.transceiverService.refresh()
        self.adcService.refresh()
        self.patternService.refresh()
        self.acquisitionService.refresh()
        self.plotService.refresh()

        # also refreshes timer to start plotting 
        self.plotService.plotOptions()
        self.plotService.showPlot()

        self.patternService.getPatViewerColors()
        self.patternService.getPatViewerWaitParameters()
        self.patternService.getPatViewerLoopParameters()
        self.patternService.updatePatViewerParameters()
        self.plotService.showPatternViewer(False)

        if self.alias_file is not None:
            self.loadAliasFile()

        self.signalShortcutAcquire.connect(self.pushButtonStart.click)
        self.signalShortcutTabUp.connect(partial(self.changeTabIndex, True))
        self.signalShortcutTabDown.connect(partial(self.changeTabIndex, False))
        # to catch the ctrl + c to abort
        signal.signal(signal.SIGINT, signal.SIG_DFL)
        self.firstAnalogImage = True
        self.firstDigitalImage = True
        self.firstTransceiverImage = True

    def updateSettingMainWindow(self):
        self.settings.beginGroup("mainwindow");
        # window size
        width = self.settings.value('window_width')
        height = self.settings.value('window_height')
        if width is not None and height is not None:
            self.resize(int(width), int(height))
            #print(f'Main window resized to {width}x{height}')

        # window position
        pos = self.settings.value('window_pos')
        if type(pos) is QtCore.QPoint:
            #print(f'Moved main window to {pos}')
            self.move(pos)
        self.settings.endGroup();

    def saveSettingMainWindow(self):
        self.settings.beginGroup("mainwindow");
        self.settings.setValue('window_width', self.rect().width())
        self.settings.setValue('window_height', self.rect().height())
        self.settings.setValue('window_pos', self.pos())
        self.settings.endGroup();

    def updateSettingDockWidget(self):
        self.settings.beginGroup("dockwidget");

        # is docked
        if self.settings.contains('window_width') and self.settings.contains('window_height'):
            # window size
            width = self.settings.value('window_width')
            height = self.settings.value('window_height')
            if width is not None and height is not None:
                #print(f'Plot window - Floating ({width}x{height})')
                self.dockWidget.setFloating(True)
                self.dockWidget.resize(int(width), int(height))
            # window position
            pos = self.settings.value('window_pos')
            if type(pos) is QtCore.QPoint:
                #print(f'Moved plot window to {pos}')
                self.dockWidget.move(pos)
        self.settings.endGroup();

    def saveSettingDockWidget(self):
        self.settings.beginGroup("dockwidget");
        if self.dockWidget.isFloating():
            self.settings.setValue('window_width', self.dockWidget.rect().width())
            self.settings.setValue('window_height', self.dockWidget.rect().height())
            self.settings.setValue('window_pos', self.dockWidget.pos())
        else:
            self.settings.remove('window_width')
            self.settings.remove('window_height')
            self.settings.remove('window_pos')
        self.settings.endGroup();

    def updateSettingValues(self):
        self.settings = QtCore.QSettings('slsdetectorgroup', 'pyctbgui')
        self.updateSettingMainWindow()
        self.updateSettingDockWidget()

    def saveSettings(self):
        # store in ~/.config/slsdetectorgroup/pyctbgui.conf
        self.saveSettingMainWindow()
        self.saveSettingDockWidget()

    def closeEvent(self, event):
        self.saveSettings()

    def loadAliasFile(self):
        print(f'Loading Alias file: {self.alias_file}')
        try:
            bit_names, bit_plots, bit_colors, adc_names, adc_plots, adc_colors, dac_names, slowadc_names, voltage_names, pat_file_name = alias_utility.read_alias_file(self.alias_file)
        except Exception as e:
            QtWidgets.QMessageBox.warning(self, "Alias File Fail", str(e) + "<br> " + self.alias_file, QtWidgets.QMessageBox.Ok)
            return

        for i in range(64):
            if bit_names[i]:
                self.det.setSignalName(i, bit_names[i])
            if bit_plots[i]:
                getattr(self, f"checkBoxBIT{i}DB").setChecked(bit_plots[i])
                getattr(self, f"checkBoxBIT{i}Plot").setChecked(bit_plots[i])
            if bit_colors[i]:
                self.signalsService.setDBitButtonColor(i, bit_colors[i])

        for i in range(32):
            if adc_names[i]:
                self.det.setAdcName(i, adc_names[i])
            if adc_plots[i]:
                getattr(self, f"checkBoxADC{i}En").setChecked(adc_plots[i])
                getattr(self, f"checkBoxADC{i}Plot").setChecked(adc_plots[i])
            if adc_colors[i]:
                self.adcService.setADCButtonColor(i, adc_colors[i])

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

        self.signalsService.updateSignalNames()
        self.adcService.updateADCNames()
        self.slowAdcService.updateSlowAdcNames()
        self.dacService.updateDACNames()
        self.powerSuppliesService.updateVoltageNames()


    # For Action options function
    # TODO Only add the components of action option+ functions
    # Function to show info
    def showInfo(self):
        msg = QtWidgets.QMessageBox()
        msg.setWindowTitle("About")
        msg.setText("This Gui is for Chip Test Boards.\n Current Phase: Development")
        x = msg.exec_()

    def showKeyBoardShortcuts(self):
        msg = QtWidgets.QMessageBox()
        msg.setWindowTitle("Keyboard Shortcuts")
        msg.setText("Start Acquisition (from any tab): Shift + Return<br>Move Tab Right : Ctrl + '+'<br>Move Tab Left : Ctrl + '-'<br>")
        x = msg.exec_()

    def loadParameters(self):
        response = QtWidgets.QFileDialog.getOpenFileName(
            parent=self,
            caption="Select a parameter file to open",
            directory=os.getcwd(),
            # filter='README (*.md *.ui)'
        )
        if response[0]:
            try:
                parameters = response[0]
                QtWidgets.QMessageBox.information(self, "Load Parameter Success", "Parameters loaded successfully", QtWidgets.QMessageBox.Ok)
            except Exception as e:
                QtWidgets.QMessageBox.warning(self, "Load Parameter Fail", str(e), QtWidgets.QMessageBox.Ok)
                pass




    def refresh_tab(self, tab_index):
        match tab_index:
            case 0:
                self.dacService.refresh()
            case 1:
                self.powerSuppliesService.refresh()
            case 2:
                self.slowAdcService.refresh()
            case 3:
                self.transceiverService.refresh()
            case 4:
                self.signalsService.refresh()
            case 5:
                self.adcService.refresh()
            case 6:
                self.patternService.refresh()
            case 7:
                self.acquisitionService.refresh()
            case 8:
                self.plotService.refresh()




    def setup_ui(self):
        #To check detector status
        self.statusTimer = QtCore.QTimer()
        self.statusTimer.timeout.connect(self.acquisitionService.checkEndofAcquisition)

        #To auto trigger the read
        self.read_timer =  QtCore.QTimer()
        self.read_timer.timeout.connect(self.acquisitionService.read_zmq)

        self.dacService.setup_ui()
        self.powerSuppliesService.setup_ui()
        self.signalsService.setup_ui()
        self.transceiverService.setup_ui()
        self.adcService.setup_ui()
        self.patternService.setup_ui()
        self.acquisitionService.setup_ui()
        self.plotService.setup_ui()






    def keyPressEvent(self, event):
        if event.modifiers() & QtCore.Qt.ShiftModifier:
            if event.key() == QtCore.Qt.Key_Return:
                self.signalShortcutAcquire.emit()
        if event.modifiers() & QtCore.Qt.ControlModifier:
            if event.key() == QtCore.Qt.Key_Plus:
                self.signalShortcutTabUp.emit()
            if event.key() == QtCore.Qt.Key_Minus:
                self.signalShortcutTabDown.emit()

    def changeTabIndex(self, up):
        ind = self.tabWidget.currentIndex()
        if up:
            ind += 1
            if ind == Defines.Max_Tabs:
                ind = 0
        else:
            ind -= 1
            if ind == -1:
                ind = Defines.Max_Tabs -1
        self.tabWidget.setCurrentIndex(ind)

    def connect_ui(self):
        # Show info
        self.actionInfo.triggered.connect(self.showInfo)
        self.actionKeyboardShortcuts.triggered.connect(self.showKeyBoardShortcuts)
        self.actionLoadParameters.triggered.connect(self.loadParameters)

        self.dacService.connect_ui()
        self.powerSuppliesService.connect_ui()
        self.slowAdcService.connect_ui()
        self.signalsService.connect_ui()
        self.transceiverService.connect_ui()
        self.adcService.connect_ui()
        self.patternService.connect_ui()
        self.acquisitionService.connect_ui()
        self.plotService.connect_ui()
