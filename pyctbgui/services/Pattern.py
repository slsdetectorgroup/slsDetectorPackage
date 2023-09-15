import os
from functools import partial
from pathlib import Path

from PyQt5 import QtWidgets, uic
import matplotlib.pyplot as plt
from matplotlib.backends.backend_qt5agg import FigureCanvasQTAgg as FigureCanvas
from matplotlib.backends.backend_qt5agg import NavigationToolbar2QT as NavigationToolbar

from pyctbgui.utils.defines import Defines
from pyctbgui.utils.plotPattern import PlotPattern


class PatternTab(QtWidgets.QWidget):

    def __init__(self, parent):
        super().__init__(parent)
        uic.loadUi(Path(__file__).parent.parent / 'ui' / "pattern.ui", parent)
        self.view = parent
        self.mainWindow = None
        self.det = None
        self.plotTab = None

    def setup_ui(self):
        # Pattern Tab
        self.plotTab = self.mainWindow.plotTab

        for i in range(len(Defines.Colors)):
            self.view.comboBoxPatColor.addItem(Defines.Colors[i])
            self.view.comboBoxPatWaitColor.addItem(Defines.Colors[i])
            self.view.comboBoxPatLoopColor.addItem(Defines.Colors[i])
        for i in range(len(Defines.LineStyles)):
            self.view.comboBoxPatWaitLineStyle.addItem(Defines.LineStyles[i])
            self.view.comboBoxPatLoopLineStyle.addItem(Defines.LineStyles[i])
        self.updateDefaultPatViewerParameters()
        self.view.comboBoxPatColorSelect.setCurrentIndex(0)
        self.view.comboBoxPatWait.setCurrentIndex(0)
        self.view.comboBoxPatLoop.setCurrentIndex(0)
        self.view.spinBoxPatClockSpacing.setValue(self.clock_vertical_lines_spacing)
        self.view.checkBoxPatShowClockNumber.setChecked(self.show_clocks_number)
        self.view.doubleSpinBoxLineWidth.setValue(self.line_width)
        self.view.lineEditPatternFile.setText(self.det.patfname[0])
        # rest gets updated after connecting to slots
        # pattern viewer plot area
        self.figure, self.ax = plt.subplots()
        self.canvas = FigureCanvas(self.figure)
        self.toolbar = NavigationToolbar(self.canvas, self.view)
        self.mainWindow.gridLayoutPatternViewer.addWidget(self.toolbar)
        self.mainWindow.gridLayoutPatternViewer.addWidget(self.canvas)
        self.figure.clear()

    def connect_ui(self):
        # For Pattern Tab
        self.view.lineEditStartAddress.editingFinished.connect(self.setPatLimitAddress)
        self.view.lineEditStopAddress.editingFinished.connect(self.setPatLimitAddress)
        for i in range(Defines.pattern.loops_count):
            getattr(self.view,
                    f"lineEditLoop{i}Start").editingFinished.connect(partial(self.setPatLoopStartStopAddress, i))
            getattr(self.view,
                    f"lineEditLoop{i}Stop").editingFinished.connect(partial(self.setPatLoopStartStopAddress, i))
            getattr(self.view, f"lineEditLoop{i}Wait").editingFinished.connect(partial(self.setPatLoopWaitAddress, i))
            getattr(self.view,
                    f"spinBoxLoop{i}Repetition").editingFinished.connect(partial(self.setPatLoopRepetition, i))
            getattr(self.view, f"spinBoxLoop{i}WaitTime").editingFinished.connect(partial(self.setPatLoopWaitTime, i))
        self.view.pushButtonCompiler.clicked.connect(self.setCompiler)
        self.view.pushButtonUncompiled.clicked.connect(self.setUncompiledPatternFile)
        self.view.pushButtonPatternFile.clicked.connect(self.setPatternFile)
        self.view.pushButtonLoadPattern.clicked.connect(self.loadPattern)

        self.view.comboBoxPatColorSelect.currentIndexChanged.connect(self.getPatViewerColors)
        self.view.comboBoxPatWait.currentIndexChanged.connect(self.getPatViewerWaitParameters)
        self.view.comboBoxPatLoop.currentIndexChanged.connect(self.getPatViewerLoopParameters)

        self.view.comboBoxPatColor.currentIndexChanged.connect(self.updatePatViewerParameters)
        self.view.comboBoxPatWaitColor.currentIndexChanged.connect(self.updatePatViewerParameters)
        self.view.comboBoxPatLoopColor.currentIndexChanged.connect(self.updatePatViewerParameters)
        self.view.comboBoxPatWaitLineStyle.currentIndexChanged.connect(self.updatePatViewerParameters)
        self.view.comboBoxPatLoopLineStyle.currentIndexChanged.connect(self.updatePatViewerParameters)
        self.view.doubleSpinBoxWaitAlpha.editingFinished.connect(self.updatePatViewerParameters)
        self.view.doubleSpinBoxLoopAlpha.editingFinished.connect(self.updatePatViewerParameters)
        self.view.doubleSpinBoxWaitAlphaRect.editingFinished.connect(self.updatePatViewerParameters)
        self.view.doubleSpinBoxLoopAlphaRect.editingFinished.connect(self.updatePatViewerParameters)
        self.view.spinBoxPatClockSpacing.editingFinished.connect(self.updatePatViewerParameters)
        self.view.checkBoxPatShowClockNumber.stateChanged.connect(self.updatePatViewerParameters)
        self.view.doubleSpinBoxLineWidth.editingFinished.connect(self.updatePatViewerParameters)
        self.view.pushButtonViewPattern.clicked.connect(self.viewPattern)

    def refresh(self):
        self.getPatLimitAddress()
        for i in range(Defines.pattern.loops_count):
            self.getPatLoopStartStopAddress(i)
            self.getPatLoopWaitAddress(i)
            self.getPatLoopRepetition(i)
            self.getPatLoopWaitTime(i)

    # Pattern Tab functions

    def getPatLimitAddress(self):
        retval = self.det.patlimits
        self.view.lineEditStartAddress.editingFinished.disconnect()
        self.view.lineEditStopAddress.editingFinished.disconnect()
        self.view.lineEditStartAddress.setText("0x{:04x}".format(retval[0]))
        self.view.lineEditStopAddress.setText("0x{:04x}".format(retval[1]))
        self.view.lineEditStartAddress.editingFinished.connect(self.setPatLimitAddress)
        self.view.lineEditStopAddress.editingFinished.connect(self.setPatLimitAddress)

    def setPatLimitAddress(self):
        self.view.lineEditStartAddress.editingFinished.disconnect()
        self.view.lineEditStopAddress.editingFinished.disconnect()
        try:
            start = int(self.view.lineEditStartAddress.text(), 16)
            stop = int(self.view.lineEditStopAddress.text(), 16)
            self.det.patlimits = [start, stop]
        except Exception as e:
            QtWidgets.QMessageBox.warning(self.mainWindow, "Pattern Limit Address Fail", str(e),
                                          QtWidgets.QMessageBox.Ok)
            pass
        # TODO: handling double event exceptions
        self.view.lineEditStartAddress.editingFinished.connect(self.setPatLimitAddress)
        self.view.lineEditStopAddress.editingFinished.connect(self.setPatLimitAddress)
        self.getPatLimitAddress()

    def getPatLoopStartStopAddress(self, level):
        retval = self.det.patloop[level]
        lineEditStart = getattr(self.view, f"lineEditLoop{level}Start")
        lineEditStop = getattr(self.view, f"lineEditLoop{level}Stop")
        lineEditStart.editingFinished.disconnect()
        lineEditStop.editingFinished.disconnect()
        lineEditStart.setText("0x{:04x}".format(retval[0]))
        lineEditStop.setText("0x{:04x}".format(retval[1]))
        lineEditStart.editingFinished.connect(partial(self.setPatLoopStartStopAddress, level))
        lineEditStop.editingFinished.connect(partial(self.setPatLoopStartStopAddress, level))

    def setPatLoopStartStopAddress(self, level):
        lineEditStart = getattr(self.view, f"lineEditLoop{level}Start")
        lineEditStop = getattr(self.view, f"lineEditLoop{level}Stop")
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
        lineEdit = getattr(self.view, f"lineEditLoop{level}Wait")
        lineEdit.editingFinished.disconnect()
        lineEdit.setText("0x{:04x}".format(retval))
        lineEdit.editingFinished.connect(partial(self.setPatLoopWaitAddress, level))

    def setPatLoopWaitAddress(self, level):
        lineEdit = getattr(self.view, f"lineEditLoop{level}Wait")
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
        spinBox = getattr(self.view, f"spinBoxLoop{level}Repetition")
        spinBox.editingFinished.disconnect()
        spinBox.setValue(retval)
        spinBox.editingFinished.connect(partial(self.setPatLoopRepetition, level))

    def setPatLoopRepetition(self, level):
        spinBox = getattr(self.view, f"spinBoxLoop{level}Repetition")
        self.det.patnloop[level] = spinBox.value()
        self.getPatLoopRepetition(level)

    def getPatLoopWaitTime(self, level):
        retval = self.det.patwaittime[level]
        spinBox = getattr(self.view, f"spinBoxLoop{level}WaitTime")
        spinBox.editingFinished.disconnect()
        spinBox.setValue(retval)
        spinBox.editingFinished.connect(partial(self.setPatLoopWaitTime, level))

    def setPatLoopWaitTime(self, level):
        spinBox = getattr(self.view, f"spinBoxLoop{level}WaitTime")
        self.det.patwaittime[level] = spinBox.value()
        self.getPatLoopWaitTime(level)

    def setCompiler(self):
        response = QtWidgets.QFileDialog.getOpenFileName(
            parent=self.mainWindow,
            caption="Select a compiler file",
            directory=str(Path.cwd()),
            # filter='README (*.md *.ui)'
        )
        if response[0]:
            self.view.lineEditCompiler.setText(response[0])

    def setUncompiledPatternFile(self):
        filt = 'Pattern code(*.py *.c)'
        folder = Path(self.det.patfname[0]).parent
        if not folder:
            folder = Path.cwd()
        response = QtWidgets.QFileDialog.getOpenFileName(parent=self.mainWindow,
                                                         caption="Select an uncompiled pattern file",
                                                         directory=str(folder),
                                                         filter=filt)
        if response[0]:
            self.view.lineEditUncompiled.setText(response[0])

    def setPatternFile(self):
        filt = 'Pattern file(*.pyat *.pat)'
        folder = Path(self.det.patfname[0]).parent
        if not folder:
            folder = Path.cwd()
        response = QtWidgets.QFileDialog.getOpenFileName(parent=self.mainWindow,
                                                         caption="Select a compiled pattern file",
                                                         directory=str(folder),
                                                         filter=filt)
        if response[0]:
            self.view.lineEditPatternFile.setText(response[0])

    def compilePattern(self):
        compilerFile = self.view.lineEditCompiler.text()
        if not compilerFile:
            QtWidgets.QMessageBox.warning(self.mainWindow, "Compile Fail", "No compiler selected. Please select one.",
                                          QtWidgets.QMessageBox.Ok)
            return ""

        pattern_file = self.view.lineEditUncompiled.text()

        # if old compile file exists, backup and remove to ensure old copy not loaded
        oldFile = Path(pattern_file + 'at')
        if oldFile.is_file():
            print("Moving old compiled pattern file to _bck")
            exit_status = os.system('mv ' + str(oldFile) + ' ' + str(oldFile) + '_bkup')
            if exit_status != 0:
                retval = QtWidgets.QMessageBox.question(
                    self.mainWindow, "Backup Fail",
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
        if self.view.checkBoxCompile.isChecked():
            pattern_file = self.compilePattern()
        # pat name from pattern field
        else:
            pattern_file = self.view.lineEditPatternFile.text()
            if not pattern_file:
                QtWidgets.QMessageBox.warning(self.mainWindow, "Pattern Fail",
                                              "No pattern file selected. Please select one.", QtWidgets.QMessageBox.Ok)
                return ""
        return pattern_file

    def loadPattern(self):
        pattern_file = self.getCompiledPatFname()
        if not pattern_file:
            return
        # load pattern
        self.det.pattern = pattern_file
        self.view.lineEditPatternFile.setText(self.det.patfname[0])

    def getPatViewerColors(self):
        colorLevel = self.view.comboBoxPatColorSelect.currentIndex()
        color = self.colors_plot[colorLevel]
        self.view.comboBoxPatColor.currentIndexChanged.disconnect()
        self.view.comboBoxPatColor.setCurrentIndex(Defines.Colors.index(color))
        self.view.comboBoxPatColor.currentIndexChanged.connect(self.updatePatViewerParameters)

    def getPatViewerWaitParameters(self):
        waitLevel = self.view.comboBoxPatWait.currentIndex()
        color = self.colors_wait[waitLevel]
        line_style = self.linestyles_wait[waitLevel]
        alpha = self.alpha_wait[waitLevel]
        alpha_rect = self.alpha_wait_rect[waitLevel]

        self.view.comboBoxPatWaitColor.currentIndexChanged.disconnect()
        self.view.comboBoxPatWaitLineStyle.currentIndexChanged.disconnect()
        self.view.doubleSpinBoxWaitAlpha.editingFinished.disconnect()
        self.view.doubleSpinBoxWaitAlphaRect.editingFinished.disconnect()

        self.view.comboBoxPatWaitColor.setCurrentIndex(Defines.Colors.index(color))
        self.view.comboBoxPatWaitLineStyle.setCurrentIndex(Defines.LineStyles.index(line_style))
        self.view.doubleSpinBoxWaitAlpha.setValue(alpha)
        self.view.doubleSpinBoxWaitAlphaRect.setValue(alpha_rect)

        self.view.comboBoxPatWaitColor.currentIndexChanged.connect(self.updatePatViewerParameters)
        self.view.comboBoxPatWaitLineStyle.currentIndexChanged.connect(self.updatePatViewerParameters)
        self.view.doubleSpinBoxWaitAlpha.editingFinished.connect(self.updatePatViewerParameters)
        self.view.doubleSpinBoxWaitAlphaRect.editingFinished.connect(self.updatePatViewerParameters)

    def getPatViewerLoopParameters(self):
        loopLevel = self.view.comboBoxPatLoop.currentIndex()
        color = self.colors_loop[loopLevel]
        line_style = self.linestyles_loop[loopLevel]
        alpha = self.alpha_loop[loopLevel]
        alpha_rect = self.alpha_loop_rect[loopLevel]

        self.view.comboBoxPatLoopColor.currentIndexChanged.disconnect()
        self.view.comboBoxPatLoopLineStyle.currentIndexChanged.disconnect()
        self.view.doubleSpinBoxLoopAlpha.editingFinished.disconnect()
        self.view.doubleSpinBoxLoopAlphaRect.editingFinished.disconnect()

        self.view.comboBoxPatLoopColor.setCurrentIndex(Defines.Colors.index(color))
        self.view.comboBoxPatLoopLineStyle.setCurrentIndex(Defines.LineStyles.index(line_style))
        self.view.doubleSpinBoxLoopAlpha.setValue(alpha)
        self.view.doubleSpinBoxLoopAlphaRect.setValue(alpha_rect)

        self.view.comboBoxPatLoopColor.currentIndexChanged.connect(self.updatePatViewerParameters)
        self.view.comboBoxPatLoopLineStyle.currentIndexChanged.connect(self.updatePatViewerParameters)
        self.view.doubleSpinBoxLoopAlpha.editingFinished.connect(self.updatePatViewerParameters)
        self.view.doubleSpinBoxLoopAlphaRect.editingFinished.connect(self.updatePatViewerParameters)

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
        colorLevel = self.view.comboBoxPatColorSelect.currentIndex()
        color = self.view.comboBoxPatColor.currentIndex()
        # self.colors_plot[colorLevel] = f'tab:{Defines.Colors[color].lower()}'
        self.colors_plot[colorLevel] = Defines.Colors[color]

        waitLevel = self.view.comboBoxPatWait.currentIndex()
        color = self.view.comboBoxPatWaitColor.currentIndex()
        line_style = self.view.comboBoxPatWaitLineStyle.currentIndex()
        alpha = self.view.doubleSpinBoxWaitAlpha.value()
        alpha_rect = self.view.doubleSpinBoxWaitAlphaRect.value()

        self.colors_wait[waitLevel] = Defines.Colors[color]
        self.linestyles_wait[waitLevel] = Defines.LineStyles[line_style]
        self.alpha_wait[waitLevel] = alpha
        self.alpha_wait_rect[waitLevel] = alpha_rect

        loopLevel = self.view.comboBoxPatLoop.currentIndex()
        color = self.view.comboBoxPatLoopColor.currentIndex()
        line_style = self.view.comboBoxPatLoopLineStyle.currentIndex()
        alpha = self.view.doubleSpinBoxLoopAlpha.value()
        alpha_rect = self.view.doubleSpinBoxLoopAlphaRect.value()

        self.colors_loop[loopLevel] = Defines.Colors[color]
        self.linestyles_loop[loopLevel] = Defines.LineStyles[line_style]
        self.alpha_loop[loopLevel] = alpha
        self.alpha_loop_rect[loopLevel] = alpha_rect

        self.clock_vertical_lines_spacing = self.view.spinBoxPatClockSpacing.value()
        self.show_clocks_number = self.view.checkBoxPatShowClockNumber.isChecked()
        self.line_width = self.view.doubleSpinBoxLineWidth.value()

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
        self.plotTab.showPatternViewer(True)
        pattern_file = self.getCompiledPatFname()
        if not pattern_file:
            return

        signalNames = self.det.getSignalNames()
        p = PlotPattern(
            pattern_file,
            signalNames,
            self.colors_plot,
            self.colors_wait,
            self.linestyles_wait,
            self.alpha_wait,
            self.alpha_wait_rect,
            self.colors_loop,
            self.linestyles_loop,
            self.alpha_loop,
            self.alpha_loop_rect,
            self.clock_vertical_lines_spacing,
            self.show_clocks_number,
            self.line_width,
        )

        plt.close(self.figure)
        self.mainWindow.gridLayoutPatternViewer.removeWidget(self.canvas)
        self.canvas.close()
        self.mainWindow.gridLayoutPatternViewer.removeWidget(self.toolbar)
        self.toolbar.close()

        try:
            self.figure = p.patternPlot()
            self.canvas = FigureCanvas(self.figure)
            self.toolbar = NavigationToolbar(self.canvas, self.view)
            self.mainWindow.gridLayoutPatternViewer.addWidget(self.toolbar)
            self.mainWindow.gridLayoutPatternViewer.addWidget(self.canvas)
        except Exception as e:
            QtWidgets.QMessageBox.warning(self.mainWindow, "Pattern Viewer Fail", str(e), QtWidgets.QMessageBox.Ok)
            pass

    def saveParameters(self) -> list[str]:
        commands = []
        for i in range(Defines.pattern.loops_count):
            commands.append(f"patnloop {i} {getattr(self.view, f'spinBoxLoop{i}Repetition').text()}")
            commands.append(f"patloop {i} {getattr(self.view, f'lineEditLoop{i}Start').text()}, "
                            f"{getattr(self.view, f'lineEditLoop{i}Stop').text()}")

            commands.append(f"patwait {i} {getattr(self.view, f'lineEditLoop{i}Wait').text()}")
            commands.append(f"patwaittime {i} {getattr(self.view, f'spinBoxLoop{i}WaitTime').text()}")
        commands.append(f"patlimits {self.view.lineEditStartAddress.text()}, {self.view.lineEditStopAddress.text()}")
        # commands.append(f"patfname {self.view.lineEditPatternFile.text()}")
        return commands
