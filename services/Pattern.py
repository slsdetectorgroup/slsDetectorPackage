import os
from functools import partial
from pathlib import Path

from PyQt5 import QtWidgets
import matplotlib.pyplot as plt
from matplotlib.backends.backend_qt5agg import FigureCanvasQTAgg as FigureCanvas
from matplotlib.backends.backend_qt5agg import NavigationToolbar2QT as NavigationToolbar
from utils.defines import Defines
from utils.plotPattern import PlotPattern


class Pattern:
    def __init__(self, mainWindow):
        self.mainWindow = mainWindow
        self.det = self.mainWindow.det

    def setup_ui(self):
        # Pattern Tab
        for i in range(len(Defines.Colors)):
            self.mainWindow.comboBoxPatColor.addItem(Defines.Colors[i])
            self.mainWindow.comboBoxPatWaitColor.addItem(Defines.Colors[i])
            self.mainWindow.comboBoxPatLoopColor.addItem(Defines.Colors[i])
        for i in range(len(Defines.LineStyles)):
            self.mainWindow.comboBoxPatWaitLineStyle.addItem(Defines.LineStyles[i])
            self.mainWindow.comboBoxPatLoopLineStyle.addItem(Defines.LineStyles[i])
        self.updateDefaultPatViewerParameters()
        self.mainWindow.comboBoxPatColorSelect.setCurrentIndex(0)
        self.mainWindow.comboBoxPatWait.setCurrentIndex(0)
        self.mainWindow.comboBoxPatLoop.setCurrentIndex(0)
        self.mainWindow.spinBoxPatClockSpacing.setValue(self.clock_vertical_lines_spacing)
        self.mainWindow.checkBoxPatShowClockNumber.setChecked(self.show_clocks_number)
        self.mainWindow.doubleSpinBoxLineWidth.setValue(self.line_width)
        self.mainWindow.lineEditPatternFile.setText(self.det.patfname[0])
        # rest gets updated after connecting to slots
        # pattern viewer plot area
        self.figure, self.ax = plt.subplots()
        self.canvas = FigureCanvas(self.figure)
        self.toolbar = NavigationToolbar(self.canvas, self.mainWindow)
        self.mainWindow.gridLayoutPatternViewer.addWidget(self.toolbar)
        self.mainWindow.gridLayoutPatternViewer.addWidget(self.canvas)
        self.figure.clear()

    def connect_ui(self):
        # For Pattern Tab
        self.mainWindow.lineEditStartAddress.editingFinished.connect(self.setPatLimitAddress)
        self.mainWindow.lineEditStopAddress.editingFinished.connect(self.setPatLimitAddress)
        for i in range(6):
            getattr(self.mainWindow, f"lineEditLoop{i}Start").editingFinished.connect(
                partial(self.setPatLoopStartStopAddress, i)
            )
            getattr(self.mainWindow, f"lineEditLoop{i}Stop").editingFinished.connect(
                partial(self.setPatLoopStartStopAddress, i)
            )
            getattr(self.mainWindow, f"lineEditLoop{i}Wait").editingFinished.connect(
                partial(self.setPatLoopWaitAddress, i)
            )
            getattr(self.mainWindow, f"spinBoxLoop{i}Repetition").editingFinished.connect(
                partial(self.setPatLoopRepetition, i)
            )
            getattr(self.mainWindow, f"spinBoxLoop{i}WaitTime").editingFinished.connect(
                partial(self.setPatLoopWaitTime, i)
            )
        self.mainWindow.pushButtonCompiler.clicked.connect(self.setCompiler)
        self.mainWindow.pushButtonUncompiled.clicked.connect(self.setUncompiledPatternFile)
        self.mainWindow.pushButtonPatternFile.clicked.connect(self.setPatternFile)
        self.mainWindow.pushButtonLoadPattern.clicked.connect(self.loadPattern)

        self.mainWindow.comboBoxPatColorSelect.currentIndexChanged.connect(self.getPatViewerColors)
        self.mainWindow.comboBoxPatWait.currentIndexChanged.connect(self.getPatViewerWaitParameters)
        self.mainWindow.comboBoxPatLoop.currentIndexChanged.connect(self.getPatViewerLoopParameters)

        self.mainWindow.comboBoxPatColor.currentIndexChanged.connect(self.updatePatViewerParameters)
        self.mainWindow.comboBoxPatWaitColor.currentIndexChanged.connect(self.updatePatViewerParameters)
        self.mainWindow.comboBoxPatLoopColor.currentIndexChanged.connect(self.updatePatViewerParameters)
        self.mainWindow.comboBoxPatWaitLineStyle.currentIndexChanged.connect(self.updatePatViewerParameters)
        self.mainWindow.comboBoxPatLoopLineStyle.currentIndexChanged.connect(self.updatePatViewerParameters)
        self.mainWindow.doubleSpinBoxWaitAlpha.editingFinished.connect(self.updatePatViewerParameters)
        self.mainWindow.doubleSpinBoxLoopAlpha.editingFinished.connect(self.updatePatViewerParameters)
        self.mainWindow.doubleSpinBoxWaitAlphaRect.editingFinished.connect(self.updatePatViewerParameters)
        self.mainWindow.doubleSpinBoxLoopAlphaRect.editingFinished.connect(self.updatePatViewerParameters)
        self.mainWindow.spinBoxPatClockSpacing.editingFinished.connect(self.updatePatViewerParameters)
        self.mainWindow.checkBoxPatShowClockNumber.stateChanged.connect(self.updatePatViewerParameters)
        self.mainWindow.doubleSpinBoxLineWidth.editingFinished.connect(self.updatePatViewerParameters)
        self.mainWindow.pushButtonViewPattern.clicked.connect(self.viewPattern)

    def refresh(self):
        self.getPatLimitAddress()
        for i in range(6):
            self.getPatLoopStartStopAddress(i)
            self.getPatLoopWaitAddress(i)
            self.getPatLoopRepetition(i)
            self.getPatLoopWaitTime(i)

    # Pattern Tab functions

    def getPatLimitAddress(self):
        retval = self.det.patlimits
        self.mainWindow.lineEditStartAddress.editingFinished.disconnect()
        self.mainWindow.lineEditStopAddress.editingFinished.disconnect()
        self.mainWindow.lineEditStartAddress.setText("0x{:04x}".format(retval[0]))
        self.mainWindow.lineEditStopAddress.setText("0x{:04x}".format(retval[1]))
        self.mainWindow.lineEditStartAddress.editingFinished.connect(self.setPatLimitAddress)
        self.mainWindow.lineEditStopAddress.editingFinished.connect(self.setPatLimitAddress)

    def setPatLimitAddress(self):
        self.mainWindow.lineEditStartAddress.editingFinished.disconnect()
        self.mainWindow.lineEditStopAddress.editingFinished.disconnect()
        try:
            start = int(self.mainWindow.lineEditStartAddress.text(), 16)
            stop = int(self.mainWindow.lineEditStopAddress.text(), 16)
            self.det.patlimits = [start, stop]
        except Exception as e:
            QtWidgets.QMessageBox.warning(self.mainWindow, "Pattern Limit Address Fail", str(e),
                                          QtWidgets.QMessageBox.Ok)
            pass
        # TODO: handling double event exceptions
        self.mainWindow.lineEditStartAddress.editingFinished.connect(self.setPatLimitAddress)
        self.mainWindow.lineEditStopAddress.editingFinished.connect(self.setPatLimitAddress)
        self.getPatLimitAddress()

    def getPatLoopStartStopAddress(self, level):
        retval = self.det.patloop[level]
        lineEditStart = getattr(self.mainWindow, f"lineEditLoop{level}Start")
        lineEditStop = getattr(self.mainWindow, f"lineEditLoop{level}Stop")
        lineEditStart.editingFinished.disconnect()
        lineEditStop.editingFinished.disconnect()
        lineEditStart.setText("0x{:04x}".format(retval[0]))
        lineEditStop.setText("0x{:04x}".format(retval[1]))
        lineEditStart.editingFinished.connect(partial(self.setPatLoopStartStopAddress, level))
        lineEditStop.editingFinished.connect(partial(self.setPatLoopStartStopAddress, level))

    def setPatLoopStartStopAddress(self, level):
        lineEditStart = getattr(self.mainWindow, f"lineEditLoop{level}Start")
        lineEditStop = getattr(self.mainWindow, f"lineEditLoop{level}Stop")
        lineEditStart.editingFinished.disconnect()
        lineEditStop.editingFinished.disconnect()
        try:
            start = int(lineEditStart.text(), 16)
            stop = int(lineEditStop.text(), 16)
            self.det.patloop[level] = [start, stop]
        except Exception as e:
            QtWidgets.QMessageBox.warning(self.mainWindow, "Pattern Loop Start Stop Address Fail", str(e),
                                          QtWidgets.QMessageBox.Ok)
            pass
        # TODO: handling double event exceptions
        lineEditStart.editingFinished.connect(partial(self.setPatLoopStartStopAddress, level))
        lineEditStop.editingFinished.connect(partial(self.setPatLoopStartStopAddress, level))
        self.getPatLoopStartStopAddress(level)

    def getPatLoopWaitAddress(self, level):
        retval = self.det.patwait[level]
        lineEdit = getattr(self.mainWindow, f"lineEditLoop{level}Wait")
        lineEdit.editingFinished.disconnect()
        lineEdit.setText("0x{:04x}".format(retval))
        lineEdit.editingFinished.connect(partial(self.setPatLoopWaitAddress, level))

    def setPatLoopWaitAddress(self, level):
        lineEdit = getattr(self.mainWindow, f"lineEditLoop{level}Wait")
        lineEdit.editingFinished.disconnect()
        try:
            addr = int(lineEdit.text(), 16)
            self.det.patwait[level] = addr
        except Exception as e:
            QtWidgets.QMessageBox.warning(self.mainWindow, "Pattern Wait Address Fail", str(e),
                                          QtWidgets.QMessageBox.Ok)
            pass
        # TODO: handling double event exceptions
        lineEdit.editingFinished.connect(partial(self.setPatLoopWaitAddress, level))
        self.getPatLoopWaitAddress(level)

    def getPatLoopRepetition(self, level):
        retval = self.det.patnloop[level]
        spinBox = getattr(self.mainWindow, f"spinBoxLoop{level}Repetition")
        spinBox.editingFinished.disconnect()
        spinBox.setValue(retval)
        spinBox.editingFinished.connect(partial(self.setPatLoopRepetition, level))

    def setPatLoopRepetition(self, level):
        spinBox = getattr(self.mainWindow, f"spinBoxLoop{level}Repetition")
        self.det.patnloop[level] = spinBox.value()
        self.getPatLoopRepetition(level)

    def getPatLoopWaitTime(self, level):
        retval = self.det.patwaittime[level]
        spinBox = getattr(self.mainWindow, f"spinBoxLoop{level}WaitTime")
        spinBox.editingFinished.disconnect()
        spinBox.setValue(retval)
        spinBox.editingFinished.connect(partial(self.setPatLoopWaitTime, level))

    def setPatLoopWaitTime(self, level):
        spinBox = getattr(self.mainWindow, f"spinBoxLoop{level}WaitTime")
        self.det.patwaittime[level] = spinBox.value()
        self.getPatLoopWaitTime(level)

    def setCompiler(self):
        response = QtWidgets.QFileDialog.getOpenFileName(
            parent=self.mainWindow,
            caption="Select a compiler file",
            directory=os.getcwd(),
            # filter='README (*.md *.ui)'
        )
        if response[0]:
            self.mainWindow.lineEditCompiler.setText(response[0])

    def setUncompiledPatternFile(self):
        filt = 'Pattern code(*.py *.c)'
        folder = os.path.dirname(self.det.patfname[0])
        if not folder:
            folder = os.getcwd()
        response = QtWidgets.QFileDialog.getOpenFileName(
            parent=self.mainWindow,
            caption="Select an uncompiled pattern file",
            directory=folder,
            filter=filt
        )
        if response[0]:
            self.mainWindow.lineEditUncompiled.setText(response[0])

    def setPatternFile(self):
        filt = 'Pattern file(*.pyat *.pat)'
        folder = os.path.dirname(self.det.patfname[0])
        if not folder:
            folder = os.getcwd()
        response = QtWidgets.QFileDialog.getOpenFileName(
            parent=self.mainWindow,
            caption="Select a compiled pattern file",
            directory=folder,
            filter=filt
        )
        if response[0]:
            self.mainWindow.lineEditPatternFile.setText(response[0])

    def compilePattern(self):
        compilerFile = self.mainWindow.lineEditCompiler.text()
        if not compilerFile:
            QtWidgets.QMessageBox.warning(self.mainWindow, "Compile Fail", "No compiler selected. Please select one.",
                                          QtWidgets.QMessageBox.Ok)
            return ""

        pattern_file = self.mainWindow.lineEditUncompiled.text()

        # if old compile file exists, backup and remove to ensure old copy not loaded
        oldFile = Path(pattern_file + 'at')
        if oldFile.is_file():
            print("Moving old compiled pattern file to _bck")
            exit_status = os.system('mv ' + str(oldFile) + ' ' + str(oldFile) + '_bkup')
            if exit_status != 0:
                retval = QtWidgets.QMessageBox.question(self.mainWindow, "Backup Fail",
                                                        "Could not make a backup of old compiled code. Proceed anyway to compile and overwrite?",
                                                        QtWidgets.QMessageBox.Yes, QtWidgets.QMessageBox.No)
                if retval == QtWidgets.QMessageBox.No:
                    return ""

        compileCommand = compilerFile + ' ' + pattern_file
        print(compileCommand)
        print("Compiling pattern code to .pat file")
        exit_status = os.system(compileCommand)
        if exit_status != 0:
            QtWidgets.QMessageBox.warning(self.mainWindow, "Compile Fail", "Could not compile pattern.",
                                          QtWidgets.QMessageBox.Ok)
            return ""
        pattern_file += 'at'

        return pattern_file

    def getCompiledPatFname(self):
        if self.mainWindow.checkBoxCompile.isChecked():
            pattern_file = self.compilePattern()
        # pat name from pattern field
        else:
            pattern_file = self.mainWindow.lineEditPatternFile.text()
            if not pattern_file:
                QtWidgets.QMessageBox.warning(self.mainWindow, "Pattern Fail", "No pattern file selected. Please select one.",
                                              QtWidgets.QMessageBox.Ok)
                return ""
        return pattern_file

    def loadPattern(self):
        pattern_file = self.getCompiledPatFname()
        if not pattern_file:
            return
        # load pattern
        self.det.pattern = pattern_file
        self.mainWindow.lineEditPatternFile.setText(self.det.patfname[0])

    def getPatViewerColors(self):
        colorLevel = self.mainWindow.comboBoxPatColorSelect.currentIndex()
        color = self.colors_plot[colorLevel]
        self.mainWindow.comboBoxPatColor.currentIndexChanged.disconnect()
        self.mainWindow.comboBoxPatColor.setCurrentIndex(Defines.Colors.index(color))
        self.mainWindow.comboBoxPatColor.currentIndexChanged.connect(self.updatePatViewerParameters)

    def getPatViewerWaitParameters(self):
        waitLevel = self.mainWindow.comboBoxPatWait.currentIndex()
        color = self.colors_wait[waitLevel]
        line_style = self.linestyles_wait[waitLevel]
        alpha = self.alpha_wait[waitLevel]
        alpha_rect = self.alpha_wait_rect[waitLevel]

        self.mainWindow.comboBoxPatWaitColor.currentIndexChanged.disconnect()
        self.mainWindow.comboBoxPatWaitLineStyle.currentIndexChanged.disconnect()
        self.mainWindow.doubleSpinBoxWaitAlpha.editingFinished.disconnect()
        self.mainWindow.doubleSpinBoxWaitAlphaRect.editingFinished.disconnect()

        self.mainWindow.comboBoxPatWaitColor.setCurrentIndex(Defines.Colors.index(color))
        self.mainWindow.comboBoxPatWaitLineStyle.setCurrentIndex(Defines.LineStyles.index(line_style))
        self.mainWindow.doubleSpinBoxWaitAlpha.setValue(alpha)
        self.mainWindow.doubleSpinBoxWaitAlphaRect.setValue(alpha_rect)

        self.mainWindow.comboBoxPatWaitColor.currentIndexChanged.connect(self.updatePatViewerParameters)
        self.mainWindow.comboBoxPatWaitLineStyle.currentIndexChanged.connect(self.updatePatViewerParameters)
        self.mainWindow.doubleSpinBoxWaitAlpha.editingFinished.connect(self.updatePatViewerParameters)
        self.mainWindow.doubleSpinBoxWaitAlphaRect.editingFinished.connect(self.updatePatViewerParameters)

    def getPatViewerLoopParameters(self):
        loopLevel = self.mainWindow.comboBoxPatLoop.currentIndex()
        color = self.colors_loop[loopLevel]
        line_style = self.linestyles_loop[loopLevel]
        alpha = self.alpha_loop[loopLevel]
        alpha_rect = self.alpha_loop_rect[loopLevel]

        self.mainWindow.comboBoxPatLoopColor.currentIndexChanged.disconnect()
        self.mainWindow.comboBoxPatLoopLineStyle.currentIndexChanged.disconnect()
        self.mainWindow.doubleSpinBoxLoopAlpha.editingFinished.disconnect()
        self.mainWindow.doubleSpinBoxLoopAlphaRect.editingFinished.disconnect()

        self.mainWindow.comboBoxPatLoopColor.setCurrentIndex(Defines.Colors.index(color))
        self.mainWindow.comboBoxPatLoopLineStyle.setCurrentIndex(Defines.LineStyles.index(line_style))
        self.mainWindow.doubleSpinBoxLoopAlpha.setValue(alpha)
        self.mainWindow.doubleSpinBoxLoopAlphaRect.setValue(alpha_rect)

        self.mainWindow.comboBoxPatLoopColor.currentIndexChanged.connect(self.updatePatViewerParameters)
        self.mainWindow.comboBoxPatLoopLineStyle.currentIndexChanged.connect(self.updatePatViewerParameters)
        self.mainWindow.doubleSpinBoxLoopAlpha.editingFinished.connect(self.updatePatViewerParameters)
        self.mainWindow.doubleSpinBoxLoopAlphaRect.editingFinished.connect(self.updatePatViewerParameters)

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

        # print('default')
        # self.printPatViewerParameters()

    def updatePatViewerParameters(self):
        colorLevel = self.mainWindow.comboBoxPatColorSelect.currentIndex()
        color = self.mainWindow.comboBoxPatColor.currentIndex()
        # self.colors_plot[colorLevel] = f'tab:{Defines.Colors[color].lower()}'
        self.colors_plot[colorLevel] = Defines.Colors[color]

        waitLevel = self.mainWindow.comboBoxPatWait.currentIndex()
        color = self.mainWindow.comboBoxPatWaitColor.currentIndex()
        line_style = self.mainWindow.comboBoxPatWaitLineStyle.currentIndex()
        alpha = self.mainWindow.doubleSpinBoxWaitAlpha.value()
        alpha_rect = self.mainWindow.doubleSpinBoxWaitAlphaRect.value()

        self.colors_wait[waitLevel] = Defines.Colors[color]
        self.linestyles_wait[waitLevel] = Defines.LineStyles[line_style]
        self.alpha_wait[waitLevel] = alpha
        self.alpha_wait_rect[waitLevel] = alpha_rect

        loopLevel = self.mainWindow.comboBoxPatLoop.currentIndex()
        color = self.mainWindow.comboBoxPatLoopColor.currentIndex()
        line_style = self.mainWindow.comboBoxPatLoopLineStyle.currentIndex()
        alpha = self.mainWindow.doubleSpinBoxLoopAlpha.value()
        alpha_rect = self.mainWindow.doubleSpinBoxLoopAlphaRect.value()

        self.colors_loop[loopLevel] = Defines.Colors[color]
        self.linestyles_loop[loopLevel] = Defines.LineStyles[line_style]
        self.alpha_loop[loopLevel] = alpha
        self.alpha_loop_rect[loopLevel] = alpha_rect

        self.clock_vertical_lines_spacing = self.mainWindow.spinBoxPatClockSpacing.value()
        self.show_clocks_number = self.mainWindow.checkBoxPatShowClockNumber.isChecked()
        self.line_width = self.mainWindow.doubleSpinBoxLineWidth.value()

        # for debugging
        # self.printPatViewerParameters()

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
        self.mainWindow.plotTab.showPatternViewer(True)
        pattern_file = self.getCompiledPatFname()
        if not pattern_file:
            return

        signalNames = self.det.getSignalNames()
        p = PlotPattern(pattern_file, signalNames, self.colors_plot, self.colors_wait, self.linestyles_wait,
                        self.alpha_wait, self.alpha_wait_rect, self.colors_loop, self.linestyles_loop,
                        self.alpha_loop, self.alpha_loop_rect, self.clock_vertical_lines_spacing,
                        self.show_clocks_number, self.line_width, )

        plt.close(self.figure)
        self.mainWindow.gridLayoutPatternViewer.removeWidget(self.canvas)
        self.canvas.close()
        self.mainWindow.gridLayoutPatternViewer.removeWidget(self.toolbar)
        self.toolbar.close()

        try:
            self.figure = p.patternPlot()
            self.canvas = FigureCanvas(self.figure)
            self.toolbar = NavigationToolbar(self.canvas, self.mainWindow)
            self.mainWindow.gridLayoutPatternViewer.addWidget(self.toolbar)
            self.mainWindow.gridLayoutPatternViewer.addWidget(self.canvas)
        except Exception as e:
            QtWidgets.QMessageBox.warning(self.mainWindow, "Pattern Viewer Fail", str(e), QtWidgets.QMessageBox.Ok)
            pass
